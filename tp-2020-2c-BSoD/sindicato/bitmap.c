#include "bitmap.h"

t_bitarray* crear_bitmap(char *ubicacion, int cant_bloques){

	log_gamecard = log_create("log_gamecard.log", "gamecard", 1, LOG_LEVEL_TRACE);

	size_t size = (size_t) cant_bloques / 8;
	//printf("\nSize = %d\n", size);
	char *rutaBitmap = malloc(strlen(ubicacion) + 20);
	strcpy(rutaBitmap, ubicacion);
	strcat(rutaBitmap, "/Bitmap.bin");

	int fd = open(rutaBitmap, O_CREAT | O_RDWR, 0777);

	if (fd == -1) {
		log_error(log_gamecard, "Error al abrir el archivo Bitmap.bin");
		exit(1);
	}
	ftruncate(fd, size);

	void* bmap = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (bmap == MAP_FAILED) {
		close(fd);
		exit(1);
	}

	t_bitarray* bitmap = bitarray_create_with_mode((char*) bmap, size, MSB_FIRST);


	msync(bitmap, size, MS_SYNC);
	free(rutaBitmap);
	return bitmap;
}

int obtener_bloque_libre(t_bitarray* bitmap) {
	size_t tamanio = bitarray_get_max_bit(bitmap);

	int i;
	for(i=0; i<tamanio; i++) {
		if(bitarray_test_bit(bitmap, i) == 0) {
			return i;
		}
	}
	return -1;
}

void ocupar_bloque(t_bitarray* bitmap, int bloque) {
	bitarray_set_bit(bitmap, bloque);
	return;
}

void liberar_bloque(t_bitarray* bitmap, int bloque) {
	bitarray_clean_bit(bitmap, bloque);
	return;
}

void *mostrarElementos(t_bitarray *bitmap) {
	size_t tamanio = bitarray_get_max_bit(bitmap);

	int i;
	for(i=0; i<tamanio; i++) {
		printf("\nPosicion: %d, Valor: %d", i, bitarray_test_bit(bitmap, i));
	}
	return NULL;
};

/*
t_bitarray cargar_bitmap(char *ruta_bitmap) {

};
*/
