#ifndef BITMAP_H_
#define BITMAP_H_

#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <utils/utils.h>
#include <commons/bitarray.h>
#include <commons/config.h>

t_log* log_gamecard;

/*
 * Crea el bitmap desde cero de acuerdo a la ubicacion
 * y la cantidad de bloques que se le pasa por
 * parametro.
 */
t_bitarray* crear_bitmap(char *ubicacion, int cant_bloques);

/*
 * Carga un bitmap de acuerdo al archivo que se le pasa
 * por parametro.
 */
// t_bitarray cargar_bitmap(char *ruta_bitmap); --> IMPLEMENTAR

/*
 * Libera un bloque de acuerdo a la posicion que se le
 * pasa por parametro,
 *
 * En caso de que ocurra un error, devuelve un -1.
 */
void liberar_bloque(t_bitarray* bitmap, int bloque);

/*
 * Coloca un bloque como ocupado de acuerdo a la posicion
 * que se le pasa por parametro.
 *
 * En caso de que ocurra un error, devuelve -1.
 */
void ocupar_bloque(t_bitarray* bitmap, int bloque);

/*
 * Recorre el bitmap hasta encontrar un bloque libre y lo
 * devuelve.
 *
 * En caso de que ocurra un error, devuelve -1.
 */
int obtener_bloque_libre(t_bitarray* bitmap);

void *mostrarElementos(t_bitarray *bitmap);

#endif /* BITMAP_H_ */
