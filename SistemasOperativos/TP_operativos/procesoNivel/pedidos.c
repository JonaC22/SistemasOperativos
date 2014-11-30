/*
 * pedidos.c
 *
 *  Created on: 17/05/2013
 *      Author: carolina
 */

#include "pedidos.h"
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <src/commons/collections/list.h>
#include <src/commons/string.h>
#include <sockets.h>
#include <stdbool.h>
#include <ctype.h>
#include <serializacion.h>
#include <pthread.h>
#include <src/commons/log.h>
#include <stdlib.h>


extern int cantCajas;
extern t_log* logger;

extern int socketOrquestador;

typedef struct {
	char recurso;
	char* nombre;
	int  cantidadDisponible;
	int  cantidadEnUso;
	int posX;
	int posY;
	t_list *quienesTienenEsteRecurso;

} t_dispoRecurso;


//-----PA DIBUAR----
#include "tad_items.h"
#include <curses.h>

extern ITEM_NIVEL* ListaItems;

extern t_dispoRecurso recursosEnNivel[3055];

//Peticiones personajes en nivel
extern t_list *personajesPeticiones;

extern t_list *personajesEnNivel;

extern t_recursosLiberados *recursos;

extern char* nivel;

void enviar_direccionNivel(int socket,char* ipNivel,int puertoNivel,char *nivel)
{
	t_direccionNombre *direccionNivel = malloc (sizeof(t_direccionNombre));

	direccionNivel->remitente= nivel;
	direccionNivel->ip = ipNivel;
	direccionNivel->puerto= puertoNivel;

	//	direccionNivel->remitente[strlen(direccionNivel->remitente)-1]='\0';

	//elementos basicos
	char *stream_darDireccionNivel;
	void* puntero;

	puntero =  direccionNivel;
	stream_darDireccionNivel = serializer(7, puntero);
	enviar_mensaje(socket,stream_darDireccionNivel);
	free(stream_darDireccionNivel);
	free(direccionNivel);
}

void enviar_posicionDeRecurso(int socketPersonaje, char recursoPedido)
{
	t_posicion *posRecursoPedido;
	char *stream_posicion;
	void *puntero;

	posRecursoPedido = buscar_posicion_recurso(recursoPedido);

	puntero = posRecursoPedido;

	stream_posicion = serializer(0, puntero);

	enviar_mensaje(socketPersonaje, stream_posicion);

	free(stream_posicion);
	free(puntero);
}

int atender_pedidos(int cliente, char* stream_mensaje)
{

	void* puntero;
	int codigo;
	memcpy(&codigo, stream_mensaje, sizeof(int));

	//posicion recurso
	t_posicion *recurso2;

	//pedir posicion recurso
	t_pedirPosicionRecurso *pedirR2;

	//nombre personaje a sacar recursos
	t_devolverRecursos *devolverRecurso;

	//nombre y pid del personaje recién conectado
	t_personaje *personajeNuevo;

	//pedir asignacion de recurso
	t_pedirRecurso *pedirRecurso;

	t_recursosLiberados *recursosLiberados;

	//inicializaciones y reserva de memoria



	if(codigo == FINAL)
	{
		nivel_gui_terminar();
		log_info(logger,"todos los personajes terminaron su plan de niveles, se procede a finalizar el nivel\n",1);
		finalizarNivel();
		pthread_exit(0);
	}

	puntero =  deserializer(stream_mensaje);

	switch (codigo) {

	case 0:

		recurso2 = puntero;

		mover_personaje(cliente, recurso2);

		free(recurso2->remitente);
		free(recurso2);

		break;

	case 2:

		pedirR2 =  puntero;

		enviar_posicionDeRecurso(cliente, pedirR2->recurso);

		free(pedirR2->remitente);
		free(pedirR2);

		break;

	case 6:

		pedirRecurso =  puntero;

		enviar_asignacionDerecurso(cliente, pedirRecurso->recurso,pedirRecurso->remitente);

		free(pedirRecurso);

		break;

	case 5:
		devolverRecurso =  puntero;

		devolver_recursos(devolverRecurso->personaje);

		free(devolverRecurso->personaje);
		free(devolverRecurso);

		break;

	case 8:

		personajeNuevo = puntero;

		agregar_personaje(personajeNuevo->remitente, cliente);

		free(personajeNuevo->remitente);
		free(personajeNuevo);

		break;

	case 11:

		recursosLiberados = puntero;

		log_info(logger, "se llego hasta aca, la estructura contiene %s y %d\n",
				recursosLiberados->recursos, recursosLiberados->total, 0);
		break;

	default:
		break;

	}

	loguearEstadoDeNivel();


	return codigo;
}



t_posicion *buscar_posicion_recurso(char recurso){
	int i=0;

	t_posicion *posRecursoBuscado=malloc(sizeof(t_posicion));;
	extern char*  nivel;

	for(i=0;i<=cantCajas;i++){
		if (recursosEnNivel[i].recurso==toupper(recurso) ) {
			posRecursoBuscado->x=recursosEnNivel[i].posX;
			posRecursoBuscado->y=recursosEnNivel[i].posY;
			posRecursoBuscado->remitente=strdup(nivel);
		}
	}

	return posRecursoBuscado;
}

void enviar_asignacionDerecurso(int cliente,char recurso,char *remitente ){

	int i=0;
	char *stream_asignarRecurso;
	void* puntero;

	log_info(logger,"<SOLICITUD>el personaje:%s solicito el recurso:%c",remitente,recurso);

	t_boolean *puedoONoAsignar = malloc (sizeof(t_boolean));
	puedoONoAsignar->boolean = 0;

	for(i=0;i<=cantCajas;i++){

		if ((recursosEnNivel[i].recurso==toupper(recurso) ) && (recursosEnNivel[i].cantidadDisponible>0) ) {

			//elementos basicos

			puedoONoAsignar->boolean=1;

			//Opero mi estructura de control

			recursosEnNivel[i].cantidadDisponible--;
			recursosEnNivel[i].cantidadEnUso++;
			restarRecurso(ListaItems,recursosEnNivel[i].recurso );
			nivel_gui_dibujar(ListaItems);

			if (list_is_empty(recursosEnNivel[i].quienesTienenEsteRecurso))
			{
				list_add_in_index(recursosEnNivel[i].quienesTienenEsteRecurso,0,remitente);
			}
			else{
				int tamanioLista;
				tamanioLista = list_size(recursosEnNivel[i].quienesTienenEsteRecurso);
				list_add_in_index(recursosEnNivel[i].quienesTienenEsteRecurso,tamanioLista,remitente);

			}

		}
	}
	//mi forma de saber si se lo dio o no
	if((puedoONoAsignar->boolean)!=1){
		//COMO NO SE LO PUEDO DAR AGREGO O ACTUALIZO PETICIONES
		organizarListaPeticiones(recurso,remitente);
	}else{
		log_info(logger,"<ASIGNACION>Se le asigno al personaje:%s el recurso:%c",remitente,recurso);
	}
	puntero = puedoONoAsignar;


	stream_asignarRecurso = serializer(4, puntero);

	enviar_mensaje(cliente,stream_asignarRecurso);
	free(puntero);
	free(stream_asignarRecurso);
}

void organizarListaPeticiones(char recurso,char *remitente){

	//agrega de a uno, puede haber el mismo personaje pidiendo el mismo recurso mas de una vez

	t_pedirPosicionRecurso *p1=malloc(sizeof(t_pedirPosicionRecurso));

	p1->recurso=recurso;
	p1->remitente=strdup(remitente);

	log_info(logger,"<BLOQUEADO>el personaje:%s quedo bloqueado al pedir recurso:%c",remitente,recurso);

	//si la lista esta vacia lo agrega primero, si no en la posicion siguiente
	if( (list_size(personajesPeticiones) )==0){
		list_add_in_index(personajesPeticiones,0,p1);
	}else{
		int tLista;
		tLista = list_size(personajesPeticiones);
		list_add_in_index(personajesPeticiones,tLista,p1);

	}

}

void mover_personaje(int socket,t_posicion *posAMover)
{
	char simbolo;
	memcpy(&simbolo,posAMover->remitente, sizeof(char));

	MoverPersonaje(ListaItems,simbolo, posAMover->x, posAMover->y);
	nivel_gui_dibujar(ListaItems);

}


void devolver_recursos(char* alguien)
{
	char *nombre=obtenerNombre(alguien);

	log_info(logger,"%s",nombre);

	sacarPersonajeDePeticiones(nombre);

	borrarPersonajeDelMapa(nombre);

	devuelveLosRecursos(nombre);

	if ( string_ends_with(alguien,"0")  ){

		sacarPersonajeDelNivel(nombre);

	}
	else {

		dibujarEnLaPosInicial(nombre);
		volverAlFondoDeLaLista(nombre);
	}

}

void volverAlFondoDeLaLista(char *nombre){

	log_info(logger,"<REINGRESO>el personaje va al fondo de la lista de conectados");

	t_personaje *otro= malloc(sizeof(t_personaje));

	bool _es_de_mismo_nombre(t_personaje* personaje)
	{
		return string_equals_ignore_case(personaje->remitente,nombre);
	}

	otro=list_find(personajesEnNivel,(void*)_es_de_mismo_nombre);

	sacarPersonajeDelNivel(nombre);

	list_add(personajesEnNivel,otro);


}

void sacarPersonajeDelNivel(char* nombre){


	bool es_el_personaje(t_personaje* personaje)
	{
		return string_equals_ignore_case(personaje->remitente,nombre);
	}

	while(list_find(personajesEnNivel,(void*) es_el_personaje)) //Mientras el personaje tenga recursos asignados
	{
		list_remove_by_condition(personajesEnNivel,(void*) es_el_personaje); //Se le saca la asignación
		log_info(logger,"<SALIDA>el personaje:%s salio del nivel",nombre);

	}


}

void sacarPersonajeDePeticiones(char *nombre){

	int tiene_algun_recurso(t_pedirPosicionRecurso* asignacion)
	{
		return string_equals_ignore_case(asignacion->remitente,nombre);
	}

	while(list_find(personajesPeticiones,(void*) tiene_algun_recurso)) //Mientras el personaje tenga recursos asignados
	{
		list_remove_by_condition(personajesPeticiones,(void*) tiene_algun_recurso); //Se le saca la asignación
	}
   log_info(logger,"del personaje %s no quedan mas peticiones",nombre);
}

void borrarPersonajeDelMapa(char*nombre){
	char simbolo;

	memcpy(&simbolo,nombre, sizeof(char));
	BorrarItem(&ListaItems, simbolo);
}

char * obtenerNombre(char*alguien){

	char **nombre =string_split(alguien,":");

	return (*nombre);

}

void dibujarEnLaPosInicial(char *nombre){

	char simbolo;

	memcpy(&simbolo,nombre, sizeof(char));
	CrearPersonaje(&ListaItems, simbolo, 1, 1);
	nivel_gui_dibujar(ListaItems);
}

int agregados;

void agregarRecursosAlInformeDeLiberados(char recursoLiberado, int cantLiberada, int cantTiposRecursosALiberar){


	if(agregados == cantTiposRecursosALiberar-1 || cantTiposRecursosALiberar ==1)
	{
		recursos->recursos=string_from_format("%s%c%d",recursos->recursos,recursoLiberado,cantLiberada);
	}
	else
	{
		recursos->recursos=string_from_format("%s%c%d:",recursos->recursos,recursoLiberado,cantLiberada);
	}


	agregados++;

}

int tiposDeRecursosALiberar(char *unPersonaje){

	int i;
	int cantTiposRecurso=0;
	bool _es_de_mismo_nombre(char* nombre)
	{
		return string_equals_ignore_case(nombre, unPersonaje);
	}

	for(i=0;i<=cantCajas;i++)
	{
		if (list_find(recursosEnNivel[i].quienesTienenEsteRecurso, (void*) _es_de_mismo_nombre) != NULL ){
			cantTiposRecurso++;
		}
	}
	return cantTiposRecurso;
}

void devuelveLosRecursos(char *alguien)
{
	agregados = 0;
	if(strcmp(alguien,"")==0)
	{
		log_info(logger,"se recibio NULL como personaje para devolucion de recursos");
		return;
	}
	log_info(logger,"<DEVOLUCION>devolvemos los recursos que tenia:%s",alguien);

	recursos = malloc(sizeof(t_recursosLiberados));

	int i;
	int cantidadDisponible;
	int cantidadLiberada;
	int cantidadInicial;
	int cantTiposRecursosALiberar;

	cantTiposRecursosALiberar=0;
	char *stream;
	void* puntero;

	bool _es_de_mismo_nombre(char* nombre)
	{
		return string_equals_ignore_case(nombre, alguien);
	}

	cantTiposRecursosALiberar = tiposDeRecursosALiberar(alguien);
	recursos->total=cantTiposRecursosALiberar;

	recursos->recursos="";

	recursos->nivel =strdup(nivel);

	recursos->personaje=strdup(alguien);

	//Iniciializo la estructura. Solo estos tres campos son comunes.
	if(cantTiposRecursosALiberar>=1)
	{
		recursos->total=cantTiposRecursosALiberar;

		recursos->recursos="";

		recursos->nivel =strdup(nivel);

		recursos->personaje=strdup(alguien);

		log_info(logger,"<RECURSO>los distintos tipos a devolver son:%d",cantTiposRecursosALiberar);

		for(i=0;i<=cantCajas;i++)
		{

			cantidadInicial = list_size(recursosEnNivel[i].quienesTienenEsteRecurso);
			while (list_find(recursosEnNivel[i].quienesTienenEsteRecurso, (void*) _es_de_mismo_nombre) != NULL )
			{
				list_remove_by_condition(recursosEnNivel[i].quienesTienenEsteRecurso, (void*) _es_de_mismo_nombre);

			}
			cantidadLiberada = list_size(recursosEnNivel[i].quienesTienenEsteRecurso);
			if(cantidadInicial>cantidadLiberada){

				cantidadDisponible=cantidadInicial-cantidadLiberada;

				log_info(logger,"devolvio del recurso:%c una cantidad:%d",recursosEnNivel[i].recurso,cantidadDisponible);

				recursosEnNivel[i].cantidadDisponible+=cantidadDisponible;
				recursosEnNivel[i].cantidadEnUso-=cantidadDisponible;

				desbloquearPersonajes(cantidadDisponible,recursosEnNivel[i].recurso,i);

				agregarRecursosAlInformeDeLiberados(recursosEnNivel[i].recurso,cantidadDisponible,cantTiposRecursosALiberar);

			}
			BorrarItem(&ListaItems,recursosEnNivel[i].recurso);
			CrearCaja(&ListaItems, recursosEnNivel[i].recurso, recursosEnNivel[i].posX, recursosEnNivel[i].posY, recursosEnNivel[i].cantidadDisponible);
			nivel_gui_dibujar(ListaItems);

		}

		log_info(logger,"<RECURSO>se libero: %s",recursos->recursos);

		puntero = recursos;

		stream = serializer(11,puntero);

		log_info(logger,"se envia mensaje con los recursos que libera un personaje al orquestador\n");

		enviar_mensaje(socketOrquestador,stream);

	}
	else
	{

		stream = serializer(11,(void*)recursos);
		enviar_mensaje(socketOrquestador,stream);
		log_info(logger,"el personaje no tenia ningun recurso asignado y se le avisa al orquestador.");
	}


}

void desbloquearPersonajes(int cantDisponible,char recurso, int posRecurso){


	bool quiere_recurso(t_pedirPosicionRecurso*alguien){
		if(alguien->recurso==recurso){
			return true;
		}
		return false;
	}

	while((cantDisponible!=0)&&(list_find(personajesPeticiones, (void*) quiere_recurso) != NULL  ) )
	{
		t_pedirPosicionRecurso *liberado=malloc(sizeof(t_pedirPosicionRecurso));
		liberado=list_find(personajesPeticiones, (void*) quiere_recurso);
		list_add(recursosEnNivel[posRecurso].quienesTienenEsteRecurso,liberado->remitente);
		list_remove_by_condition(personajesPeticiones, (void*) quiere_recurso);

		recursosEnNivel[posRecurso].cantidadDisponible--;
		recursosEnNivel[posRecurso].cantidadEnUso++;

		cantDisponible--;

		log_info(logger,"<DESBLOQUEADO>el personaje:%s dejo de estar bloqueado por recurso:%c",liberado->remitente,liberado->recurso);

		free(liberado);
	}


	nivel_gui_dibujar(ListaItems);

}



void agregar_personaje(char * remitente, int socket){

	log_info(logger,"<NUEVO>Llego al nivel el personaje:%s,con socket:%d",remitente,socket);

	t_personaje *personajeNuevo=malloc(sizeof(t_personaje));
	personajeNuevo->socket=socket;
	personajeNuevo->remitente=strdup(remitente);

	dibujarEnLaPosInicial(remitente);

	if (list_is_empty(personajesEnNivel))
	{
		list_add_in_index(personajesEnNivel,0,personajeNuevo);
	}
	else{
		int tamanioLista;
		tamanioLista = list_size(personajesEnNivel);
		list_add_in_index(personajesEnNivel,tamanioLista,personajeNuevo);
	}

}
