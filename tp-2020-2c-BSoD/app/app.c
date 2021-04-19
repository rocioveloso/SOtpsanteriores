#include<app.h>


int main(int cant, char* argv[]){


	archivoLog = leer_config("ARCHIVO_LOG", archivoConfig);
	logger = log_create(archivoLog, "app", 1, LOG_LEVEL_DEBUG);


    servidor = iniciar_servidor(ip, puerto);
    ipComanda = leer_config("IP_COMANDA", archivoConfig);
    puertoComanda = leer_config("PUERTO_COMANDA", archivoConfig);

    puertoEschucha = leer_config("PUERTO_ESCUCHA", archivoConfig);

    algoritmo = leer_config("ALGORITMO_DE_PLANIFICACION", archivoConfig);




  if(strcmp(algoritmo, "SJF-SD") == 0){

    	estimadoInicial = atoi(leer_config("ESTIMACION_INICIAL",archivoConfig));
    	char* alphaAux = leer_config("ALPHA",archivoConfig);

    	alpha = atof(alphaAux);

    }


	app->repartidores = list_create();
	restaurantesEnElMapa = list_create();
	pedidosPendientesDeEntregar = list_create();
	inicializarRepartidor();

	conexionConRestaurante(socketRestaurante);



	pthread_create(&threadPlanificador, NULL, planificador, NULL);

	pthread_create(&threadRestaurante, NULL, conexionConRestaurante, (void*)&servidor);


  	appTermino = false;
    cantCambiosTotales = 0;
    cantCiclosCPUTotales = 0;



    sem_init(&sem_proceso,0, 0);
    sem_init(&sem_mx_proceso, 0, 1);
    sem_init(&sem_proceso_intercambio, 0, 1);

    iniciarApp();

    pthread_join(threadPlanificador,NULL);

    pthread_detach(threadPlanificador);


    log_trace(log_app, "Cantidad de ciclos de CPU totales: %d", cantCiclosCPUTotales);
    log_trace(log_app, "Cantidad de cambios de contexto realizados: %d", cantCambiosTotales);

    t_link_element* aux_repartidores = app->repartidores->head;
    while(aux_repartidores){
    	log_trace(log_app, "Cantidad total de ciclos de CPU del entrenador %c: %d", ((Repartidor*)aux_repartidores->data)->idRepartidor, ((Repartidor*)aux_repartidores->data)->cantCiclosCPUTotales);
    	aux_repartidores = aux_repartidores->next;
    }

    loguearEstadoFinal();

    liberarApp();


    close(servidor);

}


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void llegada_mensaje_app(void* socketServidor){

	t_list* lista;

	while(1) {

		int cliente = esperar_cliente(*((int*) socketServidor));
		int operacion;
		int mensaje_id = 0;
		operacion= recibir_mensaje(cliente);

		switch(operacion) {
			case CONSULTAR_RESTAURANTE:
				print("CONSULTAR RESTAURANTE\n");
				consultar_restaurante();

			case SELECCIONAR_RESTAURANTE:
				print("SELECCIONAR RESTAURANTE\n");
				seleccionar_restaurante(cliente);

			case CONSULTAR_PLATOS:
				print("CONSULTAR PLATOS\n");
				consultar_plato(cliente);

			case CREAR_PEDIDO:
				print("CREAR PEDIDO\n");
				crear_plato(cliente, restaurante);

			case ANIADIR_PLATO:
				print("ANIADIR PLATO\n");
				aniadir_plato(cliente);

			case PLATO_LISTO:
				print("PLATO LISTO\n");
				plato_listo();

			case CONFIRMAR_PEDIDO:
				print("CONFIRMAR PEDIDO\n");
				confirmar_pedido(cliente);

			case CONSULTAR_PEDIDO:
				print("CONSULTAR PEDIDO\n");
				consultar_pedido();


			default:
				printf("No se puede realizar dicha operación\n");
				break;

		}
	}
}



void consultar_restaurante(){
	t_elemento * restaurantes = lista -> head;
	if(restaurantes ==NULL){ printf("Resto Default")};
	while(restaurantes != NULL){
		log_info(logger, (char*) restaurantes -> nombre -> data);
		restaurantes = restaurantes -> next;
	}

};

void seleccionar_restaurante(int idCliente){
	lista = recibir_paquete(cliente_socket);
	t_elemento * restaurantes = lista -> head;
	if(restaurantes != NULL){
		//asigné primer restaurante de la lista
		list_add(lista, (char*)list_get(restaurantes, 0);
	}else{
		log_info(logger, "No hay restaurantes conectados");
	}
	liberar_cliente(cliente_socket);

};

void consultar_plato(int idCliente){

    lista = recibir_paquete(cliente_socket);
    nombre_restaurante = (char*)list_get(lista, 0);
    int cantidad_platos = *(int*)list_get(lista, 1);
    if (cantidad_platos > 0) {
        log_info(logger, "El restaurante %s tiene %d platos disponibles", nombre_restaurante, cantidad_platos);
        for (int i = 2; i < cantidad_platos * 2 + 1; i = i + 2) {
			log_info(logger, "Plato: %s - $%d", (char*)list_get(lista, i), *(int*)list_get(lista, i + 1));
		}
    } else {
        if (cantidad_platos == 0) {
        log_info(logger, "El restaurante %s no tiene platos disponibles", nombre_restaurante);
        } else {
            log_error(logger, "Error en  CONSULTAR_PLATOS desde restaurante %s", nombre_restaurante);
        }
    }
	liberar_cliente(cliente_socket);
};

int crear_pedido(int idCliente){

	lista = recibir_paquete(cliente_socket);
	nombre_restaurante = (char*) list_get(lista, 0);
	t_paquete* paqueteARestaurante;
	t_paquete * paqueteAComanda;
	char* nombre;
	int id_pedido;


	//enviar mensaje a restaurante
	paqueteARestaurante = crear_paquete(CREAR_PEDIDO);
	agregar_a_paquete(paqueteARestaurante, idCliente, strlen(idCliente)+1);

	if(nombre_restaurante != "Restaurante Default"){
			enviar_paquete(paqueteARestaurante, restaurante_socket);
			id_pedido = recibir_operacion(restaurante_socket);
	}else{
			id_pedido = generar_id_pedido();
	}

	list_add(lista, id_pedido);

	//enviar mensaje a comanda
	int comanda_socket = crear_conexion(IP_COMANDA, PUERTO_COMANDA);

	paqueteAComanda = crear_paquete(GUARDAR_PEDIDO);
    nombre = malloc(strlen(NOMBRE_RESTAURANTE) + 1);
    memcpy(nombre, NOMBRE_RESTAURANTE, strlen(NOMBRE_RESTAURANTE) + 1);
    agregar_a_paquete(paqueteAComanda, nombre, strlen(NOMBRE_RESTAURANTE) + 1);
 //   agregar_a_paquete(paqueteAComanda, lista, strlen(lista) +1);

    enviar_paquete(paqueteAComanda, comanda_socket);

    t_lista* listaComanda = recibir_paquete(comanda_socket);
    char* respuesta = *(int*)list_get(listaComanda, 3);
    if (respuesta != MSG_OK) {
    	log_error(logger, "Error en CREAR_PEDIDO por la Comanda");
    }

	return idCliente;

	liberar_servidor(comanda_socket);
	liberar_servidor(restaurante_socket);
	liberar_cliente(cliente_socket);
	eliminar_paquete(paqueteARestaurante);
	eliminar_paquete(paqueteAComanda);
	free(nombre);
};

int generar_id_pedido() {
    ID_PEDIDO++;
    return ID_PEDIDO;
}

int aniadir_plato(int idCliente, int id_pedido, char* plato, int cantidad){

	lista = recibir_pedido(cliente_socket);
	nombre_restaurante = (char*) list_get(lista, 0);

    int comanda_socket = crear_conexion(IP_COMANDA, PUERTO_COMANDA);

	t_paquete * paqueteRestaurante;
	t_paquete * paqueteComanda;
	char* nombre;

	if(nombre_restaurante != "Restaurante Default"){
		paqueteRestaurante = crear_paquete(ANIADIR_PEDIDO);

	    nombre = malloc(strlen(NOMBRE_RESTAURANTE) + 1);
	    memcpy(nombre, NOMBRE_RESTAURANTE, strlen(NOMBRE_RESTAURANTE) + 1);
	    agregar_a_paquete(paqueteRestaurante, nombre, strlen(NOMBRE_RESTAURANTE) + 1);

	    agregar_a_paquete(paqueteRestaurante, &id_pedido, sizeof(int));

	    nombre = malloc(strlen(plato) + 1);
	    memcpy(nombre, plato, strlen(plato) + 1);
	    agregar_a_paquete(paqueteRestaurante, nombre, strlen(plato) + 1);

	    agregar_a_paquete(paquete, &cantidad, sizeof(int));;

		enviar_paquete(paqueteRestaurante, restaurante_socket);
		t_lista * listaRestaurante = recibir_paquete(restaurante_socket);
		char* respuestaRestaurante = *(int*)list_get(listaRestaurante, 3);

		if(respuestaRestaurante != MSG_OK){
			log_error(logger, "Error en ANIADIR_PEDIDO del restaurante %s", nombre_restaurante);
		}
	}

	paqueteComanda = crear_paquete(ANIADIR_PEDIDO);

    nombre = malloc(strlen(NOMBRE_RESTAURANTE) + 1);
    memcpy(nombre, NOMBRE_RESTAURANTE, strlen(NOMBRE_RESTAURANTE) + 1);
    agregar_a_paquete(paqueteComanda, nombre, strlen(NOMBRE_RESTAURANTE) + 1);

    agregar_a_paquete(paqueteComanda, &id_pedido, sizeof(int));

    nombre = malloc(strlen(plato) + 1);
    memcpy(nombre, plato, strlen(plato) + 1);
    agregar_a_paquete(paqueteComanda, nombre, strlen(plato) + 1);

    agregar_a_paquete(paqueteComanda, &cantidad, sizeof(int));

	enviar_paquete(paqueteComanda, comanda_socket);
	t_lista * listaComanda = recibir_paquete(comanda_socket);
	char* respuestaComanda = *(int*)list_get(listaComanda, 3);

	if(respuestaComanda == MSG_OK){
		log_info(logger, "Respuesta de AGREGAR_PLATO del restaurante %s: se aniadio el plato %s correctamente en el pedido %d", nombre_restaurante, nombre_plato, id_pedido);
	}else{
		log_error(logger, "Error en ANIADIR_PEDIDO de la Comanda");
	}

	//informar al cliente ok

	liberar_servidor(comanda_socket);
	liberar_servidor(restaurante_socket);
	liberar_cliente(cliente_socket);
	eliminar_paquete(paqueteRestaurante);
	eliminar_paquete(paqueteComanda);
	free(nombre);

};

void plato_listo(char* nombre_restaurante){

	t_lista * listaRestaurante = recibir_paquete(restaurante_socket);
	char* nombre;

	int comanda_socket = crear_conexion(IP_COMANDA, PUERTO_COMANDA);

	paqueteComanda = crear_paquete(PLATO_LISTO);
    nombre = malloc(strlen(NOMBRE_RESTAURANTE) + 1);
    memcpy(nombre, NOMBRE_RESTAURANTE, strlen(NOMBRE_RESTAURANTE) + 1);
    agregar_a_paquete(paqueteComanda, nombre, strlen(NOMBRE_RESTAURANTE) + 1);

    enviar_paquete(paqueteComanda, comanda_socket);

    //2. ejecutar mensaje Obtener pedido


    liberar_servidor(comanda_stock);
    eliminar_paquete(paqueteComanda);
    free(nombre);
};

void confirmar_pedido(int idCliente, int id_pedido){

	//recibir pedido desde comanda
	t_lista* listaPedido = recibir_paquete(comanda_socket);
	nombre_restaurante = (char*)list_get(listaPedido, 0);

	if(nombre_restaurante != "Restaurante Default"){
		paqueteRestaurante= crear_paquete(PEDIDO);

		enviar_paquete(paqueteRestaurante, nombreRestaurante)

	}else{
		log_error(logger, "Error, no se encuentra restaurante %s", nombre_restaurante);
	}

	//generar pcb



};

void consultar_pedido(int idCliente, int id_pedido){

    lista = recibir_paquete(cliente_socket);
    nombre_restaurante = (char*)list_get(lista, 0);
    log_info(logger, "Restaurante: %s", nombre_restaurante);
    id_pedido = *(int*)list_get(lista, 1);
	int estado = *(int*)list_get(lista, 2);
	// TODO: loguear plato y cantidad
	if (estado == PENDIENTE) {
		log_info(logger, "Estado del pedido %d se encuentra pendiente", id_pedido, estado);
	}
	if (estado == CONFIRMADO) {
		log_info(logger, "Estado del pedido %d se encuentra confirmado", id_pedido, estado);
	}
	if (estado == TERMINADO) {
		log_info(logger, "Estado del pedido %d se encuentra finalizado", id_pedido, estado);
	}

	lista_platos = *(int*)list_get(lista,3);
	t_elemento* plato = lista_platos -> head;

	while(platos != NULL){
		nombre_plato = (char*) list_get(plato->data, 0);
		cantidad = *(int*) list_get(plato->data, 1);

		log_info(logger, "El plato %s tiene una cantidad de %d", nombre_plato, cantidad);
		platos ++;
	}

	liberar_cliente(cliente_socket);
};

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void iniciarApp(){

	app = (App*)malloc(sizeof(App));

	app->repartidores = list_create();
	restaurantesEnElMapa = list_create();
	pedidosPendientesDeEntregar = list_create();
	inicializarRepartidores();
	cargarObjetivosPendientes();
	//app->procesosDeIntercambio = list_create();
	app->procesos = list_create();

	pthread_create(&threadPlanificador, NULL, planificador, NULL);

	iniciarConexionConRestaurante();

    //mostrar_elementos(app->repartidores);
}

}

//----------------------------------------------------------------------------------------------------------------------------------------------------------

void inicializarRepartidor(){
	char* repartidores = leer_config("REPARTIDORES", archivoConfig);
	char* frecuenciaDescanso = leer_config("FRECUENCIA_DE_DESCANSO", archivoConfig);
	char* tiempoDescanso = leer_config("TIEMPO_DE_DESCANSO", archivoConfig);

	char** posicionRepartidor = string_get_string_as_array(repartidores);
	char** frecuenciaDeDescanso =  string_get_string_as_array(frecuenciaDescanso);
	char** tiempoDeDescanso =  string_get_string_as_array(tiempoDescanso);


	bool estadoLibre = True;
	//hace de un pedido a la vez.
	bool tienePedido = False;
	bool entregaFinalizada = False;

    while(posicionRepartidor[n] != NULL){
        //cargo repartidor
    	string_trim(&posicionRepartidor[n]);

        char **coordenadasRepartidor = string_split(posicionRepartidor[n], "|");
        Repartidor *nuevoRepartidor = malloc(sizeof(Repartidor));
        nuevoRepartidor->cantCiclosCPUTotales = 0;
        nuevoRepartidor -> pedido = list_create();
        //debo agregar el tiempo de descanso y el de frecuencia??
        nuevoRepartidor->idRepartidor = (char)n+65;
        //agrego las posiciones al repartidor
        nuevoRepartidor->posicion = (Posicion *) malloc(sizeof(Posicion));
        nuevoRepartidor->posicion->x = atoi(coordenadasRepartidor[0]);
        nuevoRepartidor->posicion->y = atoi(coordenadasReaprtidor[1]);
        free(coordenadasRepartidor[0]);
        free(coordenadasRepartidor[1]);


        nuevoRepartidor->EstadoRepartidor = NUEVO;

        list_add(team->repartidores, nuevoRepartidor);

        free(posicioneRepartidor[n]);
        free(coordenadasRepartidor);
        n++;

    }

    free(posicionRepartidor);

}

//--------------------------------CONEXION DE ESCUCHA CON RESTAURANTE------------------------------------------------------------------------------------------------------------


void *conexionConRestaurante(void* socketServidor){

	t_list* lista;

    while(!appTermino)
	{
        int cliente = esperar_cliente(*((int*)socketServidor));
		int operacion = recibir_conexion(cliente);
		int posicionX,posicionY;
		char* nombreRestaurante = NULL;
		t_list* listaPlatos;

		while(operacion)
		{

                lista = recibir_paquete(cliente);
                nombreRestaurante = (char*)malloc(strlen(lista->head->data)+1);
                memcpy(nombreRestaurante,lista->head->data,strlen(lista->head->data)+1);
                posicionX = *(int *) lista->head->next->data;
                posicionY = *(int *) lista->head->next->next->data;

                //pasar la lista de platos, VERIFICAR!!!!
                listaPlatos = t_list* lista->head->next->next->next->data;
                list_destroy_and_destroy_elements(lista,free);
                restauranteConectado(nombreRestaurante, posicionX, posicionY, listaPlatos);

		}
	}
    return NULL;
}



void restauranteConectado(char* nombreRestaurante, int posicionX, int posicionY, t_list* listaPlatos){

    t_link_element* aux_restauranteEnElMapa = restauranteEnElMapa->head;

    Restaurante* restaurante = (Restaurante*)malloc(sizeof(Restaurante));
    restaurante->nombre = nombreRestaurante;
    Posicion* posicion = (Posicion*)malloc(sizeof(Posicion));
    posicion->x = posicionX;
    posicion->y= posicionY;
    restaurante->posicion = posicion;
    Platos* platos = (Platos*)malloc(sizeof(Platos));
    restaurante -> platos = listaPlatos;

    while(aux_restauranteEnElMapa){
        if(((RestauranteEnMapa*)(aux_restaurantesEnElMapa->data))->nombre == nombreRestaurante){
        	//encuentra restaurante
            list_add(((RestauranteEnMapa*)(aux_restaurantesEnElMapa->data))->posiciones,posicion);
            list_add(((RestauranteEnMapa*)(aux_restaurantesEnElMapa->data))->platos,listaPlatos);
            break;
        }
        aux_restaurantesEnElMapa = aux_restaurantesEnElMapa->next;
    }
    if(aux_restaurantesEnElMapa == NULL){
    	//no encuentra restaurante
    	RestauranteEnMapa* restauranteNuevo = (RestauranteEnMapa*)malloc(sizeof(RestauranteEnMapa));
        restauranteNuevo->nombre = nombreRestaurante;
        restauranteNuevo->posiciones = list_create();
        restauranteNuevo -> platos = list_create()
        list_add(restauranteNuevo->posiciones,posicion);
        list_add(restauranteNuevo -> platos,listaPlatos);
        list_add(restaurantesEnElMapa,restauranteNuevo);

    }

    t_link_element* auxRepartidores = app->repartidores->head;
	t_list* todosPlatosPendientes = list_create();

    while(auxRepartidores != NULL){
		list_add_all(todosPlatosPendientes, ((Repartidores*)auxRepartidores->data)->pedidosPendientes);
		auxRepartidores = auxRepartidores->next;
	}


	if(list_find(todosPlatosPendientes, mismoIdPedido)!= NULL){
		//encontrar los pedidos de 1 cliente



		//seleccionar al repartidor más cercano
		repartidorCercano(restaurante->posicion->x, restaurante->posicion->y);

	}
}


//-----------------------------------------------PLANIFICACION----------------------------------------------------------------------------------------------

void planificador(){

	if(strcmp(algoritmo, "SJD-SD") == 0){

		sjd();

	}else{
			if(strcmp(algoritmo, "FIFO") == 0){

					fifo();
			}else{
				hrrn();
			}
	}

	return;
	}
}



void sjf(){

	while(!appTermino){
		sem_wait(&sem_proceso);

		sem_wait(&sem_mx_proceso);

		if(app->procesos->elements_count > 0){

			irRetirarPedido((Proceso*)app->procesos->head->data);


		}
		else
			return;
	}
}



void fifo(){

	while(!appTermino){
		sem_wait(&sem_proceso);

		sem_wait(&sem_mx_proceso);

		if(app->procesos->elements_count > 0){

			irRetirarPedido((Proceso*)app->procesos->head->data);
		}
		else
			return;
	}
}



void hrrn(){

	while(!appTermino){

			sem_wait(&sem_proceso);

			sem_wait(&sem_mx_proceso);

			if(app->procesos->elements_count > 0){

				//irRetirarPedido((Proceso*)app -> procesos -> head->data);
				irRetirarPedidoHHRR((Proceso*)app-> procesos -> head ->data);
			}
			else
				return;
		}
	}
}

void agregarProcesoACola(Repartidor* repartidor,Restaurante* restaurante){
//	if(!arrancaPlanificador){
//		arrancaPlanificador = true;
//		pthread_create(&threadPlanificador, NULL, planificador, NULL);
//	}

	Proceso* nuevoProceso = (Proceso*)malloc(sizeof(Proceso));
	nuevoProceso->restaurante = restaurante;
	nuevoProceso->repartidor = repartidor;
	list_add(app->procesos, (void*)nuevoProceso);
	if(strcmp(algoritmo,"SJF-SD") == 0){
		nuevoProceso->estimadoAnterior = estimadoInicial;
		nuevoProceso->rafagaAnterior = 0;
		nuevoProceso->estimadoActual = calcularEstimado(nuevoProceso->estimadoAnterior, nuevoProceso->rafagaAnterior);
		////printf("Repartidor: %c Estimado: %f\n", nuevoProceso->repartidor->idRepartidor, nuevoProceso->estimadoActual);
		list_sort(app->procesos, menorEstimado);

		interrupcion = strcmp(algoritmo,"SJF-SD") == 0;
	}

	repartidor->estado = LISTO;

	sem_post(&sem_proceso);
}


float distancia(int x1, int y1, int x2, int y2) {
    return sqrt(pow(x2 - x1,2) + pow(y2 - y1,2));
}

Repartidor* repartidorCercano(int posicionX,int posicionY){

	t_elemento* repartidor = repartidores->head;
    int distanciaMinima = 10000;
    Repartidor* repartidorSeleccionado = NULL;
    while(repartidor) {//se fija que repartidor tiene la menor distancia

    	if(((Repartidor*)repartidor->data)->estado == NUEVO || ((Repartidor*)repartidor->data)->estado == BLOQUEADO_REPLANIFICACION){
			if(((Repartidor*)repartidor->data)->objetivos->elements_count > ((Repartidor*)repartidor->data)->pedidoRetirado->elements_count) {
				int distanciaActual = distancia(((Repartidor *) repartidor->data)->posicion->x,
												((Repartidor *) repartidor->data)->posicion->y, posicionX, posicionY);
				if (distanciaActual < distanciaMinima) {
					distanciaMinima = distanciaActual;
					repartidorSeleccionado = (Repartidor *) repartidor->data;
				}
			}
        }
        repartidor= repartidor->next;
    }

    return repartidorSeleccionado;
}


void irRetirarPedido(Proceso* proceso){

	Restaurante* restaurante = proceso->restaurante;
	int rafaga = 0;
	Repartidor* repartidorCercano = proceso->repartidor;

	repartidorCercano-> estado = EJECUTANDO;
	cantCambiosTotales++;


	while (repartidorCercano->posicion->x != restaurante->posicion->x || reaprtidorCercano->posicion->y != restaurante->posicion->y){

		if(interrupcion){

			cantCambiosTotales++;
			repartidorCercano->estado = LISTO;
			proceso->estimadoAnterior = proceso->estimadoActual;
			proceso->rafagaAnterior = rafaga;
			proceso->estimadoActual = calcularEstimado(proceso->estimadoAnterior, proceso->rafagaAnterior);

			interrupcion = false;
			sem_post(&sem_proceso);
			sem_post(&sem_mx_proceso);
			return;

		}

		if(repartidorCercano->posicion->x != restaurante->posicion->x){


            if(repartidorCercano->posicion->x > restaurante->posicion->x){
                repartidorCercano->posicion->x--;

			}else{

				repartidorCercano->posicion->x++;
				rafaga++;
				repartidorCercano->cantCiclosCPUTotales++;
				cantCiclosCPUTotales++;
				sleep(retardoCPU);

		}else{

			if(repartidorCercano->posicion->y != restaurante->posicion->y){

				if(repartidorCercano->posicion->y > restaurante->posicion->y){
					repartidorCercano->posicion->y--;
			}else{

                repartidorCercano->posicion->y++;
                rafaga++;
                repartidorCercano->cantCiclosCPUTotales++;
                cantCiclosCPUTotales++;
                sleep(retardoCPU);
			}

        }

		//el repartidor más cercano se acerca a la posicion del restaurante
    }



    retirarPedido(Proceso * proceso);

};

void irRetirarPedidoHRRN(Proceso* proceso){


	Restaurante* restaurante = proceso->restaurante;
	int rafaga = 0;
	Repartidor* repartidorCercano = proceso->repartidor;

	repartidorCercano-> estado = EJECUTANDO;
	cantCambiosTotales++;



}


void retirarPedido(Proceso * proceso){
	//agregarle al repartidor el pedido

	Restaurante* restaurante = proceso->restaurante;
	Repartidor* repartidorCercano = proceso->repartidor;

    if(!tienePedido){

    	int id_pedido = /*tomar de restaurante*/;
    	memcpy(id_pedido, strlen(/*algo*/)+1);
    	//POSICION DEL CLIENTE??????
    	int estado =  /*tomar de restaurante*/;
    	memcpy(estado, strlen(/*algo*/)+1);
    	int platos = /*tomar de restaurante*/;
    	memcpy(platos, strlen(/*algo*/)+1);

		list_add(repartidor->pedido, id_pedido);
		//POSICION DEL CLIENTE?????
		list_add(repartidor->pedido, estado);
		list_add(repartidor->pedido, platos);

		estadoLibre = FALSE;
		tienePedido = TRUE;
		estadoFinalizado = FALSE;

		//printf("PEDIDO AGARRADO: %s \n", id_pedido);

    }else{
    	log_info(logger, "El Repartidor %s ya tiene el pedido %s", repartidor-> idRepartidor, id_pedido);
    }

}


void irAEntregarPedido(){

	Cliente* cliente = proceso->cliente;
	int rafaga = 0;
	Repartidor* repartidor = proceso->repartidor;

	while (repartidor->posicion->x != cliente->posicion->x || repartidor->posicion->y != cliente->posicion->y){

		if(interrupcion){

			cantCambiosTotales++;
			repartidor->estado = LISTO;
			proceso->estimadoAnterior = proceso->estimadoActual;
			proceso->rafagaAnterior = rafaga;
			proceso->estimadoActual = calcularEstimado(proceso->estimadoAnterior, proceso->rafagaAnterior);

			interrupcion = false;
			sem_post(&sem_proceso);
			sem_post(&sem_mx_proceso);
			return;

		}

		if(repartidor->posicion->x != cliente->posicion->x){


            if(repartidor->posicion->x > cliente->posicion->x){
                repartidor->posicion->x--;

			}else{

				repartidor->posicion->x++;
				rafaga++;
				repartidor->cantCiclosCPUTotales++;
				cantCiclosCPUTotales++;
				sleep(retardoCPU);

		}else{

			if(repartidor->posicion->y != cliente->posicion->y){

				if(repartidor->posicion->y > cliente->posicion->y){
					repartidor->posicion->y--;
			}else{

                repartidor->posicion->y++;
                rafaga++;
                repartidor->cantCiclosCPUTotales++;
                cantCiclosCPUTotales++;
                sleep(retardoCPU);
			}

        }

		//el repartidor se acerca a la posicion del cliente
    }

	entregarPedido(Proceso * proceso );

}

void entregarPedido(Proceso * proceso){

	Cliente* cliente = proceso->cliente;
	Repartidor* repartidor = proceso->repartidor;

    pedido = repartidor -> pedido;
    idCliente = cliente ->idCliente;
    id_pedido = *(int*)list_get(pedido,0);
    log_info(logger, "Pedido: ", id_pedido);
	int estado = *(int*)list_get(pedido, 1);

	if(cliente->pedido->id_pedido == id_pedido){
		list_delete(repartidor-> pedido);
		enviarPaquete(pedido, cliente_socket);
		repartidor -> estado == FINALIZADO;
		estadoLibre == TRUE;
		tienePedido == FALSE;
		estadoFinalizado == TRUE;
		log_info(logger, "El repartidor %s entregó al cliente %s el pedido %s", repartidor -> idrepartidor, id_cliente, id_pedido);
	}else{
		log_info(logger, "El pedido %s no pertenece al cliente %d", id_pedido, id_cliente);
		repartidor -> estado = BLOQUEADO;
	}
}


//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool repartidorBloqueado(void* elemento){
	Repartidor* repartidor = (Repartidor*)elemento;

	return Repartidor->estado == BLOQUEADO;
}
bool repartidorFinalizado(void* elemento){
	Repartidor* repartidor = (Repartidor*)elemento;

	return repartidor->estado == FINALIZADO;
}
bool repartidorFinalizadoOBloqueado(void* element){
	Repartidor* repartidor = (Repartidor*)element;

	return (repartidor->estado == BLOQUEADO) || (repartidor->estado == FINALIZADO);
}

float calcularEstimado(float estimadoAnterior, int rafagaAnterior){
	return estimadoAnterior * alpha + (1-alpha)*rafagaAnterior;
}


bool menorEstimado(void* proceso1, void* proceso2){

	return ((Proceso*)proceso1)->estimadoActual <= ((Proceso*)proceso2)->estimadoActual;
}

