#include "restaurante.h"

int main() {
    sem_init(&sem_cola_new, 1, 1);

    char* log_file = leer_config("ARCHIVO_LOG", ARCHIVO_CONFIG);
    LOGGER = log_create(log_file, "restaurante", true, LOG_LEVEL_DEBUG);
    log_debug(LOGGER, "Log creado. Escribiendo en %s.\n", log_file);

    NOMBRE_RESTAURANTE = leer_config("NOMBRE_RESTAURANTE", ARCHIVO_CONFIG);

    IP_SINDICATO = leer_config("IP_SINDICATO", ARCHIVO_CONFIG);
    PUERTO_SINDICATO = leer_config("PUERTO_SINDICATO", ARCHIVO_CONFIG);

    pthread_t hilo_servidor;
    pthread_create(&hilo_servidor, NULL, restaurante_server, NULL);

    iniciar_planificador();

    while (true) {
        ejecutar_un_ciclo();
    }

    pthread_join(hilo_servidor, NULL);

    return 0;
}

int iniciar_planificador() {
    // Pide metadata a sindicato
    int sindicato_socket = crear_conexion(IP_SINDICATO, PUERTO_SINDICATO);

    t_paquete* paquete = crear_paquete(OBTENER_RESTAURANTE);
    char* nombre = malloc(strlen(NOMBRE_RESTAURANTE) + 1);
    memcpy(nombre, NOMBRE_RESTAURANTE, strlen(NOMBRE_RESTAURANTE) + 1);
    agregar_a_paquete(paquete, nombre, strlen(NOMBRE_RESTAURANTE) + 1);

    enviar_paquete(paquete, sindicato_socket);
    eliminar_paquete(paquete);

    int operacion = recibir_operacion(sindicato_socket);

    t_list* lista = recibir_paquete(sindicato_socket);

    int cant_afinidades = *(int*)list_get(lista, 0);
    AFINIDADES = list_create();

    for (int i=1; i <= cant_afinidades; i++) {
        list_add(AFINIDADES, (char*)list_get(lista, i));
    }

    POSICION_X = *(int*)list_get(lista, cant_afinidades + 1);
    POSICION_Y = *(int*)list_get(lista, cant_afinidades + 2);

    int cant_platos = *(int*)list_get(lista, cant_afinidades + 3);

    PLATOS = list_create();

    for (int i=cant_afinidades+4; i <= cant_afinidades + 4 + cant_platos; i = i + 2) {
        t_plato un_plato;
        un_plato.nombre = (char*)list_get(lista, i);
        un_plato.precio = *(int*)list_get(lista, i + 1);
        list_add(PLATOS, &un_plato);
    }

    CANT_HORNOS = *(int*)list_get(lista, cant_afinidades + cant_platos * 2 + 4);
    CANT_PEDIDOS = *(int*)list_get(lista, cant_afinidades + cant_platos * 2 + 5);
    CANT_COCINEROS = *(int*)list_get(lista, cant_afinidades + cant_platos * 2 + 6);
    log_debug(LOGGER, "Metadata recibida - c_afinidades=%d, x=%d, y=%d, c_platos=%d, c_hornos=%d, c_pedidos=%d, c_cocineros=%d",
              cant_afinidades, POSICION_X, POSICION_Y, cant_platos, CANT_HORNOS, CANT_PEDIDOS, CANT_COCINEROS);
    // Termina pedir metadata

    COLA_NEW = queue_create();
    COLA_BLOCKED = queue_create();
    COLA_EXIT = queue_create();
    COLAS_ES = list_create();
    COLAS_READY = list_create();

    for (int i=0; i <= CANT_HORNOS; i++) {
        t_queue* cola_es = queue_create();
        list_add(COLAS_ES, cola_es);
    }
    log_debug(LOGGER, "Cantidad de colas ES: %d", list_size(COLAS_ES));

    for (int i=0; i <= CANT_COCINEROS; i++) {
        t_queue* cola_ready = queue_create();
        list_add(COLAS_READY, cola_ready);
    }
    log_debug(LOGGER, "Cantidad de colas READY: %d", list_size(COLAS_READY));
}

void encolar_new_pcb(t_pcb un_pcb) {
    sem_wait(&sem_cola_new);

    queue_push(COLA_NEW, &un_pcb);

    sem_post(&sem_cola_new);
}

void planificar_new() {

    if (queue_is_empty(COLA_NEW)) {
        return;
    }

    sem_wait(&sem_cola_new);

    t_pcb un_pcb = *(t_pcb*)queue_pop(COLA_NEW);

    sem_post(&sem_cola_new);

    log_debug(LOGGER, "Nuevo pcb: %s", un_pcb.nombre);

    return;

    // si la operacion actual es igual a la cantidad de operaciones el plato termino la preparacion
    int cantidad_operaciones = list_size(un_pcb.operaciones);
    int nro_operacion_actual = un_pcb.operacion_actual;
    log_debug(LOGGER, "Antes de calcular: cantidad_operaciones=%d, nro_operacion_actual=%d", cantidad_operaciones, nro_operacion_actual);
    if (cantidad_operaciones == nro_operacion_actual) {
        log_info(LOGGER, "Planificacion %s: se terminaron las operaciones, mando a EXIT", un_pcb.nombre);
        queue_push(COLA_EXIT, &un_pcb);
    }
    char* proxima_operacion = list_get(un_pcb.operaciones, un_pcb.operacion_actual);

    log_debug(LOGGER, "Operacion a planificar: %s (%d caracteres)", proxima_operacion, strlen(proxima_operacion));

    // Asigno al campo ciclos restantes los ciclos que conlleva la operacion
    int ut = list_get(un_pcb.operaciones, un_pcb.operacion_actual + 1);
    un_pcb.ciclos_restantes = ut;

    // si la operacion es reposar mandamos a blocked
    char* op = "Reposar";
    if (strcmp(proxima_operacion, op) == 0) {
        log_info(LOGGER, "Preparando %s: mando a BLOCKED (enfriar)", un_pcb.nombre);
        queue_push(COLA_BLOCKED, &un_pcb);
    }
    // si la operacion es hornear mandamos a e/s
    op = "Hornear";
    if (strcmp(proxima_operacion, op) == 0) {
        log_info(LOGGER, "Preparando %s: mando a E/S (hornos)", un_pcb.nombre);
        list_add(COLAS_ES, &un_pcb);
    } else {
        log_info(LOGGER, "Preparando %s: mando a READY (cocineros)", un_pcb.nombre);
        list_add(COLAS_READY, &un_pcb);
    }
}

void procesar_ready() {
    // 1. Recorrer cada cocinero:
    // 1a. Sacar pcb del cocinero
    // 1b. Si el tiempo restante es 0, a replanificar
    // 1c. Si no, decrementar el tiempo restante en 1 y volver a meter en cocinero (esto cambia si algoritmo == RR)
    // 2. Si hay cocineros disponibles hay que ver qué pcb van a tener (temas de afinidad)
}

void procesar_io() {
    // 1. Recorrer cada horno:
    // 1a. Sacar pcb del horno
    // 1b. Si tiempo restante es 0, a replanificar
    // 1c. Si no, decrementar tiempo restante y volver a meter en el horno
    // 2. Si hay hornos disponibles, hay que meter los pcb de la primer cola
}

void procesar_blocked() {
    if (queue_size(COLA_BLOCKED) == 0) {
        return;
    }

    t_queue* aux_queue = queue_create();

    while (queue_size(COLA_BLOCKED) != 0) {
        t_pcb un_pcb;
        un_pcb = *(t_pcb*)queue_pop(COLA_BLOCKED);

        if (un_pcb.ciclos_restantes == 0) {
            log_debug(LOGGER, "El plato %s termino su reposo", un_pcb.nombre);
            planificar_new(un_pcb);
        }

        un_pcb.ciclos_restantes = un_pcb.ciclos_restantes - 1;
        queue_push(aux_queue, &un_pcb);
    }

    COLA_BLOCKED = aux_queue;
}

void procesar_exit() {
    // Informar a sindicato para que actualice el estado del pedido y al módulo que solicitó el pedido
}

void ejecutar_un_ciclo() {
    log_info(LOGGER, "Ejecutando un ciclo");
    planificar_new();
    procesar_blocked();
    procesar_io();
    procesar_ready();
    procesar_exit();
    sleep(1);
}

void preparacion_platos(t_list* platos, int id_pedido) {
    char* nombre;
    t_paquete* paquete;

    for (int i = 0; i < list_size(platos); i++) {
        paquete = crear_paquete(OBTENER_RECETA);

        nombre = malloc(strlen(NOMBRE_RESTAURANTE) + 1);
        memcpy(nombre, NOMBRE_RESTAURANTE, strlen(NOMBRE_RESTAURANTE) + 1);
        agregar_a_paquete(paquete, nombre, strlen(NOMBRE_RESTAURANTE) + 1);

        char* plato = list_get(platos, i);
        log_debug(LOGGER, "Pidiendo receta para el plato %s...", plato);

        nombre = malloc(strlen(plato) + 1);
        memcpy(nombre, plato, strlen(plato) + 1);
        agregar_a_paquete(paquete, nombre, strlen(plato) + 1);

        int sindicato_socket = crear_conexion(IP_SINDICATO, PUERTO_SINDICATO);
        enviar_paquete(paquete, sindicato_socket);

        int operacion = recibir_operacion(sindicato_socket);
        t_list* lista = recibir_paquete(sindicato_socket);

        // Armando PCB
        t_pcb nuevo_pcb;
        nuevo_pcb.nombre = plato;
        nuevo_pcb.id_pedido = id_pedido;
        nuevo_pcb.operaciones = list_create();
        nuevo_pcb.operacion_actual = 0;

        int cantidad_operaciones = *(int*)list_get(lista, 0);
        log_debug(LOGGER, "La preparacion del plato %s conlleva %d operaciones", nuevo_pcb.nombre, cantidad_operaciones);

        // Arma las operaciones del plato
        for (int j = 1; j < cantidad_operaciones * 2 + 1; j = j + 2) {
            char* operacion = (char*)list_get(lista, j);
            log_debug(LOGGER, "Operacion %d: %s", j, operacion);
            list_add(nuevo_pcb.operaciones, operacion);
            int t = *(int*)list_get(lista, j + 1);
            list_add(nuevo_pcb.operaciones, t);
        }

        int co = list_size(nuevo_pcb.operaciones);
        log_debug(LOGGER, "Cantidad operaciones: %d", co);
        log_info(LOGGER, "Arme el PCB para el plato %s del pedido %d", plato, id_pedido);

        eliminar_paquete(paquete);
        free(nombre);

        sem_wait(&sem_cola_new);

        queue_push(COLA_NEW, &nuevo_pcb);

        sem_post(&sem_cola_new);
    }

}

void restaurante_server() {
    char* puerto_escucha = leer_config("PUERTO_ESCUCHA", ARCHIVO_CONFIG);
    log_info(LOGGER, "Iniciando server en %s", puerto_escucha);
    int servidor_socket = iniciar_servidor("127.0.0.1", puerto_escucha);
    t_list* lista;
    int operacion;

    while(1) {
        int cliente_socket = esperar_cliente(servidor_socket);
        operacion = recibir_operacion(cliente_socket);

        switch(operacion) {
            case INICIAR_CLIENTE:
                log_info(LOGGER, "Me llego un mensaje de INICIAR CLIENTE");
                lista = recibir_paquete(cliente_socket);
                char* id_cliente = (char*)list_get(lista, 0);
                log_info(LOGGER, "Id del cliente: %s", id_cliente);
                IP_MODULO_EMISOR = (char*)list_get(lista, 3);
                log_info(LOGGER, "IP_MODULO_EMISOR=%s", IP_MODULO_EMISOR);
                PUERTO_MODULO_EMISOR = (char*)list_get(lista, 4);
                log_info(LOGGER, "PUERTO_MODULO_EMISOR=%s", PUERTO_MODULO_EMISOR);
                
                t_paquete *paquete;
                paquete = crear_paquete(RESTAURANTE);
                enviar_paquete(paquete, cliente_socket);

                eliminar_paquete(paquete);
                liberar_cliente(cliente_socket);
                break;
			case CONSULTAR_PLATOS:
                log_info(LOGGER, "Mensaje de CONSULTAR PLATOS");
				consultar_platos(cliente_socket);
                break;
            case CREAR_PEDIDO:
                log_info(LOGGER, "Mensaje de CREAR PEDIDO");
                crear_pedido(cliente_socket);
                break;
            case AGREGAR_PLATO:
                log_info(LOGGER, "Mensaje de AGREGAR_PLATO");
                lista = recibir_paquete(cliente_socket);
                aniadir_plato(cliente_socket, *(int*)list_get(lista, 1), (char*)list_get(lista, 0), *(int*)list_get(lista, 2));
                break;
            case CONFIRMAR_PEDIDO:
                log_info(LOGGER, "Mensaje de CONFIRMAR PLATO");
                lista = recibir_paquete(cliente_socket);
                confirmar_pedido(cliente_socket, *(int*)list_get(lista, 0));
                break;
            case CONSULTAR_PEDIDO:
                log_info(LOGGER, "Mensaje de CONSULTAR PEDIDO");
                lista = recibir_paquete(cliente_socket);
                consultar_pedido(cliente_socket, *(int*)list_get(lista, 0));
                break;
            case -1:
                log_warning(LOGGER, "El cliente %d se desconecto.\n", cliente_socket);
                liberar_cliente(cliente_socket);
                return EXIT_FAILURE;
            default:
                log_error(LOGGER, "Operacion desconocida: %d", operacion);
                break;
		}
	}
}

void consultar_platos(int cliente_socket) {
    // Envia consultar_platos() a sindicato
    char* nombre;
    t_paquete* paquete;

    int sindicato_socket = crear_conexion(IP_SINDICATO, PUERTO_SINDICATO);

    paquete = crear_paquete(CONSULTAR_PLATOS);

    nombre = malloc(strlen(NOMBRE_RESTAURANTE) + 1);
    memcpy(nombre, NOMBRE_RESTAURANTE, strlen(NOMBRE_RESTAURANTE) + 1);
    agregar_a_paquete(paquete, nombre, strlen(NOMBRE_RESTAURANTE) + 1);

    enviar_paquete(paquete, sindicato_socket);
    // Me quedo esperando la respuesta de sindicado y la reenvio a quien me haya llamado
    int operacion = recibir_operacion(sindicato_socket);

    t_list* lista = recibir_paquete(sindicato_socket);

    int socket_respuesta = crear_conexion(IP_MODULO_EMISOR, PUERTO_MODULO_EMISOR);
    paquete = crear_paquete(CONSULTAR_PLATOS);

    agregar_a_paquete(paquete, nombre, strlen(NOMBRE_RESTAURANTE) + 1);

    int cantidad_platos = *(int*)list_get(lista, 1);

    agregar_a_paquete(paquete, &cantidad_platos, sizeof(int));

    if (cantidad_platos > 0) {
        for (int i = 2; i < cantidad_platos * 2 + 1; i = i + 2) {
            char* nombre_plato = (char*)list_get(lista, i);
            nombre = malloc(strlen(nombre_plato) + 1);
            memcpy(nombre, nombre_plato, strlen(nombre_plato) + 1);
            agregar_a_paquete(paquete, nombre, strlen(nombre_plato) + 1);

            int precio_plato = *(int*)list_get(lista, i + 1);
            agregar_a_paquete(paquete, &precio_plato, sizeof(int));
        }
    }

    enviar_paquete(paquete, socket_respuesta);

    liberar_cliente(cliente_socket);
    eliminar_paquete(paquete);
    free(nombre);
}

int crear_pedido(int cliente_socket) {
    // Envia guardar_pedido() a sindicato con un ID unico dentro del restaurante
    char* nombre;
    t_paquete* paquete;

    int sindicato_socket = crear_conexion(IP_SINDICATO, PUERTO_SINDICATO);

    paquete = crear_paquete(GUARDAR_PEDIDO);

    nombre = malloc(strlen(NOMBRE_RESTAURANTE) + 1);
    memcpy(nombre, NOMBRE_RESTAURANTE, strlen(NOMBRE_RESTAURANTE) + 1);
    agregar_a_paquete(paquete, nombre, strlen(NOMBRE_RESTAURANTE) + 1);

    int id_pedido = generar_id_pedido();
    agregar_a_paquete(paquete, &id_pedido, sizeof(int));

    enviar_paquete(paquete, sindicato_socket);
    // Me quedo esperando la respuesta de sindicato
    int operacion = recibir_operacion(sindicato_socket);
    t_list* lista = recibir_paquete(sindicato_socket);

    int socket_respuesta = crear_conexion(IP_MODULO_EMISOR, PUERTO_MODULO_EMISOR);

    paquete = crear_paquete(CREAR_PEDIDO);

    nombre = malloc(strlen(NOMBRE_RESTAURANTE) + 1);
    memcpy(nombre, NOMBRE_RESTAURANTE, strlen(NOMBRE_RESTAURANTE) + 1);
    agregar_a_paquete(paquete, nombre, strlen(NOMBRE_RESTAURANTE) + 1);

    int check = *(int*)list_get(lista, 0);

    if (check < 1) {
        id_pedido = 0;
    }

    agregar_a_paquete(paquete, &id_pedido, sizeof(int));

    enviar_paquete(paquete, socket_respuesta);

    liberar_cliente(cliente_socket);
    eliminar_paquete(paquete);
    free(nombre);
}

void aniadir_plato(int cliente_socket, int id_pedido, char* plato, int cantidad) {
    char* nombre;
    t_paquete* paquete;

    int sindicato_socket = crear_conexion(IP_SINDICATO, PUERTO_SINDICATO);

    paquete = crear_paquete(GUARDAR_PLATO);

    nombre = malloc(strlen(NOMBRE_RESTAURANTE) + 1);
    memcpy(nombre, NOMBRE_RESTAURANTE, strlen(NOMBRE_RESTAURANTE) + 1);
    agregar_a_paquete(paquete, nombre, strlen(NOMBRE_RESTAURANTE) + 1);

    agregar_a_paquete(paquete, &id_pedido, sizeof(int));

    nombre = malloc(strlen(plato) + 1);
    memcpy(nombre, plato, strlen(plato) + 1);
    agregar_a_paquete(paquete, nombre, strlen(plato) + 1);

    agregar_a_paquete(paquete, &cantidad, sizeof(int));

    enviar_paquete(paquete, sindicato_socket);
    // esperar respuesta
    int operacion = recibir_operacion(sindicato_socket);
    t_list* lista = recibir_paquete(sindicato_socket);

    // enviar a cliente
    int socket_respuesta = crear_conexion(IP_MODULO_EMISOR, PUERTO_MODULO_EMISOR);

    paquete = crear_paquete(AGREGAR_PLATO);

    nombre = malloc(strlen(NOMBRE_RESTAURANTE) + 1);
    memcpy(nombre, NOMBRE_RESTAURANTE, strlen(NOMBRE_RESTAURANTE) + 1);
    agregar_a_paquete(paquete, nombre, strlen(NOMBRE_RESTAURANTE) + 1);

    agregar_a_paquete(paquete, &id_pedido, sizeof(int));

    nombre = malloc(strlen(plato) + 1);
    memcpy(nombre, plato, strlen(plato) + 1);
    agregar_a_paquete(paquete, nombre, strlen(plato) + 1);

    int check = *(int*)list_get(lista, 0);
    agregar_a_paquete(paquete, &check, sizeof(int));

    enviar_paquete(paquete, socket_respuesta);

    liberar_cliente(cliente_socket);
    eliminar_paquete(paquete);
    free(nombre);
}

void confirmar_pedido(int cliente_socket, int id_pedido) {
    // 1. Obtener el pedido desde sindicato
    // 2. Por cada plato se envia obtener_receta() a sindicato
    //    Genera el PCB de cada plato del pedido
    //    Se agrega a la planificacion de platos
    // 3. Informa al modulo que lo llamo indicando pedido confirmado
    char* nombre;
    t_paquete* paquete;

    int sindicato_socket = crear_conexion(IP_SINDICATO, PUERTO_SINDICATO);

    paquete = crear_paquete(OBTENER_PEDIDO);

    nombre = malloc(strlen(NOMBRE_RESTAURANTE) + 1);
    memcpy(nombre, NOMBRE_RESTAURANTE, strlen(NOMBRE_RESTAURANTE) + 1);
    agregar_a_paquete(paquete, nombre, strlen(NOMBRE_RESTAURANTE) + 1);

    agregar_a_paquete(paquete, &id_pedido, sizeof(int));

    enviar_paquete(paquete, sindicato_socket);

    int operacion = recibir_operacion(sindicato_socket);
    t_list* lista = recibir_paquete(sindicato_socket);

    int socket_respuesta = crear_conexion(IP_MODULO_EMISOR, PUERTO_MODULO_EMISOR);

    paquete = crear_paquete(CONFIRMAR_PEDIDO);

    nombre = malloc(strlen(NOMBRE_RESTAURANTE) + 1);
    memcpy(nombre, NOMBRE_RESTAURANTE, strlen(NOMBRE_RESTAURANTE) + 1);
    agregar_a_paquete(paquete, nombre, strlen(NOMBRE_RESTAURANTE) + 1);

    agregar_a_paquete(paquete, &id_pedido, sizeof(int));

    // int estado = *(int*)list_get(lista, 0);
    int estado = MSG_OK;

    agregar_a_paquete(paquete, &estado, sizeof(int));

    enviar_paquete(paquete, socket_respuesta);

    int cantidad_platos = *(int*)list_get(lista, 1);

    t_list* platos_a_preparar = list_create();
    for (int i = 2; i < cantidad_platos * 3 + 1; i = i + 3) {
        char* plato = (char*)list_get(lista, i);
        int cantidad = *(int*)list_get(lista, i + 1);
        for (int j = 0; j < cantidad; j++) {
            log_debug(LOGGER, "Se va a preparar un %s", plato);
            list_add(platos_a_preparar, plato);
        }
    }
    preparacion_platos(platos_a_preparar, id_pedido);

    liberar_cliente(cliente_socket);
    eliminar_paquete(paquete);
    free(nombre);
}

void consultar_pedido(int cliente_socket, int id_pedido) {
    char* nombre;
    t_paquete* paquete;

    int sindicato_socket = crear_conexion(IP_SINDICATO, PUERTO_SINDICATO);

    paquete = crear_paquete(OBTENER_PEDIDO);

    nombre = malloc(strlen(NOMBRE_RESTAURANTE) + 1);
    memcpy(nombre, NOMBRE_RESTAURANTE, strlen(NOMBRE_RESTAURANTE) + 1);
    agregar_a_paquete(paquete, nombre, strlen(NOMBRE_RESTAURANTE) + 1);

    agregar_a_paquete(paquete, &id_pedido, sizeof(int));

    enviar_paquete(paquete, sindicato_socket);

    int operacion = recibir_operacion(sindicato_socket);

    t_list* lista = recibir_paquete(sindicato_socket);

    paquete = crear_paquete(CONSULTAR_PEDIDO);

    agregar_a_paquete(paquete, nombre, strlen(NOMBRE_RESTAURANTE) + 1);

    agregar_a_paquete(paquete, &id_pedido, sizeof(int));

    int estado = *(int*)list_get(lista, 0);
    agregar_a_paquete(paquete, &estado, sizeof(int));

    int cantidad_platos = *(int*)list_get(lista, 1);
    agregar_a_paquete(paquete, &estado, sizeof(int));

    for (int i = 2; i < cantidad_platos * 3 + 1; i = i + 3) {
        char* plato = (char*)list_get(lista, i);
        log_debug(LOGGER, "plato=%s", plato);
        int cantidad = *(int*)list_get(lista, i + 1);
        log_debug(LOGGER, "cantidad total=%d", cantidad);
        int cantidad_lista = *(int*)list_get(lista, i + 2);
        log_debug(LOGGER, "cantidad lista=%d", cantidad_lista);

        nombre = malloc(strlen(plato) + 1);
        memcpy(nombre, plato, strlen(plato) + 1);
        agregar_a_paquete(paquete, nombre, strlen(plato) + 1);

        agregar_a_paquete(paquete, &cantidad, sizeof(int));
        agregar_a_paquete(paquete, &cantidad_lista, sizeof(int));
    }

    int socket_respuesta = crear_conexion(IP_MODULO_EMISOR, PUERTO_MODULO_EMISOR);
    enviar_paquete(paquete, socket_respuesta);

    liberar_cliente(cliente_socket);
    eliminar_paquete(paquete);
    free(nombre);
}

int generar_id_pedido() {
    ID_PEDIDO++;
    return ID_PEDIDO;
}
