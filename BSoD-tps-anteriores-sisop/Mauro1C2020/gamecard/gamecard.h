#ifndef GAMECARD_H_
#define GAMECARD_H_

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
#include "bitmap.h"

pthread_t thread0;
pthread_t threadGameBoy,threadGameCardId;

char *archivoConfiguracion;
char *ipBroker;
char *puertoBroker;
int gamecardId;
int conexionBroker;
int tiempoIntentoReconexion;
int tiempo_reintento_operacion;
t_log* log_gamecard;
t_config* gamecardConfig;

t_bitarray *bitmap;
sem_t mutex_new_pokemon;
sem_t mutex_recibirId;

// Parámetros de configuración
//char *puntoMontaje;
char* ip;
char* puerto;
int servidorGamecard;
char* archivoDeConfiguracion;
int blocks;
int block_size;
char* magic_number;
bool reconectandoBroker;

// Parámetros de montaje

char *puntoMontaje;
char *dirMetadata;
char *dirFiles;
char *dirBlocks;
char *dirPokemon;

void *gestionarConexionConGameboy(void *socket);
bool agregarPokemon(t_list* lista);

int suscribirColaNewPokemon();
int suscribirColaCatchPokemon();
int suscribirColaGetPokemon();

void *gestionarNewPokemon(void *socket);
void *gestionarCatchPokemon(void *socket);
void *gestionarGetPokemon(void *socket);

void crearEstructuraFileSystem();
bool actualizarBloques(char * rutaDeMetadata, t_list * lista);
void actualizarPosicionLista(t_list* lista, int posicion_x, int posicion_y, int cantidad);
void cargarPuntoMontaje();

void iniciarGamecard();

void crearEstructuraFileSystem();

#endif /* GAMECARD_H_ */
