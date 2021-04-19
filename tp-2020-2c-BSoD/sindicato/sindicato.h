#ifndef SINDICATO_H_
#define SINDICATO_H_

#include <stdio.h>
#include <stdlib.h>
#include <utils/utils.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <readline/readline.h>

char* ARCHIVO_CONFIG = "sindicato.config";
pthread_t thread0;
void sindicato_server();
char *archivoConfiguracion;

t_config* sindicatoConfig;

t_bitarray *bitmap;
sem_t mutex_recibirId;


// Parámetros de configuración
char* ip;
char* puerto;
int servidorSindicato;
char* archivoDeConfiguracion;
int blocks;
int block_size;
char* magic_number;
bool reconectandoBroker;

t_log* logger;

// Parámetros de montaje

char *puntoMontaje;
char *dirMetadata;
char *dirFiles;
char *dirBlocks;
bool actualizarBloques(char * rutaDeMetadata, t_list * lista);
void actualizarPosicionLista(t_list* lista, int posicion_x, int posicion_y, int cantidad);
void crearEstructuraFileSystem();
void obtenerReceta(int restaurante_socket, char* nombre_restaurante, char* nombre_plato);
void* abrirConsola();
//t_restaurante_metadata obtenerRestaurante();
#endif /* SINDICATO_H_ */
