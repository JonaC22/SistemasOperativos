/*
 * pedidos.h
 *
 *  Created on: May 16, 2013
 *      Author: juan
 */

#ifndef PEDIDOS_H_
#define PEDIDOS_H_

#include <src/commons/collections/dictionary.h>
#include <src/commons/collections/queue.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <unistd.h>
#include <src/commons/process.h>
#include <src/commons/collections/list.h>
#include "pedidosPlanificador.h"

t_dictionary *direccionesNiveles;
t_dictionary *id_setColas; //diccionario de relacion id hilo en ejecucion y su setColas

void matar_victima(char* victima);
void definirVictima(int cliente,t_interbloqueo *interbloqueo);
int atender_pedidos_orquestador(int cliente, char* stream_mensaje);
void responder_direccion(int socketNuevaConexion, char *nombrenivel);
void agregar_nivel(t_direccionNombre *direccionNivel);
void buscarDesbloqueadosPorRecurso(char recurso,int cantidad,t_list *desbloqueados,t_queue *cola);
void verSiPuedeAsignarRecursosABloqueados(int cliente,t_recursosLiberados *liberados);
t_recursosLiberados *recibir_recursosLiberados(int);
void liberar_personajes(t_recursosLiberados *recursos,int socketNivel);
int liberar_un_personaje(t_setColas *colas, char recurso);
void enviar_recursosSobrantes(int socketNivel,char* recursosSobrantes, int total);
int recibir_terminoElNivel(int);

#endif /* PEDIDOS_H_ */
