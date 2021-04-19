#include "gamecard.h"

void verListaDeAtrapados(t_list* lista){
	t_link_element *element = lista->head;
	//puts("LISTA");
	while (element != NULL){
		//printf("|%s|\n", (char*)element->data);
		element = element->next;
	}
    //puts("\n");
}

void limpiarCharDobleAsterisco(char** vistima){
	for(int i = 0; vistima[i];i++){
		//printf("Victima numero %d: %s", i, vistima[i]);
		free(vistima[i]);
	}
	free(vistima);
}

t_list* leer_posiciones(char* rutaDeMetadata){
//	t_list* posicionesDePokemons = list_create();
	FILE* f = fopen(rutaDeMetadata,"r");
	if(f == NULL){

		log_warning(log_gamecard,"Leer_posiciones: No existe la metadata del pokemon.");
		return NULL;
	}
	t_list* posicionesDePokemons = list_create();
	 char* nuevaPosicion=NULL;
	fclose(f);
	t_config* metadata_pokemon = config_create(rutaDeMetadata);


	char *bloques_config = config_get_string_value(metadata_pokemon, "BLOCKS");
	//printf("\nLOS BLOQUES: %s\n", bloques_config);

	char **bloques_array = config_get_array_value(metadata_pokemon,"BLOCKS");
	char * ultimoElemento=NULL;

	char* linea = NULL;
	size_t len = 0;

	bool hayQueConcatenar = false;

	//puts("Creamo la lista\n");
	for(int i = 0; bloques_array[i] ; i++){

		char *bloque_archivo = (char*)malloc(strlen(bloques_array[i]) + strlen(".bin") + 2);
		strcpy(bloque_archivo, "/");
		strcat(bloque_archivo, bloques_array[i]);
		strcat(bloque_archivo, ".bin");
		//printf("Bloque archivo: %s, size: %d\n", bloque_archivo, strlen(bloque_archivo));

		char *rutaBloque = malloc(strlen(dirBlocks) + strlen(bloque_archivo) + 2 );
		strcpy(rutaBloque, dirBlocks);
		strcat(rutaBloque, bloque_archivo);

		//printf("Ruta bloque: %s\n", rutaBloque);
		f = fopen(rutaBloque,"r");
		char* lineaModificada = NULL;
		 while (getline(&linea, &len, f) != -1) {

			// //printf("LINEA DE FLOR ***************************|%s|", linea);
			 if(hayQueConcatenar){
				 //puts("HAY QUE CONCATENAR \n");
				 hayQueConcatenar = false;

				 ////printf("Ultima Linea antes del strcat: %s\n",list_get(posicionesDePokemons,posicionesDePokemons->elements_count-1));
				 if(string_contains(linea,"\n")){
					 lineaModificada = string_substring(linea, 0, (strlen(linea)-1));
					 free(linea);
					 linea = lineaModificada;
					 ////printf("Linea despues: |%s| %d\n",linea, strlen(linea));
				 }
				 strcat(list_get(posicionesDePokemons,posicionesDePokemons->elements_count-1),linea);
//				 free(linea);
				 ////printf("Ultima Linea: %s\n",list_get(posicionesDePokemons,posicionesDePokemons->elements_count-1));
			 }else{
				 ////printf("Linea antes: |%s| %d\n",linea, strlen(linea));

				 if(string_contains(linea,"\n")){
					 lineaModificada = string_substring(linea, 0, (strlen(linea)-1));
					 free(linea);
					 linea = lineaModificada;
					 ////printf("Linea despues: |%s| %d\n",linea, strlen(linea));
				 }
				 nuevaPosicion = (char*)malloc(strlen(linea)+1);

				 strcpy(nuevaPosicion,linea);
				 list_add(posicionesDePokemons,(void*)nuevaPosicion);
//				 free(linea);

				 ////printf("1. Linea agregada: |%s| %d\n",nuevaPosicion, strlen(nuevaPosicion));
			 }
			 free(linea);
			 len = 0;
			 linea = NULL;
			}//while

//		 free(linea);

         ultimoElemento=(char *)list_get(posicionesDePokemons,posicionesDePokemons->elements_count-1);
		 hayQueConcatenar = !string_contains(ultimoElemento,"\n");
		 free(rutaBloque);
		 free(bloque_archivo);
		fclose(f);
	}

	 ////printf("Cantidad de lineas: %d\n",posicionesDePokemons->elements_count);
	 verListaDeAtrapados(posicionesDePokemons);
	 config_destroy(metadata_pokemon);
	 free(bloques_array);
	 return posicionesDePokemons;
}

void recibirIdMensaje(int conexionBroker){
    //esta funcion se encarga de esperar la respuesta del broker con el id de mensaje
    sem_wait(&mutex_recibirId);
    t_list* lista=NULL;
    int respuesta;

    if (recibir_operacion(conexionBroker) == MENSAJE_ID)
    {
        lista=recibir_paquete(conexionBroker);
        respuesta = *(int*) lista->head->data;
        log_info(log_gamecard,"ID RECIBIDO DE MENSAJE %d.",respuesta);

 list_destroy_and_destroy_elements(lista,free);
    }
    else
    {
        respuesta = 0;
        log_error(log_gamecard,"No se recibió ID");

    }
    sem_post(&mutex_recibirId);
return;
}

void* obtenerId(){
	conexionBroker = crear_conexion(ipBroker, puertoBroker);
	while(conexionBroker < 0){
		log_error(log_gamecard,"No se pudo establecer conexion con Broker.");
		sleep(tiempoIntentoReconexion);
		log_trace(log_gamecard,"Reintentando conexion con Broker...");
		conexionBroker = crear_conexion(ipBroker, puertoBroker);
	}

	log_trace(log_gamecard,"Conexion con Broker exitosa.");

	FILE *f = fopen("gamecard_id.txt", "r");
	if(f == NULL) {
		f = fopen("gamecard_id.txt", "w");
		gamecardId = 0;
	} else {
		fread(&gamecardId, sizeof(int), 1, f);
	}
	fclose(f);

	t_paquete *paqueteConexion = crear_paquete(ENLAZAR);
	agregar_a_paquete(paqueteConexion, &gamecardId, sizeof(int));
	enviar_paquete(paqueteConexion, conexionBroker);
	eliminar_paquete(paqueteConexion);

	int operacion = recibir_operacion(conexionBroker);
	t_list* lista=NULL;
	switch(operacion)
	{
		case SUSCRIPTOR_ID:
			lista = recibir_paquete(conexionBroker);
			gamecardId = *(int *) list_get(lista, 0);
			log_trace(log_gamecard, "Nuevo ID: %d", gamecardId);
			f = fopen("gamecard_id.txt", "w");
			fwrite(&gamecardId, sizeof(int), 1, f);
			fclose(f);

			break;

		case REENLAZADO:
			log_trace(log_gamecard, "Gamecard REENLAZADO con exito");

			break;
	}
	return NULL;
}

t_paquete* getPokemon(t_list* lista){
	char* posiciones = NULL;
	char** posicionesAux = NULL;
	char *rutaPokemon = malloc(strlen(dirFiles) + strlen(list_get(lista, 1)) + strlen("/Metadata.bin") + 2 );
	strcpy(rutaPokemon, dirFiles);
	strcat(rutaPokemon, "/");
	strcat(rutaPokemon, list_get(lista, 1));
	strcat(rutaPokemon, "/Metadata.bin");

	//printf("RUTA POKEMON: %s\n", rutaPokemon);

	int mensaje_id = *(int*)list_get(lista, 0);
	t_list* listaDePosiciones = leer_posiciones(rutaPokemon);
	t_paquete *paqueteLocalized = crear_paquete(LOCALIZED_POKEMON);
	agregar_a_paquete(paqueteLocalized, &mensaje_id, sizeof(int));
	agregar_a_paquete(paqueteLocalized, list_get(lista, 1), strlen(list_get(lista,1))+1);

	if (!listaDePosiciones)
	{
		int vacio=0;
		agregar_a_paquete(paqueteLocalized, &vacio, sizeof(int));
		log_warning(log_gamecard,"%s No fue creado. (ツ)_/¯",(char*)list_get(lista, 1));
		sem_post(&mutex_new_pokemon);
		return paqueteLocalized;
	}

	int cantidadDePokemones = list_size(listaDePosiciones);//->elements_count;

	if (cantidadDePokemones==0)
	{
		int vacio=0;
		agregar_a_paquete(paqueteLocalized, &vacio, sizeof(int));
		log_warning(log_gamecard,"%s Fue creado pero no tiene posiciones. (ツ)_/¯",(char*)list_get(lista, 1));
		sem_post(&mutex_new_pokemon);
		return paqueteLocalized;
	}

	agregar_a_paquete(paqueteLocalized, &cantidadDePokemones, sizeof(int));
	char* posicionX = NULL;
	char** estamosHaciendoUnSplitDOSVECES = NULL;
	char* posicionY = NULL;
	char* linea = NULL;
	int x = -1, y = -1;

	log_trace(log_gamecard,"%d Posiciones de %s:",cantidadDePokemones, (char*)list_get(lista, 1));

	for(int i = 0; i < listaDePosiciones->elements_count; i++){
		 linea = list_get(listaDePosiciones, i);

		posicionesAux = string_split(linea, "=");
		posiciones = posicionesAux[0];
		estamosHaciendoUnSplitDOSVECES = string_split(posiciones, "-");
		posicionX= estamosHaciendoUnSplitDOSVECES[0];
		posicionY =estamosHaciendoUnSplitDOSVECES[1];
		 x =  atoi(posicionX);
		 y =  atoi(posicionY);

		 log_trace(log_gamecard,"\tPosicion %d: (%d,%d)",i+1,x,y);

		agregar_a_paquete(paqueteLocalized, &x, sizeof(int));
		agregar_a_paquete(paqueteLocalized, &y, sizeof(int));
		free(posicionX);
		free(posicionY);
		free(posiciones);
		free(estamosHaciendoUnSplitDOSVECES);
		free(posicionesAux[1]);
		free(posicionesAux);
	}
	log_trace(log_gamecard,"\tNo hay mas (ツ)_/¯");
	list_destroy_and_destroy_elements(listaDePosiciones, free);
	free(rutaPokemon);
	sem_post(&mutex_new_pokemon);
	return paqueteLocalized;
}


void *gestionarConexionConGameboy(void* socketServidor)
{
    t_list* lista;

    while(1)
    {
    	int cliente = esperar_cliente(*((int*)socketServidor));
		int operacion;
		int mensaje_id = 0;
        operacion = recibir_operacion(cliente);
        switch(operacion)
        {
            case NEW_POKEMON:
            	sem_wait(&mutex_new_pokemon);
                lista = recibir_paquete(cliente);

                log_debug(log_gamecard, "NEW_POKEMON: ID = %d; Pokemon = %s; X = %d;  Y = %d; Cantidad = %d",
					*(int*) list_get(lista, 0),(char *)list_get(lista, 1),*(int*) list_get(lista, 2),
						*(int*) list_get(lista, 3),*(int*) list_get(lista, 4));

				 conexionBroker = crear_conexion(ipBroker, puertoBroker);


				 if(conexionBroker > 0)
				 {
					 if(agregarPokemon(lista))
					 {
						 mensaje_id =  *(int*)list_get(lista, 0);
						//char *pokemon = list_get(lista, 0);
						int coordenada_x = *(int*)list_get(lista, 2);
						int coordenada_y = *(int*)list_get(lista, 3);
						t_paquete *paquete2 = crear_paquete(APPEARED_POKEMON);
						agregar_a_paquete(paquete2, &mensaje_id, sizeof(int));
						agregar_a_paquete(paquete2, list_get(lista, 1), strlen(list_get(lista, 1))+1);
						agregar_a_paquete(paquete2, &coordenada_x, sizeof(int));
						agregar_a_paquete(paquete2, &coordenada_y, sizeof(int));

						enviar_paquete(paquete2, conexionBroker);
						eliminar_paquete(paquete2);
						log_trace(log_gamecard,"APPEARED_POKEMON enviado. \n");
						//recibirIdMensaje(conexionBroker);
					 }
					 else
						 log_warning(log_gamecard,"No se pudo agregar el pokemon.");

					 liberar_cliente(conexionBroker);
				 }
				else
				{
					log_error(log_gamecard,"No se puede establecer conexion con BROKER -> continuamos ejecucion.");
					if(agregarPokemon(lista))
						log_trace(log_gamecard,"Se pudo agregar el pokemon, pero no informar a broker.");
					else
						log_warning(log_gamecard,"No se pudo agregar el pokemon.");
				}

				list_destroy_and_destroy_elements(lista,free);
                break;

            case CATCH_POKEMON:
            	sem_wait(&mutex_new_pokemon);
                lista = recibir_paquete(cliente);

                log_debug(log_gamecard, "CATCH_POKEMON: ID = %d; Pokemon = %s; X = %d; Y = %d",*(int*) list_get(lista, 0),
                		(char *)list_get(lista, 1),*(int*) list_get(lista, 2),*(int*) list_get(lista, 3));

              conexionBroker = crear_conexion(ipBroker, puertoBroker);

              //ELEMENTO CATCH
				void* atrapar=(void*)malloc(sizeof(int));
				int menosUno=-1;
				memcpy(atrapar,&menosUno,sizeof(int));
				list_add(lista,atrapar);

				if(conexionBroker > 0)
				{
					mensaje_id = *(int*) list_get(lista, 0);
					t_paquete *paquete3 = crear_paquete(CAUGHT_POKEMON);
					agregar_a_paquete(paquete3,&mensaje_id, sizeof(int));

					if(agregarPokemon(lista))
						agregar_a_paquete(paquete3, "OK", strlen("OK")+1);
					else
						agregar_a_paquete(paquete3, "FAIL", strlen("FAIL")+1);

					enviar_paquete(paquete3, conexionBroker);
					eliminar_paquete(paquete3);
					log_trace(log_gamecard,"CAUGHT_POKEMON enviado. \n");
					//recibirIdMensaje(conexionBroker);
					liberar_cliente(conexionBroker);


				}
				else
				{
					log_error(log_gamecard,"No se puede establecer conexion con BROKER -> continuamos ejecucion.");
					if(agregarPokemon(lista))
						log_trace(log_gamecard,"Se pudo atrapar, pero no informar a broker.");
					else
						log_trace(log_gamecard,"No se pudo atrapar.");
				}

				list_destroy_and_destroy_elements(lista,free);
                break;

            case GET_POKEMON:
            	sem_wait(&mutex_new_pokemon);
                lista = recibir_paquete(cliente);

                log_debug(log_gamecard, "GET_POKEMON: Mensaje ID = %d; Pokemon = %s",
					*(int*) list_get(lista, 0),
					(char *)list_get(lista, 1)
				);

                conexionBroker = crear_conexion(ipBroker, puertoBroker);
                t_paquete* paquetengas;
				if(conexionBroker > 0)
				{
					paquetengas = getPokemon(lista);
					enviar_paquete(paquetengas, conexionBroker);
					eliminar_paquete(paquetengas);
					log_trace(log_gamecard,"LOCALIZED_POKEMON enviado. \n");
					//recibirIdMensaje(conexionBroker);
					liberar_cliente(conexionBroker);

				}
				else
				{
					log_error(log_gamecard,"No se puede establecer conexion con BROKER -> continuamos ejecucion.");
					paquetengas = getPokemon(lista);
					eliminar_paquete(paquetengas);
				}
                list_destroy_and_destroy_elements(lista, free);

                break;


            default:
                break;
        }
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

void crearEstructuraFileSystem()
{
	FILE *f;

	puntoMontaje = config_get_string_value(gamecardConfig,"PUNTO_MONTAJE_TALLGRASS");
	dirMetadata = malloc(strlen(puntoMontaje) + strlen("/Metadata") + 1);
	strcpy(dirMetadata, puntoMontaje);
	strcat(dirMetadata, "/Metadata");

	dirFiles = malloc(strlen(puntoMontaje) + strlen("/Files") + 1);
	strcpy(dirFiles, puntoMontaje);
	strcat(dirFiles, "/Files");

	dirBlocks = malloc(strlen(puntoMontaje) + strlen("/Blocks") + 1);
	strcpy(dirBlocks, puntoMontaje);
	strcat(dirBlocks, "/Blocks");

	char *metadataRuta = malloc(strlen(dirMetadata) + strlen("/Metadata.bin") + 1);
	strcpy(metadataRuta, dirMetadata);
	strcat(metadataRuta, "/Metadata.bin");


	if(mkdir(puntoMontaje, 0777) != 0)
	{
		//Si el fileSystem esta creado se toman los datos de la metadata existente.
		log_info(log_gamecard, "El directorio %s ya existe. ", puntoMontaje);
		t_config* metadataConfig=config_create(metadataRuta);

		block_size=atoi(config_get_string_value(metadataConfig,"BLOCK_SIZE"));
		blocks=atoi(config_get_string_value(metadataConfig,"BLOCKS"));
		magic_number=malloc(20);
		strcpy(magic_number,(char*)config_get_string_value(metadataConfig,"MAGIC_NUMBER"));

		config_destroy(metadataConfig);

		log_info(log_gamecard,"BLOCK_SIZE: %d. BLOCKS: %d. MAGIC_NUMBER: %s. ", block_size, blocks, magic_number);
		bitmap = crear_bitmap(dirMetadata, blocks);
		free(metadataRuta);
		return;
	}
	else
	{
		//Si el fileSystem NO esta creado se toman los datos del archivo de configuracion.

		block_size=atoi(config_get_string_value(gamecardConfig,"BLOCK_SIZE"));
		blocks=atoi(config_get_string_value(gamecardConfig,"BLOCKS"));
		magic_number=malloc(20);
		strcpy(magic_number,(char*)config_get_string_value(gamecardConfig,"MAGIC_NUMBER"));

		log_info(log_gamecard,"BLOCK_SIZE: %d. BLOCKS: %d. MAGIC_NUMBER: %s. ", block_size, blocks, magic_number);

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
			log_error(log_gamecard, "crearEstructuraFileSystem: No se pudo crear el directorio Metadata");
			free(metadataRuta);
			return;
		}

		// Directorio Files

		if(mkdir(dirFiles, 0777) == 0)
		{
			// Creo archivo Metadata.bin
			char* metadataRuta = malloc(strlen(dirFiles) + strlen("/Metadata.bin") + 1);
			strcpy(metadataRuta, dirFiles);
			strcat(metadataRuta, "/Metadata.bin");
			// Creo el archivo Metadata.bin (para indicar que es un directorio)
			f = fopen(metadataRuta, "w");
			fputs("DIRECTORY=Y", f);
			fclose(f);
			free(metadataRuta);
		}
		else
		{
			log_error(log_gamecard, "Ha ocurrido un error al crear el directorio Files.");
			free(metadataRuta);
			return;
		}

		// Directorio Blocks

		if(mkdir(dirBlocks, 0777) == 0) {crearBloques(dirBlocks);}
		else {
			log_error(log_gamecard, "Ha ocurrido un error al crear el directorio Blocks.");
			return;
		}

		log_trace(log_gamecard, "Estructura creada.");
	}
}

// ############# Suscripciones a las diferentes colas #############

int suscribirColaNewPokemon() {
	int conexionBrokerNew = crear_conexion(ipBroker, puertoBroker);

	while(conexionBrokerNew < 0){
		log_error(log_gamecard,"No se pudo establecer conexion con Broker.");
		sleep(tiempoIntentoReconexion);
		log_trace(log_gamecard,"Reintentando conexion con Broker...");
		conexionBrokerNew = crear_conexion(ipBroker, puertoBroker);
	}

	t_paquete* paquete = crear_paquete(SUSCRIBIR);
	int cola = (int) NEW_POKEMON;
	agregar_a_paquete(paquete, &gamecardId, sizeof(int));
	agregar_a_paquete(paquete, &cola, sizeof(int));
	enviar_paquete(paquete, conexionBrokerNew);
	eliminar_paquete(paquete);

	log_trace(log_gamecard, "Se ha suscripto a la cola NEW_POKEMON");

	return conexionBrokerNew;
}

int suscribirColaCatchPokemon() {
	int conexionBrokerCatch = crear_conexion(ipBroker, puertoBroker);
	while(conexionBrokerCatch < 0){
		log_error(log_gamecard,"No se pudo establecer conexion con Broker.");
		sleep(tiempoIntentoReconexion);
		log_trace(log_gamecard,"Reintentando conexion con Broker...");
		conexionBrokerCatch = crear_conexion(ipBroker, puertoBroker);
	}

	t_paquete* paquete = crear_paquete(SUSCRIBIR);
	int cola = (int) CATCH_POKEMON;
	agregar_a_paquete(paquete, &gamecardId, sizeof(int));
	agregar_a_paquete(paquete, &cola, sizeof(int));
	enviar_paquete(paquete, conexionBrokerCatch);
	eliminar_paquete(paquete);

	log_trace(log_gamecard, "Se ha suscripto a la cola CATCH_POKEMON");

	return conexionBrokerCatch;
}

int suscribirColaGetPokemon() {
	int conexionBrokerGet = crear_conexion(ipBroker, puertoBroker);
	while(conexionBrokerGet < 0){
		log_error(log_gamecard,"No se pudo establecer conexion con Broker.");
		sleep(tiempoIntentoReconexion);
		log_trace(log_gamecard,"Reintentando conexion con Broker...");
		conexionBrokerGet = crear_conexion(ipBroker, puertoBroker);
	}

	t_paquete* paquete = crear_paquete(SUSCRIBIR);
	int cola = (int) GET_POKEMON;
	agregar_a_paquete(paquete, &gamecardId, sizeof(int));
	agregar_a_paquete(paquete, &cola, sizeof(int));
	enviar_paquete(paquete, conexionBrokerGet);
	eliminar_paquete(paquete);

	log_trace(log_gamecard, "Se ha suscripto a la cola GET_POKEMON");

	return conexionBrokerGet;
}

// ############# Gestión de las diferentes colas #############



// Función que tiene la lógica para crear un pokemon
bool agregarPokemon(t_list* lista) {

	char *rutaPokemon = malloc(strlen(dirFiles) + strlen(list_get(lista, 1)) + strlen("/Metadata.bin") + 2 );
	strcpy(rutaPokemon, dirFiles);
	strcat(rutaPokemon, "/");
	strcat(rutaPokemon, list_get(lista, 1));

	char *dirPokemon = malloc(strlen(dirFiles) + strlen(list_get(lista, 1)) + 2);
	strcpy(dirPokemon, rutaPokemon);
	strcat(rutaPokemon, "/Metadata.bin");

	t_config* metadata_pokemon;

	//printf("DIRECTORIO POKEMON: %s\n", dirPokemon);
	//printf("RUTA POKEMON: %s\n", rutaPokemon);

	FILE *f; f = fopen(rutaPokemon, "r");
	if(f == NULL)
	{
//		printf("MENOS UNO? %d\n",*(int*)list_get(lista,4));
			if(list_size(lista) == 5 && *(int*)list_get(lista,4) == -1){
			log_warning(log_gamecard, "Se esta intentando atrapar un pokemon que no existe");
			sem_post(&mutex_new_pokemon);
			//list_destroy_and_destroy_elements(lista,free);
			free(rutaPokemon);
			free(dirPokemon);
			return false;
		}
		//printf("No existe el pokemon %s, se crea el directorio...\n", (char*)list_get(lista, 1));
		if(mkdir(dirPokemon, 0777) == 0) {
			//printf("Creando la metadata...\n");
			f = fopen(rutaPokemon, "w");
			fputs("DIRECTORY=N\n", f);
			fputs("SIZE=0\n", f);
			fputs("BLOCKS=[]\n", f);
			fputs("OPEN=N", f);
			fclose(f);
			metadata_pokemon = config_create(rutaPokemon);
		}
		else
		{
			log_error(log_gamecard,"Error al crear el directorio del pokemon\n");
			sem_post(&mutex_new_pokemon);
			//list_destroy_and_destroy_elements(lista,free);
			free(rutaPokemon);
			free(dirPokemon);
			return false;
		}
	}
	else
	{
		//printf("El pokemon %s existe, se carga su metadata...\n", (char*)list_get(lista, 1));
		fclose(f);
		metadata_pokemon = config_create(rutaPokemon);
	}

	char *is_open = config_get_string_value(metadata_pokemon, "OPEN");
	while(strcmp(is_open , "Y")==0)
	{
		sleep(tiempo_reintento_operacion);
		is_open = config_get_string_value(metadata_pokemon, "OPEN");
		log_warning(log_gamecard,"El archivo esta siendo utilizado por otro proceso.\n");
	}

	// Seteo OPEN=Y para que otro proceso no pueda acceder al pokemon que esta siendo utilizado.
	config_set_value(metadata_pokemon, "OPEN", "Y");
	if(config_save(metadata_pokemon) < 0)
	{
		log_error(log_gamecard,"Error al actualizar la metadata del pokemon\n");
		config_destroy(metadata_pokemon);
		sem_post(&mutex_new_pokemon);
		//list_destroy_and_destroy_elements(lista,free);
		free(rutaPokemon);
		free(dirPokemon);
		return false;
	}
	config_destroy(metadata_pokemon);

	// --------------------------------------------------------------------------
	

	t_list* listaPosiciones = leer_posiciones(rutaPokemon);

	if(listaPosiciones==NULL)
	{
		config_destroy(metadata_pokemon);
		free(rutaPokemon);
		free(dirPokemon);
		sem_post(&mutex_new_pokemon);
		return false;
	}

	bool actualizacionCorrecta=false;
	if(list_size(lista) == 5)
		actualizarPosicionLista(listaPosiciones,*(int*) list_get(lista,2),*(int*)list_get(lista,3),*(int*)list_get(lista,4));
	else
		actualizarPosicionLista(listaPosiciones,*(int*) list_get(lista,2),*(int*)list_get(lista,3), -1);


	//puts("\tActualizar... Actualiza\n");
	verListaDeAtrapados(listaPosiciones);

	actualizacionCorrecta=actualizarBloques(rutaPokemon,listaPosiciones);

	if(!actualizacionCorrecta)
		{
			log_error(log_gamecard, "No se pudo ejecutar correctamente.");
			config_destroy(metadata_pokemon);
	//		list_destroy_and_destroy_elements(lista,free);
			free(rutaPokemon);
			free(dirPokemon);
			sem_post(&mutex_new_pokemon);
			return false;
		}

//	//puts("\tVER LISTA ... VE?\n");
//	verListaDeAtrapados(listaPosiciones);
	metadata_pokemon = config_create(rutaPokemon);
	config_set_value(metadata_pokemon, "OPEN", "N");
	if(config_save(metadata_pokemon) < 0)
	{
		log_error(log_gamecard,"Error al cerrar la metadata del pokemon.");
		config_destroy(metadata_pokemon);
//		list_destroy_and_destroy_elements(lista,free);
		free(rutaPokemon);
		free(dirPokemon);
		sem_post(&mutex_new_pokemon);
		return false;
	}
	config_destroy(metadata_pokemon);
	free(rutaPokemon);
	free(dirPokemon);
	sem_post(&mutex_new_pokemon);

	return true;

}


void actualizarPosicionLista(t_list* lista, int posicion_x, int posicion_y, int cantidad) {
	char *x = malloc(20);
	char *y = malloc(20);
	char *cantidad_str = malloc(20);
	sprintf(x, "%d", posicion_x);
	sprintf(y, "%d", posicion_y);
	sprintf(cantidad_str, "%d", cantidad);
	char *posicionString =NULL;
	posicionString =(char *) malloc(strlen(x) + strlen(y) + 2);
	strcpy(posicionString, x);
	strcat(posicionString, "-");
	strcat(posicionString, y);
	char *actual=NULL;
	char **posicionActual=NULL;
	int nuevaCantidad = 0;
	char *nuevaCantidad_str = malloc(20);

	// char **string_split(char* texto, char* separador);
	if(list_size(lista) == 0 && cantidad > 0){
		char* nuevoElemento = (char*) malloc(strlen(x)+strlen(y)+strlen(cantidad_str)+4);
		strcpy(nuevoElemento,x);
		strcat(nuevoElemento,"-");
		strcat(nuevoElemento,y);
		strcat(nuevoElemento,"=");
		strcat(nuevoElemento,cantidad_str);
		log_trace(log_gamecard,"Primera posicion: '%s'.",nuevoElemento);
		list_add(lista, (void*)nuevoElemento);
		free(x);
		free(y);
		free(cantidad_str);
		free(nuevaCantidad_str);
		free(posicionString);
		return;
	}
	for(int i = 0; i < list_size(lista); i++) {
		posicionActual = string_split(list_get(lista, i), "=");
		actual = posicionActual[0];
		if(strcmp(posicionString, actual) == 0) {
			//printf("\nLa posicion existe!: %d\n", i);
			//printf("\n%s\n", (char *) list_get(lista, i));

			nuevaCantidad = atoi(posicionActual[1]);
			nuevaCantidad += cantidad;
			sprintf(nuevaCantidad_str,"%d",nuevaCantidad);

			if(nuevaCantidad <= 0) {
				log_trace(log_gamecard,"Se elimina la posicion '%s'.", posicionString);
				char *aEliminar = list_remove(lista, i);
				free(aEliminar);
				free(x);
				free(y);
				free(cantidad_str);
				free(nuevaCantidad_str);
				free(actual);
				free(posicionActual[1]);
				free(posicionString);
				free(posicionActual);
				return;
			}
			else {

                char *nuevaData=NULL;
				char *viejaData = (char*)list_remove(lista, i);


				nuevaData =(char*)malloc(strlen(posicionActual[0]) + strlen(nuevaCantidad_str) + 2);
				strcpy(nuevaData, posicionActual[0]);
				strcat(nuevaData, "=");
				strcat(nuevaData, nuevaCantidad_str);
				log_trace(log_gamecard,"Se actualiza la cantidad de '%s' por '%s'.",viejaData,nuevaData);
				//printf("%s \n",nuevaData);
				list_add_in_index(lista,i,(void*)nuevaData);
				free(viejaData);viejaData=NULL;
				free(x);
				free(y);
				free(cantidad_str);
				free(nuevaCantidad_str);
				free(posicionString);
				free(actual);
				free(posicionActual[1]);
				free(posicionActual);
				return;
			}
		}

	}
	if(cantidad > 0) {
		nuevaCantidad += cantidad;
		sprintf(nuevaCantidad_str,"%d",nuevaCantidad);
		char *nuevaPosicion = malloc(strlen(posicionString) + strlen(nuevaCantidad_str) + 2);

		strcpy(nuevaPosicion, posicionString);
		strcat(nuevaPosicion, "=");
		strcat(nuevaPosicion, nuevaCantidad_str);
		list_add(lista, nuevaPosicion);
		log_trace(log_gamecard,"Se agrega la nueva posicion: '%s'.", nuevaPosicion);
	} else {
		log_warning(log_gamecard,"El pokemon en la posicion '%s' ya fue atrapado.",posicionString);
		free(x);
		free(y);
		free(cantidad_str);
		free(nuevaCantidad_str);
		free(posicionString);
		return;
	}
	free(x);
	free(y);
	free(cantidad_str);
	free(nuevaCantidad_str);
	free(posicionString);
	return;
}

int calcularNuevoTamanio (t_list * lista)
{
	int i=0;
	int suma=0;

	while(i < lista->elements_count)
	{
		//printf("\t PRINT F DE I : %d\n",i);
		suma+= strlen((char*)list_get(lista,i))+1;
		i++;
	}
 //printf("Suma: %d \n ", suma);
	return suma;
}

void liberar_un_bloque(char* bloque)
{
	FILE*f;

	char *bloque_archivo = malloc(strlen(bloque) + strlen(".bin") + 2);
	strcpy(bloque_archivo, "/");
	strcat(bloque_archivo, bloque);
	strcat(bloque_archivo, ".bin");
	char *rutaBloque = malloc(strlen(dirBlocks) + strlen(bloque_archivo) + 2 );
//	char *rutaBloque = malloc(100);
    strcpy(rutaBloque, dirBlocks);
	//strcpy(rutaBloque, "/home/utnso/Escritorio/TALL_GRASS/Blocks");
	strcat(rutaBloque, bloque_archivo);
	f=fopen(rutaBloque,"w");
	if(f)
	{
		fclose(f);
		//printf("\nSe libero el bloque: %s\n", bloque);
	}
	else{
		//puts("Error al liberar el bloque\n");
		}

	free(rutaBloque);
	free(bloque_archivo);
}

int redondeoBloques(int tamanio)
{
	int cantidadDeBloques=tamanio/block_size;
	if(tamanio%blocks > 0) { cantidadDeBloques++; }
	//printf("tamanio: %d\n",cantidadDeBloques);
	return cantidadDeBloques;
}

bool actualizarBloques(char * rutaDeMetadata, t_list * lista)
{

	FILE* f = fopen(rutaDeMetadata,"r");
	if(f == NULL){
		log_error(log_gamecard,"actualizarBloques: No existe la metadata del pokemon.");
		return false;
	}else
	{
		fclose(f);
	}

	t_config* metadata_pokemon = config_create(rutaDeMetadata);

	char *bloques_config = config_get_string_value(metadata_pokemon, "BLOCKS");
	//printf("\nLOS BLOQUES: %s\n", bloques_config);
	char **bloques_array = config_get_array_value(metadata_pokemon,"BLOCKS");
    int viejoTamanio=atoi( config_get_string_value(metadata_pokemon, "SIZE"));
    //printf("Viejo tamanio: %d\n",viejoTamanio);
    int nuevoTamanio=calcularNuevoTamanio(lista);
    //printf("Nuevo tamanio: %d\n",nuevoTamanio);
    int viejaCantidadDeBloques= redondeoBloques(viejoTamanio);
    //printf("Viejos bloques: %d\n",viejaCantidadDeBloques);
    int nuevaCantidadDeBloques=redondeoBloques(nuevoTamanio);
    //printf("Nuevos bloques: %d\n",nuevaCantidadDeBloques);
    char* nuevoSize = malloc(10);
    int tamanio_bloques_config=strlen(bloques_config);
    int i=0;
	char * aEliminar=NULL;
    char*nuevo_bloques_config=NULL;
    char* nuevoBloqueString = malloc(10);

    log_trace(log_gamecard,"Bloques actuales: %s. Size actual: %d", bloques_config, viejoTamanio);

    	if(viejaCantidadDeBloques<nuevaCantidadDeBloques)
    	{
    		int nuevoBloque=obtener_bloque_libre(bitmap);
    		ocupar_bloque(bitmap,nuevoBloque);
    		log_trace(log_gamecard,"Bloque nuevo: %d",nuevoBloque);


    		sprintf(nuevoBloqueString, "%d",nuevoBloque);
    		//printf("\n*******************NUEVO BLOQUE DE EMI: %s *****************\n",nuevoBloqueString);

    		if(tamanio_bloques_config > 2){
			nuevo_bloques_config=(char*)malloc(tamanio_bloques_config+strlen(nuevoBloqueString)+2);
			strcpy(nuevo_bloques_config,"[");
			for(i=0; bloques_array[i]; i++)
			{
				strcat(nuevo_bloques_config, bloques_array[i]);
				strcat(nuevo_bloques_config, ",");
			}
			strcat(nuevo_bloques_config,nuevoBloqueString);
			strcat(nuevo_bloques_config,"]");
    		}else{
    			nuevo_bloques_config=(char*)malloc(tamanio_bloques_config+strlen(nuevoBloqueString)+2);
    			strcpy(nuevo_bloques_config,"[");
    			strcat(nuevo_bloques_config,nuevoBloqueString);
				strcat(nuevo_bloques_config,"]");

    		}
//    		log_trace(log_gamecard,"Nuevo bloques_config: %s.", nuevo_bloques_config);

    		config_set_value(metadata_pokemon, "BLOCKS", nuevo_bloques_config);
    		sprintf(nuevoSize,"%d",nuevoTamanio);
			config_set_value(metadata_pokemon,"SIZE",nuevoSize);

			if(config_save(metadata_pokemon) < 0)
			{
				log_error(log_gamecard,"actualizarBloques: Error al actualizar la metadata del pokemon.");
				free(nuevoSize);
				free(nuevo_bloques_config); config_destroy(metadata_pokemon);
				free(bloques_array);
				free(nuevoBloqueString);
				sem_post(&mutex_new_pokemon);
				return false;
			}else{

				log_trace(log_gamecard,"Nuevos bloques: %s Nuevo Size: %s", nuevo_bloques_config, nuevoSize);

			}
    	}
    	else
    	{
    		if(nuevaCantidadDeBloques< viejaCantidadDeBloques)
    		{
    			liberar_un_bloque(bloques_array[viejaCantidadDeBloques-1]);
    			int elbloque=atoi(bloques_array[viejaCantidadDeBloques-1]);
    			liberar_bloque(bitmap,elbloque);
    			log_trace(log_gamecard,"Se libera: %d bloque/es.", viejaCantidadDeBloques-1);

    			if(nuevaCantidadDeBloques == 0)
    			{
    				nuevo_bloques_config=malloc(3);
    				strcpy(nuevo_bloques_config,"[");
    				strcat(nuevo_bloques_config,"]");

    			}else
    			{
    			nuevo_bloques_config=malloc(tamanio_bloques_config-strlen(bloques_array[viejaCantidadDeBloques-1])-1);
    			memcpy(nuevo_bloques_config,bloques_config,tamanio_bloques_config-strlen(bloques_array[viejaCantidadDeBloques-1])-2);
    			strcat(nuevo_bloques_config,"]");
    			}

    			//printf("\t EMI: NUEVO BLOQUES CONFIG: %s ", nuevo_bloques_config);
    			config_set_value(metadata_pokemon, "BLOCKS", nuevo_bloques_config);
    			sprintf(nuevoSize,"%d",nuevoTamanio);
				config_set_value(metadata_pokemon,"SIZE",nuevoSize);

				if(config_save(metadata_pokemon) < 0)
				{
					log_error(log_gamecard,"ERROR: Hubo un error al actualizar la metadata del pokemon.");
					free(nuevoSize);
					free(nuevo_bloques_config);
					config_destroy(metadata_pokemon);
					free(bloques_array);
					free(nuevoBloqueString);
					sem_post(&mutex_new_pokemon);
					return false;
				}else
				{
					log_trace(log_gamecard,"Nuevos Bloques: %s Nuevo Size: %s.", nuevo_bloques_config, nuevoSize);
				}
    		}
    		else
    		{
    			sprintf(nuevoSize,"%d",nuevoTamanio);
					config_set_value(metadata_pokemon,"SIZE",nuevoSize);

				if(config_save(metadata_pokemon) < 0)
				{
					log_error(log_gamecard,"ERROR: Hubo un error al actualizar la metadata del pokemon\n");
					 free(nuevoSize);
					 free(nuevo_bloques_config); config_destroy(metadata_pokemon);
					free(bloques_array);
					free(nuevoBloqueString);
					sem_post(&mutex_new_pokemon);

					return false;
				}else
				{
					log_trace(log_gamecard,"Nuevo Size: %s", nuevoSize);
				}
    			//Si la cantidad de bloques es la misma
    			//liberar_un_bloque(bloques_arraỵ̣[viejaCantidadDeBloques-1]);
    		}

    	}
    	config_destroy(metadata_pokemon);
	int bytes_actuales = 0, bytes_restantes = 0, bytes_escritos = 0, longitudPersistida=0;
	char* posicionPersistida = NULL;


	bloques_config = NULL;
	metadata_pokemon = config_create(rutaDeMetadata);
	bloques_config = config_get_string_value(metadata_pokemon, "BLOCKS");
	//printf("\nLOS NUEVOS BLOQUES: %s\n", bloques_config);
	bloques_array = config_get_array_value(metadata_pokemon,"BLOCKS");
	char* rutaBloque =NULL;
	for(int i= 0; bloques_array[i];i++)
	{
    		bytes_escritos = 0;

    		liberar_un_bloque(bloques_array[i]);
			rutaBloque = (char*)malloc(strlen(dirBlocks)+ strlen(bloques_array[i])+ 10);
    		//strcpy(rutaBloque,"/home/utnso/Escritorio/TALL_GRASS/Blocks");
    		strcpy(rutaBloque, dirBlocks);
    		strcat(rutaBloque, "/");
    		strcat(rutaBloque, bloques_array[i]);
    		strcat(rutaBloque, ".bin");

		while( bytes_escritos < blocks/*tamanio de bloque*/)
		{
    		//printf("%s\n", rutaBloque);

    		if(bytes_restantes > 0 && bytes_escritos==0)
    		{
    			f = fopen(rutaBloque, "w");
    			if(f==NULL){
    				log_error(log_gamecard,"NO SE PUEDE ABRIR EL BLOQUE %d", i);
    				free(nuevoSize);
					free(nuevo_bloques_config);
					config_destroy(metadata_pokemon);
					free(bloques_array);
					free(nuevoBloqueString);
					free(rutaBloque);
					free(bloques_config);
					return false;
    			}

				fwrite(posicionPersistida+bytes_actuales,bytes_restantes /* +1*/,1,f);

				//printf("escribe el resto :%s, de tamaño: %d\n",posicionPersistida+bytes_actuales,bytes_restantes);

				bytes_escritos+=bytes_restantes;
				bytes_restantes = 0;

//				free(posicionPersistida); posicionPersistida=NULL;
				fclose(f);
    		}
    		else
    		{
    			if(list_size(lista) == 0){
					free(nuevoSize);
					free(nuevo_bloques_config);
					free(bloques_array);
					free(nuevoBloqueString);
					config_destroy(metadata_pokemon);
					list_destroy(lista);
					free(rutaBloque);
					return true;
				}

    			f = fopen(rutaBloque, "a");
    			if(f==NULL){
    				log_error(log_gamecard,"NO SE PUEDE ABRIR EL BLOQUE %d", i);
    				free(nuevoSize);
					free(nuevo_bloques_config);
					free(bloques_array);
					free(nuevoBloqueString);
					config_destroy(metadata_pokemon);
					free(rutaBloque);
					free(bloques_config);
					return false;
    			}

             if(aEliminar){
            	 free(aEliminar);
         		free(posicionPersistida);
             }
    		aEliminar=(char*)list_remove(lista,0);

    //		//printf("--------aEliminar: %s\n", aEliminar);
    		posicionPersistida = malloc(strlen(aEliminar) + 2);
    		strcpy(posicionPersistida, aEliminar);
    		strcat(posicionPersistida,"\n");
    		//free(aEliminar);
    		longitudPersistida=strlen(posicionPersistida);



    		//printf("Bytes restantes del bloque:%d, hay que escribir:%d, de: %s...\n",blocks-bytes_escritos,longitudPersistida, posicionPersistida);

    		if(strlen(posicionPersistida)+bytes_escritos </*=*/ blocks)
    		{
    			//printf("Entra completo (apa) y escribe %s\n",posicionPersistida);
				fwrite(posicionPersistida,longitudPersistida,1,f);
				bytes_escritos+=longitudPersistida;
//				free(posicionPersistida); posicionPersistida=NULL;

    		}
    		else
    		{
    			bytes_actuales = blocks - bytes_escritos;
    			fwrite(posicionPersistida,bytes_actuales,1,f);
    			bytes_restantes = longitudPersistida - bytes_actuales;
    			bytes_escritos+=bytes_actuales;
    			//printf("No entra una linea entera, escribe %d bytes\n",bytes_actuales);
    		}
    		fclose(f);
    		}
    	}//while
		free(rutaBloque);
		//rutaBloque=NULL;

    }//for
	free(bloques_array);
	free(nuevoBloqueString);
	free(nuevoSize);
	free(nuevo_bloques_config);
	config_destroy(metadata_pokemon);
	return true;

}

void *gestionarNewPokemon(void *socket)
{
	int cliente = (int) socket;
	////printf("Socket de New Pokemon: %d\n", cliente);
	t_list* lista;
	int operacion;
	int mensaje_id = 0;
	while(1)
	{
		 operacion = recibir_operacion(cliente);
		switch(operacion)
		{
			case NEW_POKEMON:

					sem_wait(&mutex_new_pokemon);
					lista = recibir_paquete(cliente);

					log_debug(log_gamecard, "NEW_POKEMON: ID = %d; Pokemon = %s; X = %d;  Y = %d; Cantidad = %d",
						*(int*) list_get(lista, 0),(char *)list_get(lista, 1),*(int*) list_get(lista, 2),
							*(int*) list_get(lista, 3),*(int*) list_get(lista, 4));

					t_paquete *paquete = crear_paquete(RECIBIDO);
					agregar_a_paquete(paquete, "OK", strlen("OK")+1);
					enviar_paquete(paquete, cliente);
					eliminar_paquete(paquete);
					 conexionBroker = crear_conexion(ipBroker, puertoBroker);


					 if(conexionBroker > 0)
					 {
						 if(agregarPokemon(lista))
						 {
							 mensaje_id =  *(int*)list_get(lista, 0);
							//char *pokemon = list_get(lista, 0);
							int coordenada_x = *(int*)list_get(lista, 2);
							int coordenada_y = *(int*)list_get(lista, 3);
							t_paquete *paquete2 = crear_paquete(APPEARED_POKEMON);
							agregar_a_paquete(paquete2, &mensaje_id, sizeof(int));
							agregar_a_paquete(paquete2, list_get(lista, 1), strlen(list_get(lista, 1))+1);
							agregar_a_paquete(paquete2, &coordenada_x, sizeof(int));
							agregar_a_paquete(paquete2, &coordenada_y, sizeof(int));

							enviar_paquete(paquete2, conexionBroker);
							eliminar_paquete(paquete2);
							log_trace(log_gamecard,"APPEARED_POKEMON enviado. \n");
							//recibirIdMensaje(conexionBroker);
						 }
						 else
							 log_trace(log_gamecard,"No se pudo agregar el pokemon.");

						 liberar_cliente(conexionBroker);
					 }
					else
					{
						log_error(log_gamecard,"No se puede establecer conexion con BROKER -> continuamos ejecucion.");
						if(agregarPokemon(lista))
							log_trace(log_gamecard,"Se pudo agregar el pokemon.");
						else
							log_trace(log_gamecard,"No se pudo agregar el pokemon.");
					}

					list_destroy_and_destroy_elements(lista,free);
					break;
        //---------------------------------------------------------------------------------------------------
            case -1: cliente=suscribirColaNewPokemon(); break;
            default: break;

		}
	}
}

void *gestionarCatchPokemon(void *socket)
{
	int cliente = (int) socket;
	////printf("Socket de Catch Pokemon: %d\n", cliente);
	t_list* lista;
	int operacion;
	int mensaje_id = 0;
	while(1)
	{
		 operacion = recibir_operacion(cliente);
		switch(operacion)
		{
		 case CATCH_POKEMON:
			sem_wait(&mutex_new_pokemon);
			lista = recibir_paquete(cliente);

			log_debug(log_gamecard, "CATCH_POKEMON: ID = %d; Pokemon = %s; X = %d; Y = %d",*(int*) list_get(lista, 0),
					(char *)list_get(lista, 1),*(int*) list_get(lista, 2),*(int*) list_get(lista, 3));

			t_paquete *paquete = crear_paquete(RECIBIDO);
			agregar_a_paquete(paquete, "OK", strlen("OK")+1);
			enviar_paquete(paquete, cliente);
			eliminar_paquete(paquete);

			conexionBroker = crear_conexion(ipBroker, puertoBroker);

			//ELEMENTO CATCH
			void* atrapar=(void*)malloc(sizeof(int));
			int menosUno=-1;
			memcpy(atrapar,&menosUno,sizeof(int));
			list_add(lista,atrapar);

			if(conexionBroker > 0)
			{
				mensaje_id = *(int*) list_get(lista, 0);
				t_paquete *paquete3 = crear_paquete(CAUGHT_POKEMON);
				agregar_a_paquete(paquete3,&mensaje_id, sizeof(int));

				if(agregarPokemon(lista))
					agregar_a_paquete(paquete3, "OK", strlen("OK")+1);
				else
					agregar_a_paquete(paquete3, "FAIL", strlen("FAIL")+1);

				enviar_paquete(paquete3, conexionBroker);
				eliminar_paquete(paquete3);
				log_trace(log_gamecard,"CAUGHT_POKEMON enviado. \n");
				//recibirIdMensaje(conexionBroker);
				liberar_cliente(conexionBroker);


			}
			else
			{
				log_error(log_gamecard,"No se puede establecer conexion con BROKER -> continuamos ejecucion.");
				if(agregarPokemon(lista))
					log_trace(log_gamecard,"Se pudo atrapar.");
				else
					log_trace(log_gamecard,"No se pudo atrapar.");
			}

			list_destroy_and_destroy_elements(lista,free);
			break;
			//---------------------------------------------------------------------------------------------------

            case -1: cliente=suscribirColaCatchPokemon(); break;
            default: break;
		}
	}
}

void *gestionarGetPokemon(void *socket)
{
	int cliente = (int) socket;
	////printf("\nSocket de Get Pokemon: %d\n", cliente);
	t_list* lista;
	int operacion;
	while(1)
	{
		operacion = recibir_operacion(cliente);

		switch(operacion)
		{
		case GET_POKEMON:
			sem_wait(&mutex_new_pokemon);
			lista = recibir_paquete(cliente);

					log_debug(log_gamecard, "GET_POKEMON: Mensaje ID = %d; Pokemon = %s",
							*(int*) list_get(lista, 0),
							(char *)list_get(lista, 1)
						);
		                // Envio paquete para avisar al broker que recibi el mensaje anterior
						t_paquete *paquete = crear_paquete(RECIBIDO);
						agregar_a_paquete(paquete, "OK", strlen("OK")+1);
						enviar_paquete(paquete, cliente);
						eliminar_paquete(paquete);

		                conexionBroker = crear_conexion(ipBroker, puertoBroker);
		                t_paquete* paquetengas;
						if(conexionBroker > 0)
						{
							paquetengas = getPokemon(lista);
							enviar_paquete(paquetengas, conexionBroker);
							eliminar_paquete(paquetengas);
							log_trace(log_gamecard,"LOCALIZED_POKEMON enviado. \n");
							//recibirIdMensaje(conexionBroker);
							liberar_cliente(conexionBroker);

						}
						else
						{
							log_error(log_gamecard,"No se puede establecer conexion con BROKER -> continuamos ejecucion.");
							paquetengas = getPokemon(lista);
							eliminar_paquete(paquetengas);
						}
		                list_destroy_and_destroy_elements(lista, free);
		                break;
		//---------------------------------------------------------------------------------------------
            case -1: cliente=suscribirColaGetPokemon(); break;
            default: break;
		}
	}
}

/**
 * Función que inicializa el módulo de Gamecard
 */
void iniciarGamecard()
{
	log_gamecard = log_create("log_gamecard.log", "gamecard", 1, LOG_LEVEL_TRACE);
	//cargarPuntoMontaje();
	crearEstructuraFileSystem();

	// Levantamos configuracion Gamecard
	ip = config_get_string_value(gamecardConfig, "IP");
	puerto = config_get_string_value(gamecardConfig, "PUERTO");
	tiempoIntentoReconexion = config_get_int_value(gamecardConfig,"TIEMPO_DE_REINTENTO_CONEXION");
	tiempo_reintento_operacion = config_get_int_value(gamecardConfig,"TIEMPO_DE_REINTENTO_OPERACION");

	servidorGamecard = iniciar_servidor(ip, puerto);
	//printf("Esperando clientes... IP: %s, PUERTO: %s, ID:%d \n", ip, puerto, servidorGamecard);
	//puts("------------------------------------------------------------------");

	// Levantamos configuracion Broker
	ipBroker = config_get_string_value(gamecardConfig,"IP_BROKER");
	puertoBroker = config_get_string_value(gamecardConfig,"PUERTO_BROKER");

	//Hilo de gestion gameboy
	pthread_create(&threadGameCardId,NULL,obtenerId,NULL);

	pthread_create(&threadGameBoy, NULL, gestionarConexionConGameboy, (void*)&servidorGamecard);

	pthread_join(threadGameCardId, NULL);
	pthread_detach(threadGameCardId);
	// Suscripción a las diferentes colas en broker
	int conexionBrokerNew=suscribirColaNewPokemon();
	pthread_create(&thread0, NULL, gestionarNewPokemon, (void *)conexionBrokerNew);

	int conexionBrokerCatch=suscribirColaCatchPokemon();
	pthread_create(&thread0, NULL, gestionarCatchPokemon, (void *)conexionBrokerCatch);

	int conexionBrokerGet=suscribirColaGetPokemon();
	pthread_create(&thread0, NULL, gestionarGetPokemon, (void *)conexionBrokerGet);


}

int main(int cantidad, char* argumentos[])
{
	//Creamos el logg.

	sem_init(&mutex_recibirId, 0, 1);
	sem_init(&mutex_new_pokemon, 0, 1);

	//Guardamos la ruta del archivo de configuracion en variable global.
    gamecardConfig = config_create(argumentos[1]);

    iniciarGamecard();

    pthread_join(threadGameBoy, NULL);


    //printf("\nFIN GAMECARD\n");
    liberar_servidor(servidorGamecard);
    return 0;
}
