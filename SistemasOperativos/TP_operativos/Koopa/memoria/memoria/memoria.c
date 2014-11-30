#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "memoria.h"
#include <malloc.h>
#include <stdbool.h>

//----------------------------------Estructuras Globales---------------------------------------//

typedef struct { //struct de los nodos de la lista de control
	char id;
	char * dirInicial;
	char * dirFinal;
	bool libre;} part;
t_list* listaPart; // lista de control
char* dirActual;
char* dir_final_seg; //dir del final del segmento
int tamanioSegmento, tamanioP;
t_link_element * dir_nodo;

//-----------------------------------Funciones Privadas---------------------------------------//

char * buscar_hueco(char *, int); //IMPORTANTE
int agregar_nodo(part*); //IMPORTANTE
char* traerParticionVacia(int); //IMPORTANTE
int get_index(char*);
int pos_particionExiste(char *);
t_link_element * siguiente(t_link_element*); //Corta
void * element_destroyer(void*); //Corta
bool libre_con_tamanio(part*); //Corta

//------------------------------------Funciones de lib----------------------------------------//

t_memoria crear_memoria(int tamanio) {
	t_memoria segmento = NULL;
	part* inicial = malloc(sizeof(part));

	segmento = malloc(tamanio);
	tamanioSegmento = tamanio;
	dirActual = segmento;
	dir_final_seg = segmento + tamanio;

	listaPart = list_create();
	//creo una particion inicial vacia
	inicial->id = '\0';
	inicial->dirInicial = segmento;
	inicial->dirFinal = segmento + tamanio;
	inicial->libre = 1;

	list_add(listaPart, inicial);

	return segmento;
}

// ----------------------------------------------------------------------------------------//

int almacenar_particion(t_memoria segmento, char id, int tamanio,
		char* contenido) {
	char * hueco;
	part *nuevaPart = malloc(sizeof(part));
	part *partValid;
	int noEsta, error;

	tamanioP = tamanio;
	partValid = list_find(listaPart, (void*) libre_con_tamanio);

	//Valida que el tamanio de la particion sea menor al tamanio del segmento
	//y que haya algun hueco donde entre
	if (tamanio <= tamanioSegmento){
		if(partValid != NULL ) {
		//busca el id en la lista, si no esta devuelve -1 que es lo que quiero validar
		noEsta = get_index(&id);
		// dentro de ésta función está implementado el algoritmo next fit
		hueco = buscar_hueco(segmento, tamanio);

		// valida que no este el id duplicado
		if (noEsta == -1) {

			// guarda en el agujero el contenido de la partición
			memcpy(hueco, contenido, tamanio);

			nuevaPart->id = id;
			nuevaPart->dirInicial = hueco;
			nuevaPart->dirFinal = hueco + tamanio;
			nuevaPart->libre = 0;

			agregar_nodo(nuevaPart);
			error=1;
		}
		else {
			printf("Ya existe una particion con el Id: %c!\n", id);
			error=-1;
		}
	}
		else {
			printf("No hay particion libre donde almacenar!\n");
			error=0;
		}
	}
	else {
		printf("No se puede almacenar la particion %c!\n",id);
		error = -1;
	}

	return error;
}

// -----------------------------------------------------------------------------------//

//buscar con el id el nodo de la particion en la listaPart y ponerle al ocupado 1
int eliminar_particion(t_memoria segmento, char id) {
	part * nodo;

	int pos;

	//traigo la posicion de la particion que tiene como id la pasada por parametro
	pos = get_index(&id);
	//con la posicion traigo la data de la particion
	if (pos != -1) {

		nodo = list_get(listaPart, pos);

		// la marco como libre
		nodo->libre = 1;
		nodo->id= '\0';
	}
	else {
		printf("No se puede eliminar Id: %c. No existe particion.\n",id);
	}
	return 0;

}

// -----------------------------------------------------------------------------------

void liberar_memoria(t_memoria segmento) {

	free(segmento);
	list_destroy_and_destroy_elements(listaPart, (void*) element_destroyer);

}

// -----------------------------------------------------------------------------------

t_list* particiones(t_memoria segmento) {
	t_list* list = list_create();
	t_link_element* p = listaPart->head;
	part * nodo_listaPart;
	t_memoria seg = segmento;

	while (p != NULL ) {
		nodo_listaPart = p->data;
		t_particion *parti_a_guardar= malloc(sizeof(t_particion));

		parti_a_guardar->id = nodo_listaPart->id;
		parti_a_guardar->dato = nodo_listaPart->dirInicial;
		parti_a_guardar->inicio = (nodo_listaPart->dirInicial - seg);
		parti_a_guardar->libre = nodo_listaPart->libre;
		parti_a_guardar->tamanio = nodo_listaPart->dirFinal	- nodo_listaPart->dirInicial;

		list_add(list, parti_a_guardar);
		element_destroyer((void*)nodo_listaPart);
		p = p->next;

	}
	return list;
}

// -----------------------Definicion de Funciones Privadas-------------------------------------//

char* buscar_hueco(char * seg, int tamanioC) {
	int list_length;
	char * hueco;

	list_length = list_size(listaPart);

	if (list_length == 1) { // si en la lista de control esta solo la part inicial, devuelvo el primer lugar del segmento
		hueco=seg;
	}

	else {// trae la primera particion vacia en la cual puede entrar el contenido
		hueco = traerParticionVacia(tamanioC);

	}

	return hueco;
}

// -----------------------------------------------------------------------------------

//get index: devuelve el la pos de un nodo que tiene como id la i pasada por parametro


int get_index(char* i) {
	t_link_element *element = listaPart->head;
	int cont = 0;
	int index = -1;
	part* n;
	char* h;
	int bandera = 1;

	h = i;
	while ((element != NULL )&&(bandera!=0)){
	n=element->data;

	if (n->id==*h) {
		index=cont;
		bandera=0;
	}
	cont++;
	element = element->next;
}
	return index;
}

//agregar_nodo : justamente agrega un nodo a la lista de control

int agregar_nodo(part* particion) {
	char * dir_Inicial = particion->dirInicial;
	int pos, tamanioOld, tamanioP, list_length;
	part* vacio = malloc(sizeof(part));
	part* old;
	part* ini;

	tamanioP = particion->dirFinal - particion->dirInicial;
	pos = pos_particionExiste(dir_Inicial);
	list_length = list_size(listaPart);

	if (list_length == 1) { //si en la lista esta solo la particion inicial, lo pongo primero
		ini = listaPart->head->data;
		list_replace(listaPart, 0, particion);
		ini->dirInicial = particion->dirFinal;
		if ((ini->dirFinal - ini->dirInicial) > 0) { //si la particion inicial queda con memoria
			list_add(listaPart, ini);
		} else { //si queda vacia la borro
			free(ini);
		}

	}
	else {
		// si ya hay una particion creada vacia, con la misma dirInicial, la reemplazo
		if (pos > -1) {
			old = list_get(listaPart, pos);
			tamanioOld = old->dirFinal - old->dirInicial;

				list_replace(listaPart, pos, particion);

				//si sobro lugar entre las particiones, creo una particion vacia con lo que sobra y la inserto
				if (tamanioOld > tamanioP) {
					vacio->id = '\0';
					vacio->dirInicial = particion->dirFinal;
					vacio->dirFinal = old->dirFinal;
					vacio->libre = 1;

					list_add_in_index(listaPart, pos + 1, vacio);

				}
		free(old);
		}

	}

	return 0;
}

// traerParticionVacia: busca en la lista la primer particion en la cual pueda entrar el contenido de la particion
// implementado algoritmo next fit
// empieza a recorrer la lista desde el nodo cuya direccion inicial es inicio
char* traerParticionVacia(int tamanioC) {
	t_link_element * p;
	part* reg;
	char * dir_particion = NULL;
	bool lo_encontre = 0;
	int tamanio_en_nodo, dio_la_vuelta = 0;

	pos_particionExiste(dirActual);	// llamo para que me guarde en la variable global dir_nodo la dir de la particion con esa dir_Inicial
	p = dir_nodo;

	reg = p->data;
	tamanio_en_nodo = (reg->dirFinal) - (reg->dirInicial);
	//comparo el primer nodo
	if ((reg->libre == 1) && (tamanioC <= tamanio_en_nodo)) {
		dir_particion = reg->dirInicial;
		dirActual=dir_particion;
	}
	//sino recorro lista
	else {
		p = siguiente(p);
		reg = p->data;
		tamanio_en_nodo = (reg->dirFinal) - (reg->dirInicial);

		while ((dio_la_vuelta == 0) && (lo_encontre != 1)) {

			if (reg->dirInicial != dirActual) {
				if ((reg->libre == 1) && (tamanioC <= tamanio_en_nodo)) {
					dir_particion = reg->dirInicial;
					dirActual=dir_particion;
					lo_encontre = 1;

				}
			}
			else {
				dio_la_vuelta = 1;
			}
			p = siguiente(p);
			reg = p->data;
			tamanio_en_nodo = (reg->dirFinal) - (reg->dirInicial);
		}
	}

	return dir_particion;
}

//pos_particionExiste: busca una particion en la lista que tenga como dirInicial la pasada por parametro
// si no la encuentra devuelve -1


int pos_particionExiste(char *dir_Inicial) {
	t_link_element * p = listaPart->head;
	part* reg;
	int pos = -1, cont = 0, bandera = -1;

	while ((p != NULL )&&(bandera!=0)){
	reg = p->data;
	if (reg->dirInicial==dir_Inicial) {
		bandera=0;
		pos=cont;
		dir_nodo=p;
	}
	cont++;
	p=p->next;
}
	return pos;
}


t_link_element * siguiente(t_link_element *p) {
	t_link_element* elemento;

	if (p->next == NULL ) {
		elemento = listaPart->head;
	} else {
		elemento = p->next;
	}

	return elemento;
}

void * element_destroyer(void* parti) {
	void* a=NULL;
	free(parti);
	return a;
}

bool libre_con_tamanio(part* p) {
	int tam = p->dirFinal - p->dirInicial;
	if ((p->libre == 1) && (tam >= tamanioP)) {
		return 1;
	} else {
		return 0;
	}
}

