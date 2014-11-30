/*
 * testSerializador.c
 *
 *  Created on: 24/05/2013
 *      Author: Jonathan
 */

#include <stdio.h>
#include <stdlib.h>
#include <serializacion.h>
#include <src/commons/collections/dictionary.h>

int main() {
	t_dictionary *dic_serializado;
	dic_serializado = dictionary_create();
	//posicion recurso
	t_posicion *recurso = malloc(sizeof(t_posicion));
	t_posicion *recurso2;
	//direccion
	t_direccion *dir1 = malloc(sizeof(t_direccion));
	t_direccion *dir2;

	//pedir posicion recurso
	t_pedirPosicionRecurso *pedirR1 = malloc(sizeof(t_pedirPosicionRecurso));
	t_pedirPosicionRecurso *pedirR2;

	//pedir direccion nivel y planificador
	t_pedirDireccion *pedirD1 = malloc(sizeof(t_pedirDireccion));
	t_pedirDireccion *pedirD2;

	//booleano
	t_boolean *bool1 = malloc(sizeof(t_boolean));
	t_boolean *bool2;

	//devolucion de recursos a un personaje
	t_devolverRecursos *dev1 = malloc(sizeof(t_devolverRecursos));
	t_devolverRecursos *dev2;

	//pedir un determinado recurso
	t_pedirRecurso *pideRec1 = malloc(sizeof(t_pedirRecurso));
	t_pedirRecurso *pideRec2;

	//nombre del nivel
	t_direccionNombre *dirNom1 = malloc(sizeof(t_direccionNombre));
	t_direccionNombre *dirNom2;

	//pid y remitente
	t_personaje *inicial1 = malloc(sizeof(t_personaje));
	t_personaje *inicial2;

	//estado de obtencion de recurso
	t_estadoRecurso *estado1 = malloc(sizeof(t_estadoRecurso));
	t_estadoRecurso *estado2;

	//posibles victimas de interbloqueo
	t_interbloqueo *victimas1 = malloc(sizeof(t_interbloqueo));
	t_interbloqueo *victimas2;

	//cantidad de recursos recuperados de interbloqueo
	t_recursosLiberados *recursosLiberados = malloc(sizeof(t_recursosLiberados));
	t_recursosLiberados *recursosLiberados2;

	//elementos basicos
	char *stream;
	void *puntero;
	int codigo;
	int err = 0;

	printf("ingrese codigo:\n");
	scanf("%d", &codigo);

	//inicializaciones y reserva de memoria
	recurso->x = 10;
	recurso->y = 15;
	recurso->remitente = "mario";

	dir1->puertoPlan = 1400;
	dir1->puertoNiv = 300;
	dir1->ipNiv = "255.555.000.111";
	dir1->ipPlan = "444.222.333.111";

	pedirR1->recurso = 'H';
	pedirR1->remitente = "mario";

	pedirD1->nombrenivel = "primer nivel";

	bool1->boolean = 0;

	dev1->personaje = "mario";

	pideRec1->recurso = 'F';
	pideRec1->remitente = "luigi";

	dirNom1->ip = "244.111.222.333";
	dirNom1->puerto = 505;
	dirNom1->remitente = "nivel3";

	inicial1->socket = 50;
	inicial1->remitente = "toto";

	estado1->estado = 0;
	estado1->recurso = 'H';

	victimas1->nivel = "nivel6-6";
	victimas1->personajes = "mario:luigi:goomba:tortuga";

	recursosLiberados->recursos = "F1:H4:M5:Z6";
	recursosLiberados->total = 4;
	recursosLiberados->nivel= "deadlock";
	recursosLiberados->personaje= "&goomba";

	char *clave = "claveHardcodeada";

	dictionary_put(dic_serializado, clave, &codigo);

	int *numero;
	numero = dictionary_remove(dic_serializado, clave);

	printf("tiene: %d\n", *numero);

	switch (codigo) {

	case 0:
		puntero = recurso;
		break;
	case 1:
		puntero = dir1;
		break;
	case 2:
		puntero = pedirR1;
		break;
	case 3:
		puntero = pedirD1;
		break;
	case 4:
		puntero = bool1;
		break;
	case 5:
		puntero = dev1;
		break;
	case 6:
		puntero = pideRec1;
		break;
	case 7:
		puntero = dirNom1;
		break;
	case 8:
		puntero = inicial1;
		break;
	case 9:
		puntero = estado1;
		break;
	case 10:
		puntero = victimas1;
		break;
	case 11:
		puntero = recursosLiberados;
		break;
	default:
		printf("codigo erroneo\n");
		err = 1;
	}

	if (err == 0) {
		stream = serializer(codigo, puntero);

		dictionary_put(dic_serializado, clave, stream);

		char *p_stream;

		p_stream = dictionary_get(dic_serializado, clave);

		puntero = deserializer(p_stream);

		free(stream);

		switch (codigo) {

		case 0:

			recurso2 = puntero;

			printf("posicion en X: %d, posicion en Y: %d, y lo pide: %s\n",
					recurso2->x, recurso2->y, recurso2->remitente);

			free(recurso2->remitente);
			free(recurso2);

			break;

		case 1:

			dir2 = puntero;

			printf("ip de nivel: %s ip de plan: %s\n", dir2->ipNiv,
					dir2->ipPlan);
			printf("puerto nivel: %d puerto plan: %d\n", dir2->puertoNiv,
					dir2->puertoPlan);

			free(dir2->ipNiv);
			free(dir2->ipPlan);
			free(dir2);

			break;

		case 2:

			pedirR2 = puntero;
			printf("recurso: %c, y lo pide: %s\n", pedirR2->recurso,
					pedirR2->remitente);

			free(pedirR2->remitente);
			free(pedirR2);

			break;

		case 3:

			pedirD2 = puntero;
			printf("nombre del nivel: %s\n", pedirD2->nombrenivel);
			free(pedirD2->nombrenivel);
			free(pedirD2);

			break;

		case 4:

			bool2 = puntero;
			char* resultado;

			if (bool2->boolean == 0)
				resultado = "false";
			else
				resultado = "true";

			printf("otorgo recurso? %s\n", resultado);

			free(bool2);
			break;
		case 5:

			dev2 = puntero;
			printf("personaje: %s\n", dev2->personaje);

			free(dev2->personaje);
			free(dev2);
			break;

		case 6:

			pideRec2 = puntero;
			printf("recurso: %c, y lo pide: %s\n", pideRec2->recurso,
					pideRec2->remitente);

			free(pideRec2->remitente);
			free(pideRec2);

			break;

		case 7:

			dirNom2 = puntero;
			printf("ip: %s, puerto: %d, nombre: %s\n", dirNom2->ip,
					dirNom2->puerto, dirNom2->remitente);
			free(dirNom2->ip);
			free(dirNom2->remitente);
			free(dirNom2);
			break;

		case 8:

			inicial2 = puntero;
			printf("socket del proceso: %d, nombre: %s\n", inicial2->socket,
					inicial2->remitente);

			free(inicial2->remitente);
			free(inicial2);
			break;
		case 9:

			estado2 = puntero;

			char* resp;

			if (estado2->estado == 0)
				resp = "false";
			else
				resp = "true";

			printf("en el recurso: %c,se bloqueo? %s\n", estado2->recurso,
					resp);

			free(estado2);
			break;
		case 10:

			victimas2 = puntero;
			printf("nivel con bloqueo: %s, personajes involucrados: %s\n", victimas2->nivel, victimas2->personajes);

			free(victimas2->nivel);
			free(victimas2->personajes);
			free(victimas2);
			break;

		case 11:

			recursosLiberados2 = puntero;
			printf("concatenacion de recursos liberados:%s, son %d distintos, en el nivel %s del personaje %s\n",
					recursosLiberados2->recursos,recursosLiberados2->total,
					recursosLiberados2->nivel, recursosLiberados2->personaje);

			free(recursosLiberados2->personaje);
			free(recursosLiberados2->nivel);
			free(recursosLiberados2->recursos);
			free(recursosLiberados2);
			break;
		default:
			printf("error codigo\n");
		}
	}

	dictionary_destroy(dic_serializado);
	//posicion recurso
	free(recurso);
	//direccion
	free(dir1);
	//pedir posicion recurso
	free(pedirR1);
	//pedir direccion nivel y planificador
	free(pedirD1);
	//booleano
	free(bool1);
	//devolucion de recursos a un personaje
	free(dev1);
	//pedir un determinado recurso
	free(pideRec1);
	//nombre del nivel
	free(dirNom1);
	//pid y remitente
	free(inicial1);
	//estado de obtencion de recurso
	free(estado1);
	//posibles victimas de interbloqueo
	free(victimas1);

	free(recursosLiberados);

	stream = serializer(14,NULL);

	puntero = deserializer(stream);

	return EXIT_SUCCESS;

}
