/*
 * serializacion.h
 *
 *  Created on: May 13, 2013
 *      Author: juan
 */

#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_
#define FINAL 77
#define	BLOQUEA_TODO 66
#define BLOQUEO 55

#include <stdbool.h>
#include <src/commons/collections/list.h>

typedef struct {
	int x;
	int y;
	char* remitente;

}__attribute__ ((__packed__)) t_posicion;

typedef struct {
	int puertoPlan;
	int puertoNiv;
	char *ipPlan;
	char *ipNiv;

}__attribute__ ((__packed__)) t_direccion;

typedef struct {
	char recurso;
	char* remitente;

}__attribute__ ((__packed__)) t_pedirPosicionRecurso;

typedef struct {
	char *nombrenivel;

}__attribute__ ((__packed__)) t_pedirDireccion;

typedef struct {
	int boolean;

}__attribute__ ((__packed__)) t_boolean;

typedef struct {
	char *personaje;
}__attribute__ ((__packed__)) t_devolverRecursos;

typedef struct {
	char recurso;
	char* remitente;
}__attribute__ ((__packed__)) t_pedirRecurso;

typedef struct {
	int puerto;
	char *ip;
	char *remitente;

}__attribute__ ((__packed__)) t_direccionNombre;

typedef struct {
	int socket;
	char* remitente;
}__attribute__ ((__packed__)) t_personaje;

typedef struct {
	int estado;
	char recurso;
}__attribute__ ((__packed__)) t_estadoRecurso;

typedef struct {
	char * nivel;
	char * personajes;
}__attribute__ ((__packed__)) t_interbloqueo;


typedef struct{
	char* recursos;
	int total;
	char* nivel;
	char* personaje;
}__attribute__ ((__packed__)) t_recursosLiberados;

char *serializer(int, void*);
void *deserializer(char*);
void liberarVector(char**);

#endif /* SERIALIZACION_H_ */
