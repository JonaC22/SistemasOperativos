/*
 * pedidos.h
 *
 *  Created on: 17/05/2013
 *      Author: utnso
 */

#ifndef PEDIDOS_H_
#define PEDIDOS_H_
#include <src/commons/collections/list.h>
#include <serializacion.h>

void volverAlFondoDeLaLista(char *nombre);
char* nombrePersonaje(t_interbloqueo *victima);
void desbloquearPersonajes(int cantDisponible,char recurso, int posRecurso);

t_pedirPosicionRecurso recibir_pedidoRecurso(int socketPersonaje);
void enviar_posicionDeRecurso(int socketPersonaje, char recursoPedido);
int atender_pedidos(int cliente, char* stream_mensaje);
void enviar_direccionNivel(int socket,char* ipNivel,int puertoNivel,char *nivel);

void enviar_asignacionDerecurso(int cliente,char recurso,char *remitente );
t_posicion *buscar_posicion_recurso(char recurso);
void enviar_direccionNivel(int socket,char* ipNivel,int puertoNivel,char *nivel);
void agregar_personaje(char *nombre, int socket);

void mover_personaje(int ,t_posicion *);
void organizarListaPeticiones(char recurso,char *remitente);

void devolver_recursos(char* alguien);
void sacarPersonajeDelNivel(char* nombre);
void sacarPersonajeDePeticiones(char *nombre);
void borrarPersonajeDelMapa(char*nombre);
char * obtenerNombre(char*alguien);
void dibujarEnLaPosInicial(char *nombre);
void devuelveLosRecursos(char *alguien);
int tiposDeRecursosALiberar(char *unPersonaje);
void agregarRecursosAlInformeDeLiberados(char recursoLiberado, int cantLiberada, int cantTiposRecursosALiberar);
void finalizarNivel();
void loguearEstadoDeNivel();

#endif /* PEDIDOS_H_ */
