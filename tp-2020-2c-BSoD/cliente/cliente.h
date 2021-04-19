#ifndef CLIENTE_H
#define CLIENTE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utils/utils.h>
#include <commons/config.h>
#include <readline/readline.h>

char* ip;
char* puerto;
char* puerto_app;
char* archivo_log;
int posicion_x;
int posicion_y;
char* id_cliente;
char* puerto_escucha;

int socket_conexion;
int app_socket;

t_log* logger;

int servidor;

//t_log* logger;

void* leerOpcion(int opcion);
void* gestionarMenuApp();
void* gestionarMenuComanda();
void* gestionarMenuRestaurante();
void* gestionarMenuSindicato();
void* gestionarMenu(int modulo_conectado);

#endif
