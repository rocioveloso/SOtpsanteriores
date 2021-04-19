#include"team.h"
#include <commons/collections/dictionary.h>
#include <commons/collections/node.h>
#include <pthread.h>
#include <math.h>
#include <commons/string.h>
#include "liberarTeam.h"
#include "conexionConBroker.h"

bool mismoPokemonMismaPosicionPendientes(void* parametro){
	Pokemon* poke = (Pokemon*)parametro;
	return strcmp(poke->nombre, pokemonAuxLocalized->nombre) == 0 &&  poke->posicion->x == pokemonAuxLocalized->posicion->x &&  poke->posicion->y == pokemonAuxLocalized->posicion->y;
}
bool mismoPokemonMismaPosicion(void* parametro){
	Proceso* proceso = (Proceso*)parametro;
	return strcmp(proceso->pokemon->nombre, pokemonAuxLocalized->nombre) == 0 &&  proceso->pokemon->posicion->x == pokemonAuxLocalized->posicion->x &&  proceso->pokemon->posicion->y == pokemonAuxLocalized->posicion->y;
}

bool origenNoBloqueado(void* proceso){
	ProcesoIntercambio* procesoIntercambio = (ProcesoIntercambio*)proceso;
	return procesoIntercambio->entrenador1->estado != BLOQUEADO_ESPERA_INTERCAMBIO;
}


bool entrenadorBloqueado(void* elemento){
	Entrenador* entrenador = (Entrenador*)elemento;
	//printf("Entado %c: %d\n", entrenador->idEntrenador, entrenador->estado);
	return entrenador->estado == BLOQUEADO_DEADLOCK;
}
bool entrenadorFinalizado(void* elemento){
	Entrenador* entrenador = (Entrenador*)elemento;
	return entrenador->estado == FINALIZADO;
}
bool EntrenadorFinalizadoOBloqueado(void* element){
	Entrenador* entrenador = (Entrenador*)element;
	//printf("Entrenador %c %d\n", entrenador->idEntrenador,entrenador->estado);
	//printf("DEADLOCK %d\n", entrenador->estado == BLOQUEADO_DEADLOCK);
	//printf("FINALIZADO %d\n", entrenador->estado == FINALIZADO);
	//printf("OR %d\n", (entrenador->estado == BLOQUEADO_DEADLOCK) || (entrenador->estado == FINALIZADO));
	return (entrenador->estado == BLOQUEADO_DEADLOCK) || (entrenador->estado == FINALIZADO);
}

float calcularEstimado(float estimadoAnterior, int rafagaAnterior){
	return estimadoAnterior * alpha + (1-alpha)*rafagaAnterior;
}

bool mismoProcesoIntercambio(void* proceso){
	ProcesoIntercambio* procesoIntercambio = (ProcesoIntercambio*)proceso;

	bool mismosEntrenadores = procesoIntercambioEjecutando->entrenador1->idEntrenador == procesoIntercambio->entrenador1->idEntrenador && procesoIntercambioEjecutando->entrenador2->idEntrenador == procesoIntercambio->entrenador2->idEntrenador;
	bool mismosPokemones = strcmp(procesoIntercambio->pokemonQueNecesitaE1, procesoIntercambioEjecutando->pokemonQueNecesitaE1) == 0 && strcmp(procesoIntercambio->pokemonQueNecesitaE2, procesoIntercambioEjecutando->pokemonQueNecesitaE2) == 0;
	return mismosEntrenadores &&  mismosPokemones;
}

bool menorEstimado(void* proceso1, void* proceso2){
	return ((Proceso*)proceso1)->estimadoActual <= ((Proceso*)proceso2)->estimadoActual;
}
bool menorEstimadoIntercambio(void* proceso1, void* proceso2){
	return ((ProcesoIntercambio*)proceso1)->estimadoActual <= ((ProcesoIntercambio*)proceso2)->estimadoActual;
}

void verListaDeAtrapados(t_list* lista){
	t_link_element *element = lista->head;
	//puts("LISTA");
	while (element != NULL){
		//printf("%s-", (char*)element->data);
		element = element->next;
	}
    //puts("\n");
}

void verListaPendientes(t_list* lista){
	t_link_element *element = lista->head;
	//puts("LISTA PENDIENTES");
	while (element != NULL){
		//printf("Pokemon: %s\n", ((Pokemon*)element->data)->nombre);
		element = element->next;
	}
    //puts("\n");
}


int buscarPokemonGet(t_list* lista, char* pokemon) {
    t_link_element *element = lista->head;
    int position = 0;

    while (element != NULL && strcmp(pokemon, ((MensajePendienteGet*)element->data)->pokemon) != 0) {
        element = element->next;
        position++;
    }

    if (element == NULL) {
        position = -1;
    }

    return position;
}

int buscarPokemon(t_list* lista, char* pokemon) {
    t_link_element *element = lista->head;
    int position = 0;

    while (element != NULL && strcmp(pokemon, (char*)element->data) != 0) {
        element = element->next;
        position++;
    }

    if (element == NULL) {
        position = -1;
    }

    return position;
}

void objetivosCumplidos(Entrenador* entrenador1, Entrenador* entrenador2){

	if(entrenador1->objetivosPendientes->elements_count || entrenador1->pokemonsAtrapados->elements_count < entrenador1->objetivos->elements_count){
		entrenador1->estado = BLOQUEADO_DEADLOCK; // habilito al entrenador para poder volver a operar
		log_trace(log_team, "Entrenador %c terminó proceso de intercambio. No puede atrapar más pokemones y no cumplió sus objetivos. Estado: BLOQUEADO\n", entrenador1->idEntrenador);
	}else{
		entrenador1->estado = FINALIZADO;
		log_trace(log_team, "Entrenador %c terminó proceso de intercambio. Cumplió sus objetivos. Estado: FINALIZADO\n", entrenador1->idEntrenador);
	}

	if(entrenador2->objetivosPendientes->elements_count || entrenador2->pokemonsAtrapados->elements_count < entrenador2->objetivos->elements_count){
		entrenador2->estado = BLOQUEADO_DEADLOCK; // habilito al entrenador para poder volver a operar
		log_trace(log_team, "Entrenador %c terminó proceso de intercambio. No puede atrapar más pokemones y no cumplió sus objetivos. Estado: BLOQUEADO\n", entrenador2->idEntrenador);
	}else{
		entrenador2->estado = FINALIZADO;
		log_trace(log_team, "Entrenador %c terminó proceso de intercambio. Cumplió sus objetivos. Estado: FINALIZADO\n", entrenador2->idEntrenador);
	}

	if(team->procesosDeIntercambio->elements_count == 0){
		if(list_any_satisfy(team->entrenadores, entrenadorBloqueado)){
			////puts("ARRANCA LA DETECCIÓN DE DEADLOCKS DE NUEVO");
			deteccionDeadlock();
		}else{
			//puts("FINALIZADO");
			teamTermino = true;
			sem_post(&sem_proceso);
			pthread_cancel(threadGameboy);
			pthread_detach(threadGameboy);
			//liberarConexionConBroker();
		}
	}
}

void loguearEstadoFinal(){

	t_link_element* aux = team->entrenadores->head;
	log_trace(log_team, "Estado final entrenadores:");
	//printf("Cantidad de entrenadores: %d\n",team->entrenadores->elements_count);
	while(aux){
		log_trace(log_team, "Entrenador %c", ((Entrenador*)aux->data)->idEntrenador);
		t_link_element* auxAtrapados = ((Entrenador*)aux->data)->pokemonsAtrapados->head;
		t_link_element* auxObjetivosOriginales = ((Entrenador*)aux->data)->objetivos->head;
		t_link_element* auxObjetivosPendientes = ((Entrenador*)aux->data)->objetivosPendientes->head;


	//	verListaDeAtrapados(((Entrenador*)aux->data)->objetivos);
		log_trace(log_team, "Objetivos originales:");
		while(auxObjetivosOriginales){
			log_trace(log_team, "\t%s", auxObjetivosOriginales->data);
			auxObjetivosOriginales = auxObjetivosOriginales->next;
		}

		log_trace(log_team, "Pokemones atrapados:");
		while(auxAtrapados){
			log_trace(log_team, "\t%s", auxAtrapados->data);
			auxAtrapados = auxAtrapados->next;
		}
		log_trace(log_team, "Objetivos pendientes:");
		while(auxObjetivosPendientes){
			log_trace(log_team, "\t%s", auxObjetivosPendientes->data);
			auxObjetivosPendientes = auxObjetivosPendientes->next;
		}


		aux = aux->next;
	}

}

void intercambio(ProcesoIntercambio* proceso){
	//////puts("\n---------intercambio ver lista de objetivos--------\n");
	////printf("entrenador %c\n", proceso->entrenador1->idEntrenador);
	//verListaDeAtrapados(proceso->entrenador1->objetivos);
	////printf("entrenador %c\n", proceso->entrenador2->idEntrenador);
	verListaDeAtrapados(proceso->entrenador2->objetivos);
	char* pokemonQueNecesitaE1 = proceso->pokemonQueNecesitaE1;
	char* pokemonQueNecesitaE2 = proceso->pokemonQueNecesitaE2;
	Entrenador* entrenador1 = proceso->entrenador1;
	Entrenador* entrenador2 = proceso->entrenador2;

	log_trace(log_team,"Se realiza el intercambio entre el entrenador %c y el %c", entrenador1->idEntrenador,entrenador2->idEntrenador);

	sleep(5*retardoCPU);
	cantCiclosCPUTotales +=5;
	entrenador1->cantCiclosCPUTotales +=5;
	entrenador2->cantCiclosCPUTotales +=5;

	list_add(entrenador1->pokemonsAtrapados, pokemonQueNecesitaE1);
	list_add(entrenador2->pokemonsAtrapados, pokemonQueNecesitaE2);

	////puts("\n\nINTERCAMBIO REALIZADO CON EXITO\n\n");
	verListaDeAtrapados(entrenador1->pokemonsAtrapados);
	verListaDeAtrapados(entrenador2->pokemonsAtrapados);
	////puts("\n-----objetivos pendientes-----\n");
	verListaDeAtrapados(entrenador1->objetivosPendientes);
	verListaDeAtrapados(entrenador2->objetivosPendientes);

	procesoIntercambioEjecutando = proceso;

	log_trace(log_team,"Resultado del Intercambio: \n");

	log_trace(log_team,"Entrenador %c le da el pokemon %s al entrenador %c \n", entrenador2->idEntrenador, pokemonQueNecesitaE1 , entrenador1->idEntrenador);

	log_trace(log_team,"Entrenador %c le da el pokemon %s al entrenador %c \n", entrenador1->idEntrenador, pokemonQueNecesitaE2 , entrenador2->idEntrenador);

	ProcesoIntercambio * procesoAEliminar = (ProcesoIntercambio*)list_remove_by_condition(team->procesosDeIntercambio, mismoProcesoIntercambio);
	if(procesoAEliminar)
		////puts("PROCESO ELIMINADO DE COLA.\n");
	free(procesoAEliminar);
	cantCambiosDeContextoTotales++;
	objetivosCumplidos(entrenador1, entrenador2);
	sem_post(&sem_proceso_intercambio);


}

void irAIntercambiarRoundRobin(ProcesoIntercambio* proceso){
	int movimientos = 0;
	////printf("irAIntercambiarRoundRobin");

	Entrenador* entrenadorOrigen = proceso->entrenador1;
	Entrenador* entrenadorDestino = proceso->entrenador2;
	////////printf("POSICION ORIGINAL X: %d Y: %d\n", entrenadorOrigen->posicion->x, entrenadorOrigen->posicion->y);
	////////printf("POSICION DESTINO X: %d Y: %d\n", pokemon->posicion->x, pokemon->posicion->y);
	entrenadorOrigen->estado = EJECUTANDO;
	entrenadorDestino->estado = BLOQUEADO_ESPERA_INTERCAMBIO;
	////printf("***Entrenador %c Pasa a estado bloqueado por intercambio\n", entrenadorDestino->idEntrenador);
	cantCambiosDeContextoTotales++;
	//entrenadorDestino->estado = BLOQUEADO_ESPERA_INTERCAMBIO; (VER SI VA O NO VA)
	log_trace(log_team, "Entrenador %c seleccionado para ejecutar. Estado: EJECUTANDO\n", entrenadorOrigen->idEntrenador);
	log_trace(log_team,"Entrenador %c posición: (%d,%d) va a intercambiar con %c en la posición (%d,%d)\n", entrenadorOrigen->idEntrenador, entrenadorOrigen->posicion->x, entrenadorOrigen->posicion->y, entrenadorDestino->idEntrenador, entrenadorDestino->posicion->x, entrenadorDestino->posicion->y);
	while ((entrenadorOrigen->posicion->x != entrenadorDestino->posicion->x || entrenadorOrigen->posicion->y != entrenadorDestino->posicion->y) && movimientos < quantum){
		if(entrenadorOrigen->posicion->x != entrenadorDestino->posicion->x){
            if(entrenadorOrigen->posicion->x > entrenadorDestino->posicion->x)
                entrenadorOrigen->posicion->x--;
            else
                entrenadorOrigen->posicion->x++;
            sleep(retardoCPU);
            entrenadorOrigen->cantCiclosCPUTotales++;
            cantCiclosCPUTotales++;
            movimientos++;
        }
        else if(entrenadorOrigen->posicion->y != entrenadorDestino->posicion->y){
            if(entrenadorOrigen->posicion->y > entrenadorDestino->posicion->y)
                entrenadorOrigen->posicion->y--;
            else
                entrenadorOrigen->posicion->y++;
            sleep(retardoCPU);
            entrenadorOrigen->cantCiclosCPUTotales++;
            cantCiclosCPUTotales++;
            movimientos++;
        }

		log_trace(log_team, "Entrenador %c se mueve a posicion (%d,%d)", entrenadorOrigen->idEntrenador, entrenadorOrigen->posicion->x, entrenadorOrigen->posicion->y);
    }

	if(entrenadorOrigen->posicion->x == entrenadorDestino->posicion->x && entrenadorOrigen->posicion->y == entrenadorDestino->posicion->y){
		////puts("------ACA VA EL INTERCAMBIO------\n");
		//sem_post(&sem_proceso); //esto borrarlo
		intercambio(proceso);
	} else{
		entrenadorOrigen->estado = LISTO;
		cantCambiosDeContextoTotales++;
		log_trace(log_team, "Interrupción por fin de quantum. Entrenador %c pasa a estado listo. Estado: LISTO\n", entrenadorOrigen->idEntrenador);
		ProcesoIntercambio*  auxiliar = (ProcesoIntercambio*) list_remove(team->procesosDeIntercambio, 0);
		list_add(team->procesosDeIntercambio, auxiliar);
		////puts("INTERRUPCIÓN POR QUANTUM\n");
		//mostrarListaIntercambio();
		sem_post(&sem_proceso_intercambio);
	}

}

void irAIntercambiar(ProcesoIntercambio* proceso){
	int rafaga = 0;
	Entrenador* entrenadorOrigen = proceso->entrenador1;
	Entrenador* entrenadorDestino = proceso->entrenador2;
	////////printf("POSICION ORIGINAL X: %d Y: %d\n", entrenadorOrigen->posicion->x, entrenadorOrigen->posicion->y);
	////////printf("POSICION DESTINO X: %d Y: %d\n", pokemon->posicion->x, pokemon->posicion->y);
	entrenadorOrigen->estado = EJECUTANDO;
	cantCambiosDeContextoTotales++;
	log_trace(log_team, "Entrenador %c seleccionado para ejecutar. Estado: EJECUTANDO\n", entrenadorOrigen->idEntrenador);
	log_trace(log_team,"Entrenador %c posición: (%d,%d) va a intercambiar con %c en la posición (%d,%d)\n", entrenadorOrigen->idEntrenador, entrenadorOrigen->posicion->x, entrenadorOrigen->posicion->y, entrenadorDestino->idEntrenador, entrenadorDestino->posicion->x, entrenadorDestino->posicion->y);
	while (entrenadorOrigen->posicion->x != entrenadorDestino->posicion->x || entrenadorOrigen->posicion->y != entrenadorDestino->posicion->y){
		if(interrupcion){
			////puts("------INTERRUPCIÓN-----\n");
			cantCambiosDeContextoTotales++;
			entrenadorOrigen->estado = LISTO; //TODO: ver si esta bien que cambie el estado.
			log_trace(log_team, "Llego interrupción, se debe replanificar. Entrenador %c pasa a estado listo. Estado: LISTO\n", entrenadorOrigen->idEntrenador);
			proceso->estimadoAnterior = proceso->estimadoActual;
			proceso->rafagaAnterior = rafaga;
			proceso->estimadoActual = calcularEstimado(proceso->estimadoAnterior, proceso->rafagaAnterior);
			////printf("Entrenador: %c Estimado: %f\n", entrenadorOrigen->idEntrenador, proceso->estimadoActual);

			list_sort(team->procesosDeIntercambio, menorEstimadoIntercambio);
			mostrarListaIntercambio();
			interrupcion = false;
			sem_post(&sem_proceso_intercambio);
			return;
		}
		if(entrenadorOrigen->posicion->x != entrenadorDestino->posicion->x){
			if(entrenadorOrigen->posicion->x > entrenadorDestino->posicion->x)
				entrenadorOrigen->posicion->x--;
			else
				entrenadorOrigen->posicion->x++;
			rafaga++;
			entrenadorOrigen->cantCiclosCPUTotales++;
			cantCiclosCPUTotales++;
			sleep(retardoCPU);
		}
		else if(entrenadorOrigen->posicion->y != entrenadorDestino->posicion->y){
			if(entrenadorOrigen->posicion->y > entrenadorDestino->posicion->y)
				entrenadorOrigen->posicion->y--;
			else
				entrenadorOrigen->posicion->y++;
			rafaga++;
			entrenadorOrigen->cantCiclosCPUTotales++;
			cantCiclosCPUTotales++;
			sleep(retardoCPU);
			//chequear interrupcion
			//si hay interrupcion y la diferencia de ambos valores en x y en y != 0-> break
			//si no no haces nada y seguis ejecutando
		}

		log_trace(log_team, "Entrenador %c se mueve a posicion (%d,%d)", entrenadorOrigen->idEntrenador, entrenadorOrigen->posicion->x, entrenadorOrigen->posicion->y);
	}

	////puts("------ACA VA EL INTERCAMBIO------\n");
	//sem_post(&sem_proceso); //esto borrarlo
	intercambio(proceso);
	//ACA VA EL INTERCAMBIO

}

//*************************************	DEADLOCK******************************************
void fifoIntercambio(){
	while(!teamTermino){

		sem_wait(&sem_proceso_intercambio);
		if(team->procesosDeIntercambio->elements_count > 0){
			////printf("\n\n\n-------cantidad de procesos: %d----\n\n\n\n",team->procesosDeIntercambio->elements_count);
//
//			////////puts("******************ATRAPAR*******************\n");
			irAIntercambiar((ProcesoIntercambio*)team->procesosDeIntercambio->head->data);
		}
		else
			sem_post(&sem_proceso_intercambio);


	}
	////puts("Finaliza fifo");
}

void rrIntercambio(){
	while(!teamTermino){
//		sem_wait(&sem_proceso);
//		if(team->procesosDeIntercambio->elements_count > 0){
//			t_list* listaFiltrada = list_filter(team->procesosDeIntercambio, origenNoBloqueado);
//			////printf("filtro bien. Cantidad: %d\n", listaFiltrada->elements_count);
//			if(listaFiltrada->elements_count)
//				irAIntercambiarRoundRobin((ProcesoIntercambio*)listaFiltrada->head->data);
//			else
//				sem_post(&sem_proceso);
//		}else
//			sem_post(&sem_proceso);
		sem_wait(&sem_proceso_intercambio);
		if(team->procesosDeIntercambio->elements_count > 0){
					t_list* listaFiltrada = list_filter(team->procesosDeIntercambio, origenNoBloqueado);
					//printf("filtro bien. Cantidad: %d\n", listaFiltrada->elements_count);
					if(listaFiltrada->elements_count)
						irAIntercambiarRoundRobin((ProcesoIntercambio*)listaFiltrada->head->data);
					else
						sem_post(&sem_proceso_intercambio);
				}
				else
					sem_post(&sem_proceso_intercambio);

	}
}

void sjf_sdIntercambio(){
	while(!teamTermino){

		sem_wait(&sem_proceso_intercambio);
				if(team->procesosDeIntercambio->elements_count > 0){
					////printf("\n\n\n-------cantidad de procesos: %d----\n\n\n\n",team->procesosDeIntercambio->elements_count);
		//
		//			////////puts("******************ATRAPAR*******************\n");
					irAIntercambiar((ProcesoIntercambio*)team->procesosDeIntercambio->head->data);
				}
				else
					sem_post(&sem_proceso_intercambio);


		}
}

void sjf_cdIntercambio(){
	while(!teamTermino){
		sem_wait(&sem_proceso_intercambio);
				if(team->procesosDeIntercambio->elements_count > 0){
					////printf("\n\n\n-------cantidad de procesos: %d----\n\n\n\n",team->procesosDeIntercambio->elements_count);
		//
		//			////////puts("******************ATRAPAR*******************\n");
					irAIntercambiar((ProcesoIntercambio*)team->procesosDeIntercambio->head->data);
				}
				else
					sem_post(&sem_proceso_intercambio);

		}
}

void* planificadorIntercambio(){
  if(strcmp(algoritmo,"FIFO") == 0){
		fifoIntercambio();
	}else if(strcmp(algoritmo,"RR") == 0){
		rrIntercambio();
	}else if(strcmp(algoritmo,"SJF-CD") == 0){
		sjf_cdIntercambio();
	} else if(strcmp(algoritmo,"SJF-SD") == 0){
		sjf_sdIntercambio();
	}
  return NULL;
}
void crearProcesoIntercambio(Entrenador* entrenador1, Entrenador* entrenador2, char* objetivoE1, char* objetivoE2, bool favorable){
	ProcesoIntercambio* procesoNuevo = (ProcesoIntercambio*) malloc(sizeof(ProcesoIntercambio));

	procesoNuevo->entrenador1 = entrenador1;
	procesoNuevo->entrenador2 = entrenador2;
	procesoNuevo->pokemonQueNecesitaE1 = objetivoE1;
	procesoNuevo->pokemonQueNecesitaE2 = objetivoE2;
	procesoNuevo->favorableParaUnLado = favorable;

	if(strcmp(algoritmo,"SJF-CD") == 0 || strcmp(algoritmo,"SJF-SD") == 0){
		procesoNuevo->estimadoAnterior = estimadoInicial;
		procesoNuevo->rafagaAnterior = 0;
		procesoNuevo->estimadoActual = calcularEstimado(procesoNuevo->estimadoAnterior, procesoNuevo->rafagaAnterior);
		////printf("Entrenador: %c Estimado: %f\n", procesoNuevo->entrenador1->idEntrenador, procesoNuevo->estimadoActual);
		list_sort(team->procesosDeIntercambio, menorEstimado);
		mostrarListaIntercambio();
		interrupcion = strcmp(algoritmo,"SJF-CD") == 0;
	}
	procesoNuevo->entrenador1->estado = LISTO;
	list_add(team->procesosDeIntercambio,procesoNuevo);


	////printf("\n Entrenador %c necesita Pokemon %s que tiene el entrenador %c y le da %s\n", entrenador1->idEntrenador, objetivoE1, entrenador2->idEntrenador, objetivoE2);
}



char* encuentraElPokemonEnLaLista(t_link_element* pokemones, char* pokemonObjetivo){
	t_link_element* auxiliarLista = pokemones;
	char* objetivoEncontrado;
	while(auxiliarLista){
		////printf("\n AUXILIAR LISTA: %s\n",(char*)auxiliarLista->data);
		////printf("\n POKEMON OBJETIVO: %s\n", pokemonObjetivo);
		//////printf("\n Encuentra el Pokemon - %s VS %s", (char*)auxiliarLista->data,pokemonObjetivo);
		if(strcmp(auxiliarLista->data, pokemonObjetivo) == 0){
			////puts("\nLo necesita\n");
			objetivoEncontrado = auxiliarLista->data;
			////printf("\n Objetivo 2: %s",objetivoEncontrado);
			return objetivoEncontrado;
		}
		auxiliarLista = auxiliarLista->next;
	}
	return NULL;
}



int requiereIntercambio(Entrenador* entrenador1, Entrenador* entrenador2){
	//sem_wait(&sem_deadlock);
	t_link_element* aux1 = entrenador1->pokemonsAtrapados->head;
	t_link_element* aux2 = entrenador2->pokemonsAtrapados->head;
	t_link_element* objetivosEntrenador1 = entrenador1->objetivosPendientes->head;
	t_link_element* objetivosEntrenador2 = entrenador2->objetivosPendientes->head;



	//puts("\n\n------LISTA DE OBJETIVOS PENDIENTES-------\n\n");
	verListaDeAtrapados(entrenador1->objetivosPendientes);
	verListaDeAtrapados(entrenador2->objetivosPendientes);
	char* objetivoE1;
	char* objetivoE2;
	int posicionEnAtrapados = 0, posicionEnObjetivosPendientes = 0;
	////printf("\nCantidad de pokemons atrapados del entrenador 1: %d y del entrenador 2: %d\n",entrenador1->pokemonsAtrapados->elements_count,entrenador2->pokemonsAtrapados->elements_count );
	//puts("\n\n------LISTA DE ATRAPADOS-------\n\n");
	verListaDeAtrapados(entrenador1->pokemonsAtrapados);
	verListaDeAtrapados(entrenador2->pokemonsAtrapados);

	while(aux1){
		pokemonAux = (char*)aux1->data;
		////puts("WHILE(AUX1)\n");
		objetivoE2 = encuentraElPokemonEnLaLista(objetivosEntrenador2, pokemonAux);
		if(objetivoE2){
			////printf("\n Requiere Intercambio - OBJETIVO: %s \n", objetivoE2);
			//////printf("%c necesita cambiar con %c. (%s)\n", entrenador2->idEntrenador, entrenador1->idEntrenador,objetivoE1);
			//entrenador 1 tiene un pokemon que necesita el entrenador 2;
			break;
		}
		aux1 = aux1->next;
	}
	////puts("\n Termina de recorrer la lista de 1\n");
	while(aux2){
		////puts("WHILE(AUX2)\n");
		pokemonAux = aux2->data;
		objetivoE1 = encuentraElPokemonEnLaLista(objetivosEntrenador1, pokemonAux);
		////printf("\n Objetivo 2.1: %s",objetivoE1);
		if(objetivoE1){
			//////printf("\n Requiere Intercambio - OBJETIVO: %s \n", objetivo);

			////printf("%c necesita cambiar con %c. (%s)\n", entrenador1->idEntrenador, entrenador2->idEntrenador,objetivoE1);
			//entrenador 2 tiene un pokemon que necesita el entrenador 1;
			break;
			}

		aux2 = aux2->next;
	}

	////printf("\nObjetivo 1: %s Objetivo 2: %s\n", objetivoE1, objetivoE2);

	if(objetivoE1 && objetivoE2){
//		verListaDeAtrapados(entrenador1->pokemonsAtrapados);
//		verListaDeAtrapados(entrenador2->pokemonsAtrapados);


		posicionEnAtrapados = buscarPokemon(entrenador2->pokemonsAtrapados,objetivoE1);
		////printf("posicion en atrapados: %d\n", posicionEnAtrapados);
		char* pokemonEliminado = list_remove(entrenador2->pokemonsAtrapados,posicionEnAtrapados);
		////printf("\nPokemon eliminado de la lista de atrapados de E2 (posicion %d): %s \n", posicionEnAtrapados,pokemonEliminado);
		free(pokemonEliminado);

		posicionEnAtrapados = buscarPokemon(entrenador1->pokemonsAtrapados,objetivoE2);
		pokemonEliminado = list_remove(entrenador1->pokemonsAtrapados,posicionEnAtrapados);
		////printf("\nPokemon eliminado de la lista de atrapados de E1  (posicion %d): %s \n", posicionEnAtrapados,pokemonEliminado);
		free(pokemonEliminado);


//		verListaDeAtrapados(entrenador1->pokemonsAtrapados);
//		verListaDeAtrapados(entrenador2->pokemonsAtrapados);

		posicionEnObjetivosPendientes = buscarPokemon(entrenador1->objetivosPendientes,objetivoE1);
		list_remove(entrenador1->objetivosPendientes,posicionEnObjetivosPendientes);
		////printf("\nPokemon eliminado de la lista de objetivos pendientes de E1: %s \n", eliminado);

		posicionEnObjetivosPendientes = buscarPokemon(entrenador2->objetivosPendientes,objetivoE2);
		list_remove(entrenador2->objetivosPendientes,posicionEnObjetivosPendientes);
		//pdrintf("\nPokemon eliminado de la lista de objetivos pendientes de E2: %s \n", eliminado);

		cantDeadlocksProducidosyResueltos++;
		crearProcesoIntercambio(entrenador1,entrenador2,objetivoE1,objetivoE2, false);
		//sem_post(&sem_deadlock);
//		verListaDeAtrapados(entrenador1->pokemonsAtrapados);
//		verListaDeAtrapados(entrenador2->pokemonsAtrapados);

	}else{
		if(objetivoE1){
			////puts("\n Entro al E1 != NULL");
			t_link_element* aux_pokemon_atrapado_E1 = entrenador1->pokemonsAtrapados->head;
			int posicionEnAtrapados = 0;

			while(aux_pokemon_atrapado_E1){
				int index = buscarPokemon(entrenador1->objetivos,aux_pokemon_atrapado_E1->data);
				if(index == -1){
					objetivoE2 = list_remove(entrenador1->pokemonsAtrapados,posicionEnAtrapados);
					break;
				}
				posicionEnAtrapados++;
				aux_pokemon_atrapado_E1 = aux_pokemon_atrapado_E1->next;
			}
			////printf("\nObjetivo 1: %s Objetivo 2: %s\n", objetivoE1, objetivoE2);

			posicionEnObjetivosPendientes = buscarPokemon(entrenador1->objetivosPendientes,objetivoE1);
			list_remove(entrenador1->objetivosPendientes,posicionEnObjetivosPendientes);
			////printf("\nPosicion que devuelve cuando elimina en objetivos pendientes de E1: %d\n",posicionEnObjetivosPendientes);

			posicionEnAtrapados = buscarPokemon(entrenador2->pokemonsAtrapados, objetivoE1);
			////printf("\nPosicion que devuelve cuando elimina los atrapados de E2: %d \n", posicionEnAtrapados);
			char* pokemonEliminado = list_remove(entrenador2->pokemonsAtrapados,posicionEnAtrapados);
			////printf("\nPokemon eliminado de la lista de objetivos pendientes de E2: %s \n", pokemonEliminado);
			free(pokemonEliminado);


			crearProcesoIntercambio(entrenador1,entrenador2,objetivoE1,objetivoE2, true);
			return 1;
		}else{
			if(objetivoE2){
				t_link_element* aux_pokemon_atrapado_E2 = entrenador2->pokemonsAtrapados->head;
				int posicionEnAtrapados = 0;
				while(aux_pokemon_atrapado_E2){
					int index = buscarPokemon(entrenador2->objetivos,aux_pokemon_atrapado_E2->data);
					if(index == -1){
						objetivoE1 = list_remove(entrenador2->pokemonsAtrapados,posicionEnAtrapados);
						break;
					}
				posicionEnAtrapados++;
				aux_pokemon_atrapado_E2 = aux_pokemon_atrapado_E2->next;
				}

				////puts("\nEntro a objetivo E2 != NULL\n");
				posicionEnAtrapados = buscarPokemon(entrenador1->pokemonsAtrapados,objetivoE2);
				////printf("\nPosicion que devuelve cuando elimina los atrapados de E1: %d \n", posicionEnAtrapados);
				char* pokemonEliminado = list_remove(entrenador1->pokemonsAtrapados,posicionEnAtrapados);
				////printf("\nPokemon eliminado de la lista de objetivos pendientes de E1: %s \n", pokemonEliminado);
				free(pokemonEliminado);

				posicionEnObjetivosPendientes = buscarPokemon(entrenador2->objetivosPendientes,objetivoE2);
				////printf("\nPosicion que devuelve cuando elimina en objetivos pendientes de E2: %d\n",posicionEnObjetivosPendientes);
				list_remove(entrenador2->objetivosPendientes,posicionEnObjetivosPendientes);



				crearProcesoIntercambio(entrenador1,entrenador2,objetivoE1,objetivoE2, true);
				return 1;
			}else{
				////puts("\nAmbos nulos\n");
			}
		}
	}
	return 0;
}

void deteccionDeadlock(){
	entrenadoresBloqueados = list_create();
	entrenadoresBloqueados = list_filter(team->entrenadores, entrenadorBloqueado);

	for(int i=0; i < entrenadoresBloqueados->elements_count; i++){
		Entrenador* entrenador1 = (Entrenador*)list_get(entrenadoresBloqueados, i);
		for(int j=i+1; j < entrenadoresBloqueados->elements_count; j++){
			Entrenador* entrenador2 = (Entrenador*)list_get(entrenadoresBloqueados, j);
			//printf("Deteccion Deadlock - %c - %c\n", entrenador1->idEntrenador, entrenador2->idEntrenador);

			////puts("\n---------deteccionDeadlock ANTES ver lista de objetivos--------\n");
			////printf("entrenador %c\n", entrenador1->idEntrenador);
			//verListaDeAtrapados(entrenador1->objetivos);
			////printf("entrenador %c\n", entrenador2->idEntrenador);
			//verListaDeAtrapados(entrenador2->objetivos);
			if(requiereIntercambio(entrenador1, entrenador2)){
				//puts("\n---------deteccionDeadlock TRUE ver lista de objetivos--------\n");
				//printf("entrenador %c\n", entrenador1->idEntrenador);
				verListaDeAtrapados(entrenador1->objetivos);
				//printf("entrenador %c\n", entrenador2->idEntrenador);
				verListaDeAtrapados(entrenador2->objetivos);

				return;
			}
			//puts("\n---------deteccionDeadlock FALSE ver lista de objetivos--------\n");
			//printf("entrenador %c\n", entrenador1->idEntrenador);
			verListaDeAtrapados(entrenador1->objetivos);
			//printf("entrenador %c\n", entrenador2->idEntrenador);
			verListaDeAtrapados(entrenador2->objetivos);
		}
	}

}
//****************************************************************************************


void* obtenerTeamId(){
	//puts("-------------------------------------------------------------\n");
	int conexionBroker = crear_conexion(ipBroker, puertoBroker);
	team->idTeam = 0;
	while(conexionBroker < 0 && !teamTermino){
		log_error(log_team, "No se pudo reconectar con el broker.");
		sleep(tiempoReconexion);
		log_trace(log_team, "Reintentando conexión...");
		conexionBroker = crear_conexion(ipBroker, puertoBroker);
	}
	if(teamTermino)
		return 0;
	log_trace(log_team,"Conexion con Broker exitosa.");

	FILE *f = fopen(archivoId, "r");
	if(f == NULL) {
		//puts("\t\tNO EXISTE, SE CREA\n");
		f = fopen(archivoId, "w");
		team->idTeam = 0;
	} else {
		//puts("\t\tEXISTE, LO LEE\n");
		fread(&team->idTeam, sizeof(int), 1, f);
	}
	fclose(f);
	//printf("idTeam %d\n", team->idTeam);

	t_paquete *paqueteConexion = crear_paquete(ENLAZAR);
	agregar_a_paquete(paqueteConexion, &team->idTeam, sizeof(int));
	enviar_paquete(paqueteConexion, conexionBroker);
	eliminar_paquete(paqueteConexion);

	int operacion = recibir_operacion(conexionBroker);
	//puts("\t\tOPERACION RECIBIDA\n");
	t_list* lista=NULL;
	switch(operacion)
	{
		case SUSCRIPTOR_ID:
			lista = recibir_paquete(conexionBroker);
			//printf("\n--------NUEVO TEAM ID: %d\n", *(int *) list_get(lista, 0));
			team->idTeam = *(int *) list_get(lista, 0); // Guardarlo en un txt aparte.
			f = fopen(archivoId, "w");
			fwrite(&team->idTeam, sizeof(int), 1, f);
			fclose(f);
			list_destroy_and_destroy_elements(lista,free);
			liberar_cliente(conexionBroker);
			break;

		case REENLAZADO:
			//printf("\n---------TEAM REENLAZADO\n");
			liberar_cliente(conexionBroker);
			break;
		default:
			liberar_cliente(conexionBroker);
//			////printf("DEFAULT %d", operacion);
			break;
	}

	return NULL;
}



bool mismoProceso(void* proceso){
	char entrenador = ((Proceso*)proceso)->entrenador->idEntrenador;
	char* pokemon = ((Proceso*)proceso)->pokemon->nombre;

	return strcmp(pokemon, pokemonEjecutando) == 0 && entrenador == entrenadorEjecutando;
}

bool mismoNombre(void* nombre){
	return strcmp(nombre, auxNombrePokemon) == 0;
}



void mostrarLista() {
//	t_link_element *element = team->procesos->head;
//
//	while (element != NULL) {
//		char entrenadorID =  ((Proceso*)element->data)->entrenador->idEntrenador;
//		float estimacion = ((Proceso*)element->data)->estimadoActual;
//		////printf("ENTRENADOR PROCESO: %c. Estimado: %f\n", entrenadorID, estimacion);
//		element = element->next;
//	}
	////puts("_________\n");

}

void mostrarListaIntercambio() {
	t_link_element *element = team->procesosDeIntercambio->head;

	while (element != NULL) {
		////printf("ENTRENADOR PROCESO: %c - %c. Estimado: %f\n", ((ProcesoIntercambio*)element->data)->entrenador1->idEntrenador,((ProcesoIntercambio*)element->data)->entrenador2->idEntrenador,((ProcesoIntercambio*)element->data)->estimadoActual);
		element = element->next;
	}
	////puts("_________\n");

}

//************************************************ PLANIFICADOR **********************************************

void fifo(){
	while(!teamTermino){
		sem_wait(&sem_proceso);
		//puts("sem_proceso");
		sem_wait(&sem_mx_proceso);
		//puts("sem_mx_proceso");
		//printf("-------cantidad de procesos: %d----\n",team->procesos->elements_count);
		if(team->procesos->elements_count > 0){

			irAAtrapar((Proceso*)team->procesos->head->data);
		}
		else
			return;
	}
}

void rr(){
	while(!teamTermino){
//		sem_wait(&sem_proceso);
//		if(team->procesos->elements_count > 0){
//			irAAtraparRoundRobin((Proceso*)team->procesos->head->data);
//		}else
//			sem_post(&sem_proceso);

		sem_wait(&sem_proceso);
		//puts("&sem_proceso");
		sem_wait(&sem_mx_proceso);
		//puts("&sem_mx_proceso");
		//printf("-------cantidad de procesos: %d----\n",team->procesos->elements_count);
		if(team->procesos->elements_count > 0){

			irAAtraparRoundRobin((Proceso*)team->procesos->head->data);
		}
		else
			return;
	}
}

void sjf_sd(){
	while(!teamTermino){
		sem_wait(&sem_proceso);

		sem_wait(&sem_mx_proceso);
		//printf("-------cantidad de procesos: %d----\n",team->procesos->elements_count);
		if(team->procesos->elements_count > 0){

			irAAtrapar((Proceso*)team->procesos->head->data);
		}
		else
			return;
	}
}

void sjf_cd(){

	while(!teamTermino){
		sem_wait(&sem_proceso);

		sem_wait(&sem_mx_proceso);
		//printf("-------cantidad de procesos: %d----\n",team->procesos->elements_count);
		if(team->procesos->elements_count > 0){

			irAAtrapar((Proceso*)team->procesos->head->data);
		}
		else
			return;
	}
}
void* planificador(){



    if(strcmp(algoritmo,"FIFO") == 0){
        fifo();
    }else if(strcmp(algoritmo,"RR") == 0){
        rr();
    }else if(strcmp(algoritmo,"SJF-CD") == 0){
        sjf_cd();
    } else if(strcmp(algoritmo,"SJF-SD") == 0){
        sjf_sd();
    }

    return NULL;
}

void AgregarProcesoACola(Entrenador* entrenador,Pokemon* pokemon){
//	if(!arrancaPlanificador){
//		arrancaPlanificador = true;
//		pthread_create(&threadPlanificador, NULL, planificador, NULL);
//	}

	Proceso* nuevoProceso = (Proceso*)malloc(sizeof(Proceso));
	nuevoProceso->pokemon = pokemon;
	nuevoProceso->entrenador = entrenador;
	list_add(team->procesos, (void*)nuevoProceso);
	if(strcmp(algoritmo,"SJF-CD") == 0 || strcmp(algoritmo,"SJF-SD") == 0){
		nuevoProceso->estimadoAnterior = estimadoInicial;
		nuevoProceso->rafagaAnterior = 0;
		nuevoProceso->estimadoActual = calcularEstimado(nuevoProceso->estimadoAnterior, nuevoProceso->rafagaAnterior);
		////printf("Entrenador: %c Estimado: %f\n", nuevoProceso->entrenador->idEntrenador, nuevoProceso->estimadoActual);
		list_sort(team->procesos, menorEstimado);
		////puts("agregar proceso a cola");
		mostrarLista();
		interrupcion = strcmp(algoritmo,"SJF-CD") == 0;
	}

	//////////printf("\t\tNUEVO PROCESO AGREGADO A COLA. ENTRANDOR: %d %d, Pokemon: %s\n", entrenador->posicion->x, entrenador->posicion->y, pokemon->nombre);
	entrenador->estado = LISTO;
	log_trace(log_team, "Nuevo proceso: se asigna al entrenador %c atrapar a %s. Estado: LISTO\n", entrenador->idEntrenador, pokemon->nombre);
	sem_post(&sem_proceso);
}


t_link_element* buscarMensajePendienteCatch(t_list * lista, int id) {

    t_link_element *element = lista->head;


    while (element != NULL && ((MensajePendienteCatch*)element->data)->idMensaje != id) {

        element = element->next;
    }

    return element;
}



float Distancia(int x1, int y1, int x2, int y2) {
    return sqrt(pow(x2 - x1,2) + pow(y2 - y1,2));
} //Distancia

Entrenador* entrenadorCercano(int posicionX,int posicionY){
//	////////printf("\nentrenadorCercano X: %d Y: %d\n", posicionX, posicionY);
//	////////printf("count entrenadores: %d\n", team->entrenadores->elements_count);
	t_link_element* entrenador = team->entrenadores->head;
    int distanciaMinima = 10000;
    Entrenador* entrenadorSeleccionado = NULL;
    while(entrenador) {//se fija que entrenador tiene la menor distancia
        //todo: fijarnos que el entrenador este en ready o en bloqueado y no en exit
//        ////////puts("entra al while\n");
//    	////////printf("Estado: %d\n", ((Entrenador*)entrenador->data)->estado);
    	if(((Entrenador*)entrenador->data)->estado == NUEVO || ((Entrenador*)entrenador->data)->estado == BLOQUEADO_REPLANIFICACION){
			if(((Entrenador*)entrenador->data)->objetivos->elements_count > ((Entrenador*)entrenador->data)->pokemonsAtrapados->elements_count) {
				int distanciaActual = Distancia(((Entrenador *) entrenador->data)->posicion->x,
												((Entrenador *) entrenador->data)->posicion->y, posicionX, posicionY);
				if (distanciaActual < distanciaMinima) {
					distanciaMinima = distanciaActual;
					entrenadorSeleccionado = (Entrenador *) entrenador->data;
				}
			}
        }
        entrenador=entrenador->next;
    }
    //////////printf("X: %d \tY: %d", entrenadorSeleccionado->posicion->x, entrenadorSeleccionado->posicion->y);
    return entrenadorSeleccionado;
}
void irAAtrapar(Proceso* proceso){
	Pokemon* pokemon = proceso->pokemon;
	int rafaga = 0;
	Entrenador* entrenadorMasCercano = proceso->entrenador;
	////////printf("POSICION ORIGINAL X: %d Y: %d\n", entrenadorMasCercano->posicion->x, entrenadorMasCercano->posicion->y);
	////////printf("POSICION DESTINO X: %d Y: %d\n", pokemon->posicion->x, pokemon->posicion->y);
	entrenadorMasCercano->estado = EJECUTANDO;
	cantCambiosDeContextoTotales++;
	log_trace(log_team, "Entrenador %c seleccionado para ejecutar. Estado: EJECUTANDO\n", entrenadorMasCercano->idEntrenador);
	log_trace(log_team,"Entrenador %c posición: (%d,%d) va a atrapar %s en la posición (%d,%d)\n", entrenadorMasCercano->idEntrenador, entrenadorMasCercano->posicion->x, entrenadorMasCercano->posicion->y, pokemon->nombre, pokemon->posicion->x, pokemon->posicion->y);
	while (entrenadorMasCercano->posicion->x != pokemon->posicion->x || entrenadorMasCercano->posicion->y != pokemon->posicion->y){
		if(interrupcion){

			cantCambiosDeContextoTotales++;
			entrenadorMasCercano->estado = LISTO;
			log_trace(log_team, "Llego interrupción, se debe replanificar. Entrenador %c pasa a estado listo. Estado: LISTO\n", entrenadorMasCercano->idEntrenador);
			proceso->estimadoAnterior = proceso->estimadoActual;
			proceso->rafagaAnterior = rafaga;
			proceso->estimadoActual = calcularEstimado(proceso->estimadoAnterior, proceso->rafagaAnterior);
			////printf("Entrenador: %c Estimado: %f\n", entrenadorMasCercano->idEntrenador, proceso->estimadoActual);

			list_sort(team->procesos, menorEstimado);
			////puts("inter");
			mostrarLista();
			interrupcion = false;
			sem_post(&sem_proceso);
			sem_post(&sem_mx_proceso);
			return;
		}
		if(entrenadorMasCercano->posicion->x != pokemon->posicion->x){
            if(entrenadorMasCercano->posicion->x > pokemon->posicion->x)
                entrenadorMasCercano->posicion->x--;
            else
                entrenadorMasCercano->posicion->x++;
            rafaga++;
            entrenadorMasCercano->cantCiclosCPUTotales++;
            cantCiclosCPUTotales++;
            sleep(retardoCPU);
        }
        else if(entrenadorMasCercano->posicion->y != pokemon->posicion->y){
            if(entrenadorMasCercano->posicion->y > pokemon->posicion->y)
                entrenadorMasCercano->posicion->y--;
            else
                entrenadorMasCercano->posicion->y++;
            rafaga++;
            entrenadorMasCercano->cantCiclosCPUTotales++;
            cantCiclosCPUTotales++;
            sleep(retardoCPU);
            //chequear interrupcion
            //si hay interrupcion y la diferencia de ambos valores en x y en y != 0-> break
            //si no no haces nada y seguis ejecutando
        }

		log_trace(log_team, "Entrenador %c se mueve a posicion (%d,%d)", entrenadorMasCercano->idEntrenador, entrenadorMasCercano->posicion->x, entrenadorMasCercano->posicion->y);
    }



    catchPokemon(pokemon,entrenadorMasCercano); //y esto descomentarlo

}

void irAAtraparRoundRobin(Proceso* proceso){
	int movimientos = 0;
	Pokemon* pokemon = proceso->pokemon;
	Entrenador* entrenadorMasCercano = proceso->entrenador;
	////////printf("POSICION ORIGINAL X: %d Y: %d\n", entrenadorMasCercano->posicion->x, entrenadorMasCercano->posicion->y);
	////////printf("POSICION DESTINO X: %d Y: %d\n", pokemon->posicion->x, pokemon->posicion->y);
	entrenadorMasCercano->estado = EJECUTANDO;
	cantCambiosDeContextoTotales++;
	log_trace(log_team, "Entrenador %c seleccionado para ejecutar. Estado: EJECUTANDO\n", entrenadorMasCercano->idEntrenador);
	log_trace(log_team,"Entrenador %c posición: (%d,%d) va a atrapar %s en la posición (%d,%d)\n", entrenadorMasCercano->idEntrenador, entrenadorMasCercano->posicion->x, entrenadorMasCercano->posicion->y, pokemon->nombre, pokemon->posicion->x, pokemon->posicion->y);
	while ((entrenadorMasCercano->posicion->x != pokemon->posicion->x || entrenadorMasCercano->posicion->y != pokemon->posicion->y) && movimientos < quantum){
		if(entrenadorMasCercano->posicion->x != pokemon->posicion->x){
            if(entrenadorMasCercano->posicion->x > pokemon->posicion->x)
                entrenadorMasCercano->posicion->x--;
            else
                entrenadorMasCercano->posicion->x++;
            sleep(retardoCPU);
            entrenadorMasCercano->cantCiclosCPUTotales++;
            cantCiclosCPUTotales++;
            movimientos++;
        }
        else if(entrenadorMasCercano->posicion->y != pokemon->posicion->y){
            if(entrenadorMasCercano->posicion->y > pokemon->posicion->y)
                entrenadorMasCercano->posicion->y--;
            else
                entrenadorMasCercano->posicion->y++;
            sleep(retardoCPU);
            entrenadorMasCercano->cantCiclosCPUTotales++;
            cantCiclosCPUTotales++;
            movimientos++;
        }

		log_trace(log_team, "Entrenador %c se mueve a posicion (%d,%d)", entrenadorMasCercano->idEntrenador, entrenadorMasCercano->posicion->x, entrenadorMasCercano->posicion->y);
    }

	if(entrenadorMasCercano->posicion->x == pokemon->posicion->x && entrenadorMasCercano->posicion->y == pokemon->posicion->y){
		catchPokemon(pokemon,entrenadorMasCercano);
	} else{
		entrenadorMasCercano->estado = LISTO;
		log_trace(log_team, "Interrupción por fin de quantum. Entrenador %c pasa a estado listo. Estado: LISTO\n", entrenadorMasCercano->idEntrenador);
		Proceso*  auxiliar = (Proceso*) list_remove(team->procesos, 0);
		list_add(team->procesos, auxiliar);
		////puts("INTERRUPCIÓN POR QUANTUM\n");
		//mostrarLista();
		sem_post(&sem_proceso);
		sem_post(&sem_mx_proceso);
	}

}

//********************************************* MENSAJES DE TEAM *********************************************

int recibirIdMensaje(int conexionLocalBroker){
    //esta funcion se encarga de esperar la respuesta del broker con el id de mensaje catch.

    t_list* lista;
    int respuesta;
    if (recibir_operacion(conexionLocalBroker) == MENSAJE_ID)
    {
        lista=recibir_paquete(conexionLocalBroker);
        respuesta = *(int*) lista->head->data;
//        ////////printf("RESPUESTA:(PAQUETE,%d)\n",respuesta);

    }
    else
    {
        respuesta = 0;
        //////////printf("RESPUESTA DESCONOCIDA.\n");
    }
    return respuesta;
}


void localizedPokemon(t_list* respuestaRecibida){
   // t_link_element *listaRecibida= respuestaRecibida->head;
    t_link_element* auxMensajesGet = mensajesPendientesGet->head;
    t_link_element* aux_pokemonesEnElMapa = pokemonesEnElMapa->head;
    t_list* posicionesRecibidas = list_create();
//    int cont=0;
    int idMensaje = *(int*)list_get(respuestaRecibida,0);
    ////printf("\nID MENSAJE: %d", idMensaje);
    //listaRecibida = listaRecibida->next;
       char* nombrePokemon = (char*)malloc(strlen(list_get(respuestaRecibida, 1))+1);
       memcpy(nombrePokemon,list_get(respuestaRecibida,1),strlen(list_get(respuestaRecibida,1))+1);
    ////printf("\nNOMBRE POKEMON: %s",nombrePokemon);
       //listaRecibida = listaRecibida->next;
          int cantidadDePares =*(int*)list_get(respuestaRecibida,2);
    ////printf("\nCANTIDAD DE PARES: %d",cantidadDePares);
    log_trace(log_team, "Mensaje recibido: LOCALIZED_POKEMON ID Correlativo: %d, Pokemon: %s, Cantidad De Pares:%d",idMensaje, nombrePokemon, cantidadDePares);

    //    while(listaRecibida &&  (cantidadDePares > cont)){
   //        Posicion* posicion = (Posicion*)malloc(sizeof(Posicion));
   //        listaRecibida= listaRecibida->next;
   //        posicion->x = *(int*)listaRecibida->data;
   //        //printf("      X: %d\n", *(int*)listaRecibida->data);
   //        listaRecibida= listaRecibida->next;
   //        posicion->y = *(int*)listaRecibida->data;
   //       //printf("      y: %d\n", *(int*)listaRecibida->data);
   //
   //        list_add(posicionesRecibidas, posicion);
   //
   //        cont++;
   //    }
       int i=0;
   	while(i<cantidadDePares*2)
   	{
   		//printf("\t\tPosicion (X:%d, Y:%d)\n",*(int*) list_get(respuestaRecibida, i+3),*(int*) list_get(respuestaRecibida, i+4));
   		Posicion* posicion = (Posicion*)malloc(sizeof(Posicion));
   		posicion->x = *(int*) list_get(respuestaRecibida, i+3);
   		posicion->y = *(int*) list_get(respuestaRecibida, i+4);
   		pokemonAuxLocalized = (Pokemon*)malloc(sizeof(Pokemon));
   		pokemonAuxLocalized->nombre = nombrePokemon;
   		pokemonAuxLocalized->posicion = posicion;
   		if(!list_find(team->procesos, mismoPokemonMismaPosicion) && !list_find(pokemonsPendientesDeCaptura, mismoPokemonMismaPosicionPendientes))
   			list_add(posicionesRecibidas, posicion);
   		else{
   			//puts("Ignoro pokemon");
   		}
		i+=2;
   	}
    list_destroy_and_destroy_elements(respuestaRecibida,free);
    while(aux_pokemonesEnElMapa){

        if(((PokemonEnMapa*)(aux_pokemonesEnElMapa->data))->nombre == nombrePokemon){

            list_add_all(((PokemonEnMapa*)(aux_pokemonesEnElMapa->data))->posiciones,posicionesRecibidas);
            break;
        }
        aux_pokemonesEnElMapa = aux_pokemonesEnElMapa->next;
    }
    if(aux_pokemonesEnElMapa == NULL){

        PokemonEnMapa* pokemonNuevo = (PokemonEnMapa*)malloc(sizeof(PokemonEnMapa));
        pokemonNuevo->nombre = nombrePokemon;
        pokemonNuevo->posiciones=posicionesRecibidas;
        list_add(pokemonesEnElMapa,pokemonNuevo);
    }


    while(auxMensajesGet){
    	if(((MensajePendienteGet*)auxMensajesGet->data)->idMensaje == idMensaje){

            while((int)((MensajePendienteGet*)auxMensajesGet->data)->cantidadDePokemonesNecesarios > 0){//mientras hayan pokemones necesarios
                if(posicionesRecibidas->elements_count > 0){//y hay posiciones del pokemon disponibles
                    //haz tu magia
                    Posicion* posicion = (Posicion*)list_remove(posicionesRecibidas,0);
                    //TODO: planifica a un entrenador para que le haga el catch y ver como involucrar a los entrenadores
                    Pokemon* pokemon = (Pokemon*)malloc(sizeof(Pokemon));
                    pokemon->nombre = nombrePokemon;
                    pokemon->posicion = posicion;
                    //planificador se necesita atrapar(pokemon)
                    //printf("Busco entrenador más cercano para %s (%d,%d)\n", pokemon->nombre, pokemon->posicion->x,pokemon->posicion->y);
                    Entrenador* entrenadorMasCercano = entrenadorCercano(pokemon->posicion->x,pokemon->posicion->y);
                    if(entrenadorMasCercano){
                    	AgregarProcesoACola(entrenadorMasCercano, pokemon);
                    }else{
                    	list_add(pokemonsPendientesDeCaptura, pokemon);
                    	//////////printf("POKEMON %s AGREGADO A LA LISTA DE PENDIENTES DE CAPTURA\n", pokemon->nombre);
                    }

                }
                if(posicionesRecibidas->elements_count <= 0){// si necesita mas de ese pokemon pero no hay mas disponibles en el mapa se las toma
					return;
                }

//                (*(int*)auxMensajesGet->data)--;

                (int)((MensajePendienteGet*)auxMensajesGet->data)->cantidadDePokemonesNecesarios--;
            }


            bool coincideMensaje(void* idMensaje){
            	int idMensajeCasteado = *((int*)idMensaje);
                return idMensajeCasteado == ((MensajePendienteGet*)auxMensajesGet->data)->idMensaje;
            }

//            if((*(int*)auxMensajesGet->data) <= 0 && posicionesRecibidas->elements_count <= 0 ){
            if((int)((MensajePendienteGet*)auxMensajesGet->data)->cantidadDePokemonesNecesarios == 0) {
                MensajePendienteGet* mensajeAEliminar = (MensajePendienteGet*)list_remove_by_condition(mensajesPendientesGet,coincideMensaje);
                free(mensajeAEliminar);
            }
            return;
        }
        auxMensajesGet = auxMensajesGet->next;
    }
    //Si el auxMensajesGet es nulo, es porque no encontro el id de mensaje porque no lo necesita o porque ya fue recibido. Asique no hace nada.

}

bool mismoNombreGet(void* parametro){
	MensajePendienteGet* mensaje = (MensajePendienteGet*)parametro;
	return strcmp(mensaje->pokemon, nombreGet) == 0;
}

void appearedPokemon(char* nombrePokemon, int posicionX, int posicionY){
	log_trace(log_team, "Mensaje recibido: APPEARED_POKEMON pokemon: %s, Posición: (%d,%d)",nombrePokemon, posicionX, posicionY);
	//////puts("-----APPEARED-----\n");
    t_link_element* aux_pokemonesEnElMapa = pokemonesEnElMapa->head;

    Pokemon* pokemon = (Pokemon*)malloc(sizeof(Pokemon));
    pokemon->nombre = nombrePokemon;
    Posicion* posicion = (Posicion*)malloc(sizeof(Posicion));
    posicion->x = posicionX;
    posicion->y= posicionY;
    pokemon->posicion = posicion;

    while(aux_pokemonesEnElMapa){
        if(((PokemonEnMapa*)(aux_pokemonesEnElMapa->data))->nombre == nombrePokemon){
        	//////printf("Encontro pokemon %s en el mapa\n", nombrePokemon);
            list_add(((PokemonEnMapa*)(aux_pokemonesEnElMapa->data))->posiciones,posicion);
            break;
        }
        aux_pokemonesEnElMapa = aux_pokemonesEnElMapa->next;
    }
    if(aux_pokemonesEnElMapa == NULL){
    	//////printf("No encontro pokemon %s en el mapa. Se crea\n", nombrePokemon);
    	PokemonEnMapa* pokemonNuevo = (PokemonEnMapa*)malloc(sizeof(PokemonEnMapa));
        pokemonNuevo->nombre = nombrePokemon;
        pokemonNuevo->posiciones = list_create();
        list_add(pokemonNuevo->posiciones,posicion);
        list_add(pokemonesEnElMapa,pokemonNuevo);
       ////////puts("creado\n");
    }

    t_link_element* auxEntrenadores = team->entrenadores->head;
	t_list* todosLosPendientes = list_create();

    while(auxEntrenadores != NULL){
		list_add_all(todosLosPendientes, ((Entrenador*)auxEntrenadores->data)->objetivosPendientes);
		auxEntrenadores = auxEntrenadores->next;
	}
    //////printf("-----Cantidad de pendientes %d\n", todosLosPendientes->elements_count);
    auxNombrePokemon = (char *)nombrePokemon;


	if(list_find(todosLosPendientes, mismoNombre)){
		Entrenador* entrenadorMasCercano = entrenadorCercano(pokemon->posicion->x,pokemon->posicion->y);
		nombreGet = pokemon->nombre;
		MensajePendienteGet* mensajeGet = list_find(mensajesPendientesGet, mismoNombreGet);
		if(mensajeGet){
			mensajeGet->cantidadDePokemonesNecesarios--;
			//printf("Elimino 1 petición de %s", mensajeGet->pokemon);
			if(mensajeGet->cantidadDePokemonesNecesarios){
				MensajePendienteGet* mensajeEliminado =list_remove(mensajesPendientesGet, buscarPokemonGet(mensajesPendientesGet, pokemon->nombre));
				//printf("mensaje %d eliminado. Pokemon: %s\n", mensajeEliminado->idMensaje, mensajeEliminado->pokemon);
			}
		}
		if(entrenadorMasCercano){
			AgregarProcesoACola(entrenadorMasCercano, pokemon);
		}else{
			list_add(pokemonsPendientesDeCaptura, pokemon);
			//printf("POKEMON %s AGREGADO A LA LISTA DE PENDIENTES DE CAPTURA\n", pokemon->nombre);
			verListaPendientes(pokemonsPendientesDeCaptura);
		}
	}else{
		//printf("El pokemon %s no es un objetivo del team\n",nombrePokemon);
	}

	//puts("Pokemons pendientes de captura en Appeared: \n");
	verListaPendientes(pokemonsPendientesDeCaptura);
	list_destroy(todosLosPendientes);
}


void getPokemon(void* pokemon,int cantidadDePokemonesNecesarios,t_list* entrenadores){
    int conexionLocalBroker = crear_conexion(ipBroker,puertoBroker);
    if(conexionLocalBroker != -1){

    	t_paquete* paquete = crear_paquete(GET_POKEMON);
		agregar_a_paquete(paquete,pokemon,strlen(pokemon)+1);
		agregar_a_paquete(paquete,&team->idTeam,sizeof(int));
		enviar_paquete(paquete,conexionLocalBroker);
		eliminar_paquete(paquete);

		int idMensaje = recibirIdMensaje(conexionLocalBroker);

		MensajePendienteGet* mensajePendiente = (MensajePendienteGet*)malloc(sizeof(MensajePendienteGet));
		mensajePendiente->idMensaje = idMensaje;
		mensajePendiente->pokemon = pokemon;
		mensajePendiente->cantidadDePokemonesNecesarios = cantidadDePokemonesNecesarios;
		mensajePendiente->entrenadores = entrenadores;

		list_add(mensajesPendientesGet,mensajePendiente);


       liberar_cliente(conexionLocalBroker);
    }else{
        ////////printf("\nNo se pudo establecer conexion con broker\n");
        //Se toma como default en el caso de que no pueda establecer conexion en el broker, que el pokemon no esta en el mapa. Asique no agrego nada.
    	log_error(log_team, "Falló conexión con el broker. Se realizo operación por default. Pokemon %s no esta en el mapa", pokemon);
    }


}
void caughtPokemonDefault(Pokemon* pokemon,Entrenador* entrenador){

	 ////puts("\n---------caughtPokemonDefault ANTES ver lista de objetivos--------\n");
		 ////printf("entrenador %c\n", entrenador->idEntrenador);
	//puts("Objetivos pendientes antes de Caught\n");
	   ////printf("\n---CAUGHT POKEMON---\n");
	log_error(log_team, "Falló conexión con el broker. Se realizo operación por default. Pokemon %s atrapado con exito",pokemon->nombre);

	list_add(entrenador->pokemonsAtrapados, pokemon->nombre); // agrego pokemon a pokemones atrapados del entrenador
	//printf("POKEMON |%s| AGREGADO A LISTA DE ATRAPADOS\n", pokemon->nombre);
	//printf("Pokemons en atrapados de %c:\n", entrenador->idEntrenador);
	verListaDeAtrapados(entrenador->pokemonsAtrapados);

	//lo elimino de la lista de objetivos
	int index = buscarPokemon(entrenador->objetivosPendientes, pokemon->nombre);
	if(index != -1){
		list_remove(entrenador->objetivosPendientes, index);
		 ////printf("POKEMON ELIMINADO DE LISTA DE OBJETIVOS\n");
		//free(aEliminar); comente este free para probar
	}
	index = buscarPokemon(pokemonesEnElMapa, pokemon->nombre);

	if(index != -1){
		char* aEliminar=list_remove(pokemonesEnElMapa, index);
		 ////printf("POKEMON %s ELIMINADO DEL MAPA\n", aEliminar);
		free(aEliminar);
	}

	verListaDeAtrapados(entrenador->objetivosPendientes);
	verListaDeAtrapados(entrenador->pokemonsAtrapados);
	if(entrenador->objetivosPendientes->elements_count){
		if(entrenador->pokemonsAtrapados->elements_count == entrenador->objetivos->elements_count){
			entrenador->estado = BLOQUEADO_DEADLOCK; // habilito al entrenador para poder volver a operar
			log_trace(log_team, "Entrenador %c terminó proceso. No puede atrapar más pokemones y no cumplió sus objetivos. Estado: BLOQUEADO\n", entrenador->idEntrenador);
		}else{
			entrenador->estado = BLOQUEADO_REPLANIFICACION; // habilito al entrenador para poder volver a operar
			log_trace(log_team, "Entrenador %c terminó proceso. Tiene pokemones por atrapar. Estado: BLOQUEADO\n", entrenador->idEntrenador);
		}
	}else{
		entrenador->estado = FINALIZADO;
		log_trace(log_team, "Entrenador %c terminó proceso. Cumplio sus objetivos. Estado: FINALIZADO\n", entrenador->idEntrenador);
	}
	 ////puts("\n----pokemones pendientes de captura-------\n");

	 //printf("\n Pokemones pendientes de captura: %d",pokemonsPendientesDeCaptura->elements_count);
	if(pokemonsPendientesDeCaptura->elements_count){
		int contador = 0;
		while(contador < list_size(pokemonsPendientesDeCaptura)){
			Pokemon* pokemon = (Pokemon*)list_get(pokemonsPendientesDeCaptura,contador);
			Entrenador* entrenadorMasCercano = entrenadorCercano(pokemon->posicion->x,pokemon->posicion->y);
			if(entrenadorMasCercano){
				Pokemon* pokemonRemovido = (Pokemon*)list_remove(pokemonsPendientesDeCaptura, 0);
				//printf("Entrenador %c esta liberado. Se asigna a atrapar a %s\n",entrenadorMasCercano->idEntrenador,pokemonRemovido->nombre);
				AgregarProcesoACola(entrenadorMasCercano, pokemonRemovido);
				//puts("Proceso agregado a la cola\n");
				verListaPendientes(pokemonsPendientesDeCaptura);
			}else{
				 //printf("No hay entrenador");
				 verListaPendientes(pokemonsPendientesDeCaptura);
				break;
			}
			contador++;
		}
	}
	if(list_all_satisfy(team->entrenadores, entrenadorFinalizado)){
		 ////puts("FINALIZADO");
		teamTermino = true;
		sem_post(&sem_proceso);
		pthread_cancel(threadGameboy);
		pthread_detach(threadGameboy);
		//a chequear
		return;
	}

	if(list_all_satisfy(team->entrenadores, EntrenadorFinalizadoOBloqueado)){
		//pthread_create() --> aca creamos el hilo de planificacion para los intercambios
		log_trace(log_team,"Inicia el algoritmo de deteccion de deadlock\n");
		pthread_create(&threadPlanificadorIntercambio, NULL, planificadorIntercambio, NULL);
		deteccionDeadlock();
	}else{
		 //puts("NO ARRANCA");
	}
	////puts("\n---------caughtPokemonDefault DESPUES ver lista de objetivos--------\n");
	 ////printf("entrenador %c\n", entrenador->idEntrenador);

	free(pokemon->posicion);
	free(pokemon);
	//puts("\n OBJETIVOS: \n");
	 verListaDeAtrapados(entrenador->objetivos);
	 //puts("\n OBJETIVOS PENDIENTES: \n");
	 verListaDeAtrapados(entrenador->objetivosPendientes);
	 //puts("\n ATRAPADOS: \n");
	 verListaDeAtrapados(entrenador->pokemonsAtrapados);
}



void caughtPokemon(int idMensaje, int resultadoCaught){
	////////printf("\n---CAUGHT POKEMON---\n");
	log_trace(log_team, "Mensaje recibido: CAUGHT_POKEMON ID MENSAJE: %d, RESULTADO: %d",idMensaje, resultadoCaught);
	t_link_element * elemento = (t_link_element*)buscarMensajePendienteCatch(mensajesPendientesCatch, idMensaje);

	if(elemento){//aca hay que validar si el id del mensaje correspondo al de algun catch de nuestro team
		MensajePendienteCatch * mensajePendiente = (MensajePendienteCatch*)elemento->data;
		if(resultadoCaught != 0){
			//TODO VALIDAR SI CORRESPENDO ATRAPARLO O NO
			list_add(mensajePendiente->entrenador->pokemonsAtrapados, mensajePendiente->pokemon->nombre); //agrego pokemon a pokemones atrapados del entrenador
			////printf("POKEMON |%s| AGREGADO A LISTA DE ATRAPADOS\n", mensajePendiente->pokemon->nombre);

			////puts("\n\nPOKEMON AGREGADO A LA LISTA DE ATRAPADOS: \n\n");
			////printf("Entrenador %c\n", mensajePendiente->entrenador->idEntrenador);
			verListaDeAtrapados(mensajePendiente->entrenador->pokemonsAtrapados);
			//lo elimino de la lista de objetivo
			int index = buscarPokemon(mensajePendiente->entrenador->objetivosPendientes, mensajePendiente->pokemon->nombre);
			////////printf("INDEX %d\n", index);
			if(index != -1){
				list_remove(mensajePendiente->entrenador->objetivosPendientes, index);
				////printf("POKEMON %s ELIMINADO DE LISTA DE OBJETIVOS\n", aEliminar);
				//free(aEliminar);
			}

			index = buscarPokemon(pokemonesEnElMapa, mensajePendiente->pokemon->nombre);

			if(index != -1){
				char* aEliminar=list_remove(pokemonesEnElMapa, index);
				////printf("POKEMON %s ELIMINADO DEL MAPA\n", aEliminar);
				free(aEliminar);
			}

			if(mensajePendiente->entrenador->objetivosPendientes->elements_count){
				if(mensajePendiente->entrenador->pokemonsAtrapados->elements_count == mensajePendiente->entrenador->objetivos->elements_count){
					mensajePendiente->entrenador->estado = BLOQUEADO_DEADLOCK; // habilito al entrenador para poder volver a operar
					log_trace(log_team, "Entrenador %c terminó proceso. No puede atrapar más pokemones y no cumplió sus objetivos. Estado: BLOQUEADO\n", mensajePendiente->entrenador->idEntrenador);
				}else{
					mensajePendiente->entrenador->estado = BLOQUEADO_REPLANIFICACION; // habilito al entrenador para poder volver a operar
					log_trace(log_team, "Entrenador %c terminó proceso. Tiene pokemones por atrapar. Estado: BLOQUEADO\n", mensajePendiente->entrenador->idEntrenador);
				}
			}else{
				mensajePendiente->entrenador->estado = FINALIZADO;
				log_trace(log_team, "Entrenador %c terminó proceso. Cumplio sus objetivos. Estado: FINALIZADO\n", mensajePendiente->entrenador->idEntrenador);
			}

			if(pokemonsPendientesDeCaptura->elements_count){
				t_link_element * auxiliar = pokemonsPendientesDeCaptura->head;

				while(auxiliar){
					Pokemon* pokemon = (Pokemon*)auxiliar->data;
					Entrenador* entrenadorMasCercano = entrenadorCercano(pokemon->posicion->x,pokemon->posicion->y);
					if(entrenadorMasCercano){
						AgregarProcesoACola(entrenadorMasCercano, pokemon);
						list_remove(pokemonsPendientesDeCaptura, 0);

					}else{
						break;
					}
					auxiliar = auxiliar->next;
				}
			}

			if(list_all_satisfy(team->entrenadores, entrenadorFinalizado)){
				////puts("FINALIZADO");
				teamTermino = true;
				sem_post(&sem_proceso);
				pthread_cancel(threadGameboy);
				pthread_detach(threadGameboy);
				return;
			}
			//puts("\n-------------------------------------------------\n");
			//loguearEstadoFinal();
			//puts("\n-------------------------------------------------\n");


			if(list_all_satisfy(team->entrenadores, EntrenadorFinalizadoOBloqueado)){
				//puts("ARRANCA LA DETECCIÓN DE DEADLOCKS");
				pthread_create(&threadPlanificadorIntercambio, NULL, planificadorIntercambio, NULL);
				deteccionDeadlock();
			}else{
				//puts("NO ARRANCA");
			}

		}else{
			mensajePendiente->entrenador->estado = BLOQUEADO_REPLANIFICACION; // habilito al entrenador para poder volver a operar
			log_trace(log_team, "Entrenador %c no pudo atrapar a %s. Pasa a estado bloqueado. Estado: BLOQUEADO\n", mensajePendiente->entrenador->idEntrenador, mensajePendiente->pokemon->nombre);

		}
		////printf("\n\nFIN DE CAUGHT ID MENSAJE: %d, RESULTADO: %d\n\n", idMensaje, resultadoCaught);
		////printf("Entrenador %c\n", mensajePendiente->entrenador->idEntrenador);
		//verListaDeAtrapados(mensajePendiente->entrenador->pokemonsAtrapados);

	}


}

void catchPokemon(Pokemon* pokemon, Entrenador* entrenador){
	int conexionLocalBroker = crear_conexion(ipBroker,puertoBroker);
	////////printf("\nconexión local brocker para catch : %d\n", conexionLocalBroker);
	if(conexionLocalBroker != -1){

		t_paquete* paquete = crear_paquete(CATCH_POKEMON);
		agregar_a_paquete(paquete, pokemon->nombre, strlen(pokemon->nombre)+1);
		agregar_a_paquete(paquete, &(pokemon->posicion->x), sizeof(int));
		agregar_a_paquete(paquete, &(pokemon->posicion->y), sizeof(int));
		enviar_paquete(paquete,conexionLocalBroker);
		////////puts("PAQUETE CATCH ENVIADO\n");
		eliminar_paquete(paquete);

		entrenador->estado = BLOQUEADO_ESPERA_MENSAJE;
		log_trace(log_team, "Entrenador %c se bloquea a la espera de un CaughtPokemon. Estado: BLOQUEADO\n", entrenador->idEntrenador);
		int idMensaje = recibirIdMensaje(conexionLocalBroker);

		entrenadorEjecutando = entrenador->idEntrenador;
		pokemonEjecutando = pokemon->nombre;

		Proceso * procesoAEliminar = (Proceso*)list_remove_by_condition(team->procesos, mismoProceso);
		////printf("PROCESO ELIMINADO DE COLA. ENTRENADOR: %c\n", procesoAEliminar->entrenador->idEntrenador);
		free(procesoAEliminar);
		cantCambiosDeContextoTotales++;

		sem_post(&sem_mx_proceso);

		MensajePendienteCatch* nuevoMensajePendiente = malloc(sizeof(MensajePendienteCatch));
		nuevoMensajePendiente->entrenador = entrenador;
		nuevoMensajePendiente->pokemon = pokemon;
		nuevoMensajePendiente->idMensaje = idMensaje;
		////printf("nuevo mensaje pendiente. Pokemon %s\n", nuevoMensajePendiente->pokemon->nombre);
		list_add(mensajesPendientesCatch, nuevoMensajePendiente);

		liberar_cliente(conexionLocalBroker);

	}else{
		////////printf("\nNo se pudo establecer conexion con broker\n");
		entrenadorEjecutando = entrenador->idEntrenador;
		pokemonEjecutando = pokemon->nombre;

		Proceso * procesoAEliminar = (Proceso*)list_remove_by_condition(team->procesos, mismoProceso);
		////printf("PROCESO ELIMINADO DE COLA. ENTRENADOR: %c\n", procesoAEliminar->entrenador->idEntrenador);
		free(procesoAEliminar);
		cantCambiosDeContextoTotales++;

		sem_post(&sem_mx_proceso);
		caughtPokemonDefault(pokemon, entrenador);


	}

}

//************************************** GESTORES DE MENSAJES RECIBIDOS **************************************

void *gestionarConexionConGameBoy(void* socketServidor){

	t_list* lista;
////puts("\n GESTIONAR GAMEBOY \n");
    while(!teamTermino)
	{
        int cliente = esperar_cliente(*((int*)socketServidor));
		int operacion = recibir_operacion(cliente);
		int posicionX,posicionY;
		char* nombrePokemon = NULL;

		switch(operacion)
		{
            case APPEARED_POKEMON:
                lista = recibir_paquete(cliente);
                nombrePokemon = (char*)malloc(strlen(lista->head->data)+1);
                memcpy(nombrePokemon,lista->head->data,strlen(lista->head->data)+1);
                ////printf("\n      Pokemon: |%s|\n", nombrePokemon);
                posicionX = *(int *) lista->head->next->data;
               ////printf("      Posicion X: |%d|\n", posicionX);
                posicionY = *(int *) lista->head->next->next->data;
                ////printf("      Posicion Y: |%d|\n", posicionY);
                list_destroy_and_destroy_elements(lista,free);
                appearedPokemon(nombrePokemon, posicionX, posicionY);
                break;
			default:
                break;
		}
	}
    return NULL;
}

void *gestionarAppeared(void* socket){

        int cliente=*((int*)socket);

        t_list* lista;

    while(!teamTermino) {
            int operacion = recibir_operacion(cliente);
            int posicionX, posicionY;
        	char* nombrePokemon = NULL;

            switch (operacion) {
                case APPEARED_POKEMON:
                    lista = recibir_paquete(cliente);
                    nombrePokemon = (char*)malloc(strlen(list_get(lista,1))+1);
                    memcpy(nombrePokemon,list_get(lista,1),strlen(list_get(lista,1))+1);
                    ////////printf("\n      Pokemon: %s\n", nombrePokemon);
                    posicionX = *(int *) list_get(lista,2);
                    ////////printf("      Posicion X: %d\n", posicionX); crucen los dedos
                    posicionY = *(int *) list_get(lista,3);
                    ////////printf("      Posicion Y: %d\n", posicionY);
                    appearedPokemon(nombrePokemon, posicionX, posicionY);
                    list_destroy_and_destroy_elements(lista,free);
                    t_paquete *paquete = crear_paquete(RECIBIDO);
                    agregar_a_paquete(paquete, "OK", strlen("OK")+1);
                    enviar_paquete(paquete, cliente);
                    eliminar_paquete(paquete);
                    ////////printf("RECIBIDO\n");
                    break;
                case -1:
                	cliente = suscriboColaAppeared();
                	break;
                default:
                    break;
            }
        }
    return NULL;
}

void *gestionarCaught(void* socket){
        int cliente=*((int*)socket);
        t_list* lista;
        int resultadoCaught;

    while(!teamTermino) {
		int operacion = recibir_operacion(cliente);
		int idMensaje;

		switch (operacion) {
			case CAUGHT_POKEMON:
				lista = recibir_paquete(cliente);
				idMensaje = *(int*)lista->head->data;
				////////printf("      id Mensaje: %d\n", *(int*)lista->head->data);
				resultadoCaught = *(int*)lista->head->next->data;
				////////printf("      Ok/fail: %d\n", *(int*)lista->head->next->data);
				caughtPokemon(idMensaje, resultadoCaught);
				 list_destroy_and_destroy_elements(lista,free);

				t_paquete *paquete = crear_paquete(RECIBIDO);
				agregar_a_paquete(paquete, "OK", strlen("OK")+1);
				enviar_paquete(paquete, cliente);
				eliminar_paquete(paquete);
				////////printf("RECIBIDO CAUGHT\n");
				break;
			 case -1:
				cliente = suscriboColaCaught();
				break;
			default:
				break;
		}
	}
    ////puts("terminó caught");
    return NULL;
 }

void *gestionarLocalized(void *socket){
    int cliente=*((int*)socket);
    ////////printf("\n%d\n", cliente);
    t_list* lista;


    while(!teamTermino) {
        int operacion = recibir_operacion(cliente);

        switch (operacion) {
            case LOCALIZED_POKEMON:
                lista = recibir_paquete(cliente);
                t_paquete *paquete = crear_paquete(RECIBIDO);
                agregar_a_paquete(paquete, "OK", strlen("OK")+1);
                enviar_paquete(paquete, cliente);
                eliminar_paquete(paquete);
                ////////printf("RECIBIDO LOCALIZED\n");
                localizedPokemon(lista);

                break;
            case -1:
				cliente = suscriboColaLocalized();
				break;
            default:
            	////////printf("Default. Cod.: %d", operacion);
                break;
        }
    }
    return NULL;
}


//*********************************** SUSCRIPCIONES A LAS COLAS DE MENSAJES **********************************
int suscriboColaAppeared()
{

    int conexionBrokerAppeared = crear_conexion(ipBroker, puertoBroker);

    while(conexionBrokerAppeared < 0 && !teamTermino){
    	log_error(log_team, "No se pudo reconectar con el broker. Reintentando conexión.");
    	sleep(tiempoReconexion);
    	log_trace(log_team, "Reintentando conexión...");
    	conexionBrokerAppeared = crear_conexion(ipBroker, puertoBroker);
    }
    if(teamTermino)
    		return 0;

    log_trace(log_team,"Conexion con Broker exitosa.");
    t_paquete* paquete = crear_paquete(SUSCRIBIR);
    int cola=(int) APPEARED_POKEMON;
    int id=team->idTeam;
    agregar_a_paquete(paquete,&id, sizeof(int));
    agregar_a_paquete(paquete,&cola, sizeof(int));
    enviar_paquete(paquete, conexionBrokerAppeared);

    eliminar_paquete(paquete);


    return conexionBrokerAppeared;

}

int suscriboColaLocalized()
{

	int conexionBrokerLocalized = crear_conexion(ipBroker, puertoBroker);

	while(conexionBrokerLocalized < 0 && !teamTermino){
		log_error(log_team, "No se pudo reconectar con el broker. Reintentando conexión.");
		sleep(tiempoReconexion);
		log_trace(log_team, "Reintentando conexión...");
		conexionBrokerLocalized = crear_conexion(ipBroker, puertoBroker);
	}

	if(teamTermino)
			return 0;

	log_trace(log_team,"Conexion con Broker exitosa.");
    t_paquete* paquete = crear_paquete(SUSCRIBIR);
    int cola=(int) LOCALIZED_POKEMON;
    int id=team->idTeam;
    agregar_a_paquete(paquete,&id, sizeof(int));
    agregar_a_paquete(paquete,&cola, sizeof(int));
    enviar_paquete(paquete, conexionBrokerLocalized);

    eliminar_paquete(paquete);


    return conexionBrokerLocalized;
}

int suscriboColaCaught()
{

    int conexionBrokerCaught = crear_conexion(ipBroker, puertoBroker);

	while(conexionBrokerCaught < 0 && !teamTermino){
		log_error(log_team, "No se pudo reconectar con el broker. Reintentando conexión.");
		sleep(tiempoReconexion);
		log_trace(log_team, "Reintentando conexión...");
		conexionBrokerCaught = crear_conexion(ipBroker, puertoBroker);
	}

	if(teamTermino)
			return 0;

	log_trace(log_team,"Conexion con Broker exitosa.");
    ////printf("\nCONEXION CAUGHT %d\n", conexionBrokerCaught);
    t_paquete* paquete = crear_paquete(SUSCRIBIR);
    int cola=(int) CAUGHT_POKEMON;
    int id=team->idTeam;
    agregar_a_paquete(paquete,&id, sizeof(int));
    agregar_a_paquete(paquete,&cola, sizeof(int));
    enviar_paquete(paquete, conexionBrokerCaught);

    eliminar_paquete(paquete);


    return conexionBrokerCaught;

}


//********************************************* INICIALIZAR TEAMS ********************************************


void cargarObjetivosPendientes(){
    t_link_element* aux = team->entrenadores->head;
    int i = 0;

    while(aux){
        t_list* listaAtrapadosDuplicada = list_duplicate(((Entrenador*)aux->data)->pokemonsAtrapados);
        t_list* listaObjetivosDuplicada = list_duplicate(((Entrenador*)aux->data)->objetivos);

        //agrego los objetivos pendientes al entrenador
        t_link_element *element = listaObjetivosDuplicada->head;
        while(element){

            int index = buscarPokemon(listaAtrapadosDuplicada, element->data);
            if(index == -1){
                list_add(((Entrenador*)aux->data)->objetivosPendientes, element->data);
            }else{
                list_remove(listaAtrapadosDuplicada, index);

            }
            element = element->next;

            /*t_link_element *element = lista->head;


    while (element != NULL && ((MensajePendienteCatch*)element->data)->idMensaje != id) {
        ////////printf("idMensaje: %d\n",((MensajePendienteCatch*)element->data)->idMensaje);
        element = element->next;
    }
             * */
        }


        aux = aux->next;
        i++;;

        list_destroy(listaAtrapadosDuplicada);
        list_destroy(listaObjetivosDuplicada);


    }

}


void mostrar_elementos(t_list *lista)
{
//    ////////puts("\nMostrando elementos de la lista entrenador\n");
//    int i=0;
//    int count=list_size(lista);
//    ////////printf("\nEl tamaño de la lista es: %d \n", count);
//    Entrenador *unEntrenador;
//    while(count !=0){
//        unEntrenador=(Entrenador*)list_get(lista,i);
//        ////printf ("Posicion %d, x: %d. \n", i, unEntrenador->posicion->x);
//        ////printf ("Posicion Y%d, y: %d. \n", i, unEntrenador->posicion->y);
//        ////printf ("Posicion %d, Estado: %d. \n", i, unEntrenador->estado);
//        count --;
//        i++;
//    }
}

void inicializarEntrenadores() {
    char *posiciones = leer_config("POSICIONES_ENTRENADORES", archivoDeConfiguracion);
    char *pokemonesEntrenadores =  leer_config("POKEMON_ENTRENADORES", archivoDeConfiguracion);
    char *objetivosEntrenadores =  leer_config("OBJETIVOS_ENTRENADORES", archivoDeConfiguracion);

    char** pokemonsSeparadorPorEntrenador = string_get_string_as_array(pokemonesEntrenadores);
    char** posicionesSeparadasPorEntrenador = string_get_string_as_array(posiciones);
    char** objetivosSeparadosPorEntrenador = string_get_string_as_array(objetivosEntrenadores);

    int n = 0;//, i=0;

    bool atrapadosFinalizados = false;
    bool objetivosFinalizados = false;

    while(posicionesSeparadasPorEntrenador[n] != NULL){
        //puts("\nCARGO ENTRENADOR\n");
    	string_trim(&posicionesSeparadasPorEntrenador[n]);

        char **coordenadasEntrenador = string_split(posicionesSeparadasPorEntrenador[n], "|");
        Entrenador *nuevoEntrenador = malloc(sizeof(Entrenador));
        nuevoEntrenador->cantCiclosCPUTotales = 0;
        nuevoEntrenador->objetivosPendientes = list_create();
        nuevoEntrenador->pokemonsAtrapados = list_create();
        nuevoEntrenador->objetivos = list_create();
        nuevoEntrenador->idEntrenador = (char)n+65;
        //agrego las posiciones al entrenador
        nuevoEntrenador->posicion = (Posicion *) malloc(sizeof(Posicion));
        nuevoEntrenador->posicion->x = atoi(coordenadasEntrenador[0]);
        nuevoEntrenador->posicion->y = atoi(coordenadasEntrenador[1]);
        free(coordenadasEntrenador[0]);
        free(coordenadasEntrenador[1]);
  //      //printf("Posicion: %d %d\n", atoi(coordenadasEntrenador[0]), atoi(coordenadasEntrenador[1]));


        //agrego los pokemones atrapados al entrenador
        //puts("if\n");
        ////printf("\npokemonsSeparadorPorEntrenador[%d] %s\n\n", n, pokemonsSeparadorPorEntrenador[n]);
        if(!atrapadosFinalizados && pokemonsSeparadorPorEntrenador[n]){
			//puts("entra\n");
        	string_trim(&pokemonsSeparadorPorEntrenador[n]);
			//printf("\npokemonsSeparadorPorEntrenador[%d] %s\n\n", n, pokemonsSeparadorPorEntrenador[n]);
			char **pokemones = string_split(pokemonsSeparadorPorEntrenador[n], "|");
			for (int j = 0; pokemones[j] != NULL; j++) {
				string_trim(&(pokemones[j]));
				char* pokemon = (char*)malloc(strlen(pokemones[j])+1);
				memcpy(pokemon,pokemones[j],strlen(pokemones[j])+1);
				list_add(nuevoEntrenador->pokemonsAtrapados, pokemon);
				//printf("POKEMON ATRAPADO: %s \n", pokemones[j]);
				free(pokemones[j]);
			}
				free(pokemones);
			//i++;
        }else
        	atrapadosFinalizados = true;




        //agrego los objetivos al entrenador
        if(!objetivosFinalizados && objetivosSeparadosPorEntrenador[n]){
			string_trim(&objetivosSeparadosPorEntrenador[n]);
			char **objetivos = string_split(objetivosSeparadosPorEntrenador[n], "|");

			for (int j = 0; objetivos[j] != NULL; j++) {
				string_trim(&(objetivos[j]));
				char* objetivo = (char*)malloc(strlen(objetivos[j])+1);
				memcpy(objetivo,objetivos[j],strlen(objetivos[j])+1);
				list_add(nuevoEntrenador->objetivos, objetivo);
				//printf("OBJETIVO: %s \n", objetivos[j]);
				free(objetivos[j]);
			}

			free(objetivos);
        }else
        	objetivosFinalizados = true;

        //le asigno el estado default al esentrenador
        nuevoEntrenador->estado = NUEVO;

        list_add(team->entrenadores, nuevoEntrenador);
        //puts("\n\nAGREGO NUEVO ENTRENADOR\n\n");
		//printf("Entrenador %c\n", nuevoEntrenador->idEntrenador);
		verListaDeAtrapados(nuevoEntrenador->pokemonsAtrapados);
        log_trace(log_team, "Creación de entrenador %c. Estado: NUEVO\n", nuevoEntrenador->idEntrenador);

        free(objetivosSeparadosPorEntrenador[n]);
        free(posicionesSeparadasPorEntrenador[n]);
        //free(pokemonsSeparadorPorEntrenador[n]);
        free(coordenadasEntrenador);
        n++;

    }
    free(objetivosSeparadosPorEntrenador);
    free(posicionesSeparadasPorEntrenador);
    free(pokemonsSeparadorPorEntrenador);
    //free(posiciones);
    free(pokemonesEntrenadores);

}


void peticionesDePokemones(){
    t_link_element* auxEntrenadores = team->entrenadores->head;
    t_list* todosLosPendientes = list_create();
    t_list* pokemonesSolicitados = list_create();


    while(auxEntrenadores != NULL){
    	list_add_all(todosLosPendientes, ((Entrenador*)auxEntrenadores->data)->objetivosPendientes);
        auxEntrenadores = auxEntrenadores->next;
    }

    //printf("-----Cantidad de pendientes: %d\n", todosLosPendientes->elements_count);
    while(todosLosPendientes->head != NULL){
    	auxNombrePokemon = (char *)todosLosPendientes->head->data;

    	if(!list_find(pokemonesSolicitados, mismoNombre)){
    		int cantidadDeSolicitudes = list_count_satisfying(todosLosPendientes, mismoNombre);
    		//printf("Solicito %d %s\n", cantidadDeSolicitudes, auxNombrePokemon);

    		getPokemon(auxNombrePokemon, cantidadDeSolicitudes, NULL);

    		list_add(pokemonesSolicitados, auxNombrePokemon);
    	}

    	todosLosPendientes->head = todosLosPendientes->head->next;
    }

    list_destroy_and_destroy_elements(todosLosPendientes,free);
    list_destroy(pokemonesSolicitados);

}

void iniciarTeam(){
	  ////////puts("Iniciar Team");
	team = (Team*)malloc(sizeof(Team));

	team->entrenadores = list_create();
	pokemonesEnElMapa = list_create();
	pokemonsPendientesDeCaptura = list_create();
	inicializarEntrenadores();
	cargarObjetivosPendientes();
	team->procesosDeIntercambio = list_create();
	team->procesos = list_create();

	pthread_create(&threadPlanificador, NULL, planificador, NULL);
	pthread_create(&threadGameboy, NULL, gestionarConexionConGameBoy, (void*)&servidor);

    //mostrar_elementos(team->entrenadores);
	int conexionConBroker = crear_conexion(ipBroker,puertoBroker);
	if(conexionConBroker < 0){
		peticionesDePokemones();
		pthread_create(&threadTeamId, NULL, obtenerTeamId, NULL);
		pthread_join(threadTeamId,NULL);
		pthread_detach(threadTeamId);
		iniciarConexionConBroker();
	}else{
		obtenerTeamId();
		iniciarConexionConBroker();
		peticionesDePokemones();
	}
    //Pregunto por los pokemons objetivo

	////puts("peticionesDePokemones\n");
	//getPokemon("Pikachu", 1, NULL);


    //sleep(5);

}




int main(int cantidad,char* argumentos[]) {

	archivoDeConfiguracion = (char*)malloc(20);
//    if(cantidad == 1){
	////printf("%s", argumentos[1]);
	memcpy(archivoDeConfiguracion, argumentos[1], strlen(argumentos[1]) + 1);
//    }else{

    	//archivoDeConfiguracion = "team2.config";
//    	}
	////printf("archivo de configuración %s", archivoDeConfiguracion);
  //  teamConfig = config_create(archivoDeConfiguracion);


    archivoDeLog = leer_config("LOG_FILE",archivoDeConfiguracion);
    archivoId = leer_config("TEAM_ID_FILE",archivoDeConfiguracion);
    //printf("archivo txt %s\n", archivoId);
    log_team = log_create(archivoDeLog,"team",1,LOG_LEVEL_TRACE);

    char *ip = leer_config("IP",archivoDeConfiguracion);
    char *puerto = leer_config("PUERTO",archivoDeConfiguracion);

    mensajesPendientesCatch = list_create();
    mensajesPendientesGet = list_create();

    servidor = iniciar_servidor(ip, puerto);
    ipBroker = leer_config("IP_BROKER", archivoDeConfiguracion);
    puertoBroker = leer_config("PUERTO_BROKER", archivoDeConfiguracion);
    ////printf("Broker: \nIP: %s\nPuerto: %s", sipBroker, puertoBroker);
    algoritmo = leer_config("ALGORITMO_PLANIFICACION",archivoDeConfiguracion);
    retardoCPU = atoi(leer_config("RETARDO_CICLO_CPU",archivoDeConfiguracion));


    if(strcmp(algoritmo, "SJF-CD") == 0 || strcmp(algoritmo, "SJF-SD") == 0){
    	estimadoInicial = atoi(leer_config("ESTIMACION_INICIAL",archivoDeConfiguracion));
    	char* alphaAux = leer_config("ALPHA",archivoDeConfiguracion);
    	////printf("alphaAux: %s\n",alphaAux);
    	alpha = atof(alphaAux);
    	////printf("alpha: %f\n",alpha);
    }

    if(strcmp(algoritmo, "RR") == 0)
    	quantum = atoi(leer_config("QUANTUM",archivoDeConfiguracion));

    tiempoReconexion = atoi(leer_config( "TIEMPO_RECONEXION",archivoDeConfiguracion));
    teamTermino = false;
    cantCambiosDeContextoTotales = 0;
    cantCiclosCPUTotales = 0;
    cantDeadlocksProducidosyResueltos = 0;




    //inicializo semaforo
	sem_init(&sem_proceso, 0, 0);
	sem_init(&sem_mx_proceso, 0, 1);
	sem_init(&sem_proceso_intercambio, 0, 1);



    iniciarTeam();



//    iniciarConexionConBroker();
//
//
	//pthread_join(threadTeamId,NULL);
	//////puts("JOIN threadTeamId\n");
	pthread_join(threadPlanificador,NULL);
	//puts("JOIN threadPlanificador\n");
	pthread_join(threadPlanificadorIntercambio,NULL);
	//puts("JOIN threadPlanificadorIntercambio\n");

	//pthread_detach(threadTeamId);
	pthread_detach(threadPlanificador);
	pthread_detach(threadPlanificadorIntercambio);
    //liberariamos las variables que sean necesarias
    //liberariamos el servidor
    //broker tenemos que mandarle un mensaje de Desconectado (hacerlo por cada hilo que tenga conexion con broker)
    //tiramos las metricas
    log_trace(log_team, "Metricas: ");
    //todo agregar detalle de los intercambios en las metricas
    log_trace(log_team, "Cantidad de ciclos de CPU totales: %d", cantCiclosCPUTotales);
    log_trace(log_team, "Cantidad de cambios de contexto realizados: %d", cantCambiosDeContextoTotales);
    log_trace(log_team, "Deadlocks producidos y resueltos: %d", cantDeadlocksProducidosyResueltos);
    t_link_element* aux_entrenadores = team->entrenadores->head;
    while(aux_entrenadores){
    	log_trace(log_team, "Cantidad total de ciclos de CPU del entrenador %c: %d", ((Entrenador*)aux_entrenadores->data)->idEntrenador, ((Entrenador*)aux_entrenadores->data)->cantCiclosCPUTotales);
    	aux_entrenadores = aux_entrenadores->next;
    }

    loguearEstadoFinal();

    liberarTeam();
    close(servidor);

}
