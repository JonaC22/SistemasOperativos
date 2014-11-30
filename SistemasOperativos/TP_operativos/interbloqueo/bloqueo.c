
#include <src/commons/config.h>
#include <src/commons/string.h>
#include <src/commons/collections/list.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <src/commons/collections/list.h>
#include <src/commons/config.h>
#include <src/commons/string.h>
#include <ctype.h>

typedef struct {
	char recurso;
	char* nombre;
	int  cantidadDisponible;
	int  cantidadEnUso;
	int posX;
	int posY;
	t_list *quienesTienenEsteRecurso;

}__attribute__ ((__packed__)) t_dispoRecurso;

typedef struct {
	int pid;
	char* remitente;

}__attribute__ ((__packed__)) t_personaje;

typedef struct {
	int cantidad;
	t_list *procesosEnInterbloqueo;
}__attribute__ ((__packed__)) t_interbloqueo;

t_dispoRecurso recursosEnNivel[3];

void obtenerRecursosYGuardarlosEnVectorDeRecursos(t_config* unPath);
void usaUnRecurso2(char* alguien);
void usaUnRecurso(char* alguien);

//nro de instancias disponibles de cada recurso
int trabajo[3];

typedef struct {
	int f;
	int c;
}__attribute__ ((__packed__)) pos;

typedef struct {
	char *remitente;
	char recurso;
	int pid;
}__attribute__ ((__packed__)) personaje;

//cantidad de recursos en el sistema
int cantCajas;

//cantidad de personajes en el system
int cantPersonajes;

int interbloqueo();

int main(){


	t_config* configNivel = config_create("/home/utnso/workspace/operativos/nivel.cfg");
	int recovery;

	if( ((config_has_property(configNivel,"Recovery")) == true) ){

		recovery = config_get_int_value(configNivel,"Recovery");
		obtenerRecursosYGuardarlosEnVectorDeRecursos(configNivel);
	}

	if(recovery==0){
		printf("no esta activado el recovery \n");
	}else{
		interbloqueo();
	}

	printf("g");

	return 0;
}

int interbloqueo(){

	char *persona="carolina";
	char *otraPersona="leo";

	/*char l='f';
    printf("%c\n",l);
	l=toupper(l);
	printf("%c\n",l);*/

	//ahora se que cantidad de recursos que tengo en cantCajas

	// ME ARME ESTA LISTA PARA PROBAR, LA LISTA LA VOY A TENER ARMADA.

	personaje *p1=malloc(sizeof(personaje));
	t_list *personajesPeticiones=list_create(personajesPeticiones);

	p1->remitente="carolina";
	p1->recurso='H';

	list_add_in_index(personajesPeticiones,0,p1);

	personaje *p2=malloc(sizeof(personaje));

	p2->remitente="leo";
	p2->recurso='F';

	list_add_in_index(personajesPeticiones,1,p2);


    //personajesEnNivel es REMITENTE Y PID

	t_personaje *p5=malloc(sizeof(t_personaje));
	t_list *personajesEnNivel=list_create(personajesEnNivel);

	p5->remitente="carolina";
	p5->pid=1;

	list_add_in_index(personajesEnNivel,0,p5);

	t_personaje *p6=malloc(sizeof(t_personaje));

	p6->remitente="leo";
	p6->pid=2;

	list_add_in_index(personajesEnNivel,1,p6);
	//Ya tengo los personajes con los que voy a jugar
	//ahora los agrego asi , dps me copio "personajesEnNivel" que ya esta listo
	//y solo deberia hacer una asignacion

	cantPersonajes=2;

	//ahora tengo que cantidad de recursos y personajes tengo


	usaUnRecurso(persona);
	usaUnRecurso2(otraPersona);

	//cantidad de instancias asignadas a los procesos y control de terminados

	int asignado[cantPersonajes][cantCajas+1];
	int peticion[cantPersonajes][cantCajas+1];
	int recurso[cantCajas+1];
	char acabado[cantPersonajes];

	int m,n;
	for(m=0;m<cantPersonajes;m++){
		for( n=0;n<=cantCajas;n++){
			asignado[m][n]=0;
			peticion[m][n]=0;
			acabado[m]='t';
		}
	}
	//recursosEnNivel

	int trabajo[cantCajas];
	for( n=0;n<=cantCajas;n++){
		trabajo[n]=recursosEnNivel[n].cantidadDisponible;
		recurso[n]=recursosEnNivel[n].recurso;
		printf("para el recurso %c hay %d disponibles \n",recurso[n],trabajo[n]);

	}
        printf("\n");


		//Se segun la matriz de asignados de que personaje hablo, segun el indice
		char *posConPersonaje[cantPersonajes];


		//lleno la matriz de vectores asignados


		void llenarMatrizAsignado(){
			//int asignado[cantPersonajes][cantCajas]; posConPersonaje[]
			int q=0;
			int nroPersonaje=-1;

			for (;q<=cantCajas;q++){
				t_list *lAux=malloc(sizeof(t_list));
						lAux=recursosEnNivel[q].quienesTienenEsteRecurso;

				bool _es_de_mismo_nombre(char* nombre) {
					return string_equals_ignore_case(nombre, list_get(lAux,0));
				}

				while((list_size(lAux))!=0){

					int cantRqueTieneElPersonaje=0;

					//uso una bandera para agregar a mi vector de personajes
					//si a es igual a uno quiere decir que lo agregue
					int b,a,y;

					b=0;
					a=0;
					nroPersonaje++;
					//BUSCA RECURSOS PARA EL PRIMER PERSONAJE DE LA LISTA
					while (list_find(lAux, (void*) _es_de_mismo_nombre) != NULL )
					{
						int traerPosPersonaje(char *personaje){
								int u =0;
								int pos=-1;
								for(;u<cantPersonajes;u++){
									if(posConPersonaje[u]==personaje){
										pos=u;
									}
								}
					            return pos;
							}


						//si mi bandera es cero agrego por unica vez el
						if(b==0){
							y=traerPosPersonaje(list_get(lAux,0));
							if(y!=(-1)){
								//ya tengo la posicion y no tengo que agregarlo
							}else{
								posConPersonaje[nroPersonaje]=list_get(lAux,0);
								a++;
							}
						}
						cantRqueTieneElPersonaje++;
						list_remove_by_condition(lAux, (void*) _es_de_mismo_nombre);
						b=1;
					}


					//Ya se que recursos tiene la persona

					//ahora se los asigno

					//1) si yo agregue el personaje al vector
					if(a!=0){
						printf("cuando llego tenia %d de ese recurso\n",asignado[nroPersonaje][q]);
						asignado[nroPersonaje][q]+=cantRqueTieneElPersonaje;
						printf("para el personaje %d asignamos %d\n",nroPersonaje,asignado[nroPersonaje][q]);
					printf("\n");
					}else{
						asignado[y][q]+=cantRqueTieneElPersonaje;
					}


				}
free(lAux);
			}

		}

		void llenarMatrizPeticion(){
			//personajesEnNivel peticion[cantPersonajes][cantCajas]; recurso[n]
			t_list *lAux=malloc(sizeof(t_list));
					lAux=personajesPeticiones;

					int traerPosPersonaje(char *personaje){
							int u =0;
							int pos=-1;
							for(;u<cantPersonajes;u++){
								if(posConPersonaje[u]==personaje){
									pos=u;
								}
							}
				            return pos;
						}
					int posRecurso(char r){
								int l;
								int pos=-1;
								for (l=0;l<=cantCajas;l++){
									if(recurso[l]==r){
										pos=l;

									}
								}
								return pos;
							}

		    	while((list_size(lAux))!=0){

				int rec;
				int per;

				personaje *p3 = malloc(sizeof(personaje));

				p3=list_get(lAux,0);

				printf("\n");
				printf("el personaje %s pide este %c recurso\n",p3->remitente,p3->recurso);

				rec = posRecurso(p3->recurso);
				printf("la pos del recurso es %d\n",rec);
			    per = traerPosPersonaje(p3->remitente);
				printf("la pos del personae es %d\n",per);
                printf("%d\n",peticion[0][2]);
			    peticion[per][rec]++;
			    printf("%d\n",peticion[0][2]);
			    printf("\n");
			    		for(m=0;m<cantPersonajes;m++){
			    					printf("asi esta dps de que pasa uno al de peticion ");
			    					for( n=0;n<=cantCajas;n++){
			    						printf(" %d",peticion[m][n]);
			    					}
			    					printf("\n");
			    				}

			    list_remove(lAux,0);
                free(p3);
			}

			 free(lAux);

		}

		llenarMatrizAsignado();
		llenarMatrizPeticion();
		printf("\n");
		for(m=0;m<cantPersonajes;m++){
			printf("el proceso %d tiene su asignacion",m);
			for( n=0;n<=cantCajas;n++){
				printf(" %d",asignado[m][n]);
			}
			printf("\n");
		}
printf("\n");
		for(m=0;m<cantPersonajes;m++){
					printf("el proceso %d tiene su peticion",m);
					for( n=0;n<=cantCajas;n++){
						printf(" %d",peticion[m][n]);
					}
					printf("\n");
				}

		//cantCajas cantPersonajes
		//para todos los que tengan asignados algo, quiere decir que no terminaron
		int f,c;
		//----------Aca le pone false aquellos que tengan cosas asignadas----------
		for(f=0;f<cantPersonajes;f++){
			for( c=0;c<=cantCajas;c++){
				if(asignado[f][c]!=0){
					acabado[f]='f';
					//Si encuentra un recurso que no siga buscando, con una basta y sobra.
					c=3;
				}
			}

		}



		/*EXPLICACION
		 *busca si la fila del proceso podria asignarle con lso recursos que quedan
		 *lo que pide. Para que sea 1 debe poder hacerlo con TODOS los recursos
		 */
		int acabadoesFalsoyPeticionMenorATrabajo(fil){
			int r=0;
			int i;
			for(i=0;i<=cantCajas;i++){
				if((acabado[fil]=='f')&&(peticion[fil][i]<=trabajo[i])){
					//if(i==2){
					//return 1;

				}
				else{
					i=3;
					r=4;
				}
			}
			//si r es falso no cumple la condicion
			if(r==4){
				return 0;
			}else{
				return 1;}

		}

		f=0;
		while (acabadoesFalsoyPeticionMenorATrabajo(f)){
			c=0;

			for(;c<=cantCajas;c++){
				trabajo[c]=trabajo[c] + asignado[f][c];
				acabado[f]='t';
			}
			f++;

			//cuando f es cantPersonajes vuelve a cero para volverse a fijar si ahora quedo alguno
			if(f==cantPersonajes){
				f=0;
			}
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

	t_personaje traerInformacionPersonaje(char *personaje){

    t_personaje *unPersonaje = malloc(sizeof(t_personaje));

	bool _es_de_mismo_nombre(t_personaje *infoPersonaje) {
			return string_equals_ignore_case(personaje,infoPersonaje->remitente);
		}

	unPersonaje = list_find(personajesEnNivel, (void*) _es_de_mismo_nombre);

    return *unPersonaje;
	}

	void   AgregarloAlVectorYActualizarContador(t_personaje *unPersonaje,t_interbloqueo *resumenInterBloqueo){

		printf("agregue a %s\n",unPersonaje->remitente);
        list_add_in_index(resumenInterBloqueo->procesosEnInterbloqueo,list_size(resumenInterBloqueo->procesosEnInterbloqueo),unPersonaje);

		}

	int cantidadPersonajesEnInterbloqueo(){
		int cantidad=0;
		for(c=0;c<cantPersonajes;c++){
				if(acabado[c]=='f'){
                  cantidad++;
				}

		    }
		return cantidad;
		}

	void agegarPersonajeAInformeBloqueados(int nroPersonaje,int nroPersonajesBloqueados){

		t_interbloqueo *resumenInterBloqueo = malloc(sizeof(t_interbloqueo));

		t_personaje *unPersonaje = malloc(sizeof(t_personaje));

		char *personaje = traerPersonajeSegunPos(nroPersonaje);

		resumenInterBloqueo->cantidad=nroPersonajesBloqueados;
		resumenInterBloqueo->procesosEnInterbloqueo=list_create(resumenInterBloqueo->procesosEnInterbloqueo);

		*unPersonaje = traerInformacionPersonaje(personaje);

        AgregarloAlVectorYActualizarContador(unPersonaje,resumenInterBloqueo);
    	printf("la cantidad es %d\n",resumenInterBloqueo->cantidad);
		}
    int x;
	for(x=0;x<cantPersonajes;x++){
		int cantPersonajesBloqueados = cantidadPersonajesEnInterbloqueo();
					if(acabado[x]=='f'){
					printf("el proceso %d esta en interbloqueo\n",x);
	                agegarPersonajeAInformeBloqueados(x,cantPersonajesBloqueados);
				}else{
					printf("el proceso %d no esta en interbloqueo\n",x);
				}

			}
	for(c=0;c<cantCajas;c++){
				printf("%c\n",acabado[c]);
			}

		return 0;

	}

	void obtenerRecursosYGuardarlosEnVectorDeRecursos(t_config* unPath)
	{
		//si es 0 hay una caja
		//si es uno hay dos cajas
		//si es dos hay dos cajas



		int i=-1;
		if(config_has_property(unPath, "Caja1"))
		{
			char *caja1=config_get_string_value(unPath,"Caja1");
			char* *p=malloc(sizeof(int));
			i++;

			//Le mando mi string "Flores,F,3,23,0" y me lo convierte a
			//["Flores","F","3","23","0"]

			p=string_get_string_as_array(caja1);

			recursosEnNivel[i].nombre=*p;

			recursosEnNivel[i].recurso=(char)**(p+1);

			recursosEnNivel[i].cantidadDisponible=atoi(*(p+2));

			recursosEnNivel[i].cantidadEnUso=0;

			recursosEnNivel[i].posX=atoi(*(p+3));

			recursosEnNivel[i].posY=atoi(*(p+4));

			recursosEnNivel[i].quienesTienenEsteRecurso = list_create();

			free(*p);



		}/*else{
	 printf("el campo caja1 no se encuentra en el archivo de configuacion");
	}*/

		//no le hago el malloc por que en list.c ya lo hicieron cuando definieron la fun
		//el free se hace cuando la elimine



		if(config_has_property(unPath, "Caja2"))
		{
			char *caja2=config_get_string_value(unPath,"Caja2");

			char* *q=malloc(sizeof(int));;
			i++;

			q=string_get_string_as_array(caja2);

			recursosEnNivel[i].nombre=*q;

			recursosEnNivel[i].recurso=(char)**(q+1);

			recursosEnNivel[i].cantidadDisponible=atoi(*(q+2));

			recursosEnNivel[i].cantidadEnUso=0;

			recursosEnNivel[i].posX=atoi(*(q+3));

			recursosEnNivel[i].posY=atoi(*(q+4));

			recursosEnNivel[i].quienesTienenEsteRecurso = list_create();

			free(*q);
		}/*else{
				 //printf("el campo Caja2 no se encuentra en el archivo de configuacion");
				}*/

		if(config_has_property(unPath, "Caja3"))
		{	char *caja3=config_get_string_value(unPath,"Caja3");

		char* *r=malloc(sizeof(int));;
		i++;

		r=string_get_string_as_array(caja3);

		recursosEnNivel[i].nombre=*r;

		recursosEnNivel[i].recurso=(char)**(r+1);

		recursosEnNivel[i].cantidadDisponible=atoi(*(r+2));

		recursosEnNivel[i].cantidadEnUso=0;

		recursosEnNivel[i].posX=atoi(*(r+3));

		recursosEnNivel[i].posY=atoi(*(r+4));

		recursosEnNivel[i].quienesTienenEsteRecurso = list_create();

		free(*r);

		}/*else{
	 //printf("el campo Caja3 no se encuentra en el archivo de configuacion");
	}*/


		cantCajas=i;
	}

	void usaUnRecurso(char* alguien){
		recursosEnNivel[0].cantidadDisponible--;
		recursosEnNivel[0].cantidadEnUso++;
		//restarRecurso(ListaItems,recursosEnNivel[0].recurso );
		//nivel_gui_dibujar(ListaItems);

		if (list_is_empty(recursosEnNivel[0].quienesTienenEsteRecurso)){
			////printf("el tamanio de la lista es cero\n");
			list_add_in_index(recursosEnNivel[0].quienesTienenEsteRecurso,0,alguien);
		}
		else{
			//>o dispo ya esta validado en procesoNivel, solo debo modificar
			if(recursosEnNivel[0].cantidadDisponible>=0){
				int tamanioLista;
				tamanioLista = list_size(recursosEnNivel[0].quienesTienenEsteRecurso);
				////printf("el tamanio es:%d\n",tamanioLista);
				////printf("el tamanio es:%d\n",tamanioLista);
				list_add_in_index(recursosEnNivel[0].quienesTienenEsteRecurso,tamanioLista,alguien);
				//int s = list_size(recursosEnNivel[0].quienesTienenEsteRecurso);
				////printf("el tamanio ahora que agregue es:%d\n",s);
				if(recursosEnNivel[0].cantidadDisponible==0){
					//	BorrarItem(&ListaItems,recursosEnNivel[0].recurso );
				}
			}
		}
	}
	void usaUnRecurso2(char* alguien){
		recursosEnNivel[1].cantidadDisponible--;
		recursosEnNivel[1].cantidadEnUso++;
		//restarRecurso(ListaItems,recursosEnNivel[0].recurso );
		//nivel_gui_dibujar(ListaItems);

		if (list_is_empty(recursosEnNivel[1].quienesTienenEsteRecurso)){
			////printf("el tamanio de la lista es cero\n");
			list_add_in_index(recursosEnNivel[1].quienesTienenEsteRecurso,0,alguien);
		}
		else{
			//>o dispo ya esta validado en procesoNivel, solo debo modificar
			if(recursosEnNivel[1].cantidadDisponible>=0){
				int tamanioLista;
				tamanioLista = list_size(recursosEnNivel[1].quienesTienenEsteRecurso);
				////printf("el tamanio es:%d\n",tamanioLista);
				////printf("el tamanio es:%d\n",tamanioLista);
				list_add_in_index(recursosEnNivel[1].quienesTienenEsteRecurso,tamanioLista,alguien);
				//int s = list_size(recursosEnNivel[0].quienesTienenEsteRecurso);
				////printf("el tamanio ahora que agregue es:%d\n",s);
				if(recursosEnNivel[1].cantidadDisponible==0){
					//	BorrarItem(&ListaItems,recursosEnNivel[0].recurso );
				}
			}
		}
	}




