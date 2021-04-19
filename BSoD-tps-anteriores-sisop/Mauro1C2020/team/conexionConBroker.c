#include "conexionConBroker.h"
#include "team.h"
#include <pthread.h>
#include <utils/utils.h>
#include "liberarTeam.h"

int conexionBrokerAppeared ;
int conexionBrokerLocalized;
int conexionBrokerCaught;
pthread_t threadAppeared, threadLocalized, threadCaught;

void iniciarConexionConBroker(){

	conexionBrokerAppeared = suscriboColaAppeared();
	pthread_create(&threadAppeared, NULL, gestionarAppeared, (void*)&conexionBrokerAppeared);
	conexionBrokerCaught = suscriboColaCaught();
	pthread_create(&threadCaught, NULL, gestionarCaught, (void*)&conexionBrokerCaught);
	conexionBrokerLocalized = suscriboColaLocalized();
	pthread_create(&threadLocalized, NULL, gestionarLocalized, (void*)&conexionBrokerLocalized);

}

void liberarConexionConBroker(){

	pthread_cancel(threadAppeared);
	//printf("appeared cancel %d\n", valor);
	pthread_cancel(threadLocalized);
	//printf("localized cancel %d\n", valor);
	pthread_cancel(threadCaught);
	//printf("caught cancel %d\n", valor);



	pthread_join(threadAppeared,NULL);
	pthread_join(threadLocalized,NULL);
	pthread_join(threadCaught,NULL);

	pthread_detach(threadAppeared);
	pthread_detach(threadLocalized);
	pthread_detach(threadCaught);

	if(team->idTeam){
    liberar_cliente((int)conexionBrokerAppeared);
    liberar_cliente((int)conexionBrokerLocalized);
    liberar_cliente((int)conexionBrokerCaught);
	}
//	void* threadAppearedCancel;
//	void* threadLocalizedCancel;
//	void* threadCaughtCancel;
//
//	valor = pthread_join(threadAppeared, &threadAppearedCancel);
//	//printf("appeared join %d\n", valor);
//	valor = pthread_join(threadLocalized, &threadLocalizedCancel);
//	//printf("localized join %d\n", valor);
//	valor = pthread_join(threadCaught, &threadCaughtCancel);
//	//printf("caught join %d\n", valor);
//
//
//	if(threadAppearedCancel != PTHREAD_CANCELED){
//		//printf("Error en cancelar hilo appeared\n");
//		liberarTeam();
//	}
//	if(threadLocalizedCancel != PTHREAD_CANCELED){
//		//printf("Error en cancelar hilo localized\n");
//		liberarTeam();
//	}
//	if(threadCaughtCancel != PTHREAD_CANCELED){
//		//printf("Error en cancelar hilo caught\n");
//		liberarTeam();
//	}
}

void desenlazarBroker(){
	 int conexionBrokerDesenlace = crear_conexion(ipBroker, puertoBroker);
	 //printf("\nidTeam %d\n", team->idTeam);
	while(conexionBrokerDesenlace < 0 && team->idTeam){
		log_error(log_team, "No se pudo reconectar con el broker. Reintentando conexión.");
		sleep(tiempoReconexion);
		log_trace(log_team, "Reintentando conexión...");
		conexionBrokerDesenlace = crear_conexion(ipBroker, puertoBroker);
	}


	log_trace(log_team,"Conexion con Broker exitosa. Team cumplió sus objetivos globales.");
	t_paquete* paquete = crear_paquete(DESENLAZAR);

	int id=team->idTeam;
	agregar_a_paquete(paquete,&id, sizeof(int));
	enviar_paquete(paquete, conexionBrokerDesenlace);

	eliminar_paquete(paquete);

remove(archivoId);

	//printf("\n\nDesenalazado\n\n");

}
