#include "team.h"
#include "liberarTeam.h"
#include "conexionConBroker.h"
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <pthread.h>


void liberarEntrenador(Entrenador* entrenador){
	list_destroy_and_destroy_elements(entrenador->objetivos,free);
	list_destroy_and_destroy_elements(entrenador->objetivosPendientes,free);
	list_destroy(entrenador->pokemonsAtrapados);
	free(entrenador->posicion);
	free(entrenador);
}
void liberarPokemon(Pokemon* pokemon){
	free(pokemon->nombre);
	free(pokemon->posicion);
	free(pokemon);
}

void liberarProceso(Proceso* proceso){
	liberarPokemon(proceso->pokemon);
	free(proceso);
}
void liberarEstructuraTeam(){
	list_destroy_and_destroy_elements(team->entrenadores,(void(*)(void*))liberarEntrenador);
	list_destroy_and_destroy_elements(team->procesos,(void(*)(void*))liberarProceso);
	list_destroy_and_destroy_elements(team->procesosDeIntercambio,free);
	free(team);
}

void liberarTeam(){

	liberarConexionConBroker();
	desenlazarBroker();
	free(archivoId);
	free(archivoDeLog);
	log_destroy(log_team);
	list_destroy_and_destroy_elements(pokemonsPendientesDeCaptura,(void(*)(void*))liberarPokemon);
	list_destroy_and_destroy_elements(mensajesPendientesCatch,free);
	list_destroy_and_destroy_elements(mensajesPendientesGet,free);
	list_destroy(entrenadoresBloqueados);
	sem_destroy(&sem_proceso);
	sem_destroy(&sem_mx_proceso);
	sem_destroy(&sem_proceso_intercambio);

	liberarEstructuraTeam();

}
