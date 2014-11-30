/*
 * estructuras.c
 *
 *  Created on: May 16, 2013
 *      Author: juan
 */
#include <malloc.h>
#include <string.h>
#include "pedidos.h"
#include "senales.h"
#include <stdlib.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/signalfd.h>

extern int socketNivel;
extern int fueMatadoPorSenal;
//--------Pedido y recepcion de las direcciones del nivel y planificador-------

void enviar_pedido_direccionNivPlan(int socket, char *nivel) {

	//pedir direccion nivel y planificador
	t_pedirDireccion *pedirDireccionPlanificadorYNivel = malloc(sizeof(t_pedirDireccion));

	pedirDireccionPlanificadorYNivel->nombrenivel = nivel;

	//elementos basicos
	char *stream_pedirDireccionPlanificador;
	void *puntero = pedirDireccionPlanificadorYNivel;

	stream_pedirDireccionPlanificador = serializer(3, puntero);

	free(puntero);

	enviar_mensaje(socket, stream_pedirDireccionPlanificador);


	free(stream_pedirDireccionPlanificador);

}

t_direccion* recibir_direccionNivPlan(int socketOrquestador)
{

	t_direccion* estructuraDireccion = deserializer(recibir_mensaje(socketOrquestador));

	cerrar_socket(socketOrquestador);
	return estructuraDireccion;
}

//--------FIN: Pedido y recepcion de las direcciones del nivel y planificador-------

//------------ Envio de PID y Nombre------------------------------------------------

void enviar_nombreYPID(int socket, char *nombre, int pid)
{
	t_personaje *nombreYPID = malloc(sizeof(t_personaje));

	char *stream_nombreYPID;

	nombreYPID->socket=pid;
	nombreYPID->remitente=nombre;

	stream_nombreYPID= serializer(8, nombreYPID);
	enviar_mensaje(socket, stream_nombreYPID);

	free(stream_nombreYPID);
	free(nombreYPID);


}

//------------ FIN: Envio de PID y Nombre-------------------------------------------

//----------- Pedido y recepcion de la posicion de un recurso-----------------------

void enviar_pedidoPosicionRecurso(int socketNivel, char recurso, char* nombre) {

	t_pedirPosicionRecurso *pedirPosicionRecurso = malloc(
			sizeof(t_pedirPosicionRecurso));
	pedirPosicionRecurso->recurso = recurso;
	pedirPosicionRecurso->remitente = nombre;

	void *puntero = pedirPosicionRecurso;

	char *stream_pedirPosicionRecurso;

	stream_pedirPosicionRecurso = serializer(2, puntero);

	free(puntero);

	enviar_mensaje(socketNivel, stream_pedirPosicionRecurso);
	free(stream_pedirPosicionRecurso);

}

t_posicion* recibir_posicionRecurso(int socket) {
	return deserializer(recibir_mensaje(socket));
}
//-----------FIN: Pedido y recepcion de la posicion de un recurso-----------------------
//----------- Envio de notificacion de movimiento---------------------------------------

void enviar_posicionActual(int socketNivel, t_posicion *posicion) {
	char *stream_posicionMovimiento;

	stream_posicionMovimiento = serializer(0, posicion);

	enviar_mensaje(socketNivel, stream_posicionMovimiento);

	free(stream_posicionMovimiento);

}
//-----------FIN: Envio de notificacion de movimiento---------------------------------------
//-----------Envio y respuesta de pedido de recurso-----------------------------------------

void enviar_pedidoRecurso(int socketNivel, char recursoEnBusqueda, char* nombre) {
	t_pedirPosicionRecurso *recurso = malloc(sizeof(t_pedirPosicionRecurso));
	recurso->remitente = nombre;
	recurso->recurso = recursoEnBusqueda;
	char *stream_recurso;
	void *puntero = recurso;

	stream_recurso =serializer(6, puntero);
	free(puntero);
	enviar_mensaje(socketNivel, stream_recurso);
	free(stream_recurso);

}

t_boolean* recibir_respuestaPedidoRecurso(int socketNivel)
{
	t_boolean *respuesta;

		char *stream_respuesta;
		void *puntero;

		stream_respuesta = (char *) recibir_mensaje(socketNivel);
		puntero = deserializer(stream_respuesta);

		respuesta = puntero;
	//	printf("Direccion planifasasicador %s %d",direccionPlanificadorYNivel->ipPlan, direccionPlanificadorYNivel->puertoPlan);

		return respuesta;
}
//-----------FIN: Enviar pedido de recursos---------------------------
//----------------Enviar devolución de recursos------------------------
void enviar_devolucionDeRecursos(int socketNivel, char *nombre)
{
	t_devolverRecursos *devolverRecursos = malloc(sizeof(t_devolverRecursos));
	devolverRecursos->personaje = nombre;
	void *puntero = devolverRecursos;
	char *stream_devolverRecursos;
	stream_devolverRecursos = serializer(5, puntero);

	enviar_mensaje(socketNivel, stream_devolverRecursos);

	free(puntero);
	free(stream_devolverRecursos);

}
//--------FIN: Enviar devolución de recursos-----------------
//-------Esperar notificación de movimiento permitido--------


int esperar_turno(int socketPlanificador)
{

	printf("Esperando turno...\n");


	t_boolean *mensajePlanificador;
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGUSR1);
	sigprocmask(SIG_BLOCK, &mask, NULL);
	int sigterm_fd = signalfd(-1, &mask, 0);

	fd_set master; // conjunto maestro de descriptores de fichero
	fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
	int fdmax; // número máximo de descriptores de fichero

	FD_ZERO(&master);	// borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);
	FD_SET(socketPlanificador, &master);
	FD_SET(sigterm_fd, &master);


	if(sigterm_fd > socketPlanificador)
	{
		fdmax = sigterm_fd;
	}
	else
	{
		fdmax = socketPlanificador;
	}

esperarTurno:

	read_fds = master;
	if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1)
	{
		perror("select");
		exit(1);
	}

	if(FD_ISSET(socketPlanificador, &read_fds))
	{
		char *stream_mensajePlanificador = recibir_mensaje(socketPlanificador);
		if(stream_mensajePlanificador != NULL)
		{
			mensajePlanificador = deserializer(stream_mensajePlanificador);

			if(mensajePlanificador->boolean)
			{
				printf("Me toca! \n");
				free(mensajePlanificador);
				return 0;
			}
			else
			{
				printf("\n\nEl personaje fue matado por Interbloqueo \n\n");

				fueMatadoPorSenal=0;
				morir(socketNivel, socketPlanificador);
				free(mensajePlanificador);
				return 1;
			}
		}
		else
		{
			printf("error en recibir turno \n");
			exit(1);
		}
		return 1;
	}
	else
	{
		if(FD_ISSET(sigterm_fd, &read_fds))
		{
		    struct signalfd_siginfo fdsi;
		    read(sigterm_fd, &fdsi, sizeof(struct signalfd_siginfo));

			if(fdsi.ssi_signo == SIGTERM || fdsi.ssi_signo == SIGUSR1)
			{
				handlerSenales(fdsi.ssi_signo);
				if(fdsi.ssi_signo == SIGTERM)
				{
					printf("\n\nEl personaje fue matado por SIGTERM \n\n");
					morir(socketNivel, socketPlanificador);
					//close(sigterm_fd);
					return 1;
				}
				goto esperarTurno;
			}
			else
			{
				printf("Señal no esperada.\n");
				exit(1);
			}
		}
		else
		{
			printf("El select salió por algo que no sé qué \n");
			exit(1);
			return 0;
		}
	}
	return 1;
}

void enviar_finalizacionTurno(int socketPlanificador, int estadoDeBloqueo, char recurso)
{

 	t_estadoRecurso *estadoRecurso = malloc(sizeof(t_estadoRecurso));
	estadoRecurso->recurso=recurso;
	estadoRecurso->estado=estadoDeBloqueo;

	char *stream_finalizacionDeTurno;
	void *puntero =estadoRecurso;

	stream_finalizacionDeTurno = serializer(9, puntero);

	free(puntero);
	enviar_mensaje(socketPlanificador,stream_finalizacionDeTurno);
	free(stream_finalizacionDeTurno);

}

