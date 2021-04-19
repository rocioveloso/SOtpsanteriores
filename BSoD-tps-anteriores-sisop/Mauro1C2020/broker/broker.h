#ifndef BROKER_H_
#define BROKER_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <utils/utils.h>
#include <commons/config.h>
#include<pthread.h>
#include<semaphore.h>
#include<math.h>
#include <commons/temporal.h>
#include <signal.h>
#include <commons/log.h>
//HILO PADRE
pthread_t thread0;

//LOGGER
t_log* logB;
t_log* logDUMP;
//SEMAFOROS
sem_t mutex_distribuidor;
sem_t mutex_new;
sem_t mutex_appeared;
sem_t mutex_caught;
sem_t mutex_catch;
sem_t mutex_get;
sem_t mutex_localized;
sem_t mutex_colas;
sem_t mutex_particiones;
sem_t mutex_victima;
sem_t mutex_suscriptor_ID;
sem_t mutex_mensaje_ID;
sem_t binarioA;
sem_t binarioB;
sem_t binarioC;
sem_t binarioD;
sem_t binarioE;
sem_t binarioF;
sem_t binarioG;
sem_t binarioH;
sem_t binarioI;
sem_t binarioJ;
sem_t binario;
sem_t agregarSuscriptor;
sem_t distribuidorD;


//CONFIGURACION

char* ip;
char* puerto;
int servidor;
uint32_t tamanioM;
uint32_t tamanioMinPar;
char* algoritmoM;
char* algoritmoR;
char* algoritmoPL;
uint32_t frecuenciaC;
char*archivoDeConfiguracion;
char*archivoDeLog;

//IDs
uint32_t contador_mensaje=0;
int contador_suscriptor=1;

//Enums para muestreo
typedef enum
{
    NEW, APPEARED, CATCH, CAUGHT, GET, LOCALIZED,
    MENSAJES, SUSCRIPTORES, TODO
}elemento_;

typedef struct
{
  bool recibido;
}check_;

typedef struct
{

  uint32_t id;
  uint32_t idc;
  void* data; //puntero a cache
  t_list *check;
}mensaje;

typedef struct
{
  uint32_t id;
  int socket;
  bool conectado;

}suscriptor;

typedef struct
{
  t_list *suscriptores;
  t_list *mensajes;
}cola;

//****colas
cola * new_=NULL;
cola * appeared_=NULL;
cola * catch_ =NULL;
cola * caught_ =NULL;
cola * get_=NULL;
cola * localized_=NULL;

t_list* colas=NULL;
t_list* semaforos=NULL;

//CACHE
void* cache=NULL;

//ALGORITMOS DE MEMORIA
int intentos=0;

typedef struct
{

	int tamanio;
	void * inicio;
	bool libre;
	char* timestamp;
	char*actualizado;

}particion_;
t_list * dinamicas=NULL;

//FUNCIONES
int generar_id_cliente();
int generar_id_mensaje();
suscriptor * obtener_suscriptor_cola(int idSuscriptor, cola* unaCola);
void agregar_check_nuevo_suscriptor(cola* unaCola);
void agregar_suscriptor(int idSuscriptor, int socket, cola* unaCola);
void actualizar_socket_suscriptor(int idSuscriptor, int socket, cola* unaCola);
void depurar_suscriptor(int idSuscriptor);
void * obtener_particion_disponible_PD(int pedido);
void *obtener_particion_disponible_BS(int tamanio);
int obtener_particion_utilizada(void *data);
void elegir_victima();
void obtener_datos_mensaje(particion_* unaDinamica, int i);
void dumpCache();
mensaje* obtener_mensaje_utilizado(particion_ * unaParticion);
void compactar();
void* obtener_particion_disponible(int tamanio);
uint32_t agregar_mensaje(op_code tipo, t_list *lista);
void consolidar (bool primero);
int distribuir(int posicionM, t_paquete * paquete, cola * unaCola);
void  completar_nombre(void * nombre, int tamanio);
void distribuidor(elemento_ tipoM);
void levantar_configuracion();
void inicializar_colas();
void particiones_dinamicas();
void particionar_cache();
void inicializar_cache();
int redondear(int numero);
void inicializar_semaforos();
void inicializar_broker();
void liberar_colas();
void finalizar_broker();
void mostrar_elementos(elemento_ laCola, elemento_ param);
void *gestionarCliente(void* socketCliente);


#endif /* BROKER_H_ */
