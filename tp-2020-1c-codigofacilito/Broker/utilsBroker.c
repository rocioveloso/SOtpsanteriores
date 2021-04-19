/*
 * conexiones.c
 *
 *  Created on: 28 abr. 2020
 *      Author: codigofacilito
 */

#include"utilsBroker.h"


//LOGGER
t_log* iniciar_logger(void)
{
	return log_create("broker.log","TP", 1, LOG_LEVEL_INFO);

}
t_config* leer_config(void)
{
return config_create("broker.config");
}

//FIN LOGGER







int iniciar_servidor(char *ip, char* puerto)
{

	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            printf("Error al bindear");
            continue;
        }
        break;

    }
     if( listen(socket_servidor, SOMAXCONN)<0){
    	 printf("Error al escuchar \n");
    	 return -1;
     }

        freeaddrinfo(servinfo);
    	printf("Escuchando desde el puerto con ID: %d \n",socket_servidor);

    	return socket_servidor;
}

