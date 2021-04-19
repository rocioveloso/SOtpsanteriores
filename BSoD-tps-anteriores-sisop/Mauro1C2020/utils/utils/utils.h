#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<readline/readline.h>
#include<commons/config.h>
#include<commons/log.h>
#include<commons/collections/list.h>


typedef enum
{
    BROKER, TEAM, GAMECARD,
    ENLAZAR, REENLAZADO, DESENLAZAR, SUSCRIBIR, SUSCRIPTOR, SUSCRIPTOR_ID, MENSAJE_ID, RECIBIDO,
	NEW_POKEMON, APPEARED_POKEMON, CATCH_POKEMON, CAUGHT_POKEMON, GET_POKEMON, LOCALIZED_POKEMON
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

char *leer_config(char* campo, char* archivo);
int crear_conexion(char* ip, char* puerto);
void liberar_cliente(int socket_cliente);
void liberar_servidor(int socket_servidor);
//
t_paquete* crear_paquete(op_code operacion);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
//
t_list* recibir_paquete(int);
int recibir_operacion(int);
//
void* recibir_buffer(int*, int);
int iniciar_servidor(char* IP, char* PUERTO);
int esperar_cliente(int);
uint32_t recibir_numero(int socket_cliente);
#endif /* UTILS_H_ */
