/*
 * recursos.h
 *
 *  Created on: May 16, 2013
 *      Author: juan
 */

#ifndef RECURSOS_H_
#define RECURSOS_H_

char siguienteRecurso(int cantidadNivelesTerminados, int cantidadRecursosConseguidos, t_config* configMario);
t_posicion *avanzarHaciaElRecurso( t_posicion *posicionActual, t_posicion *posicionRecurso);


#endif /* RECURSOS_H_ */
