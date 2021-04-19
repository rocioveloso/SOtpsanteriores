#include "cliente.h"

void levantar_configuracion() {
    ip = leer_config("IP", "cliente.config");
    puerto = leer_config("PUERTO", "cliente.config");
	puerto_app = leer_config("PUERTO_APP", "cliente.config");
    archivo_log = leer_config("ARCHIVO_LOG", "cliente.config");
	posicion_x = atoi(leer_config("POSICION_X", "cliente.config"));
	posicion_y = atoi(leer_config("POSICION_Y", "cliente.config"));
	id_cliente = leer_config("ID_CLIENTE", "cliente.config");
	puerto_escucha = leer_config("PUERTO_ESCUCHA", "cliente.config");
}

void conexion(int socket, char* id_cliente) {
	t_paquete* paquete = crear_paquete(INICIAR_CLIENTE);
	char* longitud = strlen(id_cliente);
	char* nombre = malloc(longitud + 1);
    memcpy(nombre, id_cliente, longitud + 1);
	agregar_a_paquete(paquete, nombre, longitud + 1);
	enviar_paquete(paquete, socket);
	eliminar_paquete(paquete);
}

void* gestionarMenuApp() {

	char* opcion_a = malloc(8);
	strcpy(opcion_a, "0");

	while(1) {
		printf("------ MODULO APP ------\n");
		printf("1. Consultar restaurantes\n");
		printf("2. Seleccionar restaurante\n");
		printf("3. Consultar platos\n");
		printf("4. Crear pedido\n");
		printf("5. Agregar plato\n");
		printf("6. Confirmar pedido\n");
		printf("7. Plato listo\n");
		printf("8. Consultar pedido\n\n");
		printf("Salir <intro>\n\n\n");

		free(opcion_a);
		opcion_a = readline(">");
		if(strncmp(opcion_a, "", 1) == 0) {
			free(opcion_a);
			exit(-1);
		}

		switch(atoi(opcion_a)) {
			t_paquete* paquete;
			char* opcion_r;
			int id_pedido;
			int nombre_restaurante;
			case 1:
				printf("--- CONSULTAR RESTAURANTES ---\n");
				app_socket = crear_conexion(ip, puerto);
			    paquete = crear_paquete(CONSULTAR_RESTAURANTE);
			    enviar_paquete(paquete, app_socket);
			    eliminar_paquete(paquete);
				break;

			case 2:
				printf("--- SELECCIONAR RESTAURANTE ---\n");
				app_socket = crear_conexion(ip, puerto);
			    paquete = crear_paquete(SELECCIONAR_RESTAURANTE);
			    enviar_paquete(paquete, app_socket);
			    eliminar_paquete(paquete);
				break;

			case 3:
				printf("--- CONSULTAR PLATOS ---\n");
				app_socket = crear_conexion(ip, puerto);
			    paquete = crear_paquete(CONSULTAR_PLATO);
			    enviar_paquete(paquete, app_socket);
			    eliminar_paquete(paquete);
				break;

			case 4:
				printf("--- CREAR PEDIDO ---\n");
				app_socket = crear_conexion(ip, puerto);
			    paquete = crear_paquete(CREAR_PEDIDO);
			    enviar_paquete(paquete, app_socket);
			    eliminar_paquete(paquete);
				break;

			case 5:
				printf("--- AGREGAR PLATO ---\n");
				app_socket = crear_conexion(ip, puerto);
			    paquete = crear_paquete(AGREGAR_PLATO);
                opcion_r = readline("Ingrese numero de pedido: ");
                id_pedido = atoi(opcion_r);
                agregar_a_paquete(paquete, &id_pedido, sizeof(int));
                char* nombre_plato = readline("Ingrese plato: ");
                char* nombre = malloc(strlen(nombre_plato) + 1);
                memcpy(nombre, nombre_plato, strlen(nombre_plato) + 1);
                agregar_a_paquete(paquete, nombre, strlen(nombre_plato) + 1);
                opcion_r = readline("Ingrese cantidad: ");
                int cantidad = atoi(opcion_r);
                agregar_a_paquete(paquete, &cantidad, sizeof(int));
			    enviar_paquete(paquete, app_socket);
			    eliminar_paquete(paquete);
				break;

			case 6:
				printf("--- CONFIRMAR PEDIDO ---\n");
				app_socket = crear_conexion(ip, puerto);
			    paquete = crear_paquete(CONFIRMAR_PEDIDO);
				opcion_r = readline("Ingrese numero de pedido: ");
				id_pedido = atoi(opcion_r);
				agregar_a_paquete(paquete, &id_pedido, sizeof(int));
			    enviar_paquete(paquete, app_socket);
			    eliminar_paquete(paquete);
				break;

			case 7:
				printf("--- PLATO LISTO ---\n");
				app_socket = crear_conexion(ip, puerto);
			    paquete = crear_paquete(PLATO_LISTO);
				opcion_r = readline("Ingrese nombre del restaurante: ");
				nombre_restaurante = atoi(opcion_r);
				agregar_a_paquete(paquete, &nombre_restaurante, sizeof(int));
			    enviar_paquete(paquete, app_socket);
			    eliminar_paquete(paquete);
				break;

			case 8:
				printf("--- CONSULTAR PEDIDO ---\n");
				app_socket = crear_conexion(ip, puerto);
			    paquete = crear_paquete(CONSULTAR_PEDIDO);
				opcion_r = readline("Ingrese numero de pedido: ");
				id_pedido = atoi(opcion_r);
				agregar_a_paquete(paquete, &id_pedido, sizeof(int));
			    enviar_paquete(paquete, app_socket);
			    eliminar_paquete(paquete);
				break;

			default:
				printf("ERROR: opción no válida\n");
				break;
		}

	}

	free(opcion_a);

//	return;
}

void* gestionarMenuComanda() {
	char* opcion_c = malloc(8);
	strcpy(opcion_c, "0");
	socket_conexion = crear_conexion(ip, puerto);

	while(1) {
		printf("------ MODULO COMANDA ------\n");
		printf("1. Guardar Pedido\n");
		printf("2. Guardar Plato\n");
		printf("3. Obtener Pedido\n");
		printf("4. Confirmar pedido\n");
		printf("5. Plato Listo\n");
		printf("6. Finalizar Pedido\n\n");
		printf("Salir <intro>\n\n\n");
//		socket_conexion = crear_conexion(ip, puerto);

		free(opcion_c);
		printf("\n");
		opcion_c = readline(">");
		if(strncmp(opcion_c, "", 1) == 0) {
			free(opcion_c);
			exit(-1);
		}

		switch(atoi(opcion_c)) {
			case 1: ;
				//char* nombreRestaurante = malloc(50);
				//char* idPedido = malloc(10);
				printf("--- GUARDAR PEDIDO ---\n");
				char* nombreRestaurante = readline("Nombre de Restaurante: ");
				int idPedido = atoi(readline("ID de Pedido: "));

				printf("\nLos datos ingresados son: %s , %d\n", nombreRestaurante, idPedido);
				t_paquete* paquete_1 = crear_paquete(GUARDAR_PEDIDO);
				agregar_a_paquete(paquete_1, nombreRestaurante, strlen(nombreRestaurante) + 1);
				agregar_a_paquete(paquete_1, &idPedido, 4);
				enviar_paquete(paquete_1, socket_conexion);
				eliminar_paquete(paquete_1);
				printf("\nLos datos fueron enviados a comanda!!!!\n");
				liberar_cliente(&socket_conexion);

				// logica para guardar pedido
				break;

			case 2:
				printf("--- GUARDAR PLATO ---\n");
				// logica para guardar plato
				break;

			case 3:
				printf("--- OBTENER PEDIDO ---\n");
				t_paquete* paquete = crear_paquete(OBTENER_PEDIDO);
				char* nombre = "restaurante1";
				int pedido = 12;
				agregar_a_paquete(paquete, nombre, strlen(nombre) + 1);
				agregar_a_paquete(paquete, &pedido, 4);
				enviar_paquete(paquete, socket_conexion);
				eliminar_paquete(paquete);
				printf("Paquete enviado\n");


				printf("hola\n");
				t_list* respuesta = recibir_paquete(socket_conexion);
				printf("chau\n");
				t_pedido* unPedido = malloc(sizeof(t_pedido));
				//unPedido = (t_pedido*)list_get(respuesta, 0);
				printf("%d\n", sizeof(unPedido));
				printf("%d\n", sizeof(t_pedido));
				//printf("Estado: %s\n", unPedido->estado);
				/*t_pedido* unPedido = (t_pedido*)list_get(respuesta, 0);
				printf("Estado: %s\n", unPedido->estado);
*/
				// logica para obtener pedido
				break;

			case 4:
				printf("--- CONFIRMAR PEDIDO ---\n");
				// logica para confirmar pedido
				break;

			case 5:
				printf("--- PLATO LISTO ---\n");
				// logica para plato listo
				break;

			case 6:
				printf("--- FINALIZAR PEDIDO ---\n");
				// logica para finalizar pedido
				break;

			default:
				printf("ERROR: opción no válida\n");
				break;
		}
		liberar_cliente(socket_conexion);
	}

	free(opcion_c);

//	return;
}

void* gestionarMenuRestaurante() {

	char* opcion_r = malloc(8);
	strcpy(opcion_r, "0");

	int restaurante_socket;
	t_paquete* paquete;
	int operacion;
	t_list* lista;
	int id_pedido;

	while(1) {
		printf("------ MODULO RESTAURANTE ------\n");
		printf("1. Consultar platos\n");
		printf("2. Crear pedido\n");
		printf("3. Agregar plato\n");
		printf("4. Confirmar pedido\n");
		printf("5. Consultar pedido\n\n");
		printf("Salir <intro>\n\n\n");

		//free(opcion_r);
		opcion_r = readline(">");
		if(strncmp(opcion_r, "", 1) == 0) {
			free(opcion_r);
			exit(-1);
		}
		switch(atoi(opcion_r)) {
			case 1:
				restaurante_socket = crear_conexion(ip, puerto);
			    paquete = crear_paquete(CONSULTAR_PLATOS);
			    enviar_paquete(paquete, restaurante_socket);
			    eliminar_paquete(paquete);
				break;
			case 2:
                restaurante_socket = crear_conexion(ip, puerto);
                paquete = crear_paquete(CREAR_PEDIDO);
                enviar_paquete(paquete, restaurante_socket);
                eliminar_paquete(paquete);
                break;
			case 3:
				restaurante_socket = crear_conexion(ip, puerto);
                paquete = crear_paquete(AGREGAR_PLATO);
                char* nombre_plato = readline("Ingrese plato: ");
                char* nombre = malloc(strlen(nombre_plato) + 1);
                memcpy(nombre, nombre_plato, strlen(nombre_plato) + 1);
                agregar_a_paquete(paquete, nombre, strlen(nombre_plato) + 1);
                opcion_r = readline("Ingrese numero de pedido: ");
                id_pedido = atoi(opcion_r);
                agregar_a_paquete(paquete, &id_pedido, sizeof(int));
                opcion_r = readline("Ingrese cantidad: ");
                int cantidad = atoi(opcion_r);
                agregar_a_paquete(paquete, &cantidad, sizeof(int));
                enviar_paquete(paquete, restaurante_socket);
                eliminar_paquete(paquete);
				break;
			case 4:
				restaurante_socket = crear_conexion(ip, puerto);
				paquete = crear_paquete(CONFIRMAR_PEDIDO);
				opcion_r = readline("Ingrese numero de pedido: ");
				id_pedido = atoi(opcion_r);
				agregar_a_paquete(paquete, &id_pedido, sizeof(int));
                enviar_paquete(paquete, restaurante_socket);
                eliminar_paquete(paquete);
				break;
			case 5:
				restaurante_socket = crear_conexion(ip, puerto);
				paquete = crear_paquete(CONSULTAR_PEDIDO);
				opcion_r = readline("Ingrese numero de pedido: ");
				id_pedido = atoi(opcion_r);
				agregar_a_paquete(paquete, &id_pedido, sizeof(int));
                enviar_paquete(paquete, restaurante_socket);
                eliminar_paquete(paquete);
				break;
			default:
				printf("ERROR: opción no válida");
				break;
		}
	}

	free(opcion_r);

	return;
}

void* gestionarMenuSindicato() {
	char* opcion_s = malloc(8);
	strcpy(opcion_s, "0");

	while(1) {
		printf("\n------ MODULO SINDICATO ------\n");
		printf("1. Consultar platos\n");
		printf("2. Guardar pédido\n");
		printf("3. Guardar plato\n");
		printf("4. Confirmar pedido\n");
		printf("5. Obtener pedido\n");
		printf("6. Obtener restaurante\n");
		printf("7. Plato listo\n");
		printf("8. Terminar pedido\n");
		printf("9. Obtener receta\n\n");
		printf("Salir <intro>\n\n\n");

		free(opcion_s);
		opcion_s = readline(">");
		if(strncmp(opcion_s, "", 1) == 0) {
			free(opcion_s);
			exit(-1);
		}

		switch(atoi(opcion_s)) {
			case 1:
				printf("--- CONSULTAR PLATOS ---\n");
				// logica para consultar los platos
				break;

			case 2:
				printf("--- GUARDAR PEDIDO ---\n");
				// logica para guardar pedido
				break;

			case 3:
				printf("--- GUARDAR PLATO ---\n");
				// logica para guardar plato
				break;

			case 4:
				printf("--- CONFIRMAR PEDIDO ---\n");
				// logica para confirmar pedido
				break;

			case 5:
				printf("--- OBTENER PEDIDO ---\n");
				// logica para obtener pedido
				break;

			case 6:
				printf("--- OBTENER RESTAURANTE ---\n");
				// logica para obtener restaurante
				break;

			case 7:
				printf("--- PLATO LISTO ---\n");
				// logica para un plato listo
				break;

			case 8:
				printf("--- TERMINAR PEDIDO ---\n");
				// logica para terminar pedido
				break;

			case 9:
				printf("--- OBTENER RECETA ---\n");
				// logica para obtener receta
				break;

			default:
				printf("ERROR: opción no válida\n");
				break;
		}

	}

	free(opcion_s);
//	return;
}

void* gestionarMenu(int modulo_conectado) {
	switch (modulo_conectado) {
		case APP:
			gestionarMenuApp();
			break;
		case COMANDA:
			gestionarMenuComanda();
			break;
		case RESTAURANTE:
			gestionarMenuRestaurante();
			break;
		case SINDICATO:
			gestionarMenuSindicato();
			break;
		default:
			printf("ERROR: opción no válida\n");
			break;
		}

//	return;
}

void* recepcionar_respuestas() {
	puerto_escucha = leer_config("PUERTO_ESCUCHA", "cliente.config");
	//log_info(logger, "Iniciando server en %s", puerto_escucha);
	int escucha_socket = iniciar_servidor("127.0.0.1", puerto_escucha);
    t_list* lista;
    int operacion;
    char* nombre_restaurante;
    int id_pedido;
	int respuesta;

	while(1) {
		int cliente_socket = esperar_cliente(escucha_socket);

		operacion = recibir_operacion(cliente_socket);
		log_info(logger, "op=%d", operacion);
		switch (operacion) {
		case CONSULTAR_PLATOS:
            lista = recibir_paquete(cliente_socket);
            nombre_restaurante = (char*)list_get(lista, 0);
            int cantidad_platos = *(int*)list_get(lista, 1);
            if (cantidad_platos > 0) {
                log_info(logger, "Respuesta de CONSULTAR_PLATOS: El restaurante %s tiene %d platos disponibles", nombre_restaurante, cantidad_platos);
                for (int i = 2; i < cantidad_platos * 2 + 1; i = i + 2) {
					log_info(logger, "Plato: %s - $%d", (char*)list_get(lista, i), *(int*)list_get(lista, i + 1));
				}
            } else {
                if (cantidad_platos == 0) {
                log_info(logger, "Repsuesta de CONSULTAR_PLATOS: El restaurante %s no tiene platos disponibles", nombre_restaurante);
                } else {
                    log_error(logger, "Error en la respuesta de CONSULTAR_PLATOS desde restaurante %s", nombre_restaurante);
                }
            }
			liberar_cliente(cliente_socket);
			break;
		case CREAR_PEDIDO:
            lista = recibir_paquete(cliente_socket);
            nombre_restaurante = (char*)list_get(lista, 0);
            id_pedido = *(int*)list_get(lista, 1);
            if (id_pedido == 0) {
                log_error(logger, "Error en CREAR_PEDIDO en restaurante %s, no se genero el pedido", nombre_restaurante);
            } else {
                log_info(logger, "Respuesta de CREAR_PEDIDO: se genero correctamente el pedido nro %d en %s", id_pedido, nombre_restaurante);
            }
			liberar_cliente(cliente_socket);
			break;
		case AGREGAR_PLATO:
            lista = recibir_paquete(cliente_socket);
            nombre_restaurante = (char*)list_get(lista, 0);
            id_pedido = *(int*)list_get(lista, 1);
            char* nombre_plato = (char*)list_get(lista, 2);
            respuesta = *(int*)list_get(lista, 3);
            if (respuesta == MSG_OK) {
                log_info(logger, "Respuesta de AGREGAR_PLATO del restaurante %s: se aniadio el plato %s correctamente en el pedido %d", nombre_restaurante, nombre_plato, id_pedido);
            } else {
                log_error(logger, "Error en AGREGAR_PEDIDO aniadiendo el plato %s al pedido %d en el restaurante %s", nombre_plato, id_pedido, nombre_restaurante);
            }
			liberar_cliente(cliente_socket);
			break;
		case CONFIRMAR_PEDIDO:
            lista = recibir_paquete(cliente_socket);
            nombre_restaurante = (char*)list_get(lista, 0);
            id_pedido = *(int*)list_get(lista, 1);
			respuesta = *(int*)list_get(lista, 2);
			if (respuesta == MSG_OK) {
				log_info(logger, "Respuesta de CONFIRMAR_PEDIDO del restaurante %s: se confirmo el pedido %d correctamente", nombre_restaurante, id_pedido);
			} else {
				log_error(logger, "Error en CONFIRMAR_PEDIDO para el restaurante %s id %d", nombre_restaurante, id_pedido);
			}
			liberar_cliente(cliente_socket);
		    break;
		case CONSULTAR_PEDIDO:
            lista = recibir_paquete(cliente_socket);
            nombre_restaurante = (char*)list_get(lista, 0);
            id_pedido = *(int*)list_get(lista, 1);
			int estado = *(int*)list_get(lista, 2);
			// TODO: loguear plato y cantidad
			if (estado == PENDIENTE) {
				log_info(logger, "Respuesta de CONSULTAR_PEDIDO del restaurante %s: el pedido %d se encuentra pendiente", nombre_restaurante, id_pedido, estado);
			}
			if (estado == CONFIRMADO) {
				log_info(logger, "Respuesta de CONSULTAR_PEDIDO del restaurante %s: el pedido %d se encuentra confirmado", nombre_restaurante, id_pedido, estado);
			}
			if (estado == TERMINADO) {
				log_info(logger, "Respuesta de CONSULTAR_PEDIDO del restaurante %s: el pedido %d se encuentra finalizado", nombre_restaurante, id_pedido, estado);
			}
			liberar_cliente(cliente_socket);
			break;

		case GUARDAR_PEDIDO:
			lista = recibir_paquete(cliente_socket);
			int respuesta = *(int*) list_get(lista, 0);
			if(respuesta == MSG_OK) {
				log_info(logger, "Respuesta de GUARDAR_PEDIDO: OK");
			} else {
				log_info(logger, "Respuesta de GUARDAR_PEDIDO: FAIL");
			}
			liberar_cliente(cliente_socket);
			break;
		default:
			log_info(logger, "Llego algo pero no se que es: %d", operacion);
			break;
		}
	}
}

int main(int cantidad, char* argumentos[]) {
	levantar_configuracion();

	logger = log_create(archivo_log, "cliente", 0, LOG_LEVEL_DEBUG);


    pthread_t hilo_recepcionar_respuestas;
    pthread_create(&hilo_recepcionar_respuestas, NULL, recepcionar_respuestas, NULL);

	int modulo_conectado;

	// == INICIA CONEXION CON EL MODULO ==
	socket_conexion = crear_conexion(ip, puerto);

	t_paquete* paqueteConexion = crear_paquete(INICIAR_CLIENTE);
	agregar_a_paquete(paqueteConexion, id_cliente, strlen(id_cliente) + 1);
	agregar_a_paquete(paqueteConexion, &posicion_x, sizeof(int));
	agregar_a_paquete(paqueteConexion, &posicion_y, sizeof(int));
	char* ip_cliente = malloc(strlen("127.0.0.1") + 1);
	memcpy(ip_cliente, "127.0.0.1", strlen("127.0.0.1") + 1);
	agregar_a_paquete(paqueteConexion, ip, strlen("127.0.0.1") + 1);
	char* puerto_cliente = malloc(strlen(puerto_escucha) + 1);
	memcpy(puerto_cliente, puerto_escucha, strlen(puerto_escucha) + 1);
	agregar_a_paquete(paqueteConexion, puerto_cliente, strlen(puerto_escucha) + 1);
	enviar_paquete(paqueteConexion, socket_conexion);
	eliminar_paquete(paqueteConexion);

	printf("Envie un paquete por la red. Esperando respuesta...\n");

	int operacion = recibir_operacion(socket_conexion);
	//liberar_cliente(socket_conexion);

	switch(operacion) {
		case RESTAURANTE:
			printf("Me estoy conectando a restaurante.\n");
			modulo_conectado = RESTAURANTE;
			break;
		case APP:
			printf("Me estoy conectando a sindicato.\n");
			modulo_conectado = APP;
			break;
		case COMANDA:
			printf("Me estoy conectando a comanda.\n");
			modulo_conectado = COMANDA;
			break;
		case SINDICATO:
			printf("Me estoy conectando a sindicato.\n");
			modulo_conectado = SINDICATO;
			break;
		case -1:
			printf("Error. Uso modulo dummy.\n");
			modulo_conectado = 99;
			exit(-1);
		default:
			printf("Desconocido. Uso modulo dummy.\n");
			modulo_conectado = 99;
			exit(-1);
	}

	gestionarMenu(modulo_conectado);

    pthread_join(hilo_recepcionar_respuestas, NULL);

	return 0;
}
