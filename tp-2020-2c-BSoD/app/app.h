
#ifndef APP_H_
#define APP_H_

#include <stdio.h>
#include <stdlib.h>
#include <utils/utils.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/socket.h>


char* config = "app.config";

t_log* logger;

pthread_t threadPlanificador, threadRestaurante;

char* archivoLog;
char* archivoConfig;
int servidor;
char* algoritmo;
char* ipComanda;
char* puertoComanda;
char* puertoEscucha;
float alpha;
bool appTermino;
int cantCiclosCPUTotales, cantCambiosTotales;
bool interrupcion;
int estimadoInicial;
int retardoCPU;


t_list* repartidoresBloqueados;
t_list* restaurantesEnElMapa;

sem_t sem_proceso;
sem_t sem_mx_proceso;
sem_t sem_proceso_intercambio;


typedef struct App{
    t_list* repartidores;
    t_list* procesos;
} App;


typedef struct Repartidor{
	char idRepartidor;
//	t_list* pedido;
    EstadoRepartidor estado;
    Posicion *posicion;
    int cantCiclosCPUTotales;
} Repartidor;

typedef enum EstadoRepartidor{
    NUEVO, BLOQUEADO, LISTO, EJECUTANDO, FINALIZADO
}EstadoRepartidor;

typedef struct Pedido{
	char id_pedido;
	EstadoPedido estadoPedido;
	t_list* platos;
}Pedido;

typedef enum EstadoPedido{
	PENDIENTE, CONFIRMADO, TERMINADO
}EstadoPedido;


typedef struct Restaurante{
    char* nombre;
    Posicion* posicion;
    t_list* listaplatos;
}Restaurante;


typedef struct RestauranteEnMapa{
    char* nombre;
    t_list* posiciones;
    t_list* platos;
}RestauranteEnMapa;


typedef struct Posicion{
   int x;
   int y;
} Posicion;


typedef struct Proceso{
	Repartidor* repartidor;
	Restaurante* restaurante;
	int rafagaAnterior;
	float estimadoAnterior;
	float estimadoActual;
}Proceso;



void llegada_mensaje_app(void* socketServidor);

void consultar_restaurante();

void seleccionar_restaurante(int idCliente);

void consultar_plato(int idCliente);

int crear_pedido(int idCliente);

int aniadir_plato(int idCliente, int id_pedido, char* plato, int cantidad);

void plato_listo(char* nombre_restaurante);

void confirmar_plato(int idCliente, int id_pedido);

void consultar_pedido(int idCliente, int id_pedido);



void iniciarApp();

void inicializarRepartidor();

void planificador();

void sjf();

void fifo();

void hrrn();

void irRetirarPedido(Proceso* proceso);

void irRetirarPedidoHRRN(Proceso* proceso);

void agregarProcesoACola(Repartidor* repartidor,Restaurante* restaurante);

float distancia(int x1, int y1, int x2, int y2);

Repartidor* repartidorCercano(int posicionX,int posicionY);








#endif /* APP_H_ */
