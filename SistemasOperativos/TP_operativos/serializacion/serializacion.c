#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <src/commons/log.h>
#include "serializacion.h"

char *serializer(int codigo, void *puntero) {

	/*
	 codigo 0 = t_posicion
	 codigo 1 = t_direccion
	 codigo 2 = t_pedirPosicionRecurso
	 codigo 3 = t_pedirDireccion
	 codigo 4 = t_boolean
	 codigo 5 = t_devolverRecursos
	 codigo 6 = t_pedirRecurso
	 codigo 7 = t_direccionNombre
	 codigo 8 = t_personaje
	 codigo 9 = t_estadoRecurso
	 codigo 10 = t_interbloqueo
	 codigo 11 = t_cantRecursosLiberados

	 */

	t_posicion *rec;
	t_direccion *dir;
	t_pedirDireccion *pedirD;
	t_pedirPosicionRecurso *pedirR;
	t_boolean *boolean;
	t_devolverRecursos *devolverRecursos;
	t_pedirRecurso *pedirRecurso;
	t_direccionNombre *dirnombre;
	t_personaje *inicial;
	t_estadoRecurso *estado;
	t_interbloqueo *victimas;
	t_recursosLiberados *recursosliberados;

	char *stream;
	int offset = 0, aux_size = 0;
	unsigned char aux_length = 0;
	int pesoStream;

	if(puntero == NULL)
	{
		printf("NULL pointer exception, se quiso serializar un puntero que estaba en null con codigo %d\n", codigo);
		return NULL;
	}

	switch (codigo) {

	case 0:

		rec = puntero;

		stream = malloc(4 * sizeof(int) + 1 + strlen(rec->remitente) + 1);

		pesoStream = (4 * sizeof(int) + 1 + strlen(rec->remitente) + 1); //codigo+longitud+info

		memcpy(stream, &codigo, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, &pesoStream, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, rec, aux_size = sizeof(int) + sizeof(int));

		offset += aux_size;
		aux_length = strlen(rec->remitente) + 1;
		memcpy(stream + offset, &aux_length, aux_size = 1);

		offset += aux_size;
		memcpy(stream + offset, rec->remitente,
				aux_size = strlen(rec->remitente) + 1);

		break;

	case 1:

		dir = puntero;

		stream = malloc(
				4 * sizeof(int) + 1 + strlen(dir->ipNiv) + 2
						+ strlen(dir->ipPlan) + 1);

		pesoStream = (4 * sizeof(int) + 1 + strlen(dir->ipNiv) + 2
				+ strlen(dir->ipPlan) + 1);

		memcpy(stream, &codigo, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, &pesoStream, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, dir, aux_size = sizeof(int) + sizeof(int));

		offset += aux_size;
		aux_length = strlen(dir->ipNiv) + 1;
		memcpy(stream + offset, &aux_length, aux_size = 1);

		offset += aux_size;
		memcpy(stream + offset, dir->ipNiv, aux_size = strlen(dir->ipNiv) + 1);

		offset += aux_size;
		aux_length = strlen(dir->ipPlan) + 1;
		memcpy(stream + offset, &aux_length, aux_size = 1);

		offset += aux_size;
		memcpy(stream + offset, dir->ipPlan,
				aux_size = strlen(dir->ipPlan) + 1);

		break;

	case 2:

		pedirR = puntero;

		stream = malloc(
				2 * sizeof(int) + sizeof(char) + 1 + strlen(pedirR->remitente)
						+ 1);

		pesoStream = (2 * sizeof(int) + sizeof(char) + 1
				+ strlen(pedirR->remitente) + 1);

		memcpy(stream, &codigo, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, &pesoStream, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, pedirR, aux_size = sizeof(char));

		offset += aux_size;
		aux_length = strlen(pedirR->remitente) + 1;
		memcpy(stream + offset, &aux_length, aux_size = 1);

		offset += aux_size;
		memcpy(stream + offset, pedirR->remitente,
				aux_size = strlen(pedirR->remitente) + 1);

		break;

	case 3:

		pedirD = puntero;

		stream = malloc(2 * sizeof(int) + 1 + strlen(pedirD->nombrenivel) + 1);

		pesoStream = (2 * sizeof(int) + 1 + strlen(pedirD->nombrenivel) + 1);

		memcpy(stream, &codigo, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, &pesoStream, aux_size = sizeof(int));

		offset += aux_size;
		aux_length = strlen(pedirD->nombrenivel) + 1;
		memcpy(stream + offset, &aux_length, aux_size = 1);

		offset += aux_size;
		memcpy(stream + offset, pedirD->nombrenivel,
				strlen(pedirD->nombrenivel) + 1);

		break;

	case 4:

		boolean = puntero;

		stream = malloc(3 * sizeof(int));

		pesoStream = (3 * sizeof(int));

		memcpy(stream, &codigo, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, &pesoStream, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, boolean, aux_size = sizeof(int));

		break;

	case 5:

		devolverRecursos = puntero;

		stream = malloc(
				2 * sizeof(int) + 1 + strlen(devolverRecursos->personaje) + 1);

		pesoStream = (2 * sizeof(int) + 1 + strlen(devolverRecursos->personaje)
				+ 1);

		memcpy(stream, &codigo, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, &pesoStream, aux_size = sizeof(int));

		offset += aux_size;
		aux_length = strlen(devolverRecursos->personaje) + 1;
		memcpy(stream + offset, &aux_length, aux_size = 1);

		offset += aux_size;
		memcpy(stream + offset, devolverRecursos->personaje,
				strlen(devolverRecursos->personaje) + 1);

		break;

	case 6:

		pedirRecurso = puntero;

		stream = malloc(
				3 * sizeof(int) + 1 + strlen(pedirRecurso->remitente) + 1);

		pesoStream = (2 * sizeof(int) + sizeof(char) + 1
				+ strlen(pedirRecurso->remitente) + 1);

		memcpy(stream, &codigo, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, &pesoStream, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, pedirRecurso, aux_size = sizeof(char));

		offset += aux_size;
		aux_length = strlen(pedirRecurso->remitente) + 1;
		memcpy(stream + offset, &aux_length, aux_size = 1);

		offset += aux_size;
		memcpy(stream + offset, pedirRecurso->remitente,
				aux_size = strlen(pedirRecurso->remitente) + 1);

		break;

	case 7:

		dirnombre = puntero;

		stream = malloc(
				3 * sizeof(int) + 1 + strlen(dirnombre->ip) + 2
						+ strlen(dirnombre->remitente) + 1);

		pesoStream = (3 * sizeof(int) + 1 + strlen(dirnombre->ip) + 2
				+ strlen(dirnombre->remitente) + 1);

		memcpy(stream, &codigo, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, &pesoStream, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, dirnombre, aux_size = sizeof(int));

		offset += aux_size;
		aux_length = strlen(dirnombre->ip) + 1;
		memcpy(stream + offset, &aux_length, aux_size = 1);

		offset += aux_size;
		memcpy(stream + offset, dirnombre->ip,
				aux_size = strlen(dirnombre->ip) + 1);

		offset += aux_size;
		aux_length = strlen(dirnombre->remitente) + 1;
		memcpy(stream + offset, &aux_length, aux_size = 1);

		offset += aux_size;
		memcpy(stream + offset, dirnombre->remitente,
				aux_size = strlen(dirnombre->remitente) + 1);

		break;

	case 8:

		inicial = puntero;

		stream = malloc(3 * sizeof(int) + 1 + strlen(inicial->remitente) + 1);

		pesoStream = (3 * sizeof(int) + 1 + strlen(inicial->remitente) + 1);

		memcpy(stream, &codigo, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, &pesoStream, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, inicial, aux_size = sizeof(int));

		offset += aux_size;
		aux_length = strlen(inicial->remitente) + 1;
		memcpy(stream + offset, &aux_length, aux_size = 1);

		offset += aux_size;
		memcpy(stream + offset, inicial->remitente,
				aux_size = strlen(inicial->remitente) + 1);

		break;

	case 9:

		estado = puntero;

		stream = malloc(3 * sizeof(int) + sizeof(char));

		pesoStream = (3 * sizeof(int) + sizeof(char));

		memcpy(stream, &codigo, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, &pesoStream, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, estado, aux_size = sizeof(int) + sizeof(char));

		break;

	case 10:

		victimas = puntero;

		pesoStream = (2 * sizeof(int) + strlen(victimas->nivel) + 1
						+ strlen(victimas->personajes) + 1);

		stream = malloc(pesoStream);

		memcpy(stream, &codigo, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, &pesoStream, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, victimas->nivel,
				aux_size = strlen(victimas->nivel) + 1);

		offset += aux_size;
		memcpy(stream + offset, victimas->personajes,
				aux_size = strlen(victimas->personajes) + 1);

		break;

	case 11:

		recursosliberados = puntero;

		pesoStream = (3 * sizeof(int) + strlen(recursosliberados->recursos) + 1
				              + strlen(recursosliberados->nivel) + 1
				              + strlen(recursosliberados->personaje) +1);

		stream = malloc(pesoStream);

		memcpy(stream, &codigo, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, &pesoStream, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, recursosliberados->recursos,
		aux_size = strlen(recursosliberados->recursos) + 1);

		offset += aux_size;
		int numero = recursosliberados->total;
		memcpy(stream + offset, &numero, aux_size = sizeof(int));

		offset += aux_size;
		memcpy(stream + offset, recursosliberados->nivel,
		aux_size = strlen(recursosliberados->nivel) + 1);

		offset += aux_size;
		memcpy(stream + offset, recursosliberados->personaje,
		aux_size = strlen(recursosliberados->personaje) + 1);

		break;

	default:
		printf("error codigo\n");
		exit(1);

	}

	return stream;

}

void *deserializer(char *stream) //recibe el stream
{
	t_posicion *rec;
	t_direccion *dir;
	t_pedirDireccion *pedirD;
	t_pedirPosicionRecurso *pedirR;
	t_boolean *boolean;
	t_devolverRecursos *devolverRecursos;
	t_pedirRecurso *pedirRecurso;
	t_direccionNombre *dirnombre;
	t_personaje *inicial;
	t_estadoRecurso *estado;
	t_interbloqueo *victimas;
	t_recursosLiberados *recursosLiberados;

	void *puntero;
	int codigo;
	int offset = 0, aux_size = 0;

	if(stream == NULL)
		{
			printf("NULL pointer exception, se quiso deserializar un stream en null\n");
			return NULL;
		}

	memcpy(&codigo, stream, aux_size = sizeof(int));
	offset += aux_size;

	switch (codigo) {

	case 0:

		rec = malloc(sizeof(t_posicion));

		aux_size = sizeof(int); //saltea el campo de largo de stream

		offset += aux_size;

		memcpy(rec, stream + offset, aux_size = sizeof(int) + sizeof(int)); //luego copia el primer campo de la info util

		offset += aux_size + 1;

		rec->remitente = strdup(stream + offset);

		puntero = rec;

		break;

	case 1:

		dir = malloc(sizeof(t_direccion));

		aux_size = sizeof(int);

		offset += aux_size;

		memcpy(dir, stream + offset, aux_size = sizeof(int) + sizeof(int));

		offset += aux_size + 1;
		dir->ipNiv = strdup(stream + offset);

		offset += strlen(dir->ipNiv) + 2;
		dir->ipPlan = strdup(stream + offset);

		puntero = dir;

		break;

	case 2:

		pedirR = malloc(sizeof(t_pedirPosicionRecurso));

		aux_size = sizeof(int);

		offset += aux_size;

		memcpy(pedirR, stream + offset, aux_size = sizeof(char));

		offset += aux_size + 1;

		pedirR->remitente = strdup(stream + offset);

		puntero = pedirR;

		break;

	case 3:

		pedirD = malloc(sizeof(t_pedirDireccion));

		aux_size = sizeof(int);

		offset += aux_size + 1;

		pedirD->nombrenivel = strdup(stream + offset);

		puntero = pedirD;

		break;

	case 4:

		boolean = malloc(sizeof(t_boolean));

		aux_size = sizeof(int);

		offset += aux_size;

		memcpy(boolean, stream + offset, aux_size = sizeof(int));

		puntero = boolean;

		break;

	case 5:

		devolverRecursos = malloc(sizeof(t_devolverRecursos));

		aux_size = sizeof(int);

		offset += aux_size + 1;

		devolverRecursos->personaje = strdup(stream + offset);

		puntero = devolverRecursos;

		break;
	case 6:
		pedirRecurso = malloc(sizeof(t_pedirRecurso));

		aux_size = sizeof(int);

		offset += aux_size;

		memcpy(pedirRecurso, stream + offset, aux_size = sizeof(char));

		offset += aux_size + 1;

		pedirRecurso->remitente = strdup(stream + offset);

		puntero = pedirRecurso;

		break;

	case 7:
		dirnombre = malloc(sizeof(t_direccionNombre));

		aux_size = sizeof(int);

		offset += aux_size;

		memcpy(dirnombre, stream + offset, aux_size = sizeof(int));

		offset += aux_size + 1;
		dirnombre->ip = strdup(stream + offset);

		offset += strlen(dirnombre->ip) + 2;
		dirnombre->remitente = strdup(stream + offset);

		puntero = dirnombre;

		break;

	case 8:

		inicial = malloc(sizeof(t_personaje));

		aux_size = sizeof(int);

		offset += aux_size;

		memcpy(inicial, stream + offset, aux_size = sizeof(int));

		offset += aux_size + 1;

		inicial->remitente = strdup(stream + offset);

		puntero = inicial;

		break;

	case 9:

		estado = malloc(sizeof(t_estadoRecurso));

		aux_size = sizeof(int);

		offset += aux_size;

		memcpy(estado, stream + offset, aux_size = sizeof(int) + sizeof(char));

		puntero = estado;

		break;

	case 10:

		victimas = malloc(sizeof(t_interbloqueo));

		aux_size = sizeof(int);

		offset += aux_size;

		victimas->nivel = strdup(stream + offset);

		offset += strlen(victimas->nivel) + 1;

		victimas->personajes = strdup(stream + offset);

		puntero = victimas;

		break;

	case 11:

		recursosLiberados = malloc(sizeof(t_recursosLiberados));

		aux_size = sizeof(int);

		offset += aux_size;

		recursosLiberados->recursos = strdup(stream + offset);

		offset += strlen(recursosLiberados->recursos) + 1;

		recursosLiberados->total = *(stream + offset);

		offset += sizeof(int);

		recursosLiberados->nivel= strdup(stream + offset);

		offset += strlen(recursosLiberados->nivel) + 1;

		recursosLiberados->personaje = strdup(stream + offset);

		puntero = recursosLiberados;

		break;

	default:
		printf("error codigo\n");

	}

	return puntero;

}

void liberarVector(char** vector) {
	int i;
	for (i = 0; vector[i] != '\0'; i++) {
		free(vector[i]);
	}
}

