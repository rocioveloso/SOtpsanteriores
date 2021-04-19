#include <stdio.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <utils/utils.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>

struct plato {
    char* nombre;
    int precio;
} typedef t_plato;

struct pcb {
    char* nombre;
    int id_pedido;
    t_list* operaciones;
    int operacion_actual;
    int ciclos_restantes;
} typedef t_pcb;

// Semaforos
sem_t sem_cola_new;

// Archivo de config (hardcodeado)
char* ARCHIVO_CONFIG = "restaurante.config";

// Variables del archivo .config
char* NOMBRE_RESTAURANTE;
char* ALGORITMO;

// Metadata proviniente de sindicato
t_list* AFINIDADES;
int POSICION_X;
int POSICION_Y;
t_list* PLATOS;
int CANT_HORNOS;
int CANT_PEDIDOS;
int CANT_COCINEROS;

// Logger
t_log* LOGGER;

// IP y puerto del modulo que se conecta a restaurante
char* IP_MODULO_EMISOR;
char* PUERTO_MODULO_EMISOR;

// IP y puerto del sindicato
char* IP_SINDICATO;
char* PUERTO_SINDICATO;

// Colas para la planificacion de los platos
t_queue* COLA_NEW;
t_queue* COLA_BLOCKED;
t_queue* COLA_EXIT;
t_list* COLAS_READY;
t_list* COLAS_ES;

// Nro de pedido generados por el restaurante
int ID_PEDIDO = 0;

t_dictionary *leer_configs();

t_restaurante_metadata pedir_metadata();

void restaurante_server();
int iniciar_planificador();

void consultar_platos(int cliente);
int crear_pedido(int cliente);

void aniadir_plato(int cliente, int id_pedido, char* plato, int cantidad);
void confirmar_pedido(int cliente, int id_pedido);
void consultar_pedido();

int generar_id_pedido();

void preparacion_platos(t_list* platos, int id_pedido);
