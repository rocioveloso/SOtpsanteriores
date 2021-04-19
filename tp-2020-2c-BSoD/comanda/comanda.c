#include "comanda.h"


TablaSegmentos* getRestaurante(char* nombreRestaurante){

	int i = 0;
	while(i < list_size(restaurantes)) {
		TablaSegmentos *restaurante = (TablaSegmentos *) list_get(restaurantes, i);

		if (strcmp((char*) restaurante->nombre, nombreRestaurante) == 0) {
			return restaurante;
		}
		i++;
	}
	return NULL;
}

TablaSegmentos *getOrCreateRestaurante(char* nombreRestaurante){
	TablaSegmentos *restaurante = getRestaurante(nombreRestaurante);
	if(restaurante == NULL) {
		TablaSegmentos* nuevoRestaurante = (TablaSegmentos *) malloc(sizeof(TablaSegmentos));
		nuevoRestaurante->nombre = nombreRestaurante;
		nuevoRestaurante->pedidos = dictionary_create();
		list_add(restaurantes, nuevoRestaurante);
		return nuevoRestaurante;
	}
	return restaurante;
}

/*
TablaPagina getPedido(TablaSegmentos* restaurante, void* pedido){
	if(dictionary_has_key(restaurante->pedidos,(char*) pedido)){
		return dictionary_get(restaurante->pedidos,(char*) pedido);
	}else{
		return -2;
	}
}

void deletePedido(t_list* pedido){
	if(list_is_empty(pedido)){
		list_destroy_and_destroy_elements(pedido);
	}else{
		print("Aún hay platos en este pedido");
	}

}


int manejadorDeErrores(int error){
	switch(error){
		case -1:
			print("El restaurante no existe");
			return false;
			break;
		case -2:
			print("El pedido no existe");
			return false;
			break;
		default:
			return true;
			break;
	}
}

void gestionarAlgoritmoDeReemplazo(int plato){
	switch(algoritmoReemplazo) {
		case "LRU":
			print("LRU %d",plato);
			break;
		case "CLOCK_MEJORADO":
			print("CLOCK_MEJORADO %d",plato);
			break;
	}
}
*/

bool* guardarPedido(char* nombreRestaurante, char* idPedido){

	TablaSegmentos* restauranteObj = getOrCreateRestaurante(nombreRestaurante);
	TablaPaginas* pedido = (TablaPaginas*) malloc(sizeof(TablaPaginas));
	pedido->estado = PENDIENTE;
	pedido->registrosPlato = list_create();
	dictionary_put(restauranteObj->pedidos, idPedido, pedido);

	return true;
}


void *gestionarCliente(int cliente) {

	printf("Cliente: %d\n", cliente);
	t_list* lista;
	int operacion;
	t_paquete *paquete;

	printf("Servidor: %d\n", servidor);

	while(1) {
		operacion = recibir_operacion(cliente);
		lista = NULL;

		printf("\nLA OPERACION ES: %d\n", operacion);

		switch(operacion) {

			case INICIAR_CLIENTE:
				lista = recibir_paquete(cliente);
				printf("Conexion con Cliente...\n");
				printf("Los datos del cliente son:\n");
				printf("ID: %s\n", (char*) list_get(lista,0));
				printf("Posicion X: %d\n", *(int*) list_get(lista,1));
				printf("Posicion Y: %d\n", *(int*) list_get(lista,2));
				ipCliente = (char*) list_get(lista, 3);
				puertoEscuchaCliente = (char*) list_get(lista, 4);
				paquete = crear_paquete(COMANDA);
				enviar_paquete(paquete, cliente);
				eliminar_paquete(paquete);
				liberar_cliente(cliente);
				break;

			case GUARDAR_PEDIDO:
				printf("Mensaje de GUARDAR PEDIDO.\n");
				lista = recibir_paquete(cliente);


				char* nombreRestaurante = (char*)list_get(lista, 0);
				char* idPedido = (char*) list_get(lista, 1);
				int respuesta;
				if(guardarPedido(nombreRestaurante, idPedido)) {
					respuesta = MSG_OK;
				} else {
					respuesta = MSG_FAIL;
				}

				int conexionCliente = crear_conexion(ipCliente, puertoEscuchaCliente);
				t_paquete* paquete = crear_paquete(GUARDAR_PEDIDO);
				printf("respuesta %d\n", respuesta);
				agregar_a_paquete(paquete, &respuesta, 4);
				enviar_paquete(paquete, conexionCliente);
				eliminar_paquete(paquete);
				liberar_cliente(conexionCliente);

				break;

			case GUARDAR_PLATO:
				printf("Mensaje de GUARDAR PLATO.\n");
				printf("Restaurante: %s\n", (char*) list_get(lista, 0));
				printf("Pedido: %d\n", *(int*) list_get(lista, 1));
				printf("Comida: %s\n", (char*) list_get(lista, 2));
				printf("Cantidad: %d\n", (int*) list_get(lista, 3));
				paquete = crear_paquete(RESPUESTA_GUARDAR_PLATO);
				agregar_a_paquete(paquete, MSG_OK, 4); // MSG_OK o MSG_FAIL
				enviar_paquete(paquete, cliente);
				eliminar_paquete(paquete);
				break;

			case OBTENER_PEDIDO:
				lista = recibir_paquete(cliente);
				printf("Mensaje de OBTENER PEDIDO.\n");
				/*
				TablaSegmentos restaurante = getRestaurante((char*) list_get(lista, 0));
				if(restaurante != -1){
					TablaPagina pedido = getPedido(restaurante, *(int*) list_get(lista, 1));
					if(pedido != -2){
						paquete = crear_paquete(RESPUESTA_OBTENER_PEDIDO);
						agregar_a_paquete(paquete, &pedido, sizeof(TablaPagina));
						enviar_paquete(paquete, cliente);
						eliminar_paquete(paquete);
						liberar_cliente(cliente);
						break;
					}else{
						paquete = crear_paquete(RESPUESTA_OBTENER_PEDIDO);
						agregar_a_paquete(paquete, NO_EXISTE_PEDIDO, 4);
						enviar_paquete(paquete, cliente);
				     	eliminar_paquete(paquete);
						liberar_cliente(cliente);
						break;
				    }
				}else{
					paquete = crear_paquete(RESPUESTA_OBTENER_PEDIDO);
					agregar_a_paquete(paquete, NO_EXISTE_RESTAURANTE, 4);
					enviar_paquete(paquete, cliente);
					eliminar_paquete(paquete);
					liberar_cliente(cliente);
					break;
				}
				*/
				break;

			case CONFIRMAR_PEDIDO:
				printf("Mensaje de CONFIRMAR PEDIDO.\n");
				printf("Pedido: %d\n", *(int*) list_get(lista, 0));
				printf("Restaurante: %s\n", (char*) list_get(lista, 1));


				//{estado, [{plato1, plato2}]}
				t_comida comida1;
				comida1.nombre = "Milanesas";
				comida1.cantTotal = 5;
				comida1.cantLista = 2;

				t_comida comida2;
				comida1.nombre = "Fideos";
				comida1.cantTotal = 4;
				comida1.cantLista = 3;



				paquete = crear_paquete(RESPUESTA_CONFIRMAR_PEDIDO);
				agregar_a_paquete(paquete, MSG_OK, 4); // MSG_OK o MSG_FAIL
				enviar_paquete(paquete, cliente);
				eliminar_paquete(paquete);
				break;

			case PLATO_LISTO:
				printf("Mensaje de PLATO LISTO.\n");
				printf("Restaurante: %s\n", (char*) list_get(lista, 0));
				printf("Pedido: %d\n", *(int*)list_get(lista, 1));
				printf("Comida: %s\n", (char*) list_get(lista, 2));
				paquete = crear_paquete(RESPUESTA_PLATO_LISTO);
				agregar_a_paquete(paquete, MSG_OK, 4); // MSG_OK o MSG_FAIL
				enviar_paquete(paquete, cliente);
				eliminar_paquete(paquete);
				break;

			case FINALIZAR_PEDIDO:
				printf("Mensaje de FINALIZAR PEDIDO.\n");
				printf("Restaurante: %s\n", (char*) list_get(lista, 0));
				printf("Pedido: %d\n", *(int*)list_get(lista, 1));
				paquete = crear_paquete(RESPUESTA_FINALIZAR_PEDIDO);
				agregar_a_paquete(paquete, MSG_OK, 4); // MSG_OK o MSG_FAIL
				enviar_paquete(paquete, cliente);
				eliminar_paquete(paquete);
				break;

			case -1:
				printf("El cliente %d se desconecto.\n", cliente);
				liberar_cliente(cliente);
				return EXIT_FAILURE;

			default:
				printf("Operacion desconocida.\n");
				break;

		}
	}
}

void levantar_configuracion()
{
     ip = leer_config("IP", "comanda.config");
     puertoEscucha = leer_config("PUERTO_ESCUCHA", "comanda.config");
     tamanioMemoria = (uint32_t) atoi(leer_config("TAMANIO_MEMORIA", "comanda.config"));
     tamanioSwap = (uint32_t) atoi(leer_config("TAMANIO_SWAP", "comanda.config"));
     algoritmoReemplazo = leer_config("ALGORITMO_REEMPLAZO", "comanda.config");
     archivoDeLog = leer_config("ARCHIVO_LOG", "comanda.config");

     printf("Esperando clientes, IP: %s, PUERTO: %s, ID:%d \n", ip, puertoEscucha, servidor);

     printf("\n\ttamanioMemoria: %u\n\ttamanioSwap: %u\n\talgortimoReemplazo: %s\n\tarchivoDeLog: %s\n"
            ,tamanioMemoria, tamanioSwap, algoritmoReemplazo, archivoDeLog);

     puts("------------------------------------------------------------------");
}


void inicializar_comanda() {
	printf("################# Modulo CoMAnda #################\n");
	logger = log_create(archivoDeLog, "CoMAnda", 1, LOG_LEVEL_DEBUG);

	levantar_configuracion();
	memoriaPrincipal = malloc(tamanioMemoria);
	memoriaSwap = malloc(tamanioSwap);
	restaurantes = list_create();
	servidor = iniciar_servidor(ip, puertoEscucha);

}


int main(int t, char *args[] ){

	inicializar_comanda();


	while(1) {
		int cliente=-1;
		cliente = esperar_cliente(servidor);
		if(cliente!=-1) {
			pthread_create(&thread0, NULL, gestionarCliente, cliente);
			pthread_detach(thread0);
		}

	}

	pthread_join(thread0, NULL);

	return 0;
};
