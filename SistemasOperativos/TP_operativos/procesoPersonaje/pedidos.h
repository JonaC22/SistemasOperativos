/*
 * estructuras.h
 *
 *  Created on: May 16, 2013
 *      Author: juan
 */


#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <sockets.h>
#include <src/commons/string.h>
#include <serializacion.h>

void enviar_pedido_direccionNivPlan(int socket, char *nivel);
t_direccion* recibir_direccionNivPlan(int socket);
void enviar_nombreYPID(int socket, char *nombre, int pid);
void enviar_pedidoPosicionRecurso(int socketNivel, char recurso, char* nombre);
t_posicion* recibir_posicionRecurso(int socket);
void enviar_posicionActual(int socketNivel, t_posicion *posicion);
void enviar_pedidoRecurso(int socketNivel, char recursoEnBusqueda, char* nombre);
t_boolean* recibir_respuestaPedidoRecurso(int socketNivel);
int esperar_turno(int socketPlanificador);
void enviar_finalizacionTurno(int socketPlanificador, int estadoDeBloqueo, char recurso);
void enviar_devolucionDeRecursos(int socketNivel, char *nombre);
int morir();

#endif /* ESTRUCTURAS_H_ */
