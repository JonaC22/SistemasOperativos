#include <src/commons/config.h>
#include <src/commons/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <src/commons/collections/list.h>
#include <src/commons/config.h>
#include <src/commons/string.h>
#include <ctype.h>
#include <sockets.h>
#include <serializacion.h>
#include <pthread.h>
#include <unistd.h>
#include "interBloqueo.h"
#include "pedidos.h"
#include <src/commons/log.h>


typedef struct {
	char recurso;
	char* nombre;
	int cantidadDisponible;
	int cantidadEnUso;
	int posX;
	int posY;
	t_list *quienesTienenEsteRecurso;

} t_dispoRecurso;

//cantidad de personajes en el system
int cantPersonajes;

//Variables Globales que uso
extern t_list *personajesEnNivel;

extern t_dispoRecurso recursosEnNivel[3055];
extern int recovery;
extern t_list *personajesPeticiones;
//extern pthread_mutex_t sema;

extern int socketOrquestador;

t_interbloqueo *resumenInterBloqueo;

extern t_log* logger;

extern char* nivel;

t_recursosLiberados *recursos;

int yaAgregoAlPrimero;

int cantCajasEnUso;

extern int cantCajas;

int verificarInterbloqueo(){

	cantPersonajes = list_size(personajesEnNivel);

	masDeDosRecursos();

	log_info(logger,"la cantidad de personajes en este nivel es:%d y la cantidad de tipos de recursos que se usan es:%d",cantPersonajes,cantCajasEnUso);
	if(cantPersonajes>=2 && (cantCajasEnUso>=2)){
		log_info(logger,"comienza la deteccion de interbloqueo");

		interbloqueo();

	}

	return 0;
}

void masDeDosRecursos(){

	int i;
	cantCajasEnUso=0;

	for(i=0;i<=cantCajas;i++){

		if (list_size(recursosEnNivel[i].quienesTienenEsteRecurso)!=0){

			cantCajasEnUso++;

		}

	}


}

int interbloqueo() {

	yaAgregoAlPrimero=0;



	//ya se cuanto va a haber en cantCajas
	//Averiguo cuantos personaes estan ahora!
	//Aca no se repiten
	cantPersonajes = list_size(personajesEnNivel);

	//Creo e inicializo los datos que voy a usar

	int asignado[cantPersonajes][cantCajasEnUso];
	int peticion[cantPersonajes][cantCajasEnUso];
	char recurso[cantCajasEnUso];
	char estasAcabado[cantPersonajes];
	int m, n;



	//Se segun la matriz de asignados de que personaje hablo, segun el indice
	char *posConPersonaje[cantPersonajes];

	//Lleno la cantidad disponible
	int trabajo[cantCajasEnUso];
	//Le pongo todo ceros de entrada,como valor default (?

	for (m = 0; m < cantPersonajes; m++) {
		for (n = 0; n < cantCajasEnUso; n++) {
			asignado[m][n] = 0;
			peticion[m][n] = 0;

		}
		estasAcabado[m] = 't';
	}



	for (n = 0; n < cantCajasEnUso; n++) {
		trabajo[n] = recursosEnNivel[n].cantidadDisponible;


	}

	for(n=0;n<cantCajasEnUso;n++){

	  if (list_size(recursosEnNivel[n].quienesTienenEsteRecurso)!=0){

		  recurso[n] = recursosEnNivel[n].recurso;

		}
	}


    int indPersonaje=0;

    void buscar_nombre_personajes(t_personaje *personaje)
	{

		posConPersonaje[indPersonaje]=strdup(personaje->remitente);
		log_info(logger,"en pos personaje:%s",posConPersonaje[indPersonaje]);
		indPersonaje++;
    }


    list_iterate(personajesEnNivel,(void*)buscar_nombre_personajes);

	//------------------------------LLENO LAS MATRICES----------------------------

	void llenarMatrizAsignado() {

		int nroRecurso = 0;

		for (; nroRecurso < cantCajasEnUso; nroRecurso++){

			void buscar_asignaciones(char *personaje){

				int traerPosPersonaje() {
					log_info(logger,"Personaje:%s",personaje);
					int u = 0;
					int pos = -1;
					for (; u < cantPersonajes; u++) {
					if ((string_equals_ignore_case(posConPersonaje[u],personaje))==true) {
					pos = u;
						}
						}
					log_info(logger,"posPersonaje:%d",pos);
					return pos;
					}

				int laPosDelPersonaje=traerPosPersonaje();
				asignado[laPosDelPersonaje][nroRecurso]++;

			}
            if(list_size(recursosEnNivel[nroRecurso].quienesTienenEsteRecurso)!=0)
            {
			list_iterate(recursosEnNivel[nroRecurso].quienesTienenEsteRecurso,(void*) buscar_asignaciones);
	     	}
      }

		log_info(logger,"%d  %d %d  %d",asignado[0][0],asignado[0][1],asignado[0][2],asignado[0][3]);
		log_info(logger,"%d  %d %d  %d",asignado[1][0],asignado[1][1],asignado[1][2],asignado[1][3]);
		log_info(logger,"%d  %d %d  %d",asignado[2][0],asignado[2][1],asignado[2][2],asignado[2][3]);
		log_info(logger,"%d  %d %d  %d",asignado[3][0],asignado[3][1],asignado[3][2],asignado[3][3]);

	}

	void llenarMatrizPeticion() {

		int posicionRecurso;
		int posicionPersonaje;

		void buscar_peticiones(t_pedirPosicionRecurso *unPersonaje){

			int traerPosPersonaje(char *personaje) {
				log_info(logger,"Personaje= %s",personaje);
				int u = 0;
				int pos = -1;
				for (; u < cantPersonajes; u++) {
					if ((string_equals_ignore_case(posConPersonaje[u],personaje))==true) {
						pos = u;
					}
				}
				log_info(logger,"posPersonaje = %d",pos);
				return pos;
			}
			int posRecurso(char r) {
				log_info(logger,"Recurso = %c",r);
				int l;
				int posRecurso = -1;
				for (l = 0; l < cantCajasEnUso; l++) {
					if (recurso[l] == r) {
						posRecurso = l;

					}
				}
				log_info(logger,"posRecurso = %d",posRecurso);
				return posRecurso;
			}


			posicionRecurso = posRecurso(unPersonaje->recurso);
			posicionPersonaje = traerPosPersonaje(unPersonaje->remitente);
			peticion[posicionPersonaje][posicionRecurso]++;

		}
		if(list_size(personajesPeticiones)!=0)
		  {
		list_iterate(personajesPeticiones,(void*) buscar_peticiones);
		  }

		log_info(logger,"Llenar matriz peticion",0);
		log_info(logger,"%d  %d %d  %d",peticion[0][0],peticion[0][1],peticion[0][2],peticion[0][3]);
		log_info(logger,"%d  %d %d  %d",peticion[1][0],peticion[1][1],peticion[1][2],peticion[1][3]);
		log_info(logger,"%d  %d %d  %d",peticion[2][0],peticion[2][1],peticion[2][2],peticion[2][3]);
		log_info(logger,"%d  %d %d  %d",peticion[3][0],peticion[3][1],peticion[3][2],peticion[3][3]);
	}

	//------------------------------LLENO LAS MATRICES----------------------------

	llenarMatrizAsignado();

	llenarMatrizPeticion();

	//------------------DETECCION------------------------

	//para todos los que tengan asignados algo, quiere decir que no terminaron
	int nroPersonaje, nroCaja;
	//----------Aca le pone false aquellos que tengan cosas asignadas----------
	for (nroPersonaje = 0; nroPersonaje < cantPersonajes;nroPersonaje++) {
		for (nroCaja = 0; nroCaja < cantCajasEnUso; nroCaja++) {
			if (asignado[nroPersonaje][nroCaja] != 0) {
				estasAcabado[nroPersonaje] = 'f';
				//Si encuentra un recurso que no siga buscando, con una basta y sobra.
				nroCaja = cantCajasEnUso + 1;
			}
		}
	}

	/*EXPLICACION
	 *busca si la fila del proceso podria asignarle con lso recursos que quedan
	 *lo que pide. Para que sea 1 debe poder hacerlo con TODOS los recursos*/

	int acabadoesFalsoyPeticionMenorATrabajo(int fila) {
		int bandera = 0;
		int nroRecurso;
		for (nroRecurso = 0; nroRecurso < cantCajasEnUso; nroRecurso++) {
			if ((estasAcabado[fila] == 'f')
					&& (peticion[fila][nroRecurso] <= trabajo[nroRecurso])) {

			} else {
				nroRecurso = (cantCajasEnUso+1);
				bandera = 4;
			}
		}
		//si bandera es falso no cumple la condicion
		if (bandera == 4) {
			return 0;
		} else {
			return 1;
		}
	}

	nroPersonaje = 0;
	while (acabadoesFalsoyPeticionMenorATrabajo(nroPersonaje)) {
		nroCaja = 0;

		for (; nroCaja < cantCajasEnUso; nroCaja++) {
			trabajo[nroCaja] = trabajo[nroCaja] + asignado[nroPersonaje][nroCaja];
			estasAcabado[nroPersonaje] = 't';
		}
		nroPersonaje++;

		//cuando f es cantPersonajes vuelve a cero para volverse a fijar si ahora quedo alguno
		if (nroPersonaje == cantPersonajes) {
			nroPersonaje = 0;
		}
	}



	//------------------DETECCION------------------------

	//-------------------------RECOVERY ACTIVADO------------------------------

	int traerPosPersonaje(char *personaje) {

		int u = 0;
		int pos = -1;
		for (; u < cantPersonajes; u++) {
			if ((string_equals_ignore_case(posConPersonaje[u],personaje))==true) {
				pos = u;
			}
		}
		return pos;
	}
	char* traerPersonajeSegunPos(char nro){
		int u =0;
		char *personaje;
		for(;u<cantPersonajes;u++){
			if(u==nro){
				personaje=posConPersonaje[u];
				u=cantPersonajes;
			}
		}
		return personaje;
	}

	int esta_bloqueado(char* nombre)
	{
		int es_el_personaje(t_pedirPosicionRecurso *bloqueado)
		{
			return string_equals_ignore_case(bloqueado->remitente,nombre);
		}

		return (int)list_find(personajesPeticiones, (void*)es_el_personaje);
	}

	int cantidadPersonajesEnInterbloqueo()
	{
		int cantidad=0;
		int c;
		for(c=0;c<cantPersonajes;c++)
		{
			if(estasAcabado[c]=='f' && esta_bloqueado(traerPersonajeSegunPos(c)))
			{
				cantidad++;
			}

		}
		return cantidad;
	}

	int agregados=0;

	void agregarPersonajeAInformeBloqueados(int nroPersonaje,int nroPersonajesBloqueados)
	{

		char *personaje = traerPersonajeSegunPos(nroPersonaje);
		log_info(logger,"el personaje:%s forma parte del interbloqueo",personaje);

		if(strcmp(resumenInterBloqueo->personajes,"")==0)//Si todavía no hay nombres
		{

			resumenInterBloqueo->personajes=string_from_format("%s%s",resumenInterBloqueo->personajes,personaje);
		}
		else{

			if (agregados==(nroPersonajesBloqueados-1)) //Si solo queda uno por agregar
			{
				resumenInterBloqueo->personajes=string_from_format("%s:%s",resumenInterBloqueo->personajes,personaje); //Lo concatena con un ":" Y no agrega nada más
			}
			else //Si ya hay nombres, los concatena con ":" de por medio y deja un ":" al final esperando al siguiente nombre.
			{
				resumenInterBloqueo->personajes=string_from_format("%s:%s",resumenInterBloqueo->personajes,personaje);
			}
		}

		agregados++;

	}
	int primeroQueLLegoANivel(){

		int posPrimero;

		int posAux;

		void es_el_primero(t_personaje *personaje){


			posAux=traerPosPersonaje(personaje->remitente);

			if( (estasAcabado[posAux]=='f') && (yaAgregoAlPrimero==0) ){
				yaAgregoAlPrimero++;
				posPrimero=posAux;
			}

		}
		list_iterate(personajesEnNivel, (void*) es_el_primero);
		log_info(logger,"la posicion del primero que llego es:%d",posPrimero);
		log_info(logger,"el que primero llego:%s",posConPersonaje[posPrimero]);

		return posPrimero;
	}



	void enviarPersonajesBloqueados(){
		int x;

		int posPrimeroQueLLegoANivel;

		int cantPersonajesBloqueados = cantidadPersonajesEnInterbloqueo();

		posPrimeroQueLLegoANivel = primeroQueLLegoANivel();

		char* nombre = traerPersonajeSegunPos(posPrimeroQueLLegoANivel);
		if(esta_bloqueado(nombre))
		{
			agregarPersonajeAInformeBloqueados(posPrimeroQueLLegoANivel,cantPersonajesBloqueados);
		}


		for(x=0;x<cantPersonajes;x++)
		{
			int cantPersonajesBloqueados = cantidadPersonajesEnInterbloqueo();

			if((estasAcabado[x]=='f')&&(x!=posPrimeroQueLLegoANivel) && esta_bloqueado(traerPersonajeSegunPos(x)))
			{

				agregarPersonajeAInformeBloqueados(x,cantPersonajesBloqueados);
			}
		}
	}

	if (recovery&&(cantidadPersonajesEnInterbloqueo()>1)){

		log_info(logger,"recovery activado",0);
		char *stream;
		void* puntero;
		resumenInterBloqueo = malloc(sizeof(t_interbloqueo));

		resumenInterBloqueo->nivel=nivel;
		resumenInterBloqueo->personajes="";

		enviarPersonajesBloqueados();

		log_info(logger,"<INTERBLOQUEO> personajes involucrados: %s\n",resumenInterBloqueo->personajes);

		puntero = resumenInterBloqueo;


		//log_info(logger,"Llegue aca",0);
		stream = serializer(10, puntero);

		//stream_asignarRecurso = serializer(10, puntero);
		log_info(logger,"<INTERBLOQUEO> se envia mensaje con los personajes involucrados al orquestador, se espera respuesta");

		enviar_mensaje(socketOrquestador,stream);

//		free(stream);
//		free(resumenInterBloqueo->personajes);
//		free(resumenInterBloqueo);


////////CODIGO DE VALIDACION DE ENVIO DE VICTIMA SEGURO //////////////////////////

		char* streamDeMandameDevuelta = malloc(12);
		int unNumero = 94;
		int longitud = 12;
		int otroNumero = 40;
		memcpy(streamDeMandameDevuelta,&unNumero,4);
		memcpy(streamDeMandameDevuelta,&longitud,4);
		memcpy(streamDeMandameDevuelta,&otroNumero,4);

		sleep(2);

		int numero = 0;
		stream = recibir_mensaje(socketOrquestador);
		memcpy(&numero, stream, 4);
		int largo;
	    memcpy(&largo, stream+4, 4);
		log_info(logger,"codigo %d, largo %d, data %s\n", numero, largo,(stream+8));

		while (numero != 10) {  //si no es codigo 10, entonces esta deserializando
			                     //un stream cualquiera o basura que quedo en buffer,
			                    //desconozco las causas de porque puede suceder esto.
		enviar_mensaje(socketOrquestador,streamDeMandameDevuelta);
		sleep(1);
		printf("se pidio que mande devuelta, esperando respuesta\n");
		stream = recibir_mensaje(socketOrquestador);
		memcpy(&numero, stream, 4);
		}

//////////////////////////////////////////////////////////////////////////////////////

		log_info(logger,"<INTERBLOQUEO> se recibio mensaje del orquestador\n");

		resumenInterBloqueo = deserializer(stream);

		log_info(logger,"<INTERBLOQUEO> la victima seleccionada fue %s, el orquestador solicita conocer los recursos que le fueron asignados\n",resumenInterBloqueo->personajes);

		 //vuelvo a serializar un paquete y enviarlo porque el orquestador ya lo esta esperando, es sincronico

		//recursos->personaje=strdup(resumenInterBloqueo->personajes);

		sacarPersonajeDePeticiones(resumenInterBloqueo->personajes);

		devuelveLosRecursos(resumenInterBloqueo->personajes);

		//borrarPersonajeDelMapa(resumenInterBloqueo->personajes);

		//dibujarEnLaPosInicial(resumenInterBloqueo->personajes);

		//volverAlFondoDeLaLista(resumenInterBloqueo->personajes);
	}

	//-------------------------RECOVERY ACTIVADO------------------------------

	return 0;
}
