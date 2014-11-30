/*
 * sockets.h
 *
 *  Created on: Apr 26, 2013
 *      Author: juan
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

typedef struct {
	int length;
	char *data;
} t_stream;

typedef struct {
	int puerto;
	int socket;
} t_socket_random;

struct direccion separ_ip (char*);

int conectar_socket(char* ip, int puerto);
int crear_socket_servidor(char* ip, int puerto);
int aceptar_conexion(int socketEscucha);
int atender_conexion(int socketEscucha, int socketNuevaConexion);
int crear_socket();
int enviar_mensaje(int unSocket, char *stream);
char* recibir_mensaje(int unSocket);
int cerrar_socket(int unSocket);
t_socket_random* crear_servidor_random(char* ip);


#endif /* SOCKETS_H_ */
