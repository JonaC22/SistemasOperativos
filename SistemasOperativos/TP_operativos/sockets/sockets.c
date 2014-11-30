/*
 * libsockets.c
 *
 *  Created on: Apr 24, 2013
 *      Author: juan
 */

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "sockets.h"

#define BUFF_SIZE 1024

//Parte de serialización

char buffer_in[BUFF_SIZE];
char buffer_out[BUFF_SIZE];
//char *stream;
int medida;
//t_log* logger;

int crear_socket_servidor(char* ip, int puerto) {

	int socketEscucha;

	socketEscucha = crear_socket();
	// t_log* logger;
	//logger = log_create("/home/utnso/logueo/debug.log", "TEST",true, LOG_LEVEL_ERROR);

	struct sockaddr_in socketInfo;
	int optval = 1;

	// Hacer que el SO libere el puerto inmediatamente luego de cerrar el socket.
	setsockopt(socketEscucha, SOL_SOCKET, SO_REUSEADDR, &optval,
			sizeof(optval));

	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = inet_addr(ip); //Notar que aca no se usa inet_addr()
	socketInfo.sin_port = htons(puerto);

	// Vincular el socket con una direccion de red almacenada en 'socketInfo'.
	if (bind(socketEscucha, (struct sockaddr*) &socketInfo, sizeof(socketInfo))
			!= 0) {

		//	log_error(logger,"Error al bindear el socket %s", "servidor");
		return -1;
	}

	// Escuchar nuevas conexiones entrantes.
	if (listen(socketEscucha, 10) != 0) {

		//	log_error(logger,"Error al poner a escuchar el socket %s", "servidor");
		return -1;

	}

	printf("Escuchando conexiones entrantes.\n");

	return socketEscucha;

}

int conectar_socket(char* ip, int puerto)
{
	int unSocket = crear_socket(ip, puerto);

	struct sockaddr_in socketInfo;

	printf("Conectando...\n");

	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = inet_addr(ip);
	socketInfo.sin_port = htons(puerto);

	// Conectar el socket con la direccion 'socketInfo'.
	if (connect(unSocket, (struct sockaddr*) &socketInfo, sizeof(socketInfo))
			!= 0) {
		perror("Error al conectar socket");
		return NULL;
	}

	printf("Conectado!\n");
	return unSocket;
}

int cerrar_socket(int unSocket) {
	return close(unSocket);
}

int crear_socket() {
	int unSocket;
	if ((unSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error al crear socket");
		return 0;
	}

	return unSocket;
}

//enviar_mensaje y recibir_mensaje son solo para el cliente.
int enviar_mensaje(int unSocket, char *stream)
{

	// Enviar los datos pasados por parametro a traves del socket.
	int nbytes;
	int cod;

	memcpy(&cod, stream, sizeof(uint8_t));
	memcpy(&medida, stream + sizeof(uint32_t), sizeof(uint32_t));
	memcpy(&buffer_out, stream, medida);

	// Enviar los datos leidos por teclado a traves del socket.

	if ((nbytes = send(unSocket, buffer_out, medida, MSG_NOSIGNAL)) < 0)
	{
		printf("Socket %d. ",unSocket);
		perror("Error al enviar datos. Server no encontrado.\n");
		exit(1);
	}

	return nbytes;
}

//char* recibir_mensaje(int unSocket, char* buffer, int tamanioBuffer)

typedef struct {
	int type;
	int length;
} __attribute__ ((__packed__)) header_t ;

char* recibir_mensaje(int unSocket)
{

	header_t *header = malloc(sizeof(header_t));

	if (recv(unSocket, header, sizeof(header_t),MSG_WAITALL) > 0)
	{
		memcpy(buffer_in, header, sizeof(header_t));
		if ((recv(unSocket, buffer_in + sizeof(header_t),(header->length - sizeof(header_t)), MSG_WAITALL)) > 0)
		{
			free(header);
			return buffer_in;
		}
		return NULL;
	}
	else
	{
		free(header);
		return NULL;
	}
}

t_socket_random* crear_servidor_random(char* ip) {


	t_socket_random *socketRandom = malloc(sizeof(t_socket_random));

	int socketEscucha;
	int puerto=7000;
	socketEscucha = crear_socket();

	struct sockaddr_in socketInfo;
	int optval = 1;

	// Hacer que el SO libere el puerto inmediatamente luego de cerrar el socket.
	setsockopt(socketEscucha, SOL_SOCKET, SO_REUSEADDR, &optval,
			sizeof(optval));

	socketRandom->socket=socketEscucha;

	socketInfo.sin_family = AF_INET;
	socketInfo.sin_addr.s_addr = inet_addr(ip);
	socketInfo.sin_port = htons(puerto);

	// Vincular el socket con una direccion de red almacenada en 'socketInfo'.
	while (bind(socketEscucha, (struct sockaddr*) &socketInfo, sizeof(socketInfo))!= 0)
	{

		puerto++;
		socketInfo.sin_port= htons(puerto);
	}
	socketRandom->puerto=socketInfo.sin_port= puerto;
	// Escuchar nuevas conexiones entrantes.
	if (listen(socketEscucha, 10) != 0) {

		//	log_error(logger,"Error al poner a escuchar el socket %s", "servidor");
		socketRandom->socket=-1;
		return socketRandom;
	}

	printf("Escuchando conexiones entrantes.\n");

	return socketRandom;

}

