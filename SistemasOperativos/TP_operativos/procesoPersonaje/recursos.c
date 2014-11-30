/*
 * recursos.c
 *
 *  Created on: May 16, 2013
 *      Author: juan
 */
#include "pedidos.h"
#include <malloc.h>
#include <src/commons/config.h>


char siguienteRecurso(int cantidadNivelesTerminados, int cantidadRecursosConseguidos, t_config* configMario)
{
	//Obtengo el plan de niveles del personaje
	//char recurso;
	char **planDeNiveles;
	planDeNiveles = config_get_array_value(configMario,"planDeNiveles");

	//Obtengo los objetivos de ese nivel
	char **objetivosNivel;
	char* clave= string_from_format("obj[%s]",*(planDeNiveles+cantidadNivelesTerminados));
	objetivosNivel = config_get_array_value(configMario,clave);

	//Calculo el recurso por el que voy
	char recurso = objetivosNivel[cantidadRecursosConseguidos][0];
	liberarVector(objetivosNivel);
	free(objetivosNivel);
	free(clave);
	liberarVector(planDeNiveles);
	free(planDeNiveles);
	return recurso;
}


void avanzarHaciaElRecurso( t_posicion *posicionActual, t_posicion *posicionRecurso)
{
	if(posicionActual->x != posicionRecurso->x)
	{
		if(posicionActual->x < posicionRecurso->x)
		{
			 posicionActual->x++;
		}
		else
		{
			 posicionActual->x--;
		}
	}
	else
	{
	if(posicionActual->y != posicionRecurso->y)
	{
		if(posicionActual->y < posicionRecurso->y)
		{
			 posicionActual->y++;
		}
		else
		{
			 posicionActual->y--;
		}
	}
	}
}
