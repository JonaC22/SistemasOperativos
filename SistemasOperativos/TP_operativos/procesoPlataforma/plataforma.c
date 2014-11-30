/*
 * plataforma.c
 *
 *  Created on: May 12, 2013
 *      Author: juan
 */

#include <src/commons/error.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <src/commons/config.h>
#include <src/commons/log.h>
#include <src/commons/string.h>
#include <src/commons/collections/queue.h>
#include <src/commons/collections/dictionary.h>
#include <pthread.h>
#include "planificador.h"
#include "pedidos.h"
#include <sockets.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/inotify.h>

#define EVENTO_TAMANIO ( sizeof (struct inotify_event) + 24 )
#define EVENTO_BUF_LONGITUD     ( 1024 * ( EVENTO_TAMANIO + 16 ) )


int obtenerQuantum(char* rutaConfig, int primeraVez);
int aceptar_conexion_orquestador(int servidor);

t_dictionary *direccionesNiveles;
t_dictionary *id_setColas;
char* ipOrquestador;
t_config* configPlataforma;
t_log* logger;
int quantum;
char* rutaConfig;
int personajesSinTerminarPlan;
t_log* logger;
char* rutaLogger;

int servidor;

t_list *todosLosPersonajesConectados;
pthread_mutex_t semaforoConexiones;

t_dictionary *planificadores;

int main(int argc, char** argv)
{
	if(argc != 3){
		error_show("parametros de entrada incorrectos\n");
		return 1;
	}
	rutaConfig=argv[1];
	rutaLogger=argv[2];

	configPlataforma = config_create(rutaConfig);
	logger = log_create(rutaLogger, "Plataforma" ,true, LOG_LEVEL_INFO);


	char* ippuerto =config_get_string_value(configPlataforma, "orquestador");
	char** ippuertoSeparados = string_split(ippuerto, ":");
	ipOrquestador = strdup(ippuertoSeparados[0]);
	free(ippuertoSeparados[0]);
	int puertoOrquestador = atoi(ippuertoSeparados[1]);
	free(ippuertoSeparados[1]);
	free(ippuertoSeparados);

	quantum=obtenerQuantum(rutaConfig,1);
	personajesSinTerminarPlan = 0;

	todosLosPersonajesConectados = list_create();
	pthread_mutex_init(&semaforoConexiones,NULL);
	planificadores=dictionary_create();
	direccionesNiveles=	dictionary_create();
	id_setColas = dictionary_create();

	servidor = crear_socket_servidor(ipOrquestador, puertoOrquestador);
	if(servidor != -1)
	{
		aceptar_conexion_orquestador(servidor);
	}
	else
	{
		log_error(logger,"Error al conectar el socket de la plataforma");
	}

	free(ipOrquestador);
	list_destroy(todosLosPersonajesConectados);
	dictionary_destroy(direccionesNiveles);
	dictionary_destroy(planificadores);
	dictionary_destroy(id_setColas);
	log_destroy(logger);
	config_destroy(configPlataforma);
	return 0;

}

int aceptar_conexion_orquestador(int servidor) {

	int socketEscucha = (int) servidor;
	log_info(logger,"Plataforma en socket %d \n", socketEscucha);
	// Aceptar una nueva conexion entrante. Se genera un nuevo socket con la nueva conexion.

	int socketNuevaConexion;
	fd_set master; // conjunto maestro de descriptores de fichero
	fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
	int fdmax; // número máximo de descriptores de fichero
	int descriptor_archivo;
	int descriptor_reloj;
	int longitud=0;

	FD_ZERO(&master);	// borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);

	FD_SET(socketEscucha, &master);	// añadir socketEscucha al conjunto maestro


	// bucle principal
	 char buffer[EVENTO_BUF_LONGITUD];
	descriptor_archivo = inotify_init();//crea la instancia del inotify

	  if ( descriptor_archivo < 0 ) {//si es menor a cero hubo un error al crear la instancia (descriptor de archivo)
		  log_error(logger,"Error en inotify_init");
		 perror( "inotify_init" );
	  }

	  descriptor_reloj = inotify_add_watch( descriptor_archivo,rutaConfig, IN_MODIFY);

	  if ( descriptor_reloj < 0 ) {//si es menor a cero hubo un error al crear la instancia
		  log_error(logger,"Error en inotify_init");
		  perror( "inotify_init" );
	  }
	  else
	  {
		  FD_SET(descriptor_archivo,&master);
	  }

	  // descriptor de fichero mayor inicial
	  if(socketEscucha > descriptor_archivo)
	  {
		  fdmax = socketEscucha;
	  }
	  else
	  {
		  fdmax = descriptor_archivo;
	  }

	for (;;) {
		read_fds = master; // cópialo
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			log_error(logger,"Error en select");
			perror("select");
			exit(1);
		}
		int i;
		// explorar conexiones existentes en busca de datos que leer
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds))  // ¡¡tenemos datos!!
			{
				if (i == socketEscucha)
				{
					// gestionar nuevas conexiones
					if ((socketNuevaConexion = accept(socketEscucha, NULL, 0))< 0)
					{
						log_error(logger,"Error en accept");
						perror("accept");
					}
					else //El server aceptó la conexion del cliente
					{
						FD_SET(socketNuevaConexion, &master); // añadir al conjunto maestro
						if (socketNuevaConexion > fdmax)
						{
							fdmax = socketNuevaConexion;// actualizar el máximo
						}
						printf("Se aceptó la conexión del socket:%d\n",socketNuevaConexion);

					}
				}
				else
				{

					if(i==descriptor_archivo)
					{

						longitud = read( descriptor_archivo, buffer, EVENTO_BUF_LONGITUD  );
						if ( longitud< 0 )
						{
							log_error(logger,"Error en read");
							perror( "read" );
							break;
						}

						int valorquantum = obtenerQuantum(rutaConfig,0);

						if(valorquantum >0 && valorquantum != quantum)
						{
							quantum = valorquantum;
							printf("Nuevo quamtum, %d.\n", quantum);
//							log_info( logger,"Nuevo quamtum, %d.\n", quantum);
						}
					}
					else // Es un personaje que ya estaba conectado y quiere que lo atiendan
					{
						printf("Esperando recibir mensaje...\n");
						char *stream;
						if ((stream = recibir_mensaje(i)) <= 0) // Error o conexión cerrada por el personaje
						{

							printf("Mensaje recibido!\n");
							if(stream == NULL)
							{
								printf("Se ha desconectado el socket: %d \n", i);

							}

							cerrar_socket(i); // Cierro el socket del cliente que se desconectó
							FD_CLR(i, &master); // Elimino el socket que se desconectó del conjunto maestro

						}
						else //El personaje mando un pedido
						{
							int codigo = atender_pedidos_orquestador(i, stream);
							switch (codigo)
							{
								case 3:
									cerrar_socket(i);
									FD_CLR(i, &master);
								break;

							}
						}
					}
				}
			}
		}
	}
	return socketNuevaConexion;
}


int obtenerQuantum(char* rutaConfig, int primeraVez)
{
	int quantum;
	//config_destroy(configPlataforma);
	configPlataforma = config_create(rutaConfig);

	if(config_has_property(configPlataforma,"quantum"))
	{
		quantum= config_get_int_value(configPlataforma,"quantum");
		if(quantum>0)
		{
			return quantum;
		}

		else
		{
			if(primeraVez)
			{
				quantum=1;
				log_error(logger,"El quantum en el archivo de configuracion es erróneo. Se inicializó en 1 por defecto. \n Por favor, corregir el archivo.\n");
			}
			return -1;
		}
	}
	return -1;
}

void loguearConectados(t_planificador *planificador)
{
	log_info(planificador->logger,"Todos los personajes conectados: ");
	void logPersonaje(t_personaje *elemento)
	{
		log_info(planificador->logger,"%s:%d", elemento->remitente, elemento->socket);
	}
	list_iterate(todosLosPersonajesConectados,(void*)logPersonaje);
}

