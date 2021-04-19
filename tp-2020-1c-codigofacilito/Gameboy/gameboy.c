#include "gameboy.h"


int main(int argc, char* argv[]){

		int socketDelTeam;
		int socketDelBroker;
		int socketDelGamecard;
		char* ip;
		char* puerto;

		t_log* logger;
		t_config* config;


		int flag;
		int hayCola;

		switch(op_tipo* argv[1]){
			case 1:

				// en caso de que sea broker
				logger = iniciar_logger();

				config = leer_config();
			    ip = config_get_string_value(config, "IP_BOKER");
			    puerto = config_get_string_value(config, "PUERTO_BROKER");

				socketDelBroker = crear_conexion(ip, puerto);


				//guardar en cola lo que llega dentro del tiempo determinado o cuando llegue un mensaje(sin temporizador)
				if(hayCola==1){
					if(flag==1){
						lock(&mutex_de enviar cola);
						create(thread agregar a la cola());
						create(thread_temporizador(argv[3]));
						create(thread enviar cola());
					}else{
						crear_nodo();
						agregar_a_cola();
					}
				}else{
					crear_cola();
					crear_nodo();
					agregar_a_cola();
				};




				terminar_programa(socketDelBroker, logger, config);

			break;

			case 2:

				// en caso de que sea team
				logger = iniciar_logger();

				config = leer_config();

			    ip = config_get_string_value(config, "IP_TEAM");

			    puerto = config_get_string_value(config, "PUERTO_TEAM");

				socketDelTeam = crear_conexion(ip, puerto);

				enviar_mensaje(, socketDelTeam);

				terminar_programa(socketDelTeam, logger, config);


			break;

			case 3:

				// es un proceso gamecard
				logger = iniciar_logger();

				config = leer_config();
			    ip = config_get_string_value(config, "IP_GAMECARD");
			    puerto = config_get_string_value(config, "PUERTO_GAMECARD");

				socketDelGamecard = crear_conexion(ip, puerto)

				enviar_mensaje(, socketDelGamecard);

				terminar_programa(socketDelGamecard, logger, config);

			break;

			case 4:
				//en caso de que sea modo suscriptor

				// argv[3] esta el tiempo en el que se tiene que cargar la cola
				flag = 1;
				hayCola = 1;



			default;
		}



}



t_log* iniciar_logger(void)
{
	t_log* logger;
	if((logger = log_create("gameboy.log", "Gameboy", 1, LOG_LEVEL_INFO)) == NULL){
		printf("Error creando el logger.\n");
		exit(1);
	}
	return logger;
}



t_config* leer_config(void)
{
	t_config* config;
	if((config = config_create("gameboy.config")) == NULL){
		printf("Error levantando la configuracion del path especificado.\n");
		exit(2);
	}
	return config;
}


void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	if(logger != NULL){
		log_destroy(logger);
	}

	if(config != NULL){
		config_destroy(config);
	}
	liberar_conexion(conexion);
}


// hilo temporizador


void thread_temporizador(int tiempo){
	delay(tiempo);
	pthread_mutex_lock(&mutex_salir_cargar_en_cola);
	flag=0;
	pthread_mutex_unlock(&mutex_salir_cargar_en_cola);

}


//en el broker
void thread_enviar_cola(, int socket_cliente){
	pthread_mutex_lock(&mutex);

	enviar_cola_broker(, int socket_cliente);

}


void thread_cargar_en_cola(){
	pthread_mutex_lock(&mutex_salir_cargar_en_cola);

	while(flag){
		pthread_mutex_unlock(&mutex_salir_cargar_en_cola);
		cargarAColaBroker(, int socket_cliente);
		pthread_mutex_lock(&mutex_salir_cargar_en_cola);
	};
	pthread_mutex_unlock(&mutex_salir_cargar_en_cola);
}











