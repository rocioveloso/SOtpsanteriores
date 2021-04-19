#ifndef TP_2020_1C_CAMELCASE_TEAM_H
#define TP_2020_1C_CAMELCASE_TEAM_H

#include <stdio.h>
#include <stdlib.h>
#include <utils/utils.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <semaphore.h>


pthread_t threadPlanificador, threadTeamId, threadPlanificadorIntercambio,threadGameboy;
t_config* teamConfig;
t_log* log_team;
typedef struct Posicion{
   int x;
   int y;
} Posicion;


typedef enum EstadoEntrenador{
    NUEVO, BLOQUEADO_ESPERA_MENSAJE, BLOQUEADO_DEADLOCK, BLOQUEADO_REPLANIFICACION, LISTO, EJECUTANDO, FINALIZADO, BLOQUEADO_ESPERA_INTERCAMBIO
}EstadoEntrenador;

typedef struct PokemonEnMapa{
    char* nombre;
    t_list* posiciones;
}PokemonEnMapa;

typedef struct Pokemon{
    char* nombre;
    Posicion* posicion;
}Pokemon;
typedef struct Entrenador{
	char idEntrenador;
	t_list* objetivosPendientes;
	t_list* objetivos;
	t_list* pokemonsAtrapados;
    EstadoEntrenador estado;
    Posicion *posicion;
    int cantCiclosCPUTotales;
} Entrenador;

typedef struct Proceso{
	Entrenador* entrenador;
	Pokemon* pokemon;
	int rafagaAnterior;
	float estimadoAnterior;
	float estimadoActual;
}Proceso;

typedef struct ProcesoIntercambio{
	Entrenador* entrenador1;
	Entrenador* entrenador2;
	char* pokemonQueNecesitaE1;
	char* pokemonQueNecesitaE2;
	int rafagaAnterior;
	float estimadoAnterior;
	float estimadoActual;
	bool favorableParaUnLado;
}ProcesoIntercambio;

typedef struct Team{
    t_list* entrenadores;
    int conexionBroker;
    int idTeam;
    t_list* procesos;
    t_list* procesosDeIntercambio;
    //otras estructuras de planificacion
} Team;

typedef struct MensajePendienteCatch{
	int idMensaje;
	Entrenador* entrenador;
	Pokemon* pokemon;
}MensajePendienteCatch;

typedef struct ParametrosGet{
    char* nombrePokemon;
    int conexionConBroker;
    int cantidadDePokemonesNecesarios;
    t_list* entrenadores;
} ParametrosGet;

typedef struct ParametrosCatch{
    Pokemon* pokemon;
    int conexionConBroker;
    Entrenador* entrenador;
} ParametrosCatch;

typedef struct MensajePendienteGet{
	int idMensaje;
	char* pokemon;
	int cantidadDePokemonesNecesarios;
	t_list* entrenadores;
}MensajePendienteGet;


sem_t sem_proceso;
sem_t sem_mx_proceso;
sem_t sem_proceso_intercambio;
Team* team;
t_list* pokemonesEnElMapa;
char* pokemonCatch;
t_list* mensajesPendientesCatch;
t_list* mensajesPendientesGet;
char *archivoDeConfiguracion;
char* ipBroker;
char* puertoBroker;
char* auxNombrePokemon;
t_list* pokemonsPendientesDeCaptura;
char *algoritmo;
char entrenadorEjecutando;
char* pokemonEjecutando;
bool interrupcion;
int retardoCPU;
int quantum;
int tiempoReconexion;
float alpha;
int estimadoInicial;
char* pokemonAux;
bool teamTermino, arrancaPlanificador;
int cantCiclosCPUTotales, cantCambiosDeContextoTotales, cantDeadlocksProducidosyResueltos;
t_list* entrenadoresBloqueados;
char* archivoDeLog;
int servidor;
char* archivoId;
char* nombreGet;
Pokemon* pokemonAuxLocalized;

void inicializarEntrenadores();
void iniciarTeam();
void localizedPokemon(t_list* lista);
void appearedPokemon(char* nombrePokemon, int posicionX, int posicionY);
void catchPokemon(Pokemon* pokemon, Entrenador* entrenador);
void caughPokemon(int idMensaje, int resultadoCauoight);
void cargarObjetivosPendientes();
int list_find_element(t_list* lista, char* pokemon);
void mostrar_elementos(t_list *lista);
void getPokemon(void* pokemon,int cantidadDePokemonesNecesarios,t_list*);
t_link_element* buscarMensajePendienteCatch(t_list * lista, int id);
int recibirIdMensajeCatch();
void *gestionarNew(void* socketCliente);
void *gestionarAppeared(void* socket);
void *gestionarCaught(void* socket);
void *gestionarLocalized(void *socket);
int suscriboColaAppeared();
int suscriboColaLocalized();
int suscriboColaCaught();
void deteccionDeadlock();
void mostrarListaIntercambio();
void irAAtrapar(Proceso* proceso);
void irAAtraparRoundRobin(Proceso* proceso);
ProcesoIntercambio* procesoIntercambioEjecutando;
#endif //TP_2020_1C_CAMELCASE_TEAM_H
