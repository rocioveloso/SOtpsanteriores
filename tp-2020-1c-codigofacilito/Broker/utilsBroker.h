/*
 * conexiones.h
 *
 *  Created on: 28 abr. 2020
 *      Author: codigofacilito
 */

#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<commons/config.h>
#include<string.h>
#include<pthread.h>






typedef enum
{
	MENSAJE=1,
	EMPLEADO=2,
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

typedef struct
{
	int dni;
	int len_nombre;
	char* nombre;
	int len_apellido;
	char* apellido;
	int edad;
} t_empleado;

//LOGGER
t_log* iniciar_logger(void);
t_config* leer_config(void);
void terminar_programa(int, t_log*, t_config*);
//FIN LOGGER

int iniciar_servidor(char *ip, char* puerto);



#endif /* CONEXIONES_H_ */
