/*
 * servidor.c
 *
 *  Created on: 28 abr. 2020
 *      Author: codigofacilito
 */

#include "broker.h"

int main(void)
{
	int socketEScucha;
	char* ip;
	char* puerto;
	t_log* logger;
	t_config* config;
	logger = iniciar_logger();





	//"Inicio de Broker"
	log_info(logger,"Inicio Broker");
	//

	//LEO CONFIG
	config = leer_config();
	ip= config_get_string_value(config,"IP");
	puerto= config_get_string_value(config,"PUERTO");
	//



	socketEScucha = iniciar_servidor(config_get_string_value(config,"IP"),config_get_string_value(config,"PUERTO"));


	while(1){




    	//esperar_cliente(socketEScucha);



    }

	terminar_programa(socketEScucha, logger, config);
	return EXIT_SUCCESS;
}
