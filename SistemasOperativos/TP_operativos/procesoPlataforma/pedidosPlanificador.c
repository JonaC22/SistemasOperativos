/*
 * pedidosPlanificador.c
 *
 *  Created on: Jun 3, 2013
 *      Author: juan
 */

#include <src/commons/log.h>
#include <src/commons/collections/dictionary.h>
#include <src/commons/string.h>
#include <malloc.h>
#include <pthread.h>
#include "pedidosPlanificador.h"
#include <serializacion.h>
#include <sockets.h>
#include <string.h>

static const int CONSEGUI_EL_RECURSO = 4;
static const int TERMINE_EL_PLAN = 3;
static const int TERMINE_EL_NIVEL = 2;
static const int ME_BLOQUEE = 1;

extern t_list *todosLosPersonajesConectados;
extern pthread_mutex_t semaforoConexiones;



t_estadoRecurso* recibir_turnoTerminado(int socketPersonaje)
{
	printf("Esperando respuesta de turno...\n");
	t_estadoRecurso *estado;
	char *stream_estado;
	void *puntero;

	stream_estado = recibir_mensaje(socketPersonaje);
	if(stream_estado!=NULL)
	{
		puntero = deserializer(stream_estado);
		estado = (t_estadoRecurso*) puntero;
		return estado;
	}

	return NULL;
}


void agregar_nuevoPersonaje(int socketNuevaConexion,t_planificador *planificador)
{
	t_personaje *personajeNuevo;

	personajeNuevo = recibir_nuevoPersonaje(socketNuevaConexion);
	personajeNuevo->socket = socketNuevaConexion;

	int _es_el_mismo_personaje(t_personaje* alguien)
	{
		return string_equals_ignore_case(alguien->remitente,personajeNuevo->remitente);
	}

	pthread_mutex_lock(&semaforoConexiones);
	list_remove_by_condition(todosLosPersonajesConectados,(void*) _es_el_mismo_personaje);
	list_add(todosLosPersonajesConectados, personajeNuevo);
	pthread_mutex_unlock(&semaforoConexiones);


	pthread_mutex_lock(&planificador->setColas->semaforo);
	queue_push(planificador->setColas->colaListos, personajeNuevo);
	sem_post(&planificador->setColas->contadorDeListos);
	pthread_mutex_unlock(&planificador->setColas->semaforo);

	log_info(planificador->logger,"se agrego el nuevo personaje %s socket:%d",personajeNuevo->remitente, personajeNuevo->socket);
	pthread_mutex_lock(&semaforoConexiones);
	loguearConectados(planificador);
	pthread_mutex_unlock(&semaforoConexiones);
}


t_personaje* recibir_nuevoPersonaje(int socketPersonaje)
{
	t_personaje *nuevoPersonaje = deserializer( recibir_mensaje(socketPersonaje) );
	return nuevoPersonaje;
}


void procesar_bloqueo(t_setColas *set, t_estadoRecurso *estado, t_personaje *personaje)
{
	t_personajeBloqueado *bloqueo=malloc(sizeof(t_personajeBloqueado));
	bloqueo->nombre=strdup(personaje->remitente);
	bloqueo->socket=personaje->socket;
	bloqueo->recurso=estado->recurso;

	pthread_mutex_lock(&set->semaforo);
	list_add(set->bloqueados,bloqueo);
	sem_post(&set->contadorDeBloqueados);
	pthread_mutex_unlock(&set->semaforo);

}


int enviar_permisoDeMovimiento(t_planificador **direccion_planificador)
{
		t_planificador *planificador;
		planificador = *direccion_planificador;
		int socketPersonaje = planificador->socketPersonajeActual;
		t_setColas *set = planificador->setColas;
		t_boolean *turno = malloc(sizeof(turno));
		turno->boolean = 1;

		int loMataron = 0;
		char *stream;

		stream = serializer(4, (void*) turno);
		enviar_mensaje(socketPersonaje, stream); //Un turno con boolean en 1 significa que es un turno y no una muerte.

		free(stream);
		free(turno);

		t_personaje *personaje;

		pthread_mutex_lock(&planificador->setColas->semaforo);
		sem_wait(&planificador->setColas->contadorDeListos);
		personaje=((t_personaje*)queue_peek(planificador->setColas->colaListos));
		sem_post(&planificador->setColas->contadorDeListos);
		pthread_mutex_unlock(&planificador->setColas->semaforo);

		printf("Se envio un turno a %s:%d.\n",personaje->remitente, personaje->socket);
		t_estadoRecurso* estado;

recibirRespuestaDeTurno:

		estado = recibir_turnoTerminado(socketPersonaje); //Recibe la respuesta de fin de turno

		if(estado == NULL)
		{
			return -1;
		}

		if (estado->estado == ME_BLOQUEE) //Si se bloqueó
		{
			procesar_bloqueo(set, estado, personaje); //lo agrega a la cola de bloqueados

			log_info(planificador->logger,"El personaje %s se bloqueo por el recurso %c",personaje->remitente,estado->recurso);
			free(estado);
			if(!loMataron)
			return ME_BLOQUEE;

		}
		if (estado->estado == TERMINE_EL_NIVEL || estado->estado == TERMINE_EL_PLAN) //Si termino el nivel
		{
			//asignar_recurso(personaje,estado->recurso, direccion_planificador);
			log_info(planificador->logger,"El personaje %s consiguió el recurso: %c. ",personaje->remitente,estado->recurso);

			log_info(planificador->logger,"El personaje %s con socket %d terminó este nivel.\n",personaje->remitente,socketPersonaje);
			if(!loMataron)

			return estado->estado;
		}
		if (estado->estado == CONSEGUI_EL_RECURSO)
		{
			log_info(planificador->logger,"El personaje %s consiguió el recurso: %c.\n",personaje->remitente,estado->recurso);
			//asignar_recurso(personaje,estado->recurso, direccion_planificador);

			free(estado);
			if(!loMataron)

			return CONSEGUI_EL_RECURSO;
		}
		if(estado->estado == 5)
		{
			loMataron = 1;
			pthread_mutex_lock(&set->semaforo);
			t_personaje *personajeQueMurioYTieneVidas = queue_pop(set->colaListos);
			queue_push(set->colaListos, personajeQueMurioYTieneVidas); //Lo mando al fondo (el orquestador liberó sus recursos porque el nivel le dijo)
			pthread_mutex_unlock(&set->semaforo);
			//return estado->estado;
			goto recibirRespuestaDeTurno;
		}
		 if(estado->estado == 6)
		 {
			 loMataron = 1;
				pthread_mutex_lock(&set->semaforo);
				t_personaje *personajeQueMurioYNoTieneVidas = queue_pop(set->colaListos); //Lo saco de la cola
				char *stream= serializer(8,personajeQueMurioYNoTieneVidas); //Es solo para desbloquear el recv, no importa que le mande
				enviar_mensaje(personajeQueMurioYNoTieneVidas->socket,stream);//Se le avisa al personaje que ya fue sacado de la cola de listos, para que devuelva sus recursos al nivel.
				pthread_mutex_unlock(&set->semaforo);

		 }
			//Lo pongo con dos ifs separados para que se entienda, pero se podrian sacar y retornar estado->estado.

		free(estado);

	return 0;
}

void asignar_recurso(t_personaje *personaje, char recurso, t_planificador **direccion_planificador)
{
	t_planificador *planificador;
	planificador = *direccion_planificador;
	t_asignacion *recursoAsignado;

	int tiene_recurso(t_asignacion *asignacion)
	{
		return (string_equals_ignore_case(asignacion->nombre, personaje->remitente) && asignacion->recurso == recurso);
	}

	recursoAsignado = list_find(planificador->asignaciones, (void*) tiene_recurso);
	if(recursoAsignado != NULL)
	{
		recursoAsignado->cantidad++;
	}
	else
	{
		recursoAsignado = malloc(sizeof(t_asignacion));
		recursoAsignado->cantidad=1;
		recursoAsignado->nombre=strdup(personaje->remitente);
		recursoAsignado->recurso= recurso;
		list_add(planificador->asignaciones, recursoAsignado);
	}
	log_info(planificador->logger,"<ASIGNACION> %s <- Recurso %c, ahora tiene", recursoAsignado->nombre, recursoAsignado->recurso,recursoAsignado->cantidad);
}


void liberar_recursos(t_personaje *personaje, t_planificador **direccion_planificador)
{
	t_planificador *planificador;
	planificador = *direccion_planificador;
	t_asignacion *recursoParaLiberar;
	t_personajeBloqueado *personajeParaLiberar;

	int tiene_algun_recurso(t_asignacion* asignacion)
	{
		return string_equals_ignore_case(asignacion->nombre,personaje->remitente);
	}
	int quiere_recurso(t_personajeBloqueado* recurso)
	{
		return recurso->recurso == recursoParaLiberar->recurso;
	}

	while(list_find(planificador->asignaciones,(void*) tiene_algun_recurso)) //Mientras el personaje tenga recursos asignados
	{
		recursoParaLiberar = list_remove_by_condition(planificador->asignaciones,(void*) tiene_algun_recurso); //Se le saca la asignación

		int i;
		for(i=0; i<recursoParaLiberar->cantidad; i++) //Y según la cantidad que haya liberado
		{
			if((personajeParaLiberar=list_find(planificador->setColas->bloqueados,(void*) quiere_recurso)) != NULL) //Si hay un personaje para liberar por ese recurso
			{
				t_personaje *personajeLiberado = malloc(sizeof(t_personaje));
				personajeLiberado->remitente = strdup(personajeParaLiberar->nombre);
				personajeLiberado->socket=personajeParaLiberar->socket;
				asignar_recurso(personajeLiberado, personajeParaLiberar->recurso, direccion_planificador); //Le asigno el recurso
				pthread_mutex_lock(&planificador->setColas->semaforo);
				queue_push(planificador->setColas->colaListos,personajeLiberado); //Lo pongo en la cola de listos
				sem_post(&planificador->setColas->contadorDeListos);
				pthread_mutex_unlock(&planificador->setColas->semaforo);

				pthread_mutex_lock(&planificador->setColas->semaforo);
				sem_wait(&planificador->setColas->contadorDeBloqueados);
				if(list_remove_by_condition(planificador->setColas->bloqueados,(void*) quiere_recurso) == NULL) //Y lo saco de la cola de bloqueados
				{
					sem_post(&planificador->setColas->contadorDeBloqueados);
				}
				pthread_mutex_unlock(&planificador->setColas->semaforo);

				log_info(planificador->logger,"Se liberó a %s por recurso %c", personajeParaLiberar->nombre, personajeParaLiberar->recurso);

				free(personajeParaLiberar->nombre);
				free(personajeParaLiberar);
			}
		}
	}
}
