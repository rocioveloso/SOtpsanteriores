#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>
#include<commons/config.h>
#include <commons/log.h>
#include <commons/temporal.h>
#include <utils/utils.h>
#include <commons/collections/list.h>

bool tiempoTerminado=false;
t_log* log_gameboy;
pthread_t threadSuscripcion;
pthread_t threadTiempo;

void *gestionarTiempo(void* unTiempo)
{
	int tiempo=(int)unTiempo;
    log_info(log_gameboy,"INICIO SUSCRIPCION.");
    sleep(tiempo);
    tiempoTerminado=true;
    log_info(log_gameboy,"FIN SUSCRIPCION.");
    return NULL;
}

void recibirIdMensaje(int conexionBroker){
    //esta funcion se encarga de esperar la respuesta del broker con el id de mensaje

    t_list* lista=NULL;
    int respuesta;
    if (recibir_operacion(conexionBroker) == MENSAJE_ID)
    {
        lista=recibir_paquete(conexionBroker);
        respuesta = *(int*) lista->head->data;
        log_info(log_gameboy,"ID RECIBIDO DE MENSAJE %d.",respuesta);

 list_destroy_and_destroy_elements(lista,free);
    }
    else
    {
        respuesta = 0;
        log_error(log_gameboy,"No se recibió ID");

    }
return;
}
void enviarRecibido(int socket)
{
    t_paquete *paquete;
    paquete = crear_paquete(RECIBIDO);
    agregar_a_paquete(paquete, "OK", strlen("OK")+1);
    enviar_paquete(paquete, socket);
    eliminar_paquete(paquete);
    //printf("RECIBIDO\n");
}

void *gestionarSuscripcion(void* conexion)
{
	int socket=(int)conexion;
    t_list* lista;
    while(!tiempoTerminado)
    {
        int operacion = recibir_operacion(socket);
        ////printf("Operacion recibida:%d\n",operacion);
        switch (operacion)
        {
            case NEW_POKEMON:
                log_trace(log_gameboy,"Nuevo Mensaje: NEW_POKEMON.");
                lista = recibir_paquete(socket);
                printf("\t\tID: %d\n", *(int*) list_get(lista, 0));
                printf("\t\tPokemon: %s\n", (char*)list_get(lista, 1));
                printf("\t\tPosicion X: %d\n", *(int*) list_get(lista, 2));
                printf("\t\tPosicion Y: %d\n", *(int*) list_get(lista, 3));
                printf("\t\tCantidad: %d\n", *(int*) list_get(lista, 4));

                enviarRecibido(socket);
                list_destroy_and_destroy_elements(lista,free);
                break;
            case APPEARED_POKEMON:
            	log_trace(log_gameboy,"Nuevo Mensaje: APPEARED_POKEMON.");
                lista = recibir_paquete(socket);
                printf("\t\tIDCorrelativo: %d\n", *(int*) list_get(lista, 0));
                printf("\t\tPokemon: %s\n", (char*)list_get(lista, 1));
                printf("\t\tPosicion X: %d\n", *(int*) list_get(lista, 2));
                printf("\t\tPosicion Y: %d\n", *(int*) list_get(lista, 3));

                enviarRecibido(socket);
                list_destroy_and_destroy_elements(lista,free);
                break;
            case CATCH_POKEMON:
            	log_trace(log_gameboy,"Nuevo Mensaje: CATCH_POKEMON.");
                lista = recibir_paquete(socket);
                printf("\t\tID Mensaje: %d\n", *(int*) list_get(lista, 0));
                printf("\t\tPokemon: %s\n", (char*)list_get(lista, 1));
                printf("\t\tPosicion X: %d\n", *(int*) list_get(lista, 2));
                printf("\t\tPosicion Y: %d\n", *(int*) list_get(lista, 3));

                enviarRecibido(socket);
                list_destroy_and_destroy_elements(lista,free);
                break;
            case CAUGHT_POKEMON:
            	log_trace(log_gameboy,"Nuevo Mensaje: CAUGHT_POKEMON.");
                lista = recibir_paquete(socket);
                printf("\t\tIDCorrelativo: %d\n", *(int*) list_get(lista, 0));
                printf("\t\t%s ATRAPADO\n", *(int*)list_get(lista, 1)==1?"":"NO");

                enviarRecibido(socket);
                list_destroy_and_destroy_elements(lista,free);
                break;
            case GET_POKEMON:
            	log_trace(log_gameboy,"Nuevo Mensaje: GET_POKEMON.");
                lista = recibir_paquete(socket);
                printf("\t\tID Mensaje: %d\n", *(int*) list_get(lista, 0));
                printf("\t\tPokemon: %s\n", (char*)list_get(lista, 1));

                enviarRecibido(socket);
                list_destroy_and_destroy_elements(lista,free);
                break;
            case LOCALIZED_POKEMON:
            	log_trace(log_gameboy,"Nuevo Mensaje: LOCALIZED_POKEMON.");
                lista = recibir_paquete(socket);
                printf("\t\tID Mensaje: %d\n", *(int*) list_get(lista, 0));
                printf("\t\tPokemon: %s\n", (char*)list_get(lista, 1));
                printf("\t\tCantidad de Pares: %d\n", *(int*) list_get(lista, 2));
                int pares=*(int*) list_get(lista, 2);
                int i=0;
                    while(i<pares*2)
                    {
                        printf("\t\tPosicion (X:%d, Y:%d)\n",
                                *(int*) list_get(lista, i+3),*(int*) list_get(lista, i+4));
                        i+=2;
                    }

                enviarRecibido(socket);
                list_destroy_and_destroy_elements(lista,free);
                break;
            case -1:
                log_warning(log_gameboy,"El BROKER se desconecto.");
                pthread_cancel(threadTiempo);
                tiempoTerminado=true;
                break;
            default:
            	log_error(log_gameboy,"Nuevo Mensaje: DESCONOCIDO.");
            	break;
        }
    }
    return NULL;
}


int main(int cantidad, char* argumentos[])
{
    //Hacia, desde, cuanto
	char* ip;
	char* ip_ = (char*) malloc(20);
	char* puerto;
	char* puerto_ = (char*) malloc(20);

	int servidor;
	int numero;

	log_gameboy = log_create("log_gameboy.log","gameboy",1,LOG_LEVEL_TRACE);

	if(strcmp(argumentos[1],"SUSCRIPTOR")==0)
	{
		memcpy(ip_, "IP_", 3); memcpy(ip_+3, "BROKER", 7);
		memcpy(puerto_, "PUERTO_", 7); memcpy(puerto_+7,"BROKER", 7);

		ip=leer_config(ip_,"gameboy.config");
		puerto=leer_config(puerto_,"gameboy.config");

		servidor = crear_conexion(ip, puerto);

		if(servidor == -1)
		{
			log_error(log_gameboy,"No se puede conectar con BROKER");
			free(ip_);
			free(puerto_);
			return 0;
		}

		numero = atoi(argumentos[3]);
		log_trace(log_gameboy,"SUSCRIPTOR: %d -> %s Tiempo: %u.",SUSCRIPTOR, argumentos[2],(uint32_t) numero);

		int operacion;
		if(strcmp(argumentos[2],"NEW_POKEMON")==0){operacion=NEW_POKEMON;}
		if(strcmp(argumentos[2],"APPEARED_POKEMON")==0){operacion=APPEARED_POKEMON;}
		if(strcmp(argumentos[2],"CATCH_POKEMON")==0){operacion=CATCH_POKEMON;}
		if(strcmp(argumentos[2],"CAUGHT_POKEMON")==0){operacion=CAUGHT_POKEMON;}
		if(strcmp(argumentos[2],"GET_POKEMON")==0){operacion=GET_POKEMON;}
		if(strcmp(argumentos[2],"LOCALIZED_POKEMON")==0){operacion=LOCALIZED_POKEMON;}

		t_paquete* paquete0 = crear_paquete(SUSCRIPTOR);

		numero=1;
		agregar_a_paquete(paquete0, &numero, sizeof(int));
		agregar_a_paquete(paquete0, &operacion, sizeof(int));

		enviar_paquete(paquete0, servidor);
		eliminar_paquete(paquete0);

		pthread_create(&threadSuscripcion, NULL, gestionarSuscripcion, (void *)servidor);

		numero = atoi(argumentos[3]);
		pthread_create(&threadTiempo, NULL, gestionarTiempo, (void *)numero);

		pthread_join(threadTiempo, NULL);
		//pthread_join(threadSuscripcion, NULL);

		return 0;
	}

	memcpy(ip_, "IP_", 3); memcpy(ip_+3, argumentos[1], 9);
	memcpy(puerto_, "PUERTO_", 7); memcpy(puerto_+7, argumentos[1], 9);

	ip=leer_config(ip_,"gameboy.config");
	puerto=leer_config(puerto_,"gameboy.config");
	//Ver el suscriptor

	servidor = crear_conexion(ip, puerto);

	if(servidor == -1)
	{
		log_error(log_gameboy,"No se puede conectar con %s",argumentos[1]);
		free(ip_);
		free(puerto_);
		return 0;
	}

    log_trace(log_gameboy,"Hice una conexion con %s, IP: %s, PUERTO: %s, ID:%d.",argumentos[1], ip, puerto, servidor);

	if(strcmp(argumentos[2], "NEW_POKEMON") == 0){
		log_trace(log_gameboy,"NEW_POKEMON:%d, Pokemon:%s,  X:%u,  Y:%u, Cantidad:%u.",NEW_POKEMON, argumentos[3], (uint32_t) atoi(argumentos[4]),(uint32_t) atoi(argumentos[5]),(uint32_t) atoi(argumentos[6]));

		t_paquete* paquete1 = crear_paquete(NEW_POKEMON);

		//Si es GAMECARD recibe el ID Mensaje al principio
        if(strcmp(argumentos[1],"GAMECARD")==0)
        {
            numero = atoi(argumentos[7]);
            agregar_a_paquete(paquete1, &numero, sizeof(int));
        }
		//POKEMON
		agregar_a_paquete(paquete1, argumentos[3], strlen(argumentos[3])+1);
        //printf("strlen(pokemon)=%d\n",strlen(argumentos[3]));
		//POSICIO X
		numero = atoi(argumentos[4]);
        agregar_a_paquete(paquete1, &numero, sizeof(int));
		//POSICION Y
		numero = atoi(argumentos[5]);
		agregar_a_paquete(paquete1, &numero, sizeof(int));
		//CANTIDAD
		numero = atoi(argumentos[6]);
		agregar_a_paquete(paquete1, &numero, sizeof(int));


		enviar_paquete(paquete1, servidor);
		eliminar_paquete(paquete1);

		if(strcmp(argumentos[1],"BROKER")==0)
		{
			recibirIdMensaje(servidor);
		}
	} else if (strcmp(argumentos[2], "APPEARED_POKEMON") == 0){
		if(strcmp(argumentos[1], "TEAM") == 0)
		log_trace(log_gameboy,"APPEARED_POKEMON:%d, Pokemon:%s, X:%u,  Y:%u.",APPEARED_POKEMON, argumentos[3],(uint32_t) atoi(argumentos[4]),(uint32_t) atoi(argumentos[5]));
		else
			log_trace(log_gameboy,"APPEARED_POKEMON:%d, Pokemon:%s. X:%u. Y:%u. IDC:%u.",APPEARED_POKEMON, argumentos[3],(uint32_t) atoi(argumentos[4]),(uint32_t) atoi(argumentos[5]),(uint32_t) atoi(argumentos[6]));
		t_paquete* paquete2 = crear_paquete(APPEARED_POKEMON);

		if(strcmp(argumentos[1], "TEAM")){
		//ID CORRELATIVO
		numero = atoi(argumentos[6]);
		agregar_a_paquete(paquete2, &numero, sizeof(int));
		}
		//POKEMON
		agregar_a_paquete(paquete2, argumentos[3], strlen(argumentos[3])+1);
		//printf("strlen(pokemon)=%d\n",strlen(argumentos[3]));
		//POSICION X
		numero = atoi(argumentos[4]);
		agregar_a_paquete(paquete2, &numero, sizeof(int));
		//POSICION Y
		numero = atoi(argumentos[5]);
		agregar_a_paquete(paquete2, &numero, sizeof(int));

		enviar_paquete(paquete2, servidor);
		eliminar_paquete(paquete2);
		if(strcmp(argumentos[1],"BROKER")==0)
		{
			recibirIdMensaje(servidor);
		}

	} else if (strcmp(argumentos[2], "CATCH_POKEMON") == 0){
		log_trace(log_gameboy,"CATCH_POKEMON:%d, Pokemon:%s,  X:%u, Y:%u.",CATCH_POKEMON, argumentos[3],(uint32_t) atoi(argumentos[4]),(uint32_t) atoi(argumentos[5]));

		t_paquete* paquete3 = crear_paquete(CATCH_POKEMON);

        //Si es GAMECARD recibe el ID Mensaje al principio
        if(strcmp(argumentos[1],"GAMECARD")==0)
        {
            numero = atoi(argumentos[6]);
            agregar_a_paquete(paquete3, &numero, sizeof(int));
        }
		//POKEMON
		agregar_a_paquete(paquete3, argumentos[3], strlen(argumentos[3])+1);
		//printf("strlen(pokemon)=%d\n",strlen(argumentos[3]));
		//POSICION X
		numero = atoi(argumentos[4]);
		agregar_a_paquete(paquete3, &numero, sizeof(int));
		//POSICION Y
		numero = atoi(argumentos[5]);
		agregar_a_paquete(paquete3, &numero, sizeof(int));

		enviar_paquete(paquete3, servidor);
		eliminar_paquete(paquete3);
		if(strcmp(argumentos[1],"BROKER")==0)
		{
			recibirIdMensaje(servidor);
		}

	} else if (strcmp(argumentos[2], "CAUGHT_POKEMON") == 0){

        log_trace(log_gameboy,"CAUGHT_POKEMON:%d, IDC:%u, Ok/Fail:%s.",CAUGHT_POKEMON,(uint32_t) numero,argumentos[4]);

        t_paquete* paquete4 = crear_paquete(CAUGHT_POKEMON);

        //ID CORRELATIVO
        numero = atoi(argumentos[3]);
		agregar_a_paquete(paquete4, &numero, sizeof(int));
		//OK/FAIL
        agregar_a_paquete(paquete4, argumentos[4], strlen(argumentos[4])+1);
		//printf("strlen(pokemon)=%d\n",strlen(argumentos[4]));

		enviar_paquete(paquete4, servidor);
		eliminar_paquete(paquete4);
		if(strcmp(argumentos[1],"BROKER")==0)
		{
			recibirIdMensaje(servidor);
		}

	} else if (strcmp(argumentos[2], "GET_POKEMON") == 0){
		log_trace(log_gameboy,"GET_POKEMON:%d, Pokemon:%s.",GET_POKEMON, argumentos[3]);

		t_paquete* paquete5 = crear_paquete(GET_POKEMON);

        //Si es GAMECARD recibe el ID Mensaje al principio
        if(strcmp(argumentos[1],"GAMECARD")==0)
        {
            numero = atoi(argumentos[4]);
            agregar_a_paquete(paquete5, &numero, sizeof(int));
        }
		//POKEMON
		agregar_a_paquete(paquete5, argumentos[3], strlen(argumentos[3])+1);
		//printf("strlen(pokemon)=%d\n",strlen(argumentos[3]));

		enviar_paquete(paquete5, servidor);
		eliminar_paquete(paquete5);
		if(strcmp(argumentos[1],"BROKER")==0)
		{
			recibirIdMensaje(servidor);
		}

	}  else if (strcmp(argumentos[2], "LOCALIZED_POKEMON") == 0){
		log_trace(log_gameboy,"LOCALIZED_POKEMON:%d, IDC:%d, Pokemon:%s, Cantidad de pares:%d.",LOCALIZED_POKEMON,atoi(argumentos[3]), argumentos[4],atoi(argumentos[5]));

		t_paquete* paquete7 = crear_paquete(LOCALIZED_POKEMON);

		//ID CORRELATIVO
		numero=atoi(argumentos[3]);
		agregar_a_paquete(paquete7, &numero, sizeof(int));
		//POKEMON
		agregar_a_paquete(paquete7, argumentos[4], strlen(argumentos[4])+1);
		//printf("strlen(pokemon)=%d\n",strlen(argumentos[4]));
		//CANTIDAD DE PARES
		numero = atoi(argumentos[5]);
		agregar_a_paquete(paquete7, &numero, sizeof(int));
		//POSICIONES
		int i=0;
		while (i < atoi(argumentos[5])*2)
		{
			numero = atoi(argumentos[i+6]);
			agregar_a_paquete(paquete7, &numero, sizeof(int));
			log_trace(log_gameboy,"X:%u, Y:%u.", (uint32_t) atoi(argumentos[i+6]), (uint32_t) atoi(argumentos[i+7]));

			i++;

			numero = atoi(argumentos[i+6]);
			agregar_a_paquete(paquete7, &numero, sizeof(int));

			i++;
		}

		enviar_paquete(paquete7, servidor);
		eliminar_paquete(paquete7);
		if(strcmp(argumentos[1],"BROKER")==0)
		{
			recibirIdMensaje(servidor);
		}
	}else if (strcmp(argumentos[2], "SUSCRIPTOR") == 0){
		numero = atoi(argumentos[4]);
		log_trace(log_gameboy,"SUSCRIPTOR: %d -> %s Tiempo: %u.",SUSCRIPTOR, argumentos[3],(uint32_t) numero);

		int operacion;
		if(strcmp(argumentos[3],"NEW_POKEMON")==0){operacion=NEW_POKEMON;}
        if(strcmp(argumentos[3],"APPEARED_POKEMON")==0){operacion=APPEARED_POKEMON;}
        if(strcmp(argumentos[3],"CATCH_POKEMON")==0){operacion=CATCH_POKEMON;}
        if(strcmp(argumentos[3],"CAUGHT_POKEMON")==0){operacion=CAUGHT_POKEMON;}
        if(strcmp(argumentos[3],"GET_POKEMON")==0){operacion=GET_POKEMON;}
        if(strcmp(argumentos[3],"LOCALIZED_POKEMON")==0){operacion=LOCALIZED_POKEMON;}

		t_paquete* paquete6 = crear_paquete(SUSCRIPTOR);

        numero=1;
        agregar_a_paquete(paquete6, &numero, sizeof(int));
		agregar_a_paquete(paquete6, &operacion, sizeof(int));

		enviar_paquete(paquete6, servidor);
		eliminar_paquete(paquete6);

        pthread_create(&threadSuscripcion, NULL, gestionarSuscripcion, (void *)servidor);

		numero = atoi(argumentos[4]);
		pthread_create(&threadTiempo, NULL, gestionarTiempo, (void *)numero);

		pthread_join(threadTiempo, NULL);
		//pthread_join(threadSuscripcion, NULL);
	} else {
		log_error(log_gameboy,"Argumento no valido.\n");
	}


	free(ip_);
	free(puerto_);

	liberar_servidor(servidor);
	return 0;
}
