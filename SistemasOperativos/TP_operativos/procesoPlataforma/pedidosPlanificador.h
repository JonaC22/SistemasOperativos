/*
 * pedidosPlanificador.h
 *
 *  Created on: Jun 3, 2013
 *      Author: juan
 */

#ifndef PEDIDOSPLANIFICADOR_H_
#define PEDIDOSPLANIFICADOR_H_

#include <src/commons/collections/dictionary.h>
#include <src/commons/collections/queue.h>
#include <src/commons/log.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <serializacion.h>


typedef struct
{
	char* nombre;
	int socket;
	char recurso;
} t_personajeBloqueado;

typedef struct
{
	char* nombre;
	char recurso;
	int cantidad;
} t_asignacion;



typedef struct {

	t_queue *colaListos;
	t_list *bloqueados;
	pthread_mutex_t semaforo;
	sem_t contadorDeListos;
	sem_t contadorDeBloqueados;

} t_setColas;

typedef struct
{
	t_list *asignaciones;
	t_setColas *setColas;
	int socket;
	int quantumsRestantes;
	int socketPersonajeActual;
	t_log* logger;

} t_planificador;




extern t_dictionary *direccionesNiveles;
extern t_dictionary *id_setColas; //diccionario de relacion id hilo en ejecucion y su setColas


int atender_pedidos_planificador(int cliente, char* stream_mensaje);
t_estadoRecurso* recibir_turnoTerminado(int socketPersonaje);
void procesar_bloqueo(t_setColas *set, t_estadoRecurso *estado, t_personaje *personaje);
int enviar_permisoDeMovimiento(t_planificador **direccion_planificador);
void agregar_nuevoPersonaje(int socketNuevaConexion,t_planificador *planificador);
void planificar_turnos(t_planificador **direccion_planificador);
void asignar_recurso(t_personaje *personaje, char recurso, t_planificador **direccion_planificador);
void liberar_recursos(t_personaje *personaje, t_planificador **direccion_planificador);
t_personaje* recibir_nuevoPersonaje(int socketPersonaje);
void loguearConectados(t_planificador*);

#endif /* PEDIDOSPLANIFICADOR_H_ */
