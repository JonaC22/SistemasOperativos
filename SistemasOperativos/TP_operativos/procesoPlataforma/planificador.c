/*
 * planificador.c
 *
 *  Created on: May 23, 2013
 *      Author: juan
 */

#include "planificador.h"
#include <pthread.h>
#include <malloc.h>
#include <sockets.h>
#include <src/commons/config.h>
#include <serializacion.h>
#include <stdlib.h>
#include <unistd.h>
#include <src/commons/string.h>
#include <src/commons/collections/list.h>
#include <stdint.h>
#include <string.h>
#include <semaphore.h>
#include <src/commons/log.h>

static const int ME_MORI_Y_NO_TENGO_VIDAS = 6;
static const int ME_MORI_Y_TENGO_VIDAS = 5;
static const int CONSEGUI_RECURSO = 4;
static const int TERMINE_PLAN = 3;
static const int TERMINE_NIVEL = 2;
extern int quantum;
extern t_list *todosLosPersonajesConectados;
extern pthread_mutex_t semaforoConexiones;
extern t_dictionary *planificadores;
extern t_config* configPlataforma;
extern char* rutaLogger;
extern t_log* logger;

void planificar_turnos(t_planificador **direccion_planificador)
{
	t_planificador *planificador;
	planificador= *direccion_planificador;
	t_setColas *set = planificador->setColas;

	if (!queue_is_empty(set->colaListos)) //Si hay personajes listos para planificar.
	{

		if((planificador->socketPersonajeActual == -1 || (planificador->quantumsRestantes)==0)) //Si todavia nunca planifico o ya no quedan quantums
		{	pthread_mutex_lock(&set->semaforo);
		    sem_wait(&set->contadorDeListos);
			planificador->socketPersonajeActual =((t_personaje*) queue_peek(set->colaListos))->socket; //Miro al primer personaje de la cola
			sem_post(&set->contadorDeListos);
			pthread_mutex_unlock(&set->semaforo);
			planificador->quantumsRestantes=quantum; //Restauro el valor original de quantum
		}

		int respuestaDeTurno = enviar_permisoDeMovimiento(&planificador); //Le envio el turno

		(planificador->quantumsRestantes)--; //Decremento el quantum


		if(respuestaDeTurno == -1) //Si hubo algún error (el personaje se desconectó o no pudo responder por algún otro motivo)
		{
			t_personaje *personajeFalla;
			pthread_mutex_lock(&set->semaforo);
			sem_wait(&set->contadorDeListos);
			personajeFalla = (t_personaje *) queue_pop(set->colaListos); //Saco al personaje de la cola
			pthread_mutex_unlock(&set->semaforo);
			close(planificador->socketPersonajeActual);//Cierro su socket

			int _es_el_personaje(t_personaje* alguien)
			{
				return string_equals_ignore_case(alguien->remitente,personajeFalla->remitente);
			}
			pthread_mutex_lock(&semaforoConexiones);
			list_remove_by_condition(todosLosPersonajesConectados, (void*) _es_el_personaje); //Lo saco de los conectados
			log_info(planificador->logger,"%s se sacó de la lista de personajes conectados y de cola de listos porque hubo un error en recibir la respuesta de turno. \n", personajeFalla->remitente);
			loguearConectados(planificador);
			pthread_mutex_unlock(&semaforoConexiones);
			free(personajeFalla->remitente);
			free(personajeFalla);

		}
		else //Si no hubo errores
		{


			if(respuestaDeTurno==1) //Si luego del turno el personaje se bloqueó
			{
				pthread_mutex_lock(&set->semaforo);
				sem_wait(&set->contadorDeListos);
				queue_pop(set->colaListos); //Saco al personaje actual de la cola (Ya se paso a la de bloqueados)
				pthread_mutex_unlock(&set->semaforo);
				planificador->socketPersonajeActual=-1; //Pongo el socket por defecto para que el planificador mire el socket siguiente.
			}
			else //Si no se bloqueó
			{

				if ( ( (planificador->quantumsRestantes == 0) && (respuestaDeTurno != TERMINE_NIVEL) && (respuestaDeTurno != TERMINE_PLAN) && (respuestaDeTurno != ME_MORI_Y_TENGO_VIDAS) && (respuestaDeTurno != ME_MORI_Y_NO_TENGO_VIDAS)) || respuestaDeTurno == CONSEGUI_RECURSO) //Y se le terminaron los quantums y no terminó el nivel o consiguió el recurso
				{
					pthread_mutex_lock(&set->semaforo);
					sem_wait(&set->contadorDeListos);
					t_personaje* aux= queue_pop(set->colaListos); //Lo saco de la cola para volver a ponerlo en el fondo (antes no lo habia sacado)
					queue_push(set->colaListos, (void*)aux); //Vuelvo a encolar al personaje anterior.
					sem_post(&set->contadorDeListos);
					pthread_mutex_unlock(&set->semaforo);
					planificador->socketPersonajeActual=-1;
					if(respuestaDeTurno == CONSEGUI_RECURSO)
					{
						printf("Cambio de personaje porque %s consiguió el recurso.",aux->remitente);
					}
					else
					{
						printf("Cambio de personaje porque %s terminó sus quantums.",aux->remitente);

					}

				}
				if(respuestaDeTurno == TERMINE_NIVEL || respuestaDeTurno == TERMINE_PLAN) //Si terminó el nivel
				{
					t_personaje *personaje;
					pthread_mutex_lock(&set->semaforo);
					sem_wait(&set->contadorDeListos);
					personaje = (t_personaje *) queue_pop(set->colaListos); //Saco al personaje de la cola

					char *stream= serializer(8,personaje); //Es solo para desbloquear el recv, no importa que le mande
					enviar_mensaje(personaje->socket,stream);

					pthread_mutex_unlock(&set->semaforo);

					if (respuestaDeTurno == TERMINE_PLAN) //Si terminó su plan
					{
						int _es_el_personaje(t_personaje* alguien)
						{
							return string_equals_ignore_case(alguien->remitente,personaje->remitente);
						}

						pthread_mutex_lock(&semaforoConexiones);
						list_remove_by_condition(todosLosPersonajesConectados, (void*) _es_el_personaje);
						log_info(planificador->logger,"%s se sacó de la lista de conectados porque terminó su plan de niveles. Ahora quedan %d", personaje->remitente, todosLosPersonajesConectados->elements_count);
						loguearConectados(planificador);
						pthread_mutex_unlock(&semaforoConexiones);

						pthread_mutex_lock(&semaforoConexiones);
						if(list_is_empty(todosLosPersonajesConectados))
						{
							finalizarNiveles();

							char* parametros[3];
							parametros[0] = config_get_string_value(configPlataforma,"binarioKoopa");
							parametros[1] = config_get_string_value(configPlataforma,"listaKoopa");
							parametros[2] = NULL;

							printf("Binario: %s \nLista: %s \nEjecutar Koopa? (s/n) \n", parametros[0],parametros[1]);

							char respuesta;
							scanf("%c", &respuesta);
							if(respuesta == 's')
							{

								execv(parametros[0], parametros);
								printf("Error en execv\n");
								exit(1);
							}
							else
							{
								exit(1);
							}
						}
						pthread_mutex_unlock(&semaforoConexiones);
					}

					planificador->socketPersonajeActual=-1; //Pongo el socket por defecto para que el planificador agarre el nuevo


				}
			}
			pthread_mutex_lock(&semaforoConexiones);
			pthread_mutex_lock(&planificador->setColas->semaforo);

			loguearColas(planificador);

			pthread_mutex_unlock(&planificador->setColas->semaforo);
			pthread_mutex_unlock(&semaforoConexiones);
		}
	}
}

void loguearColas(t_planificador *planificador)
{

	void nivelPlanificador(char* nombreDeNivel, t_planificador *unPlanificador)
	{
		if(unPlanificador->socket == planificador->socket)
		{
			printf("\n\nplanificador de nivel: %s\n", nombreDeNivel);
		}
	}
	dictionary_iterator(planificadores, (void*)nivelPlanificador);
	t_queue *colaAux;
	colaAux=queue_create();
	t_personaje *personaje;
	t_personajeBloqueado *personajeBloqueado;
	printf("Cola de listos: ");
	if(queue_is_empty(planificador->setColas->colaListos))
	{
		printf("no hay personajes en cola de listos");
	}
	while(!queue_is_empty(planificador->setColas->colaListos))
	{
		personaje = queue_pop(planificador->setColas->colaListos);
		queue_push(colaAux, personaje);
		printf("%s:%d <- ", personaje->remitente, personaje->socket);
	}
	while(!queue_is_empty(colaAux))
	{
		personaje = queue_pop(colaAux);
		queue_push(planificador->setColas->colaListos, personaje);
	}

	printf("\nCola de bloqueados: ");
	int tamano=list_size(planificador->setColas->bloqueados);
	int i;
	if(list_is_empty(planificador->setColas->bloqueados))
	{
		printf("no hay personajes en lista de bloqueados");
	}
	for(i=0;i<tamano;i++)
	{
		personajeBloqueado = list_get(planificador->setColas->bloqueados, i);
		printf( "%s:%c <- ", personajeBloqueado->nombre, personajeBloqueado->recurso);

	}
	printf("\n\n");
	queue_destroy(colaAux);

}


int aceptar_conexion_planificador(t_planificador *planificador)
{

	planificador->quantumsRestantes = quantum;
	int socketEscucha = planificador->socket;

	printf("Planificador en socket %d \n", socketEscucha);
	// Aceptar una nueva conexion entrante. Se genera un nuevo socket con la nueva conexion.

	int milisegundos =config_get_int_value(configPlataforma,"milisegundos")*1000;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = milisegundos;
	int socketNuevaConexion;
	fd_set master; // conjunto maestro de descriptores de fichero
	fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
	int fdmax; // número máximo de descriptores de fichero

	FD_ZERO(&master);	// borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);

	FD_SET(socketEscucha, &master);	// añadir socketEscucha al conjunto maestro

	fdmax = socketEscucha; // descriptor de fichero mayor. NO voy a agregar otros, siempre va a ser este

	for (;;)
	{
		read_fds = master; // Copio el master con el de lectura
		int retornoSelect = select(fdmax + 1, &read_fds, NULL, NULL, &tv);

		if (retornoSelect == -1)
		{
			perror("select");
		}
		else
		{
			if(retornoSelect != 0) //Si salio porque un socket tuvo actividad
			{
				// gestionar nuevas conexiones

				if ((socketNuevaConexion = accept(socketEscucha, NULL, 0))< 0)
				{
					perror("accept");
				}
				else //El server aceptó la conexion del cliente
				{
					//No actualizo el máximo ni el maestro, solo necesito al planificador aca
					printf("Se aceptó la conexión del personaje con socket:%d\n",socketNuevaConexion);
					agregar_nuevoPersonaje(socketNuevaConexion, planificador);
				}
			}
			else //Se entra a este else cuando retornoSelect es 0, es decir que salio por timeOut => Hora de planificar!
			{
				planificar_turnos(&planificador);
			}

			tv.tv_usec = milisegundos;
		}
	}
	return socketNuevaConexion;
}

t_planificador* lanzar_hilo_planificador(int socket, char* nombreNivel)
{
	t_planificador *planificador = malloc(sizeof(t_planificador));
	planificador->setColas = malloc(sizeof(t_setColas)); //Colas con las cuales se manejara el planificador (Listos y bloqueados)
	planificador->asignaciones= list_create(); //Lista de t_asignación que tiene nombre, recurso y cantidad de recurso.

	planificador->setColas->colaListos = queue_create();
	planificador->setColas->bloqueados = list_create();
	log_info(logger,"Crea todas las colas \n");

	pthread_mutex_init(&planificador->setColas->semaforo,NULL);
    sem_init(&planificador->setColas->contadorDeListos,0,0);
    sem_init(&planificador->setColas->contadorDeBloqueados,0,0);

	log_info(logger,"Inicializa todos los semaforos\n");

	char** rutaSpliteada = string_split(rutaLogger,".");

	char* ruta = string_from_format("%s_planificador_%s.log", rutaSpliteada[0], nombreNivel);

	planificador->logger = log_create(ruta, "TEST",false, LOG_LEVEL_INFO);

	log_info(logger,"Crea el logger del planificador\n");

	planificador->socket=socket;
	planificador->socketPersonajeActual=-1;

	dictionary_put(planificadores, nombreNivel, planificador); //Agrego el planificador al diccionario global para que lo acceda el orquestador tambien

	free(nombreNivel);
	pthread_t planificadorNivel;

	if(pthread_create(&planificadorNivel, NULL, (void *) aceptar_conexion_planificador,(void *)planificador))
	{
		log_info(logger,"fallo al crear hilo planificador");
	} //Lanzo el hilo con todos los datos del planificador

	return planificador;

}

t_socket_random* crear_socket_planificador(char* ip)
{

	t_socket_random *socketPlanificador;

	socketPlanificador =(t_socket_random *)crear_servidor_random(ip);

	if (socketPlanificador->socket == -1)
	{
		log_info(logger,"Error al crear el socket de hilo planificador");
		return NULL;
	}

	return socketPlanificador;
}


void enviarMensajeDeFinalizacion(char* key,void *data)
{
	int codigo = FINAL;
	char* stream = malloc(sizeof(int)*3);
	int medida = sizeof(int)*3;
	int offset = 0;
	int size;
	memcpy(stream, &codigo , size = sizeof(int));
	offset += size;
	memcpy(stream+offset, &medida, size = sizeof(int));
	offset += size;
	memcpy(stream+offset, &codigo, sizeof(int));
	int socket = conectar_socket(((t_direccion*)data)->ipNiv,((t_direccion*)data)->puertoNiv);
	enviar_mensaje(socket, stream);
	free(stream);
}

void finalizarNiveles()
{
	dictionary_iterator(direccionesNiveles,(void*)enviarMensajeDeFinalizacion);

	void destroyerNivel(void* data)
	{
		free(((t_direccion*)data)->ipNiv);
		//free(((t_direccion*)data)->ipPlan);
		free(data);
	}

	dictionary_clean_and_destroy_elements(direccionesNiveles, (void*)destroyerNivel);
	sleep(3);
}
