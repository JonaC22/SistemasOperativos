/*
 * senales.c
 *
 *  Created on: May 22, 2013
 *      Author: juan
 */

#include <stdio.h>
#include "pedidos.h"
#include "senales.h"
#include <src/commons/config.h>
#include <semaphore.h>

void handlerSenales(int signal)
{

	extern int vidas;
	extern int fueMatadoPorSenal;


	switch(signal)
	{
		case SIGUSR1:
			vidas++;
			printf("Se incrementó una vida, ahora tenés: %d \n",vidas);
		break;

		case SIGTERM:
			fueMatadoPorSenal = 1;
			printf("\n\n\n  Moriste.\n\n\n\n");
		break;
	}

}
