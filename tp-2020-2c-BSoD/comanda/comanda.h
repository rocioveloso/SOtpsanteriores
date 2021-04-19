#ifndef COMANDA_H_
#define COMANDA_H_


/*
 * Cosas a hacer:
 *
 * 1- Una Función que le entre por parametro el nombre de restaurante y devuelva el restaurante si existe, y si no existe devuelve null.
 * 2- Función que le va a entrar por parametro la lista de restaurantes y el nombre de restaurante, esto lo que hace es crear la tabla de segmentos y agregarla a la lista de restaurantes.
 * 4- Funcion para gestionar los algoritmos de reemplazo. Va a haber un switch con 2 cases, uno para cada algoritmo.
 * 8- Una funcion para consultar el estado de un pedido, entra por parametro el ID del pedido.
 * 9- Crear una funcion para verificar todo (todas las funciones de verificacion creadas), devuelve un codigo de error o true.
 * 10 - Una funcion para interpretar el codigo de error. Entra por parametro el codigo de error, devuelve el mensaje correspondiente a ese error.
 * 13 - Una funcion para eliminar un segmento (chequear que el segmento no tenga paginas)
 * 7- Una funcion para cada algoritmo.
 * 11 - Una funcion que aumente la cantidad de plato y verifique cantidad total y listos. Entra por parametro el plato. Devuelve bool
 * */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <utils/utils.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>

//CONFIGURACION

char* ip;
char* puerto;
char* puertoEscucha;
int servidor;
uint32_t tamanioMemoria;
uint32_t tamanioSwap;
char* algoritmoReemplazo;
char* archivoDeLog;
char* puertoEscuchaCliente;
char* ipCliente;

pthread_t thread0;

t_log* logger;

void* memoriaPrincipal;
void* memoriaSwap;

t_list* restaurantes; // una lista de restaurantes [tablas de segmentos]


// Tabla de segmentos
typedef struct {
	char* nombre; //--> nombre del restaurante
	t_dictionary* pedidos; //--> Diccionario del tipo: "ID de pedido" => Pedido.
}TablaSegmentos; //--> Restaurante

// Tabla de páginas
typedef struct {
	int estado; // pendiente --> confirmado --> terminado
	t_list* registrosPlato; // una lista de registros de platos
}TablaPaginas; //--> Tabla de platos de un pedido


// Página
typedef struct {
	uint32_t total;
	uint32_t listos;
	char nombre[24];
}PaginaDatos; //--> Los datos de una pagina [un plato].

typedef struct {
	int nroFramePpal; //Frame de memoria principal, es dinámico
	int nroFrameSwap; //Frame de memoria Swap, es estático
	bool presencia; //bit de presencia
	bool modificada; //bit de modificacion
	char* timestamp;
	PaginaDatos plato; // un objeto Plato donde está toda la info.
}RegistroPagina; //--> Una pagina



typedef struct
{

	int tamanio;
	void * inicio;
	bool libre;
	char* timestamp;
	char* actualizado;

}memPpal;

t_list * memPrincipal=NULL;

// ##################################################################################
//VARIABLES

TablaSegmentos* nuevoRestaurante;


// FUNCIONES
void *gestionarCliente();


/*
 * Con esta función se va a verificar si existe la tabla de segmentos del
 * restaurante, en caso de existir se agrega el pedido (es decir se crea
 * otra tabla de paginas), en caso de no existir se crea la tabla de
 * segmentos y luego la tabla de paginas.
 */
bool *guardarPedido(char* nombreRestaurante, char* idPedido);

/*
 * Se verifica si la tabla de segmentos de ese restaurante existe, en caso de que
 * no exista se devuelve error, en caso de que sí exista se debe verificar que exista
 * el segmento correspondiente a ese pedido, sino se devuelve error.
 * Verificar si existe la pagina correspondiente al plato, en caso de que no exista
 * se debe crear una nueva pagina. En caso de que el plato no esté en memoria principal,
 * se debe ejecutar el algoritmo de reemplazo para cargar dicho plato en memoria.
 * Se agrega la cantidad de dicho plato a realizar y se devuelve OK o Fail.
 */
void *guardarPlato(char* restaurante, int idPedido, char* nuevoPlato, int cantidad);

/*
 * Devuelve toda la información de un determinado pedido.
 * Se verifica si existe la tabla de segmentos, se verifica que exista
 * el segmento, se verifica que los platos estén en memoria principal sino
 * ejecutar algoritmo de reemplazo, devolver si se produjo algun error, caso
 * contrario devolver OK junto a toda la informacion.
 */
void *obtenerPedido(char* restaurante, int idPedido);

/*
 * Se verifica si existe la tabla de segmentos, se verifica que exista
 * el segmento, se verifica el estado del pedido en "Pendiente".
 * Cambiar el estado de "Pendiente" a "Confirmado", devolver Fail si se produjo un error, caso
 * contrario devolver OK junto a toda la informacion.
 */
bool *confirmarPedido(char* restaurante, int idPedido);

/*
 * Se verifica si existe la tabla de segmentos, se verifica que exista
 * el segmento, se verifica que exista el plato en el pedido. Si las páginas no estan
 * en memoria principal ejecutar algoritmo de reemplazo. Verificar que el pedido este "Confirmado".
 * se deberá aumentar en uno la cantidad lista de ese plato. Si todos los platos estan listos
 * cambiar el estado del pedido a "Terminado" devolver Fail si se produjo algun error,
 *  caso contrario devolver OK junto a toda la informacion.
 */
bool *platosListos(char* restaurante, int idPedido, char* plato);

/*
 * Se verifica si existe la tabla de segmentos, se verifica que exista
 * el segmento, se deberá eliminar las paginas correspondientes a dicho segmento
 * por último se eliminará el segmento, devolver Fail si se produjo algun error, caso
 * contrario devolver OK.
 */
bool *finalizarPedido(char* restaurante, int idPedido);

#endif /* COMANDA_H_ */
