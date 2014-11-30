/*
 * pedidos.c
 *
 *  Created on: May 16, 2013
 *      Author: juan
 */

#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "pedidos.h"
#include "planificador.h"
#include <src/commons/log.h>
#include <src/commons/config.h>
#include <src/commons/collections/list.h>
#include <src/commons/string.h>
#include <pthread.h>

static const int TERMINE_PLAN = 3;
static const int TERMINE_NIVEL = 2;
extern char* rutaLogger;
t_log* logger;
extern int quantum;
extern t_dictionary *planificadores;
extern t_list *todosLosPersonajesConectados;
extern pthread_mutex_t semaforoConexiones;

int atender_pedidos_orquestador(int cliente, char* stream_mensaje)
{
	if(logger != NULL) log_destroy(logger);
	logger = log_create(rutaLogger,"TEST",true, LOG_LEVEL_INFO);
	void *puntero = NULL;
	int codigo;
	memcpy(&codigo, stream_mensaje, sizeof(int));
//pedir direccion nivel y planificador
	t_pedirDireccion *pedirD;

//Si esta activado recovery:interbloqueados
	t_interbloqueo *procesosEnInterbloqueo;

	t_planificador *planificador;
//Recursos liberados

	t_recursosLiberados *recursosLiberados;
	t_direccionNombre *direccionNivel;

	puntero = deserializer(stream_mensaje);


	switch (codigo)
	{

	case 3:
		pedirD = puntero;
		responder_direccion(cliente, pedirD->nombrenivel);
		free(pedirD);
		break;

	case 7:

		direccionNivel = puntero;
		log_info(logger, "El %s tiene socket %d", direccionNivel->remitente, cliente);
		agregar_nivel(direccionNivel);
		break;


	case 10:

	   procesosEnInterbloqueo = puntero;

	   planificador = dictionary_get(planificadores, procesosEnInterbloqueo->nivel);

	   pthread_mutex_lock(&planificador->setColas->semaforo);

	   log_info(logger,"<INTERBLOQUEO> se recibio el resumen de personajes involucrados, estos son %s, durante el nivel %s\n"
			   ,procesosEnInterbloqueo->personajes,procesosEnInterbloqueo->nivel,0);

	  // sleep(5);
	   definirVictima(cliente,procesosEnInterbloqueo);
	 //  free(procesosEnInterbloqueo);

	   pthread_mutex_unlock(&planificador->setColas->semaforo);

	   break;


	case 11:
		recursosLiberados = puntero;
		liberar_personajes(recursosLiberados, cliente);
		free(recursosLiberados->nivel);
		free(recursosLiberados->personaje);
		free(recursosLiberados);


		break;
	default:
		printf("\n\n\n\n ---Error codigo: %d !--- \n\n\n\n",codigo);
		free(puntero);
		break;

	}
	log_destroy(logger);
	logger = NULL;

	return codigo;
}

void definirVictima(int socketNivel ,t_interbloqueo *interbloqueo)
{

	int cliente = socketNivel;
	char *stream_victimaElegida;

	t_interbloqueo *victima=malloc(sizeof(t_interbloqueo));
	t_recursosLiberados *recursos=malloc(sizeof(t_recursosLiberados));

	char **nombreVictima;
	nombreVictima = string_split(interbloqueo->personajes, ":");

	victima->personajes=strdup(*nombreVictima);

	victima->nivel=strdup(interbloqueo->nivel);

	log_info(logger,"<INTERBLOQUEO> la victima seleccionada es %s\n",victima->personajes);


	matar_victima(victima->personajes);

    stream_victimaElegida = serializer(10, (void*) victima);

  ////////// CODIGO DE VALIDACION DE DATOS DE LA VICTIMA QUE SE ENVIAN AL NIVEL ////////////////////
    int numero; //ver comentario en la parte de interbloqueo sobre esta parte de la comunicacion
    memcpy(&numero, stream_victimaElegida, 4);
    int largo;
    memcpy(&largo, stream_victimaElegida+4, 4);
    log_info(logger,"codigo %d, largo %d, data %s", numero, largo,(stream_victimaElegida+8));

    log_info(logger,"<INTERBLOQUEO> se le solicita al nivel informar los recursos que tiene asignados la victima %s del nivel %s\n", victima->personajes, victima->nivel);

    enviar_mensaje(cliente, stream_victimaElegida); //envia el personaje que solicita conocer sus recursos asignados

    log_info(logger,"<INTERBLOQUEO> se espera una respuesta del nivel");

   // int unCodigo;
    char* stream = recibir_mensaje(cliente);
 /*   memcpy(&unCodigo, stream, 4);

    while(unCodigo == 94){  // cuando recibe codigo 94 es porque le esta pidiendo que le mande devuelta,
    						// fue a causa de que quiso deserializar un stream que no tenia codigo 10
    	enviar_mensaje(cliente, stream_victimaElegida);
    	printf("se mando devuelta, esperando respuesta\n");
    	stream = recibir_mensaje(cliente);
    	memcpy(&unCodigo, stream, 4);
    }*/

    recursos = deserializer(stream);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//recursos = recibir_recursosLiberados(cliente); //recibe los recursos que tiene asignados

	log_info(logger, "<INTERBLOQUEO> los recursos que tiene asignado el personaje %s a matar del nivel %s son: %s, son %d diferentes",
				recursos->personaje, recursos->nivel ,recursos->recursos, recursos->total);

    liberar_personajes(recursos, cliente); //se liberan los personajes segun los recursos
	sleep(TERMINE_NIVEL);

    free(interbloqueo->personajes);
    free(interbloqueo->nivel);
    free(interbloqueo);
    free(recursos->nivel);
    free(recursos->personaje);
    free(recursos);
    free(victima->personajes);
    free(victima->nivel);
    free(victima);
    liberarVector(nombreVictima);
}

void matar_victima(char* victima)
{
  t_personaje *personaje;
  bool es_la_victima(t_personaje* alguien)
  {
    return string_equals_ignore_case(alguien->remitente,victima);
  }
  pthread_mutex_lock(&semaforoConexiones);
  personaje = list_find(todosLosPersonajesConectados, (void*)es_la_victima);
  pthread_mutex_unlock(&semaforoConexiones);

  if(personaje != NULL)
  {
	  t_boolean *muerte = malloc(sizeof(t_boolean));
	  muerte->boolean=0;

	  char* streamMuerte = serializer(4,muerte);

	  enviar_mensaje(personaje->socket, streamMuerte);
	  free(muerte);
	  free(streamMuerte);

	  log_info(logger,"Se envió mensaje de muerte al personaje %s con socket %d\n", personaje->remitente, personaje->socket);
  }
  else
  {
	  log_info(logger, "El personaje que se quiso matar ya no esta conectado\n");
  }
}

void liberar_personajes(t_recursosLiberados *recursos,int socketNivel)
{
	if(recursos->total > 0)
	{
		log_info(logger, "se procede a liberar los personajes bloqueados por los recursos %s de %s para el socket de nivel %d\n",
				recursos->recursos, recursos->personaje,socketNivel);
	}
	else
	{
		log_info(logger, "El personaje %s no tiene ningún recurso que liberar", recursos->personaje);
	}
	char* nivel = recursos->nivel;
	char* personaje = recursos->personaje;
	t_personaje *victimaParaColaDeListos = NULL;
	t_planificador* planificador = dictionary_get(planificadores,nivel);
	t_setColas *colas = planificador->setColas; //Traigo las colas del nivel

	int cantidadTotal;
	cantidadTotal = recursos->total;
	char *recursosSobrantes="";
	int tieneMasVidas=0;

	bool es_la_victima(t_personajeBloqueado* alguien)
	{
		return string_equals_ignore_case(alguien->nombre,personaje);
	}

	if(! list_is_empty(colas->bloqueados))
	{

	  t_personajeBloqueado *victimaBloqueada=list_remove_by_condition((t_list*)(colas->bloqueados),(void*) es_la_victima);
	  if(victimaBloqueada != NULL)//Si es una víctima y no un personaje cualquiera que terminó el nivel
	  {
		  tieneMasVidas = recibir_terminoElNivel(victimaBloqueada->socket); //Le pregunto si tiene más vidas o no.
		  if(tieneMasVidas) //Si tiene más, la preparo para encolarla. Sino, no.
		  {
			  sem_wait(&colas->contadorDeBloqueados);
			  victimaParaColaDeListos = malloc(sizeof(t_personaje));
			  victimaParaColaDeListos->socket= victimaBloqueada->socket;
			  victimaParaColaDeListos->remitente = strdup(victimaBloqueada->nombre);
			  log_info(logger, "se saco de la cola de bloqueados al personaje %s que libera recursos y se lo devolvio a la cola de listos\n", victimaParaColaDeListos->remitente);

		  }
		  else
		  {
			  log_info(logger, "%s se quedó sin vidas, se lo saca de bloqueados y no se lo vuelve a poner en la cola de listos.",victimaBloqueada->nombre);
			  cerrar_socket(victimaBloqueada->socket);
		  }

		  free(victimaBloqueada->nombre);
		  free(victimaBloqueada);
	  }
	}

	if(recursos->total > 0)
	{
	  char **recursosLiberados;
	  if(cantidadTotal>1)
	  {
		recursosLiberados = string_split((char*)recursos->recursos,":"); // F1:H2:V3:etc => [F1,H2,V3]
	  }
	  else
	  {
		recursosLiberados = &(recursos->recursos);
	  }

	  int i,j;

		for(i=0; i<cantidadTotal ;i++)
		{

			char recurso = recursosLiberados[i][0]; //Si tenemos F:1, esto sería F
			int cantidadRecurso = atoi(&(recursosLiberados[i][1])); //Si tenemos F:1, esto sería 1
			int cantidadSobrante = cantidadRecurso;
			for(j=0; j<cantidadRecurso ; j++)
			{
				int huboLiberado=liberar_un_personaje(colas, recurso);
				if(huboLiberado)
				{
					cantidadSobrante--;
				}
				else
				{
					break;
				}
			}

			if(i == cantidadTotal-1 || cantidadTotal == 1)
			{
				recursosSobrantes=string_from_format("%s%c%d",recursosSobrantes,recurso,cantidadSobrante);
			}
			else
			{
				recursosSobrantes=string_from_format("%s%c%d:",recursosSobrantes,recurso,cantidadSobrante);
			}
		}
	}

	if(victimaParaColaDeListos != NULL && tieneMasVidas)
	{
		queue_push(colas->colaListos,victimaParaColaDeListos); //Si hay victima, la encola en listos.
		sem_post(&colas->contadorDeListos);
	}

}

void enviar_recursosSobrantes(int socketNivel,char* recursosSobrantes, int total)
{
	t_recursosLiberados *recursos = malloc(sizeof(t_recursosLiberados));

	recursos->recursos= recursosSobrantes;
	recursos->personaje="";
	recursos->nivel="";
	recursos->total=total;

	char *stream_recursosSobrantes;

	if(!strcmp(recursosSobrantes,""))
	{
		log_info(logger, "se notifica al nivel que no sobraron recursos\n");
		stream_recursosSobrantes= serializer(11,recursos);
		int clave = 25;
		memcpy(stream_recursosSobrantes,&clave,4);
	}
	else
	{
		log_info(logger, "se envian los recursos sobrantes %s (en total %d) al nivel\n", recursos->recursos, recursos->total);
		stream_recursosSobrantes= serializer(11,recursos);
	}

	enviar_mensaje(socketNivel, stream_recursosSobrantes);

//	free(recursosSobrantes);
//	free(recursos);
//	free(stream_recursosSobrantes);
}

int liberar_un_personaje(t_setColas *colas, char recurso)
{
	t_personajeBloqueado *personajeParaLiberar = NULL;
	bool quiere_recurso(void *alguien)
	{
		return ((t_personajeBloqueado*)alguien)->recurso == recurso;
	}
	if(!list_is_empty(colas->bloqueados))
	{
	 sem_wait(&colas->contadorDeBloqueados);
	 personajeParaLiberar = list_find(colas->bloqueados,quiere_recurso); //Busco algún bloqueado que quiera lo que estoy liberando
	 sem_post(&colas->contadorDeBloqueados);
	}
	if(personajeParaLiberar != NULL)//Si algún bloqueado quiere el recurso que libero
	{

		t_boolean *desbloqueo;
		desbloqueo = malloc(sizeof(t_boolean));
		desbloqueo->boolean = 1;
		char *stream_desbloqueo;
		stream_desbloqueo= serializer(4,desbloqueo);
		enviar_mensaje(personajeParaLiberar->socket, stream_desbloqueo);
		free(desbloqueo);
		free(stream_desbloqueo);

		int estado = recibir_terminoElNivel(personajeParaLiberar->socket);
		if ( (estado != TERMINE_NIVEL) && (estado != TERMINE_PLAN) )
		{
			t_personaje *personajeLiberado = malloc(sizeof(t_personaje));
			personajeLiberado->remitente=strdup(personajeParaLiberar->nombre);
			personajeLiberado->socket=personajeParaLiberar->socket;

			queue_push(colas->colaListos, personajeLiberado); //Lo pongo en la cola de listos
			sem_post(&colas->contadorDeListos);

			log_info(logger, "Se liberó a %s por %c", personajeLiberado->remitente, recurso);

		}

		bool es_el_personaje(t_personaje* alguien)
		{
			return string_equals_ignore_case(alguien->remitente,personajeParaLiberar->nombre);
		}

		if(estado == TERMINE_PLAN)
		{
			pthread_mutex_lock(&semaforoConexiones);
			list_remove_by_condition(todosLosPersonajesConectados, (void*) es_el_personaje);
			pthread_mutex_unlock(&semaforoConexiones);
		}

		bool es_el_personaje_bloqueado(t_personajeBloqueado* alguien)
		{
		 return string_equals_ignore_case(alguien->nombre,personajeParaLiberar->nombre);
		}
		sem_wait(&colas->contadorDeBloqueados);
		if(list_remove_by_condition(colas->bloqueados, (void*)es_el_personaje_bloqueado) == NULL)//Lo saco de la cola de bloqueados
		{
			sem_post(&colas->contadorDeBloqueados);//vuelvo a incrementar el semaforo porque no lo saco
		}


		free(personajeParaLiberar->nombre);
		free(personajeParaLiberar);

		return 1; //Informo que se desbloqueó un personaje
	}

    return 0; //Informo que no se desbloqueó a nadie
}
int recibir_terminoElNivel(int socketPersonaje)
{
	t_estadoRecurso *respuesta;
	respuesta = deserializer(recibir_mensaje(socketPersonaje));
	return respuesta->estado;
}
void responder_direccion(int socketNuevaConexion, char *nombrenivel)
{

	//direccion
	t_direccion *dir1;
	char *stream; //Es el que uso para responder.


	printf("Se solicitó %s \n", nombrenivel);
	if (dictionary_has_key(direccionesNiveles, nombrenivel))
	{
		dir1 = dictionary_get(direccionesNiveles, nombrenivel);
		printf("Las direcciones requeridas son del %s: \n Puerto nivel: %d, Puerto plan: %d, IP nivel: %s, IP plan: %s \n",	nombrenivel, dir1->puertoNiv, dir1->puertoPlan, dir1->ipNiv,dir1->ipPlan);
		stream = serializer(1, dir1);
		enviar_mensaje(socketNuevaConexion, stream);

	}
	else
	{
		free(nombrenivel);
		printf("El nivel no está conectado \n");
		dir1 = malloc(sizeof(t_direccion));
		dir1->ipNiv="-1";
		dir1->ipPlan="-1";
		dir1->puertoNiv=-1;
		dir1->puertoPlan=-1;

		stream = serializer(1, dir1);
		enviar_mensaje(socketNuevaConexion, stream);
		free(dir1);
	}

	free(stream);

}

void agregar_nivel(t_direccionNombre *direccionNivel)
{
	extern char* ipOrquestador;
	t_direccion *dir= malloc(sizeof(t_direccion));

	dir->ipNiv = strdup(direccionNivel->ip);
	dir->puertoNiv = direccionNivel->puerto;


	dir->ipPlan = ipOrquestador;

	t_socket_random *socketPlanificador;
	socketPlanificador = (t_socket_random *)crear_socket_planificador(ipOrquestador);

	if(socketPlanificador != NULL)
	{
		dir->puertoPlan = socketPlanificador->puerto;
		char* nombreNivel = strdup(direccionNivel->remitente);

		t_planificador *planificador = lanzar_hilo_planificador(socketPlanificador->socket, nombreNivel);

		free(socketPlanificador);
		planificador->quantumsRestantes=quantum;

		printf("Se creo un hilo planificador escuchando en el puerto %d\n", dir->puertoPlan);
		dictionary_put(direccionesNiveles, direccionNivel->remitente, dir);

		t_direccion *dir2;

		dir2= dictionary_get(direccionesNiveles, direccionNivel->remitente);
		printf(	"Las direcciones agregadas son del %s: \n Puerto nivel: %d, Puerto plan: %d, IP nivel: %s, IP plan: %s \n",direccionNivel->remitente, dir2->puertoNiv, dir2->puertoPlan, dir2->ipNiv,dir2->ipPlan);

	}
	else
	{
		log_error(logger,"Error al crear el socket para el hilo planificador");
	}
	free(direccionNivel->remitente);
	free(direccionNivel);
}

t_recursosLiberados *recibir_recursosLiberados(int cliente)
{
	char* stream = recibir_mensaje(cliente);
	log_info(logger,"se recibio el mensaje del nivel con los recursos liberados de un personaje");
	void* puntero = deserializer(stream);
	return puntero;
}
