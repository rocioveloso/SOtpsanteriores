#ifndef GAMEBOY_H_
#define GAMEBOY_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>

#include<pthread.h>




t_log* iniciar_logger(void);
t_config* leer_config(void);
void terminar_programa(int t_log*, int t_config*);


// hilo temporizador

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int pthread_mutex_lock(pthread_mutex_t * mutex);
int pthread_mutex_unlock(pthread_mutex_t * mutex);

int pthread_mutex_lock(pthread_mutex_t * mutex_salir_cargar_en_cola);
int pthread_mutex_unlock(pthread_mutex_t * mutex_salir_cargar_en_cola);


#endif
