
#include "pedidos.h"
#include <stdlib.h>
#include <sockets.h>
#include <pthread.h>
#include <src/commons/collections/list.h>
#include <src/commons/config.h>
#include <src/commons/log.h>
#include <src/commons/process.h>
#include <src/commons/string.h>
#include <sys/socket.h>
#include <ctype.h>
#include <semaphore.h>
#include <unistd.h>
#include "interBloqueo.h"
#include <string.h>

#define BUFF_SIZE 1024

//-------------Struc Vector de Recursos------------

typedef struct {
	char recurso;
	char* nombre;
	int  cantidadDisponible;
	int  cantidadEnUso;
	int posX;
	int posY;
    t_list *quienesTienenEsteRecurso;

}t_dispoRecurso;

int socketOrquestador;
pthread_mutex_t sema;

int aceptar_conexion(int servidor);
void activarDetector(int time);
void removerPersonaje(int socket);


//Obtiene las cajas del Nivel, las guarda en el vec

void obtenerRecursosYGuardarlosEnVectorDeRecursos(t_config* unPath);


//-------------Variables Nivel--------------

char* nivel;
char* cajaX;

int cantCajas;

t_dispoRecurso recursosEnNivel[3055];

t_list *personajesEnNivel;

char* rutaLogger;

char* ipOrquestador;
int puertoOrquestador;

char* ipNivel;
int puertoNivel;

//-----PA DIBUAR----
#include "tad_items.h"
#include <curses.h>

//---FUNCIONES PA DIBUJAR--
void dibujarLosRecursos();

//lista a dibujar
ITEM_NIVEL* ListaItems = NULL;

int recovery;

//lista de peticiones
t_list *personajesPeticiones;

t_config* configNivel;

pthread_t hiloRecepcionista;

pthread_t hiloDetector;
t_log* logger;

int main(int argc, char** argv)
{
	if(argc != 3){
		printf("parametros de entrada incorrectos\n");
		return 1;
	}
	personajesPeticiones=list_create();
	personajesEnNivel=list_create();

	pthread_mutex_init(&sema,NULL);
	int recepcionista;
	int detector;

//  EJECUTAR CON PARÁMETROS: ./procesoNivel /workspace/operativos/nivel.cfg /workspace/operativos/debug.log argv[1] -> primer parametro

	//Crea logger y trae la configuracion

	rutaLogger = argv[2];

	configNivel = config_create(argv[1]);

	nivel = config_get_string_value(configNivel,"Nombre");

	logger = log_create(rutaLogger, nivel ,false, LOG_LEVEL_INFO);
//	t_log* logger = log_create("/home/utnso/logueo/niveldebug.log", "TEST",true, LOG_LEVEL_INFO);
//	t_config* configNivel = config_create("/home/utnso/configs/nivel.cfg");


	//---------------------------------- Obtención de datos del Nivel ---------------------------------

	//Trae el resto de los datos del nivel

	int tiempo = config_get_int_value(configNivel,"TiempoChequeoDeadlock");
	obtenerRecursosYGuardarlosEnVectorDeRecursos(configNivel);

	if( ((config_has_property(configNivel,"Recovery")) == true) ){

			recovery = config_get_int_value(configNivel,"Recovery");
	}
	//Con el orquestador
	char* ippuerto =config_get_string_value(configNivel,"orquestador");

	char** ippuertoSeparados = string_split(ippuerto, ":");

	ipOrquestador=*ippuertoSeparados;

	puertoOrquestador= atoi(*(ippuertoSeparados+1));

	log_info(logger,"La direccion del orquestador es: %s : %d",ipOrquestador,puertoOrquestador);

	//Obtengo mi direccion

	char* ippuertoNivel =config_get_string_value(configNivel,"nivel");

	ippuertoSeparados = string_split(ippuertoNivel, ":");

	ipNivel=*ippuertoSeparados;

	puertoNivel= atoi(*(ippuertoSeparados+1));

	log_info(logger,"La direccion del nivel es: %s : %d",ipNivel,puertoNivel);


	//---------------------------------- FIN: Obtención de datos del Nivel -----------------------------//


	int socketNivel;
	//crear(ip,puerto)
	if((socketNivel= crear_socket_servidor(ipNivel,puertoNivel)) == -1 )
	{
		log_error(logger, "error al crear el servidor nivel", 0);
		return 1;
	}

	else
	{
		if((detector = pthread_create(&hiloDetector,NULL,(void*) activarDetector,(void*) tiempo))==-1){
					log_error(logger, "Error al crear el hilo detector.",0);
							return 1;
				}

		if((recepcionista = pthread_create(&hiloRecepcionista, NULL,(void*) aceptar_conexion, (void*) socketNivel)) ==-1)
		{
			log_error(logger, "Error al crear el hilo recepcionista", 0);
			return 1;
		}

		if(( socketOrquestador= conectar_socket(ipOrquestador,puertoOrquestador))==0 ) //Me conecto al orquestador. Si hubo un error de conexion, sale.
		{
			log_error(logger, "Error al conectar el nivel al orquestador", 0);
			return 1;
		}
		else
		{

			log_info(logger, "Conectado al orquestador", 0);
			enviar_direccionNivel(socketOrquestador,ipNivel,puertoNivel,nivel);

		//---------------------Inicio dibujo de la pantalla-------------------------

				int rows, cols;

				nivel_gui_inicializar();

				nivel_gui_get_area_nivel(&rows, &cols);

				dibujarLosRecursos();

				nivel_gui_dibujar(ListaItems);

		//---------------------Fin dibujo de la pantalla-------------------------

				pthread_join(hiloRecepcionista, NULL);
				pthread_mutex_destroy(&sema);
		}
	}

	return 0;
}

int aceptar_conexion(int servidor)
{
	int socketEscucha = (int) servidor;
	//printf("socketEscucha %d \n", socketEscucha);
	// Aceptar una nueva conexion entrante. Se genera un nuevo socket con la nueva conexion.
	//char buffer_in[1024];
	//char buffer_out[1024];

	int socketNuevaConexion;
	fd_set master; // conjunto maestro de descriptores de fichero
	fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
	int fdmax; // número máximo de descriptores de fichero
	//int nbytes;
	FD_ZERO(&master);	// borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);

	FD_SET(socketEscucha, &master);// añadir socketEscucha al conjunto maestro

	fdmax = socketEscucha; // descriptor de fichero mayor (por ahora es éste)
	// bucle principal
	for(;;) {
	read_fds = master; // cópialo
	if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1)
	{
		log_error(logger,"Error en select");
		perror("select");
		exit(1);
	}
	int i;
	// explorar conexiones existentes en busca de datos que leer
	for(i = 0; i <= fdmax; i++)
	{
		if (FD_ISSET(i, &read_fds))  // ¡¡tenemos datos!!
		{
			if (i == socketEscucha)
			{
				// gestionar nuevas conexiones
				//printf("%d",socketEscucha);
				if ((socketNuevaConexion = accept(socketEscucha, NULL, 0)) < 0)
				{
					log_error(logger,"Error en accept");
					perror("accept");
				}
				else //El server aceptó la conexion del cliente
				{
					FD_SET(socketNuevaConexion, &master); // añadir al conjunto maestro
					if (socketNuevaConexion > fdmax)
					{
						fdmax = socketNuevaConexion;	// actualizar el máximo
					}
					//printf("Se aceptó la conexión del socket:%d\n", socketNuevaConexion);
				}
			}
			else // Es un cliente que ya estaba conectado y quiere que lo atiendan
			{
				pthread_mutex_lock(&sema);
				char *stream;
				if ((stream = (char*) recibir_mensaje(i))== NULL) // error o conexión cerrada por el cliente
				{
						removerPersonaje(i);
						cerrar_socket(i); // Cierro el socket del cliente que se desconectó
						FD_CLR(i, &master); // Elimino el socket que se desconectó del conjunto maestro
				}
				else //Tengo que atenderlo postamente: acá iría la deserialización
				{
					atender_pedidos(i,stream);
				}
				pthread_mutex_unlock(&sema);
			 }
		  }
		}



}
	return 0;
}

void obtenerRecursosYGuardarlosEnVectorDeRecursos(t_config* unPath)
{

	//int cantCajas;
	//si es 0 hay una caja
	//si es uno hay dos cajas
	//si es dos hay dos cajas

    int i =-1;
	const char *s="Caja";
	char **p;

	int nroCaja=0;
	char *siguiente;

	 char * siguienteCaja(){

		 	   nroCaja++;

		 	   char *m = string_from_format("%s%d",s,nroCaja);
		 	   return m ;

		 	}
	 siguiente= siguienteCaja();

while(config_has_property(unPath, siguiente ) )
	 {

 	//Le mando mi string "Flores,F,3,23,0" y me lo convierte a
    //["Flores","F","3","23","0"]

	i++;

	p = config_get_array_value(unPath,siguiente);

	free(siguiente);

	recursosEnNivel[i].nombre=*p;



 	recursosEnNivel[i].recurso=(char)**(p+1);

 	recursosEnNivel[i].cantidadDisponible=atoi(*(p+2));

 	recursosEnNivel[i].cantidadEnUso=0;

 	recursosEnNivel[i].posX=atoi(*(p+3));

 	recursosEnNivel[i].posY=atoi(*(p+4));

 	recursosEnNivel[i].quienesTienenEsteRecurso = list_create();

	siguiente= siguienteCaja();

		 	}


cantCajas=i;
liberarVector(p);
free(p);

}

void dibujarLosRecursos(){

	//solo dibuja la cantidad que hay en el archivo de cofig
		int i=0;
		for (;i<=cantCajas;i++){
		CrearCaja(&ListaItems, recursosEnNivel[i].recurso, recursosEnNivel[i].posX, recursosEnNivel[i].posY, recursosEnNivel[i].cantidadDisponible);
		nivel_gui_dibujar(ListaItems);
		}
}


void activarDetector(int milisegundos) {

	while (1){
		sleep(milisegundos / 1000);
		pthread_mutex_lock(&sema);
		verificarInterbloqueo();
		pthread_mutex_unlock(&sema);
	}
}

void finalizarNivel()
{
	pthread_cancel(hiloDetector);
	int q;//recursosEnNivel[q].nombre != NULL
	for(q=0;q<cantCajas; q++)
	{
		free(recursosEnNivel[q].nombre);
		list_destroy(recursosEnNivel[q].quienesTienenEsteRecurso);
	}
	list_destroy(personajesEnNivel);
	list_destroy(personajesPeticiones);
	pthread_join(hiloDetector, NULL);
	config_destroy(configNivel);
	log_destroy(logger);
	printf("se libero la memoria utilizada por el nivel, fin de ejecucion\n");
}

void loguearEstadoDeNivel()
{
	char* auxiliar;
	char* auxiliar2;
	int flag = 1;
    if(list_is_empty(personajesEnNivel))
    {
    	auxiliar = strdup("no hay personajes conectados al nivel %s\n");
    }
    else
    {
    	void loguearPersonajeEnNivel(t_personaje *elemento)
    	{
    		char* aux;
    		if(flag)
    		{
    			auxiliar = strdup(elemento->remitente);
    			flag = 0;
    		}
    		else
    		{
    			aux = auxiliar;
    			auxiliar = string_from_format("%s <- %s", aux,elemento->remitente);
    			free(aux);
    		}
    	}
    	list_iterate(personajesEnNivel, (void*)loguearPersonajeEnNivel);
    	log_info(logger,"Lista de personajes conectados al nivel %s: %s",nivel, auxiliar);
    }

    free(auxiliar);
    auxiliar = NULL;
    flag = 1;

	if(list_is_empty(personajesPeticiones))
	{
		auxiliar = strdup("no hay personajes en lista de peticiones del nivel %s\n");
	}
	else
	{
		void loguearPersonajeEnPeticion(t_pedirPosicionRecurso *elemento)
		{
			char* aux;
    		if(flag)
    		{
    			auxiliar = string_from_format("%s[%c]",elemento->remitente, elemento->recurso);
    			flag = 0;
    		}
    		else
    		{
    			aux = auxiliar;
    			auxiliar = string_from_format("%s <- %s[%c]", auxiliar, elemento->remitente, elemento->recurso);
    			free(aux);
    		}
		}
		list_iterate(personajesPeticiones, (void*)loguearPersonajeEnPeticion);
		log_info(logger,"Lista de personajes peticiones del nivel %s: %s", nivel, auxiliar);
	}

	free(auxiliar);
	auxiliar = NULL;
	flag = 1;

	int n;
	for(n=0;n<=cantCajas;n++)
	{
		if(list_is_empty(recursosEnNivel[n].quienesTienenEsteRecurso))
		{
			auxiliar2 = strdup("ningun personaje tiene este recurso\n");
		}
		else
		{
			void loguearPersonajeConTenencia(char *elemento)
			{
				char* aux;
	    		if(flag)
	    		{
	    			auxiliar2 = strdup(elemento);
	    			flag = 0;
	    		}
	    		else
	    		{
	    			aux = auxiliar2;
	    			auxiliar2 = string_from_format("%s <- %s", auxiliar2, elemento);
	    			free(aux);
	    		}
			}
			list_iterate(recursosEnNivel[n].quienesTienenEsteRecurso, (void*)loguearPersonajeConTenencia);
			flag = 1;
		}

		char* aux;
		if(!n)
		{
			auxiliar = string_from_format("recurso [%c], cantidad en uso: %d, cantidad disponible: %d, quienes lo tienen: %s\n",
				recursosEnNivel[n].recurso,
				recursosEnNivel[n].cantidadEnUso,
				recursosEnNivel[n].cantidadDisponible,
				auxiliar2);
		}
		else
		{
			aux = auxiliar;
			auxiliar = string_from_format("%srecurso [%c], cantidad en uso: %d, cantidad disponible: %d, quienes lo tienen: %s\n",
					auxiliar,
					recursosEnNivel[n].recurso,
					recursosEnNivel[n].cantidadEnUso,
					recursosEnNivel[n].cantidadDisponible,
					auxiliar2);
			free(aux);
		}
		free(auxiliar2);
		auxiliar2 = NULL;
	}

	log_info(logger, "Lista de recursos del nivel %s:\n%s",nivel,auxiliar);
	free(auxiliar);
}

void removerPersonaje(int socket)
{
	int buscarPorSocket(t_personaje *elemento)
	{
		return (elemento->socket == socket);

	}
	t_personaje *personajeARemover = list_remove_by_condition(personajesEnNivel, (void*) buscarPorSocket);
	if(personajeARemover != NULL)
	{
		char* nombrePersonaje = string_from_format("%s:0",personajeARemover->remitente);
	    devolver_recursos(nombrePersonaje);
	}
	else
	{
		log_info(logger,"no se encontro socket del personaje a remover");
	}
}

