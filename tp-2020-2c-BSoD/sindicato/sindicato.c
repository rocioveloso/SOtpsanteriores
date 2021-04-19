#include "sindicato.h"


int main(int t, char * arg[]){

	printf("sindicato:\n");
//	t_config* sindicatoConfig;
	sindicatoConfig = config_create("sindicato.config");
	ip = config_get_string_value(sindicatoConfig, "IP");
	puerto = config_get_string_value(sindicatoConfig, "PUERTO");

	logger = log_create("sindicato.log", "Sindicato", 1, LOG_LEVEL_DEBUG);

	servidorSindicato = iniciar_servidor(ip, puerto);

	crearEstructuraFileSystem();
	abrirConsola();
	sindicato_server();
	return 0;
}
void crearEstructuraFileSystem()
{
	FILE *f;

	puntoMontaje = config_get_string_value(sindicatoConfig,"PUNTO_MONTAJE");
	dirMetadata = malloc(strlen(puntoMontaje) + strlen("/Metadata") + 1);
	strcpy(dirMetadata, puntoMontaje);
	strcat(dirMetadata, "/Metadata");

	dirFiles = malloc(strlen(puntoMontaje) + strlen("/Files") + 1);
	strcpy(dirFiles, puntoMontaje);
	strcat(dirFiles, "/Files");

	dirBlocks = malloc(strlen(puntoMontaje) + strlen("/Blocks") + 1);
	strcpy(dirBlocks, puntoMontaje);
	strcat(dirBlocks, "/Blocks");

	char *metadataRuta = malloc(strlen(dirMetadata) + strlen("/Metadata.AFIP") + 1); // /Metadata.bin  /Bitmap.bin
	strcpy(metadataRuta, dirMetadata);
	strcat(metadataRuta, "/Metadata.AFIP"); ///Metadata.bin  /Bitmap.bin


	if(mkdir(puntoMontaje, 0777) != 0)
	{
		printf("El directorio de montaje ya existe!! =( \n");
		/*
		//Si el fileSystem esta creado se toman los datos de la metadata existente.
		log_info(logger, "El directorio %s ya existe. ", puntoMontaje);
		t_config* metadataConfig=config_create(metadataRuta);

		block_size=atoi(config_get_string_value(metadataConfig,"BLOCK_SIZE"));
		blocks=atoi(config_get_string_value(metadataConfig,"BLOCKS"));
		magic_number=malloc(20);
		strcpy(magic_number,(char*)config_get_string_value(metadataConfig,"MAGIC_NUMBER"));

		config_destroy(metadataConfig);

		log_info(logger,"BLOCK_SIZE: %d. BLOCKS: %d. MAGIC_NUMBER: %s. ", block_size, blocks, magic_number);
		bitmap = crear_bitmap(dirMetadata, blocks);
		free(metadataRuta);
		*/
		return;
	}
	else
	{
		printf("Se creo el directorio de montaje =) \n");
		/*
		//Si el fileSystem NO esta creado se toman los datos del archivo de configuracion.

		block_size=atoi(config_get_string_value(sindicatoConfig,"BLOCK_SIZE"));
		blocks=atoi(config_get_string_value(sindicatoConfig,"BLOCKS"));
		magic_number=malloc(20);
		strcpy(magic_number,(char*)config_get_string_value(sindicatoConfig,"MAGIC_NUMBER"));

		log_info(logger,"BLOCK_SIZE: %d. BLOCKS: %d. MAGIC_NUMBER: %s. ", block_size, blocks, magic_number);

		// Directorio Metadata
		if(mkdir(dirMetadata, 0777) == 0)
		{
			// Creo el archivo Metadata.bin
			f = fopen(metadataRuta, "w");

			char* tamanioBloque = malloc(10); sprintf(tamanioBloque, "%d",block_size);
			fputs("BLOCK_SIZE=", f); fputs(tamanioBloque,f); fputs("\n",f);

			char* bloques = malloc(10);
			sprintf(bloques, "%d",block_size);
			fputs("BLOCKS=", f);fputs(bloques,f); fputs("\n",f);

			fputs("MAGIC_NUMBER=", f); fputs(magic_number, f); fputs("\n",f);

			fclose(f);
			free(metadataRuta);
			free(tamanioBloque);
			free(bloques);
			bitmap = crear_bitmap(dirMetadata,blocks);

		}
		else
		{
			log_error(logger, "crearEstructuraFileSystem: No se pudo crear el directorio Metadata");
			free(metadataRuta);
			return;
		}

		// Directorio Files

		if(mkdir(dirFiles, 0777) == 0)
		{
			// Creo archivo Metadata.bin
			char* metadataRuta = malloc(strlen(dirFiles) + strlen("/Metadata.AFIP") + 1); // /Metadata.bin o Bitmap.bin
			strcpy(metadataRuta, dirFiles);
			strcat(metadataRuta, "/Metadata.AFIP"); // /Metadata.bin o Bitmap.bin
			// Creo el archivo Metadata.bin (para indicar que es un directorio)
			f = fopen(metadataRuta, "w");
			fputs("DIRECTORY=Y", f);
			fclose(f);
			free(metadataRuta);
		}
		else
		{
			log_error(logger, "Ha ocurrido un error al crear el directorio Files.");
			free(metadataRuta);
			return;
		}

		// Directorio Blocks

		if(mkdir(dirBlocks, 0777) == 0) {crearBloques(dirBlocks);}
		else {
			log_error(logger, "Ha ocurrido un error al crear el directorio Blocks.");
			return;
		}

		log_trace(logger, "Estructura creada.");
		*/
	}
}
void crearBloques(char *dirBlocks)
{
	FILE *f;
	char *rutaArchivo = malloc(strlen(dirBlocks) + 20);
	for(int i = 0; i < blocks; i++) {
		char* n = malloc(5);
		sprintf(n, "%d", i);
		strcpy(rutaArchivo, dirBlocks);
		strcat(rutaArchivo, "/");
		strcat(rutaArchivo, n);
		strcat(rutaArchivo, ".bin");
		f = fopen(rutaArchivo, "w");
		fclose(f);
		free(n);
	}

	free(rutaArchivo);
}
//void *gestionarSindicato(void *socketServidor) {
//
//	t_list* lista;
//	int operacion;
//
//	while(1) {
//		int sindicato = esperar_cliente(*((int*) socketServidor));
//		operacion = recibir_operacion(sindicato);
//
//		switch(operacion) {
//			    case CREAR_RESTAURANTE:
//			    printf("Mensaje de crear restaurante.\n");
//			    case OBTENER_RESTAURANTE:
//			    	//obtenerRestaurante();
//				case GUARDAR_PEDIDO:
//				printf("Mensaje de GUARDAR PEDIDO.\n");
//				lista = recibir_paquete(sindicato);
//				printf("Recibi paquete\n");
//				// El primer objeto es un int
//				printf("Nombre: %d\n", *(int*)list_get(lista, 0));
//				// El segundo objeto es una cadena
//				printf("Nombre: %s\n", (char*)list_get(lista, 1));
//				break;
//
//			case CONSULTAR_PLATO:
//			    printf("Mensaje de CONSULTAR PLATO.\n");
//			    break;
//			case GUARDAR_PLATO:
//				printf("Mensaje de GUARDAR PLATO.\n");
//				break;
//
//			case OBTENER_PEDIDO:
//				printf("Mensaje de OBTENER PEDIDO.\n");
//				break;
//
//			case CONFIRMAR_PEDIDO:
//				printf("Mensaje de CONFIRMAR PEDIDO.\n");
//				break;
//
//			case PLATO_LISTO:
//				printf("Mensaje de PLATO LISTO.\n");
//				break;
//
//			case FINALIZAR_PEDIDO:
//				printf("Mensaje de FINALIZAR PEDIDO.\n");
//				break;
//
//			case -1:
//				printf("El cliente %d se desconecto.\n", sindicato);
//				liberar_cliente(sindicato);
//				return EXIT_FAILURE;
//
//			default:
//				printf("Operacion desconocida.\n");
//				break;
//
//		}
//	}
//}

void obtenerRestaurante(int restaurante_socket, char* nombre_restaurante) {

	t_paquete* paquete = crear_paquete(OBTENER_RESTAURANTE);

	int cant_afinidades = 1;
	agregar_a_paquete(paquete, &cant_afinidades, sizeof(int));

    char* afinidad = malloc(strlen("afinidad1") + 1);
    memcpy(afinidad, "afinidad1", strlen("afinidad1") + 1);
    agregar_a_paquete(paquete, afinidad, strlen("afinidad1") + 1);

	int posicion_x = 10;
	agregar_a_paquete(paquete, &posicion_x, sizeof(int));

	int posicion_y = 5;
	agregar_a_paquete(paquete, &posicion_y, sizeof(int));

	int cant_platos = 1;
	agregar_a_paquete(paquete, &cant_platos, sizeof(int));

    char* plato = malloc(strlen("milanesa") + 1);
    memcpy(plato, "milanesa", strlen("milanesa") + 1);
    agregar_a_paquete(paquete, plato, strlen("milanesa") + 1);

	int precio = 250;
	agregar_a_paquete(paquete, &precio, sizeof(int));

	int cant_hornos = 1;
	agregar_a_paquete(paquete, &cant_hornos, sizeof(int));

	int cant_pedidos = 1;
	agregar_a_paquete(paquete, &cant_pedidos, sizeof(int));

	int cant_cocineros = 3;
	agregar_a_paquete(paquete, &cant_cocineros, sizeof(int));

	enviar_paquete(paquete, restaurante_socket);

	liberar_cliente(restaurante_socket);
    eliminar_paquete(paquete);
    free(afinidad);
    free(plato);
}

void consultarPlatos(int restaurante_socket, char* nombre_restaurante) {
	char* plato;
	int precio;

	t_paquete* paquete = crear_paquete(CONSULTAR_PLATOS);

    char* nombre = malloc(strlen(nombre_restaurante) + 1);
    memcpy(nombre, nombre_restaurante, strlen(nombre_restaurante) + 1);
    agregar_a_paquete(paquete, nombre, strlen(nombre_restaurante) + 1);

	int cant_platos = 3;
	agregar_a_paquete(paquete, &cant_platos, sizeof(int));

    plato = malloc(strlen("milanesa") + 1);
    memcpy(plato, "milanesa", strlen("milanesa") + 1);
    agregar_a_paquete(paquete, plato, strlen("milanesa") + 1);

	precio = 250;
	agregar_a_paquete(paquete, &precio, sizeof(int));

    plato = malloc(strlen("hamburguesa") + 1);
    memcpy(plato, "hamburguesa", strlen("hamburguesa") + 1);
    agregar_a_paquete(paquete, plato, strlen("hamburguesa") + 1);

	precio = 350;
	agregar_a_paquete(paquete, &precio, sizeof(int));

    plato = malloc(strlen("pollo al disco") + 1);
    memcpy(plato, "pollo al disco", strlen("pollo al disco") + 1);
    agregar_a_paquete(paquete, plato, strlen("pollo al disco") + 1);

	precio = 450;
	agregar_a_paquete(paquete, &precio, sizeof(int));

	enviar_paquete(paquete, restaurante_socket);

	liberar_cliente(restaurante_socket);
    eliminar_paquete(paquete);
    free(nombre);
    free(plato);
}

void guardarPedido(int restaurante_socket, char* nombre_restaurante, int id_pedido) {
	t_paquete* paquete = crear_paquete(GUARDAR_PLATO);

    char* nombre = malloc(strlen(nombre_restaurante) + 1);
    memcpy(nombre, nombre_restaurante, strlen(nombre_restaurante) + 1);
    agregar_a_paquete(paquete, nombre, strlen(nombre_restaurante) + 1);

	agregar_a_paquete(paquete, &id_pedido, sizeof(int));

	enviar_paquete(paquete, restaurante_socket);

	liberar_cliente(restaurante_socket);
    eliminar_paquete(paquete);
	free(nombre);
}

void guardarPlato(int restaurante_socket, char* nombre_restaurante, int id_pedido, char* nombre_plato, int cantidad) {
	t_paquete* paquete = crear_paquete(GUARDAR_PLATO);

	int check = MSG_OK;
	agregar_a_paquete(paquete, &check, sizeof(int));
	enviar_paquete(paquete, restaurante_socket);

	liberar_cliente(restaurante_socket);
    eliminar_paquete(paquete);
}

void obtenerPedido(int restaurante_socket, char* nombre_restaurante, int id_pedido) {
	t_paquete* paquete = crear_paquete(GUARDAR_PLATO);

	int estado = PENDIENTE;
	agregar_a_paquete(paquete, &estado, sizeof(int));

	int cantidad_platos = 2;
	agregar_a_paquete(paquete, &cantidad_platos, sizeof(int));

	// plato 1
    char* nombre = malloc(strlen("hamburguesa") + 1);
    memcpy(nombre, "hamburguesa", strlen("hamburguesa") + 1);
    agregar_a_paquete(paquete, nombre, strlen("hamburguesa") + 1);

	int cantidad_total = 5;
	agregar_a_paquete(paquete, &cantidad_total, sizeof(int));

	int cantidad_lista = 2;
	agregar_a_paquete(paquete, &cantidad_lista, sizeof(int));

	// plato 2
    nombre = malloc(strlen("milanesa") + 1);
    memcpy(nombre, "milanesa", strlen("milanesa") + 1);
    agregar_a_paquete(paquete, nombre, strlen("milanesa") + 1);

	cantidad_total = 1;
	agregar_a_paquete(paquete, &cantidad_total, sizeof(int));

	cantidad_lista = 0;
	agregar_a_paquete(paquete, &cantidad_lista, sizeof(int));
	enviar_paquete(paquete, restaurante_socket);

	liberar_cliente(restaurante_socket);
    eliminar_paquete(paquete);
}

void confirmarPedido(int restaurante_socket, char* nombre_restaurante, int id_pedido) {
	t_paquete* paquete = crear_paquete(CONFIRMAR_PEDIDO);

	int check = MSG_OK;
	printf("check=%d\n", check);
	agregar_a_paquete(paquete, &check, sizeof(int));

	enviar_paquete(paquete, restaurante_socket);

	liberar_cliente(restaurante_socket);
    eliminar_paquete(paquete);
}

void obtenerReceta(int restaurante_socket, char* nombre_restaurante, char* nombre_plato) {
	char* nombre;
	char* operacion;
	int t;

	t_paquete* paquete = crear_paquete(OBTENER_RECETA);

	int cantidad_operaciones = 4;
	agregar_a_paquete(paquete, &cantidad_operaciones, sizeof(int));

	// op 1

	operacion = "Trocear";
	nombre = malloc(strlen(operacion) + 1);
	memcpy(nombre, operacion, strlen(operacion) + 1);
	agregar_a_paquete(paquete, nombre, strlen(operacion) + 1);

	t = 4;
	agregar_a_paquete(paquete, &t, sizeof(int));

	// op 2

	operacion = "Trocear";
	nombre = malloc(strlen(operacion) + 1);
	memcpy(nombre, operacion, strlen(operacion) + 1);
	agregar_a_paquete(paquete, nombre, strlen(operacion) + 1);

	t = 5;
	agregar_a_paquete(paquete, &t, sizeof(int));

	// op 3

	operacion = "Reposar";
	nombre = malloc(strlen(operacion) + 1);
	memcpy(nombre, operacion, strlen(operacion) + 1);
	agregar_a_paquete(paquete, nombre, strlen(operacion) + 1);

	t = 3;
	agregar_a_paquete(paquete, &t, sizeof(int));

	// op 4

	operacion = "Hornear";
	nombre = malloc(strlen(operacion) + 1);
	memcpy(nombre, operacion, strlen(operacion) + 1);
	agregar_a_paquete(paquete, nombre, strlen(operacion) + 1);

	t = 10;
	agregar_a_paquete(paquete, &t, sizeof(int));

	enviar_paquete(paquete, restaurante_socket);

	liberar_cliente(restaurante_socket);
    eliminar_paquete(paquete);
}

void sindicato_server() {
    char* puerto_escucha = leer_config("PUERTO_ESCUCHA", ARCHIVO_CONFIG);
    char* ip = leer_config("IP", ARCHIVO_CONFIG);
    printf("Iniciando server en %s\n", puerto_escucha);
    int servidor_socket = iniciar_servidor(ip, puerto_escucha);
    t_list* lista;
    int operacion;

    while(1) {
        int cliente_socket = esperar_cliente(servidor_socket);
        operacion = recibir_operacion(cliente_socket);
        printf("op=%d\n", operacion);

        switch(operacion) {
        case INICIAR_CLIENTE:
        	printf("\n ENTRA EN INICIAR CLIENTE.\n");
        	lista = recibir_paquete(cliente_socket);
//        					printf("Conexion con Cliente...\n");
//        					printf("Los datos del cliente son:\n");
//        					printf("ID: %s\n", (char*) list_get(lista,0));
//        					printf("Posicion X: %d\n", *(int*) list_get(lista,1));
//        					printf("Posicion Y: %d\n", *(int*) list_get(lista,2));
        					t_paquete *paquete;
        					paquete = crear_paquete(SINDICATO);
        					enviar_paquete(paquete, cliente_socket);
        					eliminar_paquete(paquete);
        					liberar_cliente(cliente_socket);
        					break;
        case CREAR_RESTAURANTE:
     			    printf("Mensaje de crear restaurante.\n");
     			    break;
     			    case OBTENER_RESTAURANTE:
     			    	lista = recibir_paquete(cliente_socket);
    			    	obtenerRestaurante(cliente_socket, (char*)list_get(lista, 0));
     			    break;
     				case GUARDAR_PEDIDO:
     				printf("Mensaje de GUARDAR PEDIDO.\n");
//     				lista = recibir_paquete(sindicato);
     				printf("Recibi paquete\n");
     				// El primer objeto es un int
     				printf("Nombre: %d\n", *(int*)list_get(lista, 0));
     				// El segundo objeto es una cadena
     				printf("Nombre: %s\n", (char*)list_get(lista, 1));
					lista = recibir_paquete(cliente_socket);
					guardarPedido(cliente_socket, (char*)list_get(lista, 0), *(int*)list_get(lista, 1));
     				break;
				case CONSULTAR_PLATOS:
					lista = recibir_paquete(cliente_socket);
					consultarPlatos(cliente_socket, (char*)list_get(lista, 0));
					break;
     			case CONSULTAR_PLATO:
     			    printf("Mensaje de CONSULTAR PLATO.\n");
     			    break;
     			case GUARDAR_PLATO:
     				printf("Mensaje de GUARDAR PLATO.\n");
					lista = recibir_paquete(cliente_socket);
					guardarPlato(cliente_socket, (char*)list_get(lista, 0), *(int*)list_get(lista, 1), (char*)list_get(lista, 2), *(int*)list_get(lista, 3));
     				break;

     			case OBTENER_PEDIDO:
     				printf("Mensaje de OBTENER PEDIDO.\n");
					lista = recibir_paquete(cliente_socket);
					obtenerPedido(cliente_socket, (char*)list_get(lista, 0), *(int*)list_get(lista, 1));
     				break;

     			case CONFIRMAR_PEDIDO:
     				printf("Mensaje de CONFIRMAR PEDIDO.\n");
					lista = recibir_paquete(cliente_socket);
					confirmarPedido(cliente_socket, (char*)list_get(lista, 0), *(int*)list_get(lista, 1));
     				break;

     			case OBTENER_RECETA:
     				printf("Mensaje de OBTENER RECETA.\n");
					lista = recibir_paquete(cliente_socket);
					obtenerReceta(cliente_socket, (char*)list_get(lista, 0), (char*)list_get(lista, 1));
     				break;

     			case PLATO_LISTO:
     				printf("Mensaje de PLATO LISTO.\n");
     				break;

     			case FINALIZAR_PEDIDO:
     				printf("Mensaje de FINALIZAR PEDIDO.\n");
     				break;

     			case -1:
//     				printf("El cliente %d se desconecto.\n", sindicato);
//     				liberar_cliente(sindicato);
//     				return EXIT_FAILURE;

     			default:
     				printf("Operacion desconocida.\n");
     				break;
		}
	}
}
void* abrirConsola() {
	char* opcion_s = malloc(8);
	strcpy(opcion_s, "0");

	while(1) {
		printf("\n------ MODULO SINDICATO ------\n");
		printf("1. Crear Restaurante\n");
		printf("2. Crear Receta\n");
		printf("Salir <intro>\n\n\n");
		free(opcion_s);
		opcion_s = readline(">");
		if(strncmp(opcion_s, "", 1) == 0) {
			free(opcion_s);
			exit(-1);
		}

		switch(atoi(opcion_s)) {
			case 1:
				printf("--- CREAR RESTAURANTE ---\n");
				// logica para consultar los platos
				break;

			case 2:
				printf("--- CREAR RECETA ---\n");
				// logica para guardar pedido
				break;
			default:
				printf("ERROR: opción no válida\n");
				break;
		}

	}

	free(opcion_s);
//	return;
}
//void liberar_un_bloque(char* bloque)
//{
//	FILE*f;
//
//	char *bloque_archivo = malloc(strlen(bloque) + strlen(".bin") + 2);
//	strcpy(bloque_archivo, "/");
//	strcat(bloque_archivo, bloque);
//	strcat(bloque_archivo, ".bin");
//	char *rutaBloque = malloc(strlen(dirBlocks) + strlen(bloque_archivo) + 2 );
////	char *rutaBloque = malloc(100);
//    strcpy(rutaBloque, dirBlocks);
//	//strcpy(rutaBloque, "/home/utnso/Escritorio/TALL_GRASS/Blocks");
//	strcat(rutaBloque, bloque_archivo);
//	f=fopen(rutaBloque,"w");
//	if(f)
//	{
//		fclose(f);
//		//printf("\nSe libero el bloque: %s\n", bloque);
//	}
//	else{
//		//puts("Error al liberar el bloque\n");
//		}
//
//	free(rutaBloque);
//	free(bloque_archivo);
//}
//
//int redondeoBloques(int tamanio)
//{
//	int cantidadDeBloques=tamanio/block_size;
//	if(tamanio%blocks > 0) { cantidadDeBloques++; }
//	//printf("tamanio: %d\n",cantidadDeBloques);
//	return cantidadDeBloques;
//}
//
//bool actualizarBloques(char * rutaDeMetadata, t_list * lista)
//{
//
//	FILE* f = fopen(rutaDeMetadata,"r");
//	if(f == NULL){
//		log_error(log_gamecard,"actualizarBloques: No existe la metadata del pokemon.");
//		return false;
//	}else
//	{
//		fclose(f);
//	}
//
//	t_config* metadata_pokemon = config_create(rutaDeMetadata);
//
//	char *bloques_config = config_get_string_value(metadata_pokemon, "BLOCKS");
//	//printf("\nLOS BLOQUES: %s\n", bloques_config);
//	char **bloques_array = config_get_array_value(metadata_pokemon,"BLOCKS");
//    int viejoTamanio=atoi( config_get_string_value(metadata_pokemon, "SIZE"));
//    //printf("Viejo tamanio: %d\n",viejoTamanio);
//    int nuevoTamanio=calcularNuevoTamanio(lista);
//    //printf("Nuevo tamanio: %d\n",nuevoTamanio);
//    int viejaCantidadDeBloques= redondeoBloques(viejoTamanio);
//    //printf("Viejos bloques: %d\n",viejaCantidadDeBloques);
//    int nuevaCantidadDeBloques=redondeoBloques(nuevoTamanio);
//    //printf("Nuevos bloques: %d\n",nuevaCantidadDeBloques);
//    char* nuevoSize = malloc(10);
//    int tamanio_bloques_config=strlen(bloques_config);
//    int i=0;
//	char * aEliminar=NULL;
//    char*nuevo_bloques_config=NULL;
//    char* nuevoBloqueString = malloc(10);
//
//    log_trace(log_gamecard,"Bloques actuales: %s. Size actual: %d", bloques_config, viejoTamanio);
//
//    	if(viejaCantidadDeBloques<nuevaCantidadDeBloques)
//    	{
//    		int nuevoBloque=obtener_bloque_libre(bitmap);
//    		ocupar_bloque(bitmap,nuevoBloque);
//    		log_trace(log_gamecard,"Bloque nuevo: %d",nuevoBloque);
//
//
//    		sprintf(nuevoBloqueString, "%d",nuevoBloque);
//    		//printf("\n*******************NUEVO BLOQUE DE EMI: %s *****************\n",nuevoBloqueString);
//
//    		if(tamanio_bloques_config > 2){
//			nuevo_bloques_config=(char*)malloc(tamanio_bloques_config+strlen(nuevoBloqueString)+2);
//			strcpy(nuevo_bloques_config,"[");
//			for(i=0; bloques_array[i]; i++)
//			{
//				strcat(nuevo_bloques_config, bloques_array[i]);
//				strcat(nuevo_bloques_config, ",");
//			}
//			strcat(nuevo_bloques_config,nuevoBloqueString);
//			strcat(nuevo_bloques_config,"]");
//    		}else{
//    			nuevo_bloques_config=(char*)malloc(tamanio_bloques_config+strlen(nuevoBloqueString)+2);
//    			strcpy(nuevo_bloques_config,"[");
//    			strcat(nuevo_bloques_config,nuevoBloqueString);
//				strcat(nuevo_bloques_config,"]");
//
//    		}
////    		log_trace(log_gamecard,"Nuevo bloques_config: %s.", nuevo_bloques_config);
//
//    		config_set_value(metadata_pokemon, "BLOCKS", nuevo_bloques_config);
//    		sprintf(nuevoSize,"%d",nuevoTamanio);
//			config_set_value(metadata_pokemon,"SIZE",nuevoSize);
//
//			if(config_save(metadata_pokemon) < 0)
//			{
//				log_error(log_gamecard,"actualizarBloques: Error al actualizar la metadata del pokemon.");
//				free(nuevoSize);
//				free(nuevo_bloques_config); config_destroy(metadata_pokemon);
//				free(bloques_array);
//				free(nuevoBloqueString);
//				sem_post(&mutex_new_pokemon);
//				return false;
//			}else{
//
//				log_trace(log_gamecard,"Nuevos bloques: %s Nuevo Size: %s", nuevo_bloques_config, nuevoSize);
//
//			}
//    	}
//    	else
//    	{
//    		if(nuevaCantidadDeBloques< viejaCantidadDeBloques)
//    		{
//    			liberar_un_bloque(bloques_array[viejaCantidadDeBloques-1]);
//    			int elbloque=atoi(bloques_array[viejaCantidadDeBloques-1]);
//    			liberar_bloque(bitmap,elbloque);
//    			log_trace(log_gamecard,"Se libera: %d bloque/es.", viejaCantidadDeBloques-1);
//
//    			if(nuevaCantidadDeBloques == 0)
//    			{
//    				nuevo_bloques_config=malloc(3);
//    				strcpy(nuevo_bloques_config,"[");
//    				strcat(nuevo_bloques_config,"]");
//
//    			}else
//    			{
//    			nuevo_bloques_config=malloc(tamanio_bloques_config-strlen(bloques_array[viejaCantidadDeBloques-1])-1);
//    			memcpy(nuevo_bloques_config,bloques_config,tamanio_bloques_config-strlen(bloques_array[viejaCantidadDeBloques-1])-2);
//    			strcat(nuevo_bloques_config,"]");
//    			}
//
//    			//printf("\t EMI: NUEVO BLOQUES CONFIG: %s ", nuevo_bloques_config);
//    			config_set_value(metadata_pokemon, "BLOCKS", nuevo_bloques_config);
//    			sprintf(nuevoSize,"%d",nuevoTamanio);
//				config_set_value(metadata_pokemon,"SIZE",nuevoSize);
//
//				if(config_save(metadata_pokemon) < 0)
//				{
//					log_error(log_gamecard,"ERROR: Hubo un error al actualizar la metadata del pokemon.");
//					free(nuevoSize);
//					free(nuevo_bloques_config);
//					config_destroy(metadata_pokemon);
//					free(bloques_array);
//					free(nuevoBloqueString);
//					sem_post(&mutex_new_pokemon);
//					return false;
//				}else
//				{
//					log_trace(log_gamecard,"Nuevos Bloques: %s Nuevo Size: %s.", nuevo_bloques_config, nuevoSize);
//				}
//    		}
//    		else
//    		{
//    			sprintf(nuevoSize,"%d",nuevoTamanio);
//					config_set_value(metadata_pokemon,"SIZE",nuevoSize);
//
//				if(config_save(metadata_pokemon) < 0)
//				{
//					log_error(log_gamecard,"ERROR: Hubo un error al actualizar la metadata del pokemon\n");
//					 free(nuevoSize);
//					 free(nuevo_bloques_config); config_destroy(metadata_pokemon);
//					free(bloques_array);
//					free(nuevoBloqueString);
//					sem_post(&mutex_new_pokemon);
//
//					return false;
//				}else
//				{
//					log_trace(log_gamecard,"Nuevo Size: %s", nuevoSize);
//				}
//    			//Si la cantidad de bloques es la misma
//    			//liberar_un_bloque(bloques_arraỵ̣[viejaCantidadDeBloques-1]);
//    		}
//
//    	}
//    	config_destroy(metadata_pokemon);
//	int bytes_actuales = 0, bytes_restantes = 0, bytes_escritos = 0, longitudPersistida=0;
//	char* posicionPersistida = NULL;
//
//
//	bloques_config = NULL;
//	metadata_pokemon = config_create(rutaDeMetadata);
//	bloques_config = config_get_string_value(metadata_pokemon, "BLOCKS");
//	//printf("\nLOS NUEVOS BLOQUES: %s\n", bloques_config);
//	bloques_array = config_get_array_value(metadata_pokemon,"BLOCKS");
//	char* rutaBloque =NULL;
//	for(int i= 0; bloques_array[i];i++)
//	{
//    		bytes_escritos = 0;
//
//    		liberar_un_bloque(bloques_array[i]);
//			rutaBloque = (char*)malloc(strlen(dirBlocks)+ strlen(bloques_array[i])+ 10);
//    		//strcpy(rutaBloque,"/home/utnso/Escritorio/TALL_GRASS/Blocks");
//    		strcpy(rutaBloque, dirBlocks);
//    		strcat(rutaBloque, "/");
//    		strcat(rutaBloque, bloques_array[i]);
//    		strcat(rutaBloque, ".bin");
//
//		while( bytes_escritos < blocks/*tamanio de bloque*/)
//		{
//    		//printf("%s\n", rutaBloque);
//
//    		if(bytes_restantes > 0 && bytes_escritos==0)
//    		{
//    			f = fopen(rutaBloque, "w");
//    			if(f==NULL){
//    				log_error(log_gamecard,"NO SE PUEDE ABRIR EL BLOQUE %d", i);
//    				free(nuevoSize);
//					free(nuevo_bloques_config);
//					config_destroy(metadata_pokemon);
//					free(bloques_array);
//					free(nuevoBloqueString);
//					free(rutaBloque);
//					free(bloques_config);
//					return false;
//    			}
//
//				fwrite(posicionPersistida+bytes_actuales,bytes_restantes /* +1*/,1,f);
//
//				//printf("escribe el resto :%s, de tamaño: %d\n",posicionPersistida+bytes_actuales,bytes_restantes);
//
//				bytes_escritos+=bytes_restantes;
//				bytes_restantes = 0;
//
////				free(posicionPersistida); posicionPersistida=NULL;
//				fclose(f);
//    		}
//    		else
//    		{
//    			if(list_size(lista) == 0){
//					free(nuevoSize);
//					free(nuevo_bloques_config);
//					free(bloques_array);
//					free(nuevoBloqueString);
//					config_destroy(metadata_pokemon);
//					list_destroy(lista);
//					free(rutaBloque);
//					return true;
//				}
//
//    			f = fopen(rutaBloque, "a");
//    			if(f==NULL){
//    				log_error(log_gamecard,"NO SE PUEDE ABRIR EL BLOQUE %d", i);
//    				free(nuevoSize);
//					free(nuevo_bloques_config);
//					free(bloques_array);
//					free(nuevoBloqueString);
//					config_destroy(metadata_pokemon);
//					free(rutaBloque);
//					free(bloques_config);
//					return false;
//    			}
//
//             if(aEliminar){
//            	 free(aEliminar);
//         		free(posicionPersistida);
//             }
//    		aEliminar=(char*)list_remove(lista,0);
//
//    //		//printf("--------aEliminar: %s\n", aEliminar);
//    		posicionPersistida = malloc(strlen(aEliminar) + 2);
//    		strcpy(posicionPersistida, aEliminar);
//    		strcat(posicionPersistida,"\n");
//    		//free(aEliminar);
//    		longitudPersistida=strlen(posicionPersistida);
//
//
//
//    		//printf("Bytes restantes del bloque:%d, hay que escribir:%d, de: %s...\n",blocks-bytes_escritos,longitudPersistida, posicionPersistida);
//
//    		if(strlen(posicionPersistida)+bytes_escritos </*=*/ blocks)
//    		{
//    			//printf("Entra completo (apa) y escribe %s\n",posicionPersistida);
//				fwrite(posicionPersistida,longitudPersistida,1,f);
//				bytes_escritos+=longitudPersistida;
////				free(posicionPersistida); posicionPersistida=NULL;
//
//    		}
//    		else
//    		{
//    			bytes_actuales = blocks - bytes_escritos;
//    			fwrite(posicionPersistida,bytes_actuales,1,f);
//    			bytes_restantes = longitudPersistida - bytes_actuales;
//    			bytes_escritos+=bytes_actuales;
//    			//printf("No entra una linea entera, escribe %d bytes\n",bytes_actuales);
//    		}
//    		fclose(f);
//    		}
//    	}//while
//		free(rutaBloque);
//		//rutaBloque=NULL;
//
//    }//for
//	free(bloques_array);
//	free(nuevoBloqueString);
//	free(nuevoSize);
//	free(nuevo_bloques_config);
//	config_destroy(metadata_pokemon);
//	return true;
//
//}
//
//void crearBloques(char *dirBlocks)
//{
//	FILE *f;
//	char *rutaArchivo = malloc(strlen(dirBlocks) + 20);
//	for(int i = 0; i < blocks; i++) {
//		char* n = malloc(5);
//		sprintf(n, "%d", i);
//		strcpy(rutaArchivo, dirBlocks);
//		strcat(rutaArchivo, "/");
//		strcat(rutaArchivo, n);
//		strcat(rutaArchivo, ".bin");
//		f = fopen(rutaArchivo, "w");
//		fclose(f);
//		free(n);
//	}
//
//	free(rutaArchivo);
//}
// Función que tiene la lógica para crear un pokemon
//bool agregarRestaurante(t_list* lista) {
//
//	char *rutaPokemon = malloc(strlen(dirFiles) + strlen(list_get(lista, 1)) + strlen("/Metadata.bin") + 2 );
//	strcpy(rutaPokemon, dirFiles);
//	strcat(rutaPokemon, "/");
//	strcat(rutaPokemon, list_get(lista, 1));
//
//	char *dirPokemon = malloc(strlen(dirFiles) + strlen(list_get(lista, 1)) + 2);
//	strcpy(dirPokemon, rutaPokemon);
//	strcat(rutaPokemon, "/Metadata.bin");
//
//	t_config* metadata_pokemon;
//
//	//printf("DIRECTORIO POKEMON: %s\n", dirPokemon);
//	//printf("RUTA POKEMON: %s\n", rutaPokemon);
//
//	FILE *f; f = fopen(rutaPokemon, "r");
//	if(f == NULL)
//	{
////		printf("MENOS UNO? %d\n",*(int*)list_get(lista,4));
//			if(list_size(lista) == 5 && *(int*)list_get(lista,4) == -1){
//			log_warning(log_gamecard, "Se esta intentando atrapar un pokemon que no existe");
//			sem_post(&mutex_new_pokemon);
//			//list_destroy_and_destroy_elements(lista,free);
//			free(rutaPokemon);
//			free(dirPokemon);
//			return false;
//		}
//		//printf("No existe el pokemon %s, se crea el directorio...\n", (char*)list_get(lista, 1));
//		if(mkdir(dirPokemon, 0777) == 0) {
//			//printf("Creando la metadata...\n");
//			f = fopen(rutaPokemon, "w");
//			fputs("DIRECTORY=N\n", f);
//			fputs("SIZE=0\n", f);
//			fputs("BLOCKS=[]\n", f);
//			fputs("OPEN=N", f);
//			fclose(f);
//			metadata_pokemon = config_create(rutaPokemon);
//		}
//		else
//		{
//			log_error(log_gamecard,"Error al crear el directorio del pokemon\n");
//			sem_post(&mutex_new_pokemon);
//			//list_destroy_and_destroy_elements(lista,free);
//			free(rutaPokemon);
//			free(dirPokemon);
//			return false;
//		}
//	}
//	else
//	{
//		//printf("El pokemon %s existe, se carga su metadata...\n", (char*)list_get(lista, 1));
//		fclose(f);
//		metadata_pokemon = config_create(rutaPokemon);
//	}
//
//	char *is_open = config_get_string_value(metadata_pokemon, "OPEN");
//	while(strcmp(is_open , "Y")==0)
//	{
//		sleep(tiempo_reintento_operacion);
//		is_open = config_get_string_value(metadata_pokemon, "OPEN");
//		log_warning(log_gamecard,"El archivo esta siendo utilizado por otro proceso.\n");
//	}
//
//	// Seteo OPEN=Y para que otro proceso no pueda acceder al pokemon que esta siendo utilizado.
//	config_set_value(metadata_pokemon, "OPEN", "Y");
//	if(config_save(metadata_pokemon) < 0)
//	{
//		log_error(log_gamecard,"Error al actualizar la metadata del pokemon\n");
//		config_destroy(metadata_pokemon);
//		sem_post(&mutex_new_pokemon);
//		//list_destroy_and_destroy_elements(lista,free);
//		free(rutaPokemon);
//		free(dirPokemon);
//		return false;
//	}
//	config_destroy(metadata_pokemon);
//
//	// --------------------------------------------------------------------------
//
//
//	t_list* listaPosiciones = leer_posiciones(rutaPokemon);
//
//	if(listaPosiciones==NULL)
//	{
//		config_destroy(metadata_pokemon);
//		free(rutaPokemon);
//		free(dirPokemon);
//		sem_post(&mutex_new_pokemon);
//		return false;
//	}
//
//	bool actualizacionCorrecta=false;
//	if(list_size(lista) == 5)
//		actualizarPosicionLista(listaPosiciones,*(int*) list_get(lista,2),*(int*)list_get(lista,3),*(int*)list_get(lista,4));
//	else
//		actualizarPosicionLista(listaPosiciones,*(int*) list_get(lista,2),*(int*)list_get(lista,3), -1);
//
//
//	//puts("\tActualizar... Actualiza\n");
//	verListaDeAtrapados(listaPosiciones);
//
//	actualizacionCorrecta=actualizarBloques(rutaPokemon,listaPosiciones);
//
//	if(!actualizacionCorrecta)
//		{
//			log_error(log_gamecard, "No se pudo ejecutar correctamente.");
//			config_destroy(metadata_pokemon);
//	//		list_destroy_and_destroy_elements(lista,free);
//			free(rutaPokemon);
//			free(dirPokemon);
//			sem_post(&mutex_new_pokemon);
//			return false;
//		}
//
////	//puts("\tVER LISTA ... VE?\n");
////	verListaDeAtrapados(listaPosiciones);
//	metadata_pokemon = config_create(rutaPokemon);
//	config_set_value(metadata_pokemon, "OPEN", "N");
//	if(config_save(metadata_pokemon) < 0)
//	{
//		log_error(log_gamecard,"Error al cerrar la metadata del pokemon.");
//		config_destroy(metadata_pokemon);
////		list_destroy_and_destroy_elements(lista,free);
//		free(rutaPokemon);
//		free(dirPokemon);
//		sem_post(&mutex_new_pokemon);
//		return false;
//	}
//	config_destroy(metadata_pokemon);
//	free(rutaPokemon);
//	free(dirPokemon);
//	sem_post(&mutex_new_pokemon);
//
//	return true;
//
//}
