/*
 * personaje.c
 *
 *  Created on: Apr 25, 2013
 *      Author: juan
 */

#include <src/commons/error.h>
#include <stdio.h>
#include <src/commons/config.h>
#include <src/commons/log.h>
#include <src/commons/process.h>
#include <src/commons/string.h>
#include <stdlib.h>
#include <sockets.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include "pedidos.h"
#include "recursos.h"
#include "senales.h"
#include <semaphore.h>

static const int CONSEGUI_EL_RECURSO = 4;
static const int TERMINE_EL_PLAN = 3;
static const int TERMINE_EL_NIVEL = 2;
//Variables del personaje
char* ipOrquestador;
char* nombre;
char* simbolo;
int puertoOrquestador;
char** planDeNiveles;
char** objetivosNivel;
char recursoEnBusqueda;
char* nivelActual;
t_posicion *posicionActual;
int cantidadNivelesTerminados = 0, cantidadRecursosConseguidos = 0,	terminoElNivel = 0, consiguioElRecurso = 0, terminoPlan = 0, fueMatadoPorSenal=0, estaListo = 1;
int socketNivel;
int socketPlanificador;
int socketOrquestador;
int vidas;
int estabaBloqueado=0;
t_config* config;
t_log* logger;
t_posicion *posicionRecurso;

int morir(int socketNivel, int socketPlanificador);
int contarNiveles(t_config* config);
int contarRecursos(t_config* config, char* nivelActual);
int contarItems(char* lista);
void volverAlInicio();

void volverAlInicio()
{
	posicionActual->x = 1;
	posicionActual->y = 1;
}

int main(int argc, char** argv) {
//  EJECUTAR CON PARÁMETROS: ./procesoPersonaje /home/utnso/configs/mario.cfg /home/utnso/logueo/debug.log argv[1] -> primer parametro
if(argc != 3){
	puts("parametros de entrada incorrectos");
	return 1;
}

int loMataron;
muerte:

//Crea logger y trae la configuracion

  config = config_create(argv[1]);
  nombre = config_get_string_value(config, "nombre");
  logger = log_create(argv[2], nombre ,true, LOG_LEVEL_INFO);


	//---------------------------------- Obtención de datos del personaje ---------------------------------

	//Trae el IP y Puerto del orquestador
	char* ippuerto = config_get_string_value(config, "orquestador");
	char** ippuertoSeparados = string_split(ippuerto, ":");
	ipOrquestador = strdup(ippuertoSeparados[0]);
	free(ippuertoSeparados[0]);
	puertoOrquestador = atoi(ippuertoSeparados[1]);
	free(ippuertoSeparados[1]);
	free(ippuertoSeparados);
	int estadoParaPlanificador;

	log_info(logger, "La direccion del orquestador es: %s : %d", ipOrquestador,
			puertoOrquestador); //y me lo loguea

	//Trae el resto de los datos del personaje

	simbolo = config_get_string_value(config, "simbolo");
	nombre = string_from_format("%s%s",simbolo,nombre);
	vidas = config_get_int_value(config, "vidas");
	char **planDeNiveles;
	planDeNiveles = config_get_array_value(config, "planDeNiveles");

	int cantidadNiveles = contarNiveles(config);

	char **objetivosNivel;
	char* niveles = *planDeNiveles;
	char* clave = string_from_format("obj[%s]", niveles);
	objetivosNivel = config_get_array_value(config, clave);
	free(clave);

	int i;
	printf("Objetivos del nivel: ");
	for(i=0; objetivosNivel[i]!='\0'; i++)
	{

		printf("%c",objetivosNivel[i][0]);
	}
	printf("\n");

	//Inicializo la posicion del personaje en (0,0)
	posicionActual = malloc(sizeof(t_posicion));
	volverAlInicio();
	posicionActual->remitente = nombre;

	//---------------------------------- FIN: Obtención de datos del personaje -----------------------------//

	//----------------------------------Declaración del handler de señales----------------------------------//

	signal(SIGTERM,handlerSenales);
	signal(SIGUSR1,handlerSenales);
	//----------------------------------FIN: Declaración de los handlers de las señales-------------------------//

	//------------Conexión del personaje al orquestador para recibir ip y puerto del planificador y el nivel------------//

	int pid =  process_getpid();

	while (!terminoPlan)
	{
		nivelActual = planDeNiveles[cantidadNivelesTerminados];
		log_info(logger,"Nivel actual: %s \n", nivelActual);
		terminoElNivel = 0;
		loMataron = 0;

		int cantidadRecursos = contarRecursos(config, nivelActual);

		if (!(socketOrquestador = conectar_socket(ipOrquestador,puertoOrquestador))) //Me conecto al orquestador. Si hubo un error de conexion, sale.
		{
			liberarVector(planDeNiveles);
			liberarVector(objetivosNivel);
			free(planDeNiveles);
			free(objetivosNivel);
			log_error(logger, "Error al conectar el personaje al orquestador.");
			log_destroy(logger);
			config_destroy(config);
			free(ipOrquestador);
			free(nombre);
			free(posicionActual);

			return 1;
		}
		else //Si se pudo conectar, le pide la direccion del planificador del nivel que tiene que hacer.
		{
			log_info(logger, "%s Se conecta al orquestador.", nombre); //y me lo loguea

			sleep(1);
			log_info(logger,"%s le pide la dirección de %s al orquestador en %s:%d",nombre, nivelActual, ipOrquestador, puertoOrquestador);
			enviar_pedido_direccionNivPlan(socketOrquestador, nivelActual); //Le pido al orquestador la direccion del nivel y planificador

			t_direccion *direccionPlanificadorYNivel = recibir_direccionNivPlan(socketOrquestador); //Recibo la respuesta del orquestador

			if(direccionPlanificadorYNivel->puertoNiv ==-1)
			{
				log_error(logger, "El %s no está conectado todavía.", nivelActual);
				free(ipOrquestador);
				free(direccionPlanificadorYNivel->ipNiv);
				free(direccionPlanificadorYNivel->ipPlan);
				free(direccionPlanificadorYNivel);
				liberarVector(objetivosNivel);
				liberarVector(planDeNiveles);
				free(objetivosNivel);
				free(planDeNiveles);
				log_destroy(logger);
				config_destroy(config);
				free(nombre);
				free(posicionActual);
				return 1;
			}
			log_info(logger,"Direccion planificador %s %s %d \n",nivelActual,direccionPlanificadorYNivel->ipPlan,direccionPlanificadorYNivel->puertoPlan);
			log_info(logger,"Direccion %s %s %d \n",nivelActual, direccionPlanificadorYNivel->ipNiv,	direccionPlanificadorYNivel->puertoNiv);

		if( ! (socketPlanificador = conectar_socket(direccionPlanificadorYNivel->ipPlan, direccionPlanificadorYNivel->puertoPlan))) //Me conecto al planificador, si hay error, loguea y sale
		{
			 log_error(logger, "Error al conectar el personaje al planificador de nivel", 0);
			 log_destroy(logger);
			 liberarVector(planDeNiveles);
			 config_destroy(config);
			 liberarVector(objetivosNivel);
			 free(ipOrquestador);
			 free(planDeNiveles);
			 free(objetivosNivel);
			 free(direccionPlanificadorYNivel->ipNiv);
			 free(direccionPlanificadorYNivel->ipPlan);
			 free(direccionPlanificadorYNivel);
			 free(nombre);
			 free(posicionActual);
			 return 1;
		}
		else //Si se puede conectar al planificador
		{
			 log_info(logger, "El personaje se conectó al planificador");
			 log_info(logger, "Conectando al nivel...");
			 enviar_nombreYPID(socketPlanificador, nombre,pid);

			if (!(socketNivel = conectar_socket(direccionPlanificadorYNivel->ipNiv,	direccionPlanificadorYNivel->puertoNiv))) //Se conecta al nivel
			{
				log_error(logger, "Error al conectar el personaje al nivel",0);
				liberarVector(planDeNiveles);
				liberarVector(objetivosNivel);
				free(ipOrquestador);
				free(planDeNiveles);
				free(objetivosNivel);
				free(direccionPlanificadorYNivel->ipNiv);
				free(direccionPlanificadorYNivel->ipPlan);
				free(direccionPlanificadorYNivel);
				config_destroy(config);
				log_destroy(logger);
				free(nombre);
				free(posicionActual);
				return 1;
			}
			else
			{
				log_info(logger, "El personaje se conectó al nivel");
				free(direccionPlanificadorYNivel->ipNiv);
				free(direccionPlanificadorYNivel->ipPlan);
				free(direccionPlanificadorYNivel);
				enviar_nombreYPID(socketNivel, nombre,pid);

				sleep(1);
				while (!terminoElNivel) //Mientras no haya terminado el nivel
				{
muerteConVidas:
					estaListo = 1;
					recursoEnBusqueda = siguienteRecurso(cantidadNivelesTerminados,cantidadRecursosConseguidos, config); //Calculo el recurso que tengo que buscar
					log_info(logger,"Envio pedido de la posición del recurso %c a %s",recursoEnBusqueda, nivelActual);
					enviar_pedidoPosicionRecurso(socketNivel, recursoEnBusqueda,nombre); //Le pido al nivel que me diga la posicion de ese recurso
					posicionRecurso = recibir_posicionRecurso(socketNivel); //Recibo su respuesta
					log_info(logger,"Respuesta del nivel \n Posicion recurso (x,y) = (%d,%d) en %s \n",posicionRecurso->x, posicionRecurso->y,nivelActual);
					log_info(logger,"Posicion actual %s (x,y) = (%d,%d) \n",nombre,posicionActual->x, posicionActual->y);
					while ((posicionActual->x != posicionRecurso->x) || (posicionActual->y != posicionRecurso->y)) //Mientras no haya llegado al recurso
					{

						loMataron = esperar_turno(socketPlanificador);
						if(loMataron)
						{
							if(vidas < 1)
							{
								goto muerte;
							}
							else
							{
								goto muerteConVidas;
							}
						}

						avanzarHaciaElRecurso(posicionActual,posicionRecurso); //Me voy moviendo de a uno para llegar, cuando el planificador me lo permita.
						log_info(logger,"Posicion actual %s (x,y) = (%d,%d) en %s \n",nombre, posicionActual->x, posicionActual->y, nivelActual);
						enviar_posicionActual(socketNivel, posicionActual); //Le digo al nivel que me mueva

						if((posicionActual->x != posicionRecurso->x) || (posicionActual->y != posicionRecurso->y))
						 {
							log_info(logger,"Se envió informe de finalización de turno al planificador.");
						 	  enviar_finalizacionTurno(socketPlanificador,0,recursoEnBusqueda);
						 }
						 //Le digo al planificador que termino mi turno y que no estoy bloqueado (no pedi recursos=> no me puedo bloquear)
						//Salgo del ciclo solo cuando llego a la posicion del recurso
					}
					//free(posicionRecurso->remitente);
					//free(posicionRecurso);
					sleep(1);
					log_info(logger,"Envio pedido de recurso %c\n",recursoEnBusqueda);
					enviar_pedidoRecurso(socketNivel, recursoEnBusqueda, nombre);

					t_boolean *respuesta;

					printf("Esperando respuesta del nivel...\n");
					respuesta = recibir_respuestaPedidoRecurso(socketNivel); //Me dice si quedé bloqueado o no

					if(respuesta->boolean)
					{
						log_info(logger,"Conseguí el recurso: %c! \n",recursoEnBusqueda);
						cantidadRecursosConseguidos++;
						if (cantidadRecursosConseguidos == cantidadRecursos)
						{
							terminoElNivel = 1;
							cantidadRecursosConseguidos = 0;
							volverAlInicio();
							if(cantidadNivelesTerminados == cantidadNiveles-1)
							{
								estadoParaPlanificador = TERMINE_EL_PLAN;
							}
							else
							{

								estadoParaPlanificador = TERMINE_EL_NIVEL;
							}
							log_info(logger,"Se envió informe de finalización de turno al planificador.");
							enviar_finalizacionTurno(socketPlanificador, estadoParaPlanificador, recursoEnBusqueda);
							recibir_mensaje(socketPlanificador);

						}
						else
						{
							log_info(logger,"Se envió informe de finalización de turno al planificador.");
							enviar_finalizacionTurno(socketPlanificador,CONSEGUI_EL_RECURSO, recursoEnBusqueda);
						}
					}
					else
					{
						estaListo = 0;
						log_info(logger,"%s quedó bloqueado por %c\n",nombre,recursoEnBusqueda);
						log_info(logger,"Se envió informe de finalización de turno al planificador.");
						enviar_finalizacionTurno(socketPlanificador,!(respuesta->boolean),recursoEnBusqueda);
						loMataron = esperar_turno(socketPlanificador);
						if(loMataron)
						{
							fueMatadoPorSenal = 0;
							loMataron=0;
							if(vidas<1)
							{
								goto muerte;
							}
							else
							{
								goto muerteConVidas;
							}
						}

						cantidadRecursosConseguidos++;
						estabaBloqueado = 1;
						int estadoParaOrquestador;

						log_info(logger,"Se consiguió %c, recurso por el cual había quedado bloqueado.",recursoEnBusqueda);
						if (cantidadRecursosConseguidos == cantidadRecursos)
						{
							terminoElNivel = 1;
							cantidadRecursosConseguidos = 0;
							volverAlInicio();
							if(cantidadNivelesTerminados == cantidadNiveles-1)
							{
								terminoPlan = 1;
								estadoParaOrquestador = TERMINE_EL_PLAN;

							}
							else
							{

								estadoParaOrquestador = TERMINE_EL_NIVEL;
							}
							log_info(logger,"Se envió informe de finalización de turno al planificador.");
							enviar_finalizacionTurno(socketPlanificador, estadoParaOrquestador, recursoEnBusqueda);
							goto termineNivel;
						}
						else
						{
							estadoParaOrquestador=0;
							enviar_finalizacionTurno(socketPlanificador, estadoParaOrquestador, recursoEnBusqueda);
						}
					}


					free(respuesta);
				}
termineNivel:

				log_info(logger,"--------------------Terminé el nivel------------------------- \n");

				char* nombreycodigo;
				nombreycodigo = string_from_format("%s:%d",nombre,0);

				log_info(logger,"%s envió la devolución de recursos al nivel.",nombre);

				enviar_devolucionDeRecursos(socketNivel, nombreycodigo);


				cantidadNivelesTerminados++;
				log_info(logger,"Hice %d nivel(es) \n", cantidadNivelesTerminados);

				if (cantidadNivelesTerminados == cantidadNiveles)
				{
					terminoPlan = 1;
				}

				free(nombreycodigo);
				cerrar_socket(socketPlanificador);
				cerrar_socket(socketNivel);
			}
		 }
	   }

	}
	log_info(logger,	"------------------------Terminé todo el plan de niveles----------------------- \n");


	liberarVector(planDeNiveles);
	liberarVector(objetivosNivel);
	free(planDeNiveles);
	free(objetivosNivel);
	free(nombre);
	free(posicionActual);
	config_destroy(config);
	log_destroy(logger);
	free(ipOrquestador);


	return 0;

}




int morir(int socketNivel, int socketPlanificador)
{
	char* nombreycodigo;
	static const int ME_VOY_DEL_NIVEL = 0;
	static const int ME_QUEDO_EN_EL_NIVEL = 1;
	static const int ME_MATARON_Y_TENGO_VIDAS = 5;
	static const int ME_MATARON_Y_NO_TENGO_VIDAS = 6;

	terminoElNivel = 0, consiguioElRecurso = 0, terminoPlan = 0, estabaBloqueado=0;

	volverAlInicio();
	cantidadRecursosConseguidos=0;
	vidas--;
	log_info(logger,"Me morí. Me quedan %d vidas\n\n\n\n", vidas);

	//---------------Muerte por interbloqueo o por señal cuando está bloqueado-----------------------------------------

	if(!estaListo)
	{
		printf("El personaje murió estando bloqueado. \n");
		nombreycodigo = string_from_format("%s:%d",nombre,ME_QUEDO_EN_EL_NIVEL);

		if(vidas < 1)
		{
			nombreycodigo = string_from_format("%s:%d",nombre,ME_VOY_DEL_NIVEL);
			cantidadNivelesTerminados = 0;
			enviar_devolucionDeRecursos(socketNivel,nombreycodigo); //Me desconecto del nivel actual
			log_info(logger,"%s envió la devolución de recursos al nivel.",nombre);

			enviar_finalizacionTurno(socketPlanificador,ME_VOY_DEL_NIVEL,'W'); //Le digo al orquestador que me voy del nivel para que me saque de las colas
			log_info(logger,"%s envió la notificacion de muerte sin vidas al orquestador.",nombre);


			close(socketNivel);
			close(socketPlanificador);


			return 0;
		}


		enviar_devolucionDeRecursos(socketNivel,nombreycodigo);
		log_info(logger,"%s envió la devolución de recursos al nivel.",nombre);
		enviar_finalizacionTurno(socketPlanificador,ME_QUEDO_EN_EL_NIVEL,recursoEnBusqueda);
		log_info(logger,"%s envió la la notificación de muerte con vidas al orquestador.",nombre);


		return 1;
	}

	//---------------Muerte por señal cuando está listo o ejecutando--------------------------------------------
	else
	{
		if(fueMatadoPorSenal)
		{
			printf("El personaje murió estando listo. \n");

			fueMatadoPorSenal = 0;
			if(vidas < 1)
			{

				cantidadNivelesTerminados = 0;
				enviar_finalizacionTurno(socketPlanificador,ME_MATARON_Y_NO_TENGO_VIDAS,'W');
				log_info(logger,"%s envió la notificacion de muerte sin vidas al planificador.",nombre);

				char *stream = recibir_mensaje(socketPlanificador);
				int codigo;
				memcpy(&codigo, stream, sizeof(int));
				while(codigo != 8)
				{
					printf("codigo %d \n",codigo);
					stream = recibir_mensaje(socketPlanificador);
					memcpy(&codigo, stream, sizeof(int));

				}
				t_personaje *r=deserializer(stream);
				printf("%s fue sacado de la cola de listos del planificador", r->remitente);
				nombreycodigo = string_from_format("%s:%d",nombre,ME_VOY_DEL_NIVEL);
				enviar_devolucionDeRecursos(socketNivel,nombreycodigo);
				log_info(logger,"%s envió la devolución de recursos al nivel.",nombre);

				close(socketNivel);
				close(socketPlanificador);
				log_info(logger,"Se cerró la conexión al nivel y al planificador");

				return 0;
			}

			nombreycodigo = string_from_format("%s:%d",nombre,ME_QUEDO_EN_EL_NIVEL);
			enviar_devolucionDeRecursos(socketNivel,nombreycodigo); //Me desconecto del nivel actual
			enviar_finalizacionTurno(socketPlanificador,ME_MATARON_Y_TENGO_VIDAS,recursoEnBusqueda);
			log_info(logger,"%s envió la notificacion de muerte con vidas al planificador.",nombre);


			return 1;
		}
	}
	log_error(logger,"El personaje murió, pero no por señal ni por interbloqueo \n");
	exit(1);
}


int contarNiveles(t_config* config)
{
	char* niveles = config_get_string_value(config, "planDeNiveles");
	return contarItems(niveles);
}

int contarRecursos(t_config* config, char* nivelActual)
{
	char* objetivosNivel;
	char* clave = string_from_format("obj[%s]", nivelActual);
	objetivosNivel = config_get_string_value(config, clave);
	free(clave);
	return contarItems(objetivosNivel);
}

int contarItems(char* lista)
{

	int i, cantidad=0;
	for(i=0; lista[i] != '\0';i++)
	{
		if(lista[i] == ',')
		{
			cantidad++;
		}
	}
	return cantidad+1;
}
