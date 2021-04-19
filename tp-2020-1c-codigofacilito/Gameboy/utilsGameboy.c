#include "utilsGameboy.h"



int crear_conexion(char* ip, char* puerto){

	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
		printf("error");

	freeaddrinfo(server_info);

	return socket_cliente;

}

void enviar_mensaje(char* elMensaje, int socket_cliente){
	t_paquete* paquete = malloc(sizeof(t_paquete));
		paquete->codigo_operacion = MENSAJE;
		paquete->buffer = malloc(sizeof(t_buffer));
		paquete->buffer->stream = elMensaje;
		paquete->buffer->size = strlen(elMensaje)+1;

		int bytesASerializar;

		void* streamDelMensajeSerializado = serializar_paquete(paquete, &bytesASerializar);


		send(socket_cliente, streamDelMensajeSerializado, bytesASerializar, 0);

		free(streamDelMensajeSerializado);

}



void liberar_conexion(int socket_cliente){
		close(socket_cliente);
}

int crear_cola(){
	Cola* cola = (Cola*) malloc(sizeof(Cola));
	cola->primer = cola -> ultimo = NULL;
	Cola*cola -> mensajes = 0;
	return cola;
}

int crear_nodo(t_paquete* paquete){
	NodoBroker* nodo = (NodoBroker *) malloc(sizeof (NodoBroker));
	nodo -> paquete = paquete;
	nodo -> siguiente = NULL;
	return nodo;
}


void destruir_nodo(NodoBroker* nodo){
	nodo -> paquete = NULL;
	nodo -> siguiente = NULL;
	free(nodo);

}


void agregar_a_cola(Cola* cola, t_paquete* paquete){
	NodoBroker* nodo = crear_nodo(paquete);
	Cola*cola -> mensajes = mensajes + 1;
	if(!cola->primer){
		cola-> primer = nodo;
		cola-> ultimo = nodo;
	}else{
		cola->ultimo->siguiente = nodo;
		cola->ultimo = nodo;
	}

}



void enviar_cola_de_mensajes(int cola_de_mesajes , int socket_cliente){

	t_paquete* paquete = malloc(sizeof());
	paquete -> codigo_operación = COLA;
	paquete -> buffer = malloc(sizeof());
	paquete -> buffer -> stream = cola_de_mensajes;
	paquete -> buffer -> size = strlen(cola_de_mensajes)+1;

	int bytesASerializar;

	void* streamDeLaCola_De_MensajesSerializado = serializar_paquete(paquete, &bytesASerializar);

	send(socket_cliente, streamDeLaCola_De_MensajesSerializado, bytesASerializar, 0);

	free(streamDeLaCola_De_MensajesSerializado);


}


