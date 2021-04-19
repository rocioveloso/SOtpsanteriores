#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#include "utilsGameboy.h"


typedef enum
{
	BROKER=1;
	TEAM=2;
	GAMECARD=3;
	SUSCRIPTOR= 4;

}op_tipo;


typedef enum
{
	MENSAJE=1;
	COLA = 2;

}op_code;


typedef struct
{
	int size;
	void* stream;

}t_buffer;


typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;

} t_paquete;


// para la cola

typedef struct NodoBroker{
	t_paquete* paquete;
	struct NodoBroker* siguiente;
}NodoBroker;

typedef struct Cola{
	int mensajes;
	NodoBroker* primer;
	NodoBroker* ultimo;
}Cola;



int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* elMensaje, int socket_cliente);
void liberar_conexion(int socket_cliente);

// para la cola

int crear_nodo(t_paquete* paquete);
void destruir_nodo(Cola* cola);
void agregar_a_cola(Cola* cola, t_paquete* paquete);




#endif
