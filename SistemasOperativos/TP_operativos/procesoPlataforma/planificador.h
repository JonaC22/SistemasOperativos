/*
 * planificador.h
 *
 *  Created on: May 23, 2013
 *      Author: juan
 */

#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_

#include "pedidosPlanificador.h"
#include <sockets.h>

t_socket_random* crear_socket_planificador(char* ip);
int aceptar_conexion_planificador(t_planificador* planificador);
t_planificador* lanzar_hilo_planificador(int socket, char* nombreNivel);
void finalizarNiveles();
void enviarMensajeDeFinalizacion(char*,void*);
void loguearColas(t_planificador*);
void loguearConectados(t_planificador*);

#endif /* PLANIFICADOR_H_ */
