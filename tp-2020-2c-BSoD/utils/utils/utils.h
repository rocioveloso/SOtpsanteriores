#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<readline/readline.h>
#include<commons/config.h>
#include<commons/log.h>
#include<commons/collections/list.h>


typedef enum
{


    CLIENTE, APP, RESTAURANTE, COMANDA, SINDICATO,
	CREAR_RESTAURANTE, CONSULTAR_PLATO, AGREGAR_PLATO, CONSULTAR_PEDIDO, OBTENER_RECETA,
	INICIAR_CLIENTE, GUARDAR_PEDIDO, GUARDAR_PLATO, OBTENER_PEDIDO, CONFIRMAR_PEDIDO,OBTENER_RESTAURANTE, CONSULTAR_RESTAURANTE, SELECCIONAR_RESTAURANTE,
    PLATO_LISTO, FINALIZAR_PEDIDO, CREAR_PEDIDO, CONSULTAR_PLATOS, MSG_OK, MSG_ERROR, MSG_FAIL,
	RESPUESTA_GUARDAR_PEDIDO, RESPUESTA_GUARDAR_PLATO, RESPUESTA_OBTENER_PEDIDO, RESPUESTA_CONFIRMAR_PEDIDO, RESPUESTA_PLATO_LISTO, RESPUESTA_FINALIZAR_PEDIDO,
	PENDIENTE, CONFIRMADO, TERMINADO,
	NO_EXISTE_PEDIDO, NO_EXISTE_RESTAURANTE,
} op_code;


typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef struct {
	char* nombre;
	int precio;
} t_receta;

typedef struct {
    char* nombre_restaurante;
    int cantidad_cocineros;
    char* afinidad_cocineros[5];
    int posicion[2];
    char* platos[5];
    int precio_platos[5];
    int cantidad_hornos;
} t_restaurante_metadata;

typedef struct {
	char* nombre;
	int cantTotal;
	int cantLista;
}t_comida;

typedef struct {
	op_code estado;
	t_list* comidas;
}t_pedido;

char *leer_config(char* campo, char* archivo);
int crear_conexion(char* ip, char* puerto);
void liberar_cliente(int socket_cliente);
void liberar_servidor(int socket_servidor);
//
t_paquete* crear_paquete(op_code operacion);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
//
t_list* recibir_paquete(int);
int recibir_operacion(int);
//
void* recibir_buffer(int*, int);
int iniciar_servidor(char* IP, char* PUERTO);
int esperar_cliente(int);
uint32_t recibir_numero(int socket_cliente);
#endif /* UTILS_H_ */
