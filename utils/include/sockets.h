#ifndef UTILS_SOCKETS_H_
#define UTILS_SOCKETS_H_

#include <stdlib.h>
#include <stdio.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>

#include <signal.h>

//t_log* logger;
/**
* @fn    decir_hola
* @brief Imprime un saludo al nombre que se pase por parámetro por consola.
*/
void decir_hola(char* quien);

int iniciar_servidor(t_log*,char*, char*);
int esperar_cliente(int,t_log*,char*);
int crear_conexion(char* , char*,t_log*,char*);
void liberar_conexion(int);

#endif
