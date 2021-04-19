#include "broker.h"

int generar_id_cliente()
{
    sem_wait(&mutex_suscriptor_ID);
    contador_suscriptor++;
    sem_post(&mutex_suscriptor_ID);
    return contador_suscriptor;
}

int generar_id_mensaje()
{
    sem_wait(&mutex_mensaje_ID);
    contador_mensaje++;
    sem_post(&mutex_mensaje_ID);
    return contador_mensaje;
}

suscriptor * obtener_suscriptor_cola(int idSuscriptor, cola* unaCola)
{
    sem_wait(&binarioD);
    t_list *suscriptores=unaCola->suscriptores;
    int j = 0;
    while (list_size(suscriptores) > j)
    {
        suscriptor *unSuscriptor = (suscriptor *) list_get(unaCola->suscriptores, j);
        if ((int) unSuscriptor->id == idSuscriptor)
        {
           // //printf("\n\t\tSuscriptor encontrado, id: %d\n", unSuscriptor->id);
            return unSuscriptor;
        }
        j++;
    }
    //printf("SUSCRIPTOR NO ENCONTRADO: obtener_suscriptor_cola.");
    return NULL;
}

void agregar_check_nuevo_suscriptor(cola* unaCola)
{
    sem_wait(&binarioB);
    int cantidadMensajes=list_size(unaCola->mensajes);
    int i=0;
    while(cantidadMensajes>i)
    {
        mensaje *unMensaje=(mensaje*)list_get(unaCola->mensajes,i);
        check_ *nuevoCheck=(check_*)malloc(sizeof(check_));
        nuevoCheck->recibido=false;
        list_add(unMensaje->check,(void*)nuevoCheck);
        i++;
    }
    //printf ("\n\t\tSe agrego un check nuevo a cada uno de los %d mensajes. \n",i);
}

void agregar_suscriptor(int idSuscriptor, int socket, cola* unaCola)
{
    sem_wait(&binarioA);
    sem_post(&binarioD);
    suscriptor *suscriptor1=obtener_suscriptor_cola(idSuscriptor, unaCola);
    if(suscriptor1==NULL)
    {
        suscriptor *unSuscriptor=malloc(sizeof(suscriptor));
        unSuscriptor->id=idSuscriptor;
        unSuscriptor->socket=socket;
        unSuscriptor->conectado=true;
        list_add(unaCola->suscriptores, (void*)unSuscriptor);

        log_info(logB,"Suscriptor id: %d añadido a la lista de suscriptores", idSuscriptor);

        t_list *mensajes=unaCola->mensajes;
        int cantidadMensajes=list_size(mensajes);
        ////printf("\n\t\t La cantidad de mensajes de la cola es: %d \n", cantidadMensajes);
        if(cantidadMensajes>0)
        {
            sem_post(&binarioB);
            agregar_check_nuevo_suscriptor(unaCola);
        }
    }
    else
    {
        sem_post(&binarioC);
        //log_info(logB,"El suscriptor ya existe en la cola. Sera actualizado...");
        actualizar_socket_suscriptor(idSuscriptor, socket, unaCola);
    }
}

void actualizar_socket_suscriptor(int idSuscriptor, int socket, cola* unaCola)
{
    sem_wait(&binarioC);
   // //printf("\n\t\tActualizando los sockets del suscriptor reenlazado.\n");
    sem_post(&binarioD);
        suscriptor *unSuscriptor=obtener_suscriptor_cola(idSuscriptor, unaCola);
        if(unSuscriptor)
        {
            unSuscriptor->socket= socket;
            unSuscriptor->conectado=true;
            log_info(logB,"Suscriptor %d actualizado", unSuscriptor->id);
        }
        else
        {
        	log_error(logB,"SUSCRIPTOR (id:%d) NO ENCONTRADO: actualizar_socket_suscriptor.",idSuscriptor);
        }
}

void depurar_suscriptor(int idSuscriptor)
{
    int i= 0;
    sem_t *semaforo;
    while(6>i)
    {
        cola * unaCola = (cola*)list_get(colas,i);
        semaforo=(sem_t*)list_get(semaforos, i);
        sem_wait(semaforo);

        t_list *suscriptos=unaCola->suscriptores;
        int j=0;
        while(list_size(suscriptos)>j)
        {
            suscriptor *unSuscriptor=(suscriptor*)list_get(unaCola->suscriptores,j);

            if((int)unSuscriptor->id== idSuscriptor)
            {
                suscriptor* aEliminar=(suscriptor*)list_remove(suscriptos, j);
                log_info(logB,"Suscripcion %d eliminado", aEliminar->id);
                free(aEliminar);

                int m=0;

                while(list_size(unaCola->mensajes)>m)
                {
                    mensaje* unMensaje= (mensaje*)list_get(unaCola->mensajes, m);
                    check_* unCheck=(check_*)list_remove(unMensaje->check,j);
                    free(unCheck);
                    m++;
                }
                log_info(logB,"Junto con sus %d checks\n",m);
                break;
            }
            j++;
        }
        i++;
        sem_post(semaforo);
    }
}

void * obtener_particion_disponible_PD(int tamanio)
{
    sem_wait(&binarioG);
//    puts("Obteniendo particion disponible\n");
    int j,i;
    particion_ *restante=NULL;

    //REDONDEAMOS CON TAMAÑO MINIMO
//	int tamanio;

//	 if ( tamanioMinPar>pedido ) { tamanio=tamanioMinPar; }
//	 else { tamanio=pedido;}

	if(strcmp(algoritmoPL,"BF")==0)
	{
        particion_ *laMasChica=NULL;
        j = 0;
        i=0;
        sem_wait(&mutex_particiones);
        while (list_size(dinamicas) > j)
        {
            particion_ *unaDinamica = (particion_ *) list_get(dinamicas, j);

            if (unaDinamica->libre && unaDinamica->tamanio >= tamanio)
            {
                if(laMasChica==NULL || unaDinamica->tamanio<laMasChica->tamanio )
                {
                    laMasChica=unaDinamica;
                    i=j;
                }
            }
            j++;
        }

        if(laMasChica==NULL)
        {
            //printf("\tPD|BF: NO SE ENCONTRO PARTICION DISPONIBLE\n");
            sem_post(&mutex_particiones);
            return NULL;
        }
        else
        {
            if(laMasChica->tamanio>tamanio /*&& laMasChica->tamanio-tamanio>=tamanioMinPar*/)
            {
                restante = (particion_ *) malloc(sizeof(particion_));
                restante->libre = true;
                restante->tamanio = laMasChica->tamanio - tamanio;//3 -2 = 1
                restante->inicio = laMasChica->inicio + tamanio;//0 + 2 = 2
                restante->timestamp=NULL;
                restante->actualizado=NULL;
                // 3 -> 2 ocupada + 1 libre NO ACTUALIZAMOS TIMES DE LIBRES

                list_add_in_index(dinamicas, i+1, (void*)restante);

                //printf("\tNUEVA PARTICION P:%d, I:%d, T:%d, F: %d\n",i,restante->inicio-cache, restante->tamanio,restante->inicio+ restante->tamanio-cache );
            }

                laMasChica->libre = false;
                laMasChica->tamanio = tamanio;
                laMasChica->timestamp = temporal_get_string_time();
                laMasChica->actualizado = temporal_get_string_time();
            	//log_info(logB,"\tLRU inicial: %s ...", laMasChica->actualizado);

            //printf("\tPD|BF Tamaño: %d, Inicio:%d\n",laMasChica->tamanio,laMasChica->inicio-cache);

            sem_post(&mutex_particiones);
            intentos=0;
            //dumpCache();
            return laMasChica->inicio;
        }
	}
	else
	{
		if(strcmp(algoritmoPL,"FF")==0)
		{
            j = 0, i=0;
            particion_ * primera=NULL;

            sem_wait(&mutex_particiones);

            while (list_size(dinamicas) > j)
            {
                particion_ *unaDinamica = (particion_ *) list_get(dinamicas, j);

                if (unaDinamica->libre && unaDinamica->tamanio >= tamanio)
                {
                   primera=unaDinamica;
                   i=j;
                   j=list_size(dinamicas);
                }
                j++;
            }

            if(primera==NULL)
            {
                //printf("\tPD|FF: NO SE ENCONTRO PARTICION DISPONIBLE\n");
                sem_post(&mutex_particiones);
                return NULL;
            }
            else
            {
                if(primera->tamanio>tamanio /*&& primera->tamanio-tamanio>tamanioMinPar*/)
                {
                    restante = (particion_ *) malloc(sizeof(particion_));
                    restante->libre = true;
                    restante->tamanio = primera->tamanio - tamanio;
                    restante->inicio = primera->inicio + tamanio;
                    restante->timestamp=NULL;
                    restante->actualizado=NULL;

                    list_add_in_index(dinamicas, i+1, (void*)restante);

                    //printf("\tNUEVA PARTICION P:%d, I:%d, T:%d, F: %d\n",i,restante->inicio-cache, restante->tamanio,restante->inicio+ restante->tamanio-cache );
                }

                    primera->libre = false;
                    primera->tamanio = tamanio;
                    primera->timestamp = temporal_get_string_time();
                    primera->actualizado = temporal_get_string_time();
                	//log_info(logB,"\tLRU inicial: %s ...", primera->actualizado);

				//printf("PD|FF Tamaño: %d, Inicio:%d\n",primera->tamanio,primera->inicio-cache);

                sem_post(&mutex_particiones);
                intentos=0;
                //dumpCache();
				return primera->inicio;
            }
        }
		else
		{
			log_error(logB,"ALGORITMO NO ENCONTRADO: obtener_particion_disponible_PD.");
			return NULL;
		}
	}
}

void *obtener_particion_disponible_BS(int pedido)
{
    //printf("OBTENER_PARTICION_DISPONIBLE_BS\n");

    sem_wait(&mutex_particiones);

    //dumpCache();

    //int pedido=redondear(tamanio);
    int i=0;

    // RECORREMOS TODA LA LISTA EN BUSCA DE UNA EXISTENTE IGUAL AL REDONDEO
    while (list_size(dinamicas)>i)
    {
        particion_* actual= (particion_*) list_get(dinamicas, i);

        if (actual->libre && actual->tamanio==pedido)
        {
            actual->libre=false;
            actual->timestamp = temporal_get_string_time();
            actual->actualizado = temporal_get_string_time();
        	//log_info(logB,"\tLRU inicial: %s ...", actual->actualizado);
            //printf("Encontramos una existente\n");
            //dumpCache();
            sem_post(&mutex_particiones);

            return actual->inicio;
        }
        i++;
    }

    //printf("No encontramos una existente\n");

    // RECORREMOS LA LISTA EN BUSCA DE UNA MAS GRANDE AL REDONDEO PARA DIVIDIRLA

    i=0;

    particion_* laPrimera=NULL;
    int posPrimera;

    while (list_size(dinamicas)>i)
    {
        particion_* actual= (particion_*) list_get(dinamicas, i);

        if (actual->libre && actual->tamanio > pedido)
        {
            if(laPrimera==NULL || laPrimera->tamanio > actual->tamanio)
            {
                laPrimera=actual;
                posPrimera=i;
            }
        }
        i++;
    }


    //SI NO SE ENCUENTRA UNA MAS GRANDE PARA DIVIDIR NO HAY MAS NADA QUE HACER.

    if(laPrimera==NULL)
    {
        //printf("No encontramos una para dividir\n");
        sem_post(&mutex_particiones);
        return NULL;
    }

    //printf("Encontramos una para dividir\n");

    // AHORA DIVIDIMOS LA MAS CHICA ENCONTRADA

    bool seguimos=true;
    while(seguimos)
    {
        laPrimera->tamanio=laPrimera->tamanio/2;

        particion_*nueva=(particion_*)malloc(sizeof(particion_));
        nueva->actualizado=NULL;
        nueva->timestamp=NULL;
        nueva->tamanio=laPrimera->tamanio;
        nueva->inicio=laPrimera->inicio+laPrimera->tamanio;
        nueva->libre=true;

        list_add_in_index(dinamicas, posPrimera+1, (void*)nueva);

        if(laPrimera->tamanio==pedido)
        {
            laPrimera->libre=false;
            laPrimera->timestamp = temporal_get_string_time();
            laPrimera->actualizado = temporal_get_string_time();
        	//log_info(logB,"\tLRU inicial: %s ...", laPrimera->actualizado);
            //printf("Dividimos una\n");
            //dumpCache();
            sem_post(&mutex_particiones);
            return laPrimera->inicio;
        }
    }
    log_error(logB,"obtener_particion_disponible_BS.");
    return NULL;
}

int obtener_particion_utilizada(void *data)
{
	int j=0 , tamanio=list_size(dinamicas);
	while(tamanio>j)
	{
		particion_ * unaDinamica=(particion_*)list_get(dinamicas,j);
		if(unaDinamica->inicio==data)
		{
			return j;
		}
		j++;
	}
	log_error(logB,"NO SE ENCUENTA LA PARTICION: obtener_particion_utilizada.");
	return -1;
}

void elegir_victima()
{
	 //sem_wait(&binarioJ);
    sem_wait(&binarioI);
    //sem_wait(&mutex_victima);

	    sem_wait(&mutex_particiones);
    int victimaP=-1;
	int i=0;
	int tamanioDinamicas=list_size(dinamicas);
	particion_ * victima=NULL;
	if(strcmp(algoritmoR,"FIFO")==0)
	{
//                    char*tiempo1=temporal_get_string_time();
//                    char*tiempo2=temporal_get_string_time();
//                    //printf("TS1:%s\n",tiempo1);
//                    //printf("TS2:%s\n",tiempo2);
//                    int comparacion=strcmp(tiempo1, tiempo2);
//                    //printf("RESULTADO: %d\n", comparacion);
		while(tamanioDinamicas>i)
		{
            particion_* actual=(particion_*)list_get(dinamicas,i);

            // strcmp-> retorna > 0 si el primer elemento es mayor

			if ((actual->libre==false && victima==NULL) || (actual->libre==false && strcmp(victima->timestamp,actual->timestamp)>0))
			{
				victima=actual;
				victimaP=i;
			}
			i++;
		}

		//printf("VICTIMA|FIFO Inicio:%d\n",victima->inicio-cache);
	}
	else
	{
		if(strcmp(algoritmoR,"LRU")==0)
		{
			while(tamanioDinamicas>i)
			{
                particion_* actual=(particion_*)list_get(dinamicas,i);

                // strcmp-> retorna > 0 si el primer elemento es mayor


                if((actual->libre==false && victima==NULL) || (actual->libre==false && strcmp(victima->actualizado,actual->actualizado)>0))
                {
                    victima=actual;
					victimaP=i;
				}
				i++;
			}

			//printf("VICTIMA|LRU Inicio:%d\n",victima->inicio-cache);
		}
		else
		{
			//printf("WEY, REVISA EL ALGORITMO DE REEMPLAZO. NO EXISTE\n");
            sem_post(&mutex_particiones);
            return;
		}
	}

	int contcola=0;
	int contmensaje=0;
	int victimaC=-1;
	int victimaM=-1;
	cola * unaCola=NULL;
	sem_t *semaforo;
	//printf("el inicio de la victima es: %d \n", victima->inicio-cache);
	while(6>contcola)
	{
		unaCola= (cola*)list_get(colas,contcola);
		semaforo=(sem_t*)list_get(semaforos,contcola);
		sem_wait(semaforo);

		contmensaje=0;

		while(list_size(unaCola->mensajes)>contmensaje)
		{
			mensaje* unMensaje= (mensaje*)list_get(unaCola->mensajes,contmensaje);
			/*
			int posicionD = obtener_particion_utilizada(unMensaje->data);
			    sem_wait(&mutex_particiones);
            particion_* laDinamica=(particion_*)list_get(dinamicas,posicionD);
			int tam=(laDinamica)->tamanio;
			    sem_post(&mutex_particiones);
			//printf("posicionD: %d, inicioD: %d,  posicioM:%d, inicioM: %d, TamanioD %d, Fin: %d\n",
			    posicionD,laDinamica->inicio-cache,contmensaje, unMensaje->data-cache,tam,unMensaje->data-cache+tam);
            dumpCache();
            */
			if(unMensaje->data==victima->inicio)
			{
				victimaC=contcola;
				victimaM=contmensaje;
                //printf ("Encontramos el mensaje victima: %d en la cola: %d \n",victimaM, contcola);
                contmensaje=list_size(unaCola->mensajes);
                contcola=6;

			}
			contmensaje++;
		}
		sem_post(semaforo);
		contcola++;
	}

    if(victimaM!=-1)
    {
        //mensaje * aMostrar=(mensaje*)list_get(victimaC->mensajes,victimaM);
        cola* victimaCola=(cola*)list_get(colas,victimaC);
        sem_t *victimaS=(sem_t*)list_get(semaforos,victimaC);

        sem_wait(victimaS);
            mensaje * aEliminar=(mensaje*)list_remove(victimaCola->mensajes,victimaM);
        sem_post(victimaS);

            list_destroy_and_destroy_elements(aEliminar->check,free);
            aEliminar->data=NULL;
            free(aEliminar);

                victima->libre=true;
                log_trace(logB,"\tPARTICION ELIMINADA, Comienzo: %d",victima->inicio-cache);
                free(victima->actualizado); victima->actualizado=NULL;
                free(victima->timestamp); victima->timestamp=NULL;
                //printf("Se libero la victima inicio: %d \n", victima->inicio- cache);

            //dumpCache();
            log_trace(logB,"\tCONSOLIDAR");
            consolidar(true);
    }
    else
    {
        log_error(logB,"NO HAY MENSAJE VICTIMA: elegir_victima.");
    }
    sem_post(&mutex_particiones);
    //sem_post(&mutex_victima);
}

void obtener_datos_mensaje(particion_* unaDinamica, int i)
{
		cola * unaCola=NULL;
		void* inicio=unaDinamica->inicio;
		int contmensaje=0;
		int contcola=0;

		while(6>contcola)
		{
			unaCola= (cola*)list_get(colas,contcola);


			contmensaje=0;

			while(list_size(unaCola->mensajes)>contmensaje)
			{
				mensaje* unMensaje= (mensaje*)list_get(unaCola->mensajes,contmensaje);

				if(unMensaje->data==inicio)
				{
					switch(contcola)
					{
						case 0:
							log_debug(logDUMP,"Particion %d: %d-%d. (%s) Size:%dB LRU:%s Cola:NEW Id: %d",i,unaDinamica->inicio-cache,
								unaDinamica->inicio-cache+unaDinamica->tamanio-1,unaDinamica->libre?"L":"X",unaDinamica->tamanio,
									unaDinamica->libre?"-":unaDinamica->actualizado, unMensaje->id);
							return;
							break;
						case 1:
							log_debug(logDUMP,"Particion %d: %d-%d. (%s) Size:%dB LRU:%s Cola:APPEARED Id: %d",i,unaDinamica->inicio-cache,
								unaDinamica->inicio-cache+unaDinamica->tamanio-1,unaDinamica->libre?"L":"X",unaDinamica->tamanio,
									unaDinamica->libre?"-":unaDinamica->actualizado, unMensaje->id);
							return;
							break;
						case 2:
							log_debug(logDUMP,"Particion %d: %d-%d. (%s) Size:%dB LRU:%s Cola:CATCH Id: %d",i,unaDinamica->inicio-cache,
								unaDinamica->inicio-cache+unaDinamica->tamanio-1,unaDinamica->libre?"L":"X",unaDinamica->tamanio,
									unaDinamica->libre?"-":unaDinamica->actualizado, unMensaje->id);
							return;
							break;
						case 3:
							log_debug(logDUMP,"Particion %d: %d-%d. (%s) Size:%dB LRU:%s Cola:CAUGHT Id: %d",i,unaDinamica->inicio-cache,
								unaDinamica->inicio-cache+unaDinamica->tamanio-1,unaDinamica->libre?"L":"X",unaDinamica->tamanio,
									unaDinamica->libre?"-":unaDinamica->actualizado, unMensaje->id);
							return;
							break;
						case 4:
							log_debug(logDUMP,"Particion %d: %d-%d. (%s) Size:%dB LRU:%s Cola:GET Id: %d",i,unaDinamica->inicio-cache,
								unaDinamica->inicio-cache+unaDinamica->tamanio-1,unaDinamica->libre?"L":"X",unaDinamica->tamanio,
									unaDinamica->libre?"-":unaDinamica->actualizado, unMensaje->id);
							return;
							break;
						case 5:
							log_debug(logDUMP,"Particion %d: %d-%d. (%s) Size:%dB LRU:%s Cola:LOCALIZED Id: %d",i,unaDinamica->inicio-cache,
								unaDinamica->inicio-cache+unaDinamica->tamanio-1,unaDinamica->libre?"L":"X",unaDinamica->tamanio,
									unaDinamica->libre?"-":unaDinamica->actualizado, unMensaje->id);
							return;
							break;
					}
				}
				contmensaje++;
			}
			contcola++;
		}
							log_debug(logDUMP,"Particion %d: %d-%d. (%s) Size:%d B LRU:%s Cola:- Id: - ",i,unaDinamica->inicio-cache,
							unaDinamica->inicio-cache+unaDinamica->tamanio-1,unaDinamica->libre?"L":"X",unaDinamica->tamanio,
							unaDinamica->libre?"-":unaDinamica->actualizado);
}

void dumpCache()
{
    int tamanio=list_size(dinamicas);
    char* ahora=temporal_get_string_time();
    log_debug(logDUMP,"(%d)DUMP:%s \n",tamanio, ahora);
    free(ahora);

    int i=0;

//Cola: %s,obtener_nombre_cola(unaDinamica->inicio)

    while(tamanio>i)
    {
        particion_ *unaDinamica=(particion_*)list_get(dinamicas,i);
        obtener_datos_mensaje(unaDinamica, i);
        i++;
    }
}

mensaje* obtener_mensaje_utilizado(particion_ * unaParticion)
{
	int contcola=0;
	int contmensaje=0;
	cola* unaCola=NULL;
	sem_t * semaforo;

	while(6>contcola)
		{
			unaCola= (cola*)list_get(colas,contcola);
			semaforo=(sem_t*)list_get(semaforos,contcola);
			sem_wait(semaforo);

			contmensaje=0;

			while(list_size(unaCola->mensajes)>contmensaje)
			{
				mensaje* unMensaje= (mensaje*)list_get(unaCola->mensajes,contmensaje);
				/*
				int posicionD = obtener_particion_utilizada(unMensaje->data);
				    sem_wait(&mutex_particiones);
	            particion_* laDinamica=(particion_*)list_get(dinamicas,posicionD);
				int tam=(laDinamica)->tamanio;
				    sem_post(&mutex_particiones);
				//printf("posicionD: %d, inicioD: %d,  posicioM:%d, inicioM: %d, TamanioD %d, Fin: %d\n",
				    posicionD,laDinamica->inicio-cache,contmensaje, unMensaje->data-cache,tam,unMensaje->data-cache+tam);
	            dumpCache();
	            */
			if(unMensaje->data == unaParticion->inicio)
			{
				sem_post(semaforo);
				return unMensaje;
			}
				contmensaje++;
			}
			sem_post(semaforo);
			contcola++;
		}
	log_error(logB,"NO SE ENCONTRO EL MENSAJE BUSCADO: obtener_mensaje_utilizado.");
	return NULL;
}

void compactar()
{
    sem_wait(&binarioH);
    //log_trace(logB,"\tCOMPACTAR");
    //Se compacta distino segun el algoritmo de memoria
    //compactar se hace por la cantidad de intentos fallidos de encontrar una particion libre
    //la frecuencia define la cantidad aceptable de fallos, pasado esto se compacta
    sem_wait(&mutex_particiones);
    int bytesLibres=0;
    int cont=0;

       while(list_size(dinamicas)>cont)
		{
    		particion_ * actual=	(particion_*)list_get(dinamicas,cont);
    		if(actual->libre)
    		{
    			bytesLibres+=actual->tamanio;
    			particion_ * aEliminar=	(particion_*)list_remove(dinamicas,cont);
    			//log_trace(logB,"\tPARTICION ELIMINADA, Comienzo: %d",aEliminar->inicio-cache);
    			free(aEliminar);
    		}
    		else
    		{
                cont++;
    		}
		}
		particion_ * nuevo=	(particion_*)malloc(sizeof(particion_));
		nuevo->timestamp=NULL;
		nuevo->actualizado=NULL;
		nuevo->libre=true;
		nuevo->tamanio=bytesLibres;
		nuevo->inicio=cache+ (tamanioM-bytesLibres);
		list_add(dinamicas,nuevo);
		//dumpCache();
		cont=0;
		void* ultimoFinal=cache;
		while(list_size(dinamicas)>cont)
		{

			particion_* unaParticion= (particion_*)list_get(dinamicas,cont);
			if(!unaParticion->libre)
			{
				mensaje * unMensaje= obtener_mensaje_utilizado(unaParticion);

				void* dataAux= malloc(unaParticion->tamanio);
				memcpy(dataAux,unaParticion->inicio,unaParticion->tamanio);


				unaParticion->inicio=ultimoFinal;
				ultimoFinal+=unaParticion->tamanio;
				memcpy(unaParticion->inicio,dataAux,unaParticion->tamanio);

				unMensaje->data=unaParticion->inicio;
				free(dataAux);

			}
			cont++;
		}
    	sem_post(&mutex_particiones);
}

void* obtener_particion_disponible(int tamanio)
{
    sem_wait(&binarioF);
	void * disponible=NULL;
	while(1)
	{
		if(strcmp(algoritmoM,"BS")==0)
		{
			disponible=obtener_particion_disponible_BS(tamanio);
		//printf("INICIO: %d\n",disponible-cache);
		}
		else
        {
            if(strcmp(algoritmoM,"PD")==0)
            {
                sem_post(&binarioG);
            	disponible=obtener_particion_disponible_PD(tamanio);
            }
            else
            {
            	log_error(logB,"NO EXISTE EL ALGORITMO: obtener_particion_disponible."); return NULL;
            }
        }

        if(disponible!=NULL)
        {
        	intentos=0;
            return disponible;
        }
        else
        {  //si no hay particion disponible
        	sem_post(&binarioI);
        	elegir_victima();
        	intentos ++;
        	if(frecuenciaC==0 || frecuenciaC==intentos)
        	{
        		sem_post(&binarioH);
        		if(strcmp(algoritmoM,"PD")==0)
        		{
        			log_trace(logB,"\tCOMPACTAR");
        			compactar();
        		}
        		intentos=0;
        	}
        }
	}
	return NULL;
}

uint32_t agregar_mensaje(op_code tipo, t_list *lista)
{
    sem_wait(&binarioE);
	uint32_t id_generado=generar_id_mensaje();
	uint32_t longitud;
	void *particionDisponible=NULL;
	mensaje *nuevoMensaje=NULL;
	int tamanioSolicitado=-1;

	 //para el check
	 int i; int cantidadSuscriptores;

	 check_ *nuevoCheck=NULL;
   switch(tipo)
   {
        case NEW_POKEMON:

//            //printf("\t\tPokemon: %s\n", list_get(lista,0));
            longitud =(uint32_t)(strlen(list_get(lista,0)));
			log_trace(logB,"\tAGREGANDO MENSAJE: NEW_POKEMON %s (%d -> %d).",(char*)list_get(lista,0), 16+longitud, redondear(16+longitud));

        ////printf("STRLEN(POKEMON)=%d\n", longitud);
//            //printf("\t\tPosicion X: %d\n", *(int*)list_get(lista,1)); //IMPORTANTE EL *(int*)*/
//            //printf("\t\tPosicion Y: %d\n", *(int*)list_get(lista,2));
//            //printf("\t\tCantidad: %d\n", *(int*)list_get(lista,3));
            sem_post(&binarioF);

            tamanioSolicitado=redondear(16+longitud);
				if(tamanioSolicitado>tamanioM)
				{
					log_error(logB,"MENSAJE DESCARTADO: Supera la memoria. Se responde con id -1.");
					return -1;

				}
			particionDisponible=obtener_particion_disponible(tamanioSolicitado);
			log_trace(logB,"\tSe almacena al comienzo de: %d",particionDisponible-cache);


			sem_wait(&mutex_particiones);
            memcpy(particionDisponible,&longitud,4);
            memcpy(particionDisponible+4,list_get(lista,0),longitud);
            memcpy(particionDisponible+ 4+ longitud, list_get(lista,1),4);
            memcpy(particionDisponible+ 4+ longitud+4, list_get(lista,2),4);
            memcpy(particionDisponible+ 4+ longitud+4+4, list_get(lista,3),4);

            nuevoMensaje=(mensaje*)malloc(sizeof(mensaje));
            nuevoMensaje->data=particionDisponible;
            nuevoMensaje->id=id_generado;
            nuevoMensaje->idc=0;
            nuevoMensaje->check=list_create();
                i=0;
           sem_wait(&mutex_new);
                cantidadSuscriptores=list_size(new_->suscriptores);

                while(cantidadSuscriptores>i)
                {
                    nuevoCheck=(check_*)malloc(sizeof(check_*));
                    nuevoCheck->recibido=false;
                    list_add(nuevoMensaje->check,(void*)nuevoCheck);
                    i++;
                }


//            //printf("\t%d checks creados en el mensaje\n",i);

            list_add(new_->mensajes,(void*)nuevoMensaje);
            sem_post(&mutex_new);
           sem_post(&mutex_particiones);
//            //printf("\t Id Generado: %d\n", nuevoMensaje->id);
            //mostrar_elementos(NEW, MENSAJES);

            return nuevoMensaje->id;
            break;

        case APPEARED_POKEMON:

//             //printf("\t\tIDC: %d\n", *(int*)list_get(lista,0));
//			 //printf("\t\tPokemon: %s\n", list_get(lista,1));
             longitud =(uint32_t)(strlen(list_get(lista,1)));
			 log_trace(logB,"\tAGREGANDO MENSAJE: APPEARED_POKEMON %s (%d -> %d).",(char*)list_get(lista,1), 12+longitud, redondear(12+longitud));

//        //printf("STRLEN(POKEMON)=%d\n", longitud);
//			 //printf("\t\tPosicion X: %d\n", *(int*)list_get(lista,2));
//			 //printf("\t\tPosicion Y: %d\n", *(int*)list_get(lista,3));
             sem_post(&binarioF);

             tamanioSolicitado=redondear(12+longitud);
				 if(tamanioSolicitado>tamanioM)
				 {
					log_error(logB,"MENSAJE DESCARTADO: Supera la memoria. Se responde con id -1.");
					return -1;

				 }
			 particionDisponible = obtener_particion_disponible(tamanioSolicitado);
			 log_trace(logB,"\tSe almacena al comienzo de: %d",particionDisponible-cache);

			 sem_wait(&mutex_particiones);
			 memcpy(particionDisponible,&longitud,4);
			 memcpy(particionDisponible+4,list_get(lista,1),longitud);
			 memcpy(particionDisponible+ 4+ longitud, list_get(lista,2),4);
			 memcpy(particionDisponible+ 4+ longitud+4, list_get(lista,3),4);

			 nuevoMensaje=(mensaje*)malloc(sizeof(mensaje));
			 nuevoMensaje->data=particionDisponible;
			 nuevoMensaje->id=id_generado;
			 int idc1=*(int*)list_get(lista,0);
			 nuevoMensaje->idc=(uint32_t)idc1;
             nuevoMensaje->check=list_create();
                 i=0;
           sem_wait(&mutex_appeared);
                 cantidadSuscriptores=list_size(appeared_->suscriptores);
                 while(cantidadSuscriptores>i)
                 {
                     nuevoCheck=(check_*)malloc(sizeof(check_*));
                     nuevoCheck->recibido=false;
                     list_add(nuevoMensaje->check,(void*)nuevoCheck);
                     i++;
                 }

//             //printf("\t%d checks creados en el mensaje\n",i);

			 list_add(appeared_->mensajes,(void*)nuevoMensaje);
			 sem_post(&mutex_appeared);
           sem_post(&mutex_particiones);

//             //printf("\t Id Generado: %d\n", nuevoMensaje->id);
             //mostrar_elementos(APPEARED, MENSAJES);

             return nuevoMensaje->id;
			 break;

	    case CATCH_POKEMON:


//		      //printf("\t\tPokemon: %s\n", list_get(lista,0));
              longitud =(uint32_t)(strlen(list_get(lista,0)));
			  log_trace(logB,"\tAGREGANDO MENSAJE: CATCH_POKEMON %s (%d -> %d).",(char*)list_get(lista,0), 12+longitud, redondear(12+longitud));

//        //printf("STRLEN(POKEMON)=%d\n", longitud);
//			  //printf("\t\tPosicion X: %d\n", *(int*)list_get(lista,1));
//			  //printf("\t\tPosicion Y: %d\n", *(int*)list_get(lista,2));
              sem_post(&binarioF);
              tamanioSolicitado=redondear(12+longitud);
				 if(tamanioSolicitado>tamanioM)
				 {
					log_error(logB,"MENSAJE DESCARTADO: Supera la memoria. Se responde con id -1.");
					return -1;

				 }
			  particionDisponible=obtener_particion_disponible(tamanioSolicitado);
			  log_trace(logB,"\tSe almacena al comienzo de: %d",particionDisponible-cache);

              sem_wait(&mutex_particiones);
			  memcpy(particionDisponible,&longitud,4);
			  memcpy(particionDisponible+4,list_get(lista,0),longitud);
			  memcpy(particionDisponible+ 4+ longitud, list_get(lista,1),4);
			  memcpy(particionDisponible+ 4+ longitud+4, list_get(lista,2),4);

			  nuevoMensaje=(mensaje*)malloc(sizeof(mensaje));
			  nuevoMensaje->data=particionDisponible;
			  nuevoMensaje->id=id_generado;
			  nuevoMensaje->idc=0;
              nuevoMensaje->check=list_create();
                  i=0;
           sem_wait(&mutex_catch);
                  cantidadSuscriptores=list_size(catch_->suscriptores);
                  while(cantidadSuscriptores>i)
                  {
                      nuevoCheck=(check_*)malloc(sizeof(check_*));
                      nuevoCheck->recibido=false;
                      list_add(nuevoMensaje->check,(void*)nuevoCheck);
                      i++;
                  }
//              //printf("\t%d checks creados en el mensaje\n",i);

			  list_add(catch_->mensajes,(void*)nuevoMensaje);
              sem_post(&mutex_catch);
           sem_post(&mutex_particiones);

//			  //printf("\t Id Generado: %d\n", nuevoMensaje->id);
			  //mostrar_elementos(CATCH,MENSAJES);

			  return nuevoMensaje->id;
		      break;

	    case CAUGHT_POKEMON:
	    	log_trace(logB,"\tAGREGANDO MENSAJE: CAUGHT_POKEMON (%d -> %d).", 4, redondear(4));

//             //printf("\t\tId Correlativo: %d\n", *(int*)list_get(lista,0));
//              //printf("\t\tOk/fail: %s\n", list_get(lista,1));
//        //printf("STRLEN(RESULTADO)=%d\n", strlen(list_get(lista,1)));
             sem_post(&binarioF);
             tamanioSolicitado=redondear(4);
				 if(tamanioSolicitado>tamanioM)
				 {
					log_error(logB,"MENSAJE DESCARTADO: Supera la memoria. Se responde con id -1.");
					return -1;

				 }
			  particionDisponible=obtener_particion_disponible(tamanioSolicitado);
			  log_trace(logB,"\tSe almacena al comienzo de: %d",particionDisponible-cache);

             sem_wait(&mutex_particiones);
             if(strcmp(list_get(lista,1),"OK")==0)
             {
                 uint32_t uno=1;
                 memcpy(particionDisponible,&uno,4);
             }
             else
             {
                 uint32_t cero=0;
            	 memcpy(particionDisponible,&cero,4);
             }
			  nuevoMensaje=(mensaje*)malloc(sizeof(mensaje));
			  nuevoMensaje->data=particionDisponible;
			  nuevoMensaje->id=id_generado;
			  int idc2=*(int*)list_get(lista,0);
			  nuevoMensaje->idc=(uint32_t)idc2;
			  nuevoMensaje->check=list_create();
                  i=0;
           sem_wait(&mutex_caught);
                  cantidadSuscriptores=list_size(caught_->suscriptores);
                  while(cantidadSuscriptores>i)
                  {
                      nuevoCheck=(check_*)malloc(sizeof(check_*));
                      nuevoCheck->recibido=false;
                      list_add(nuevoMensaje->check,(void*)nuevoCheck);
                      i++;
                  }

//              //printf("\t%d checks creados en el mensaje\n",i);

			  list_add(caught_->mensajes,(void*)nuevoMensaje);
              sem_post(&mutex_caught);
           sem_post(&mutex_particiones);

//              //printf("\t Id Generado: %d\n", nuevoMensaje->id);
              //mostrar_elementos(CAUGHT,MENSAJES);

              return nuevoMensaje->id;
		      break;

	    case GET_POKEMON:

//		     //printf("\t\tPokemon: %s\n", list_get(lista,0));
             longitud =(uint32_t)(strlen(list_get(lista,0)));
			 log_trace(logB,"\tAGREGANDO MENSAJE: GET_POKEMON %s (%d -> %d).",(char*)list_get(lista,0), 4+longitud, redondear(4+longitud));

//        //printf("STRLEN(POKEMON)=%d\n", longitud);
             sem_post(&binarioF);
             tamanioSolicitado=redondear(4+longitud);
				 if(tamanioSolicitado>tamanioM)
				 {
					log_error(logB,"MENSAJE DESCARTADO: Supera la memoria. Se responde con id -1.");
					return -1;

				 }
			 particionDisponible = obtener_particion_disponible (tamanioSolicitado);
			 log_trace(logB,"\tSe almacena al comienzo de: %d",particionDisponible-cache);

             sem_wait(&mutex_particiones);
			 memcpy(particionDisponible,&longitud,4);
			 memcpy(particionDisponible+4,list_get(lista,0),longitud);

			 nuevoMensaje=(mensaje*)malloc(sizeof(mensaje));
			 nuevoMensaje->data=particionDisponible;
			 nuevoMensaje->id=id_generado;
			 nuevoMensaje->idc= 0;
			 nuevoMensaje->check=list_create();
                 i=0;
           sem_wait(&mutex_get);
                 cantidadSuscriptores=list_size(get_->suscriptores);
                 while(cantidadSuscriptores>i)
                 {
                     nuevoCheck=(check_*)malloc(sizeof(check_*));
                     nuevoCheck->recibido=false;
                     list_add(nuevoMensaje->check,(void*)nuevoCheck);
                     i++;
                 }

//             //printf("\t%d checks creados en el mensaje\n",i);

			 list_add(get_->mensajes,(void*)nuevoMensaje);
			 sem_post(&mutex_get);
           sem_post(&mutex_particiones);

//             //printf("\t Id Generado: %d\n", nuevoMensaje->id);

             //mostrar_elementos(GET,MENSAJES);

             return nuevoMensaje->id;
             break;

	    case LOCALIZED_POKEMON:


//            //printf("\t\tId Correlativo: %d\n", *(int*)list_get(lista,0));
//			//printf("\t\tPokemon: %s\n", list_get(lista,1));
            longitud =(uint32_t)(strlen(list_get(lista,1)));
//        //printf("STRLEN(POKEMON)=%d\n", longitud);
//			//printf("\t\tCantidad de pares: %d\n", *(int*)list_get(lista,2));

			int cantidadPares=*(int*)list_get(lista,2);
            log_trace(logB,"\tAGREGANDO MENSAJE: LOCALIZED_POKEMON %s (%d -> %d).",(char*)list_get(lista,1), 8+8*cantidadPares+longitud, redondear(8+8*cantidadPares+longitud));

            sem_post(&binarioF);
            tamanioSolicitado=redondear(8+8*cantidadPares+longitud);
				 if(tamanioSolicitado>tamanioM)
				 {
					log_error(logB,"MENSAJE DESCARTADO: Supera la memoria. Se responde con id -1.");
					return -1;
	
				 }
             particionDisponible = obtener_particion_disponible(tamanioSolicitado);
             log_trace(logB,"\tSe almacena al comienzo de: %d",particionDisponible-cache);

             sem_wait(&mutex_particiones);
			 memcpy(particionDisponible,&longitud,4);
			 memcpy(particionDisponible+4,list_get(lista,1),longitud);
			 memcpy(particionDisponible+ 4+ longitud, list_get(lista,2),4);

			 t_link_element * aux= lista->head->next->next;
			 int cont=0;
			 int desplazamiento=4+ longitud + 4;
			 int cantidadPosicion= (*(int*)list_get(lista,2)) *2;
             uint32_t numero=0;

			 while(aux &&  (cantidadPosicion >cont ))
			 {
				 aux= aux->next;
				 numero=(uint32_t)*(int*)aux->data;
//				 //printf("\t\tX: %d  ",numero);
				 memcpy(particionDisponible+desplazamiento,&numero,4);
				 desplazamiento+=4;
				 aux= aux->next;

                 numero=(uint32_t)*(int*)aux->data;
//                 //printf("Y: %d\n", numero);
				 memcpy(particionDisponible+desplazamiento,&numero,4);
				 desplazamiento+=4;
				 cont +=2;
			 }

			 nuevoMensaje=(mensaje*)malloc(sizeof(mensaje));
			 nuevoMensaje->data=particionDisponible;
			 nuevoMensaje->id=id_generado;
			 int idc3=*(int*)list_get(lista,0);
			 nuevoMensaje->idc=(uint32_t)idc3;
			 nuevoMensaje->check=list_create();
                 i=0;
           sem_wait(&mutex_localized);
                 cantidadSuscriptores=list_size(localized_->suscriptores);
                 while(cantidadSuscriptores>i)
                 {
                     nuevoCheck=(check_*)malloc(sizeof(check_*));
                     nuevoCheck->recibido=false;
                     list_add(nuevoMensaje->check,(void*)nuevoCheck);
                     i++;
                 }
//             //printf("\t%d checks creados en el mensaje\n",i);

			 list_add(localized_->mensajes,(void*)nuevoMensaje);
             sem_post(&mutex_localized);
           sem_post(&mutex_particiones);

//             //printf("\t Id Generado: %d\n", nuevoMensaje->id);

             //mostrar_elementos(LOCALIZED,MENSAJES);


             return nuevoMensaje->id;
             break;

        default: log_error(logB,"\tTIPO DE MENSAJE NO ENCONTRADO: agregar_mensaje (%d)",tipo);return -1;break;
	}
}

void consolidar (bool primero)
{
	//log_trace(logB,"\tCONSOLIDAR");

  if(strcmp("BS",algoritmoM)==0)
  {
//	  Dos bloques A y B son buddies si:
//	  ◆ Poseen el mismo tamaño
//	  ◆ Están ubicados en forma contigua
//	  ◆ dirA = dirB XOR tamA && dirB = dirA XOR tamB (XOR binario: ^)

	  int i=0;

	  while(list_size(dinamicas)>i+1)
	  {
		  particion_ *actual=(particion_*)list_get(dinamicas,i);
		  particion_ *siguiente=(particion_*)list_get(dinamicas,i+1);

		  if(actual->libre &&  siguiente->libre)
		  {
//			  printf("%d y %d estan libres\n",i,i+1);
			  if(actual->tamanio==siguiente->tamanio)
			  {
//				  printf("%d y %d tienen el mismo tamanio\n",i,i+1);

				  int dirA=(int)(actual->inicio-cache);
				  int dirB=(int)(siguiente->inicio-cache);
				  int tamA=(int)actual->tamanio;
				  int tamB=(int)siguiente->tamanio;

//						  //printf("dirA = dirB ^ tamA -> %d = %d ^ %d = %d\n ",(int)actual->inicio,(int)siguiente->inicio,
//						  (int)actual->tamanio,(int)siguiente->inicio ^ (int)actual->tamanio*2);
//				  //printf("dirB = dirA ^ tamB -> %d = %d ^ %d = %d\n ",(int)siguiente->inicio,(int)actual->inicio,
//						  (int)siguiente->tamanio,(int)actual->inicio ^ (int)siguiente->tamanio*2);
//
//				 if((int)actual->inicio==((int)siguiente->inicio ^ (int)actual->tamanio)
//						 &&	(int)siguiente->inicio==((int)actual->inicio ^ (int)siguiente->tamanio))

//				  printf("dirA = dirB ^ tamA -> %d = %d ^ %d = %d \n ",dirA,dirB,tamA,(dirB ^ tamA));
//				  printf("dirB = dirA ^ tamB -> %d = %d ^ %d = %d \n ",dirB,dirA,tamB,(dirA ^ tamB));
//
//				  printf("dirA == dirB ^ tamA = %s\n",(dirA == (dirB ^ tamA ))?"true":"false");
//				  printf("dirB == dirA ^ tamB = %s\n",(dirB == (dirA ^ tamB))?"true":"false");

				 if(dirA == (dirB ^ tamA )&& dirB == (dirA ^ tamB))
				 {
					 //puts("SON BUDDYYES");

					 particion_* eliminar=list_remove(dinamicas,i+1);
					 //log_trace(logB,"\tPARTICION ELIMINADA, Comienzo: %d",eliminar->inicio-cache);
					 log_trace(logB,"\tASOCIACION, ComienzoA: %d, ComienzoB: %d",actual->inicio-cache,siguiente->inicio-cache);

					 actual->tamanio=actual->tamanio+eliminar->tamanio;

					 free(eliminar);
					 consolidar(false);
				 }
			  }
		  }
		  i++;
	  }
  }
  else
  {
	  int i=0;
	  while(list_size(dinamicas)>i+1)
		  {
			  particion_ *actual=(particion_*)list_get(dinamicas,i);
			  particion_ *siguiente=(particion_*)list_get(dinamicas,i+1);


			  if(actual->libre &&  siguiente->libre)
			  {
//				  if(primero)
//				  {
//		                log_trace(logB,"\tPARTICION ELIMINADA, Comienzo: %d",actual->inicio-cache);
//
//				  }
//
//    	 		  log_trace(logB,"\tPARTICION ELIMINADA, Comienzo: %d",siguiente->inicio-cache);

    	 		  actual->tamanio+=siguiente->tamanio;
    	 		  free(list_remove(dinamicas,i+1));
    	 		  consolidar(false);

			  }

			  i++;
		  }
	  //◆ PODIAMOS HACERLO DESDE EL PRINCIPIO :C, inutil.
//	  int romper=0;
//
//	  while(posicion-1>=0&& romper == 0)
//	    {
//	 	   particion_*anterior= (particion_*)list_get(dinamicas,posicion-1);
//	 	   if(!anterior->libre)
//	 	   {
//	 		   romper=1;
//
//	 	   }
//	 	   else
//	 	   {    //printf("\n\tSe consolido la particion en la posicion hacia atras %d \n", posicion);
//	 		   anterior->tamanio+= ((particion_*)list_get(dinamicas,posicion))->tamanio;
//	 		   particion_* eliminar =(particion_*)list_remove(dinamicas,posicion);
//	 		  log_trace(logB,"\tPARTICION ELIMINADA, Comienzo: %d",eliminar->inicio-cache);
////	 		  free(eliminar->timestamp);eliminar->timestamp=NULL;
////	 		  free(eliminar->actualizado);eliminar->actualizado=NULL;
//	 		   free(eliminar);
//	 		   posicion--;
//	 	   }
//
//	    }
//
//	  if (posicion==-1) {posicion=0;}
//
//		while(list_size(dinamicas)>posicion+1)
//		{
//		   particion_*siguiente= (particion_*)list_get(dinamicas,posicion+1);
//
//		   if(!siguiente->libre)
//		   {
//			  posicion=list_size(dinamicas);
//		   }
//		   else
//		   {
//			   //printf("\n\tSe consolido la particion en la posicion hacia adelante %d \n", posicion);
//			   particion_*actual= (particion_*)list_get(dinamicas,posicion);
//			   actual->tamanio+= ((particion_*)list_get(dinamicas,posicion+1))->tamanio;
//			   particion_* eliminar =(particion_*)list_remove(dinamicas,posicion+1);
//			   log_trace(logB,"\tPARTICION ELIMINADA, Comienzo: %d",eliminar->inicio-cache);
////			   free(eliminar->timestamp);eliminar->timestamp=NULL;
////			   free(eliminar->actualizado);eliminar->actualizado=NULL;
//			   free(eliminar);
//		   }
//		}
  }

}

int distribuir(int posicionM, t_paquete * paquete, cola * unaCola)
{

	//***********LAS COLAS DE MENSAJE ESTAN SINCRONIZADAS EN DISTRIBUIDOR
	int  i=0, envios=0;
	mensaje * elMensaje=(mensaje*)list_get(unaCola->mensajes,posicionM);
	t_list * suscriptores= unaCola->suscriptores;
    int tamanio= list_size(suscriptores);


	while(tamanio>i)
	{
	    suscriptor * unSuscriptor=(suscriptor*)	list_get(suscriptores,i);
        check_ * check=(check_*)list_get(elMensaje->check, i);
      if(check==NULL)
      {
          log_error(logB,"\tLOS CHECKS DEL MENSAJE NO FUERON CREADOS: distribuir.");
          break;
      }
      else
      {

            	  if(!check->recibido) //enviamos solo si no fue recibido antes.
    	         {

            	    	if(  unSuscriptor->conectado)
					    	{
								 //ACTUALIZACION LRU
								 //Deberiamos acceder a memoria solo cuando sabemos que el mensaje no fue enviado.
								int posicion=obtener_particion_utilizada(elMensaje->data);

								 particion_* unaDinamica=(particion_*)list_get(dinamicas,posicion);
								 ////printf("Actualizamos LRU, Antiguo: %s, ",unaDinamica->actualizado);
								 free(unaDinamica->actualizado);
								 unaDinamica->actualizado=NULL;
								 unaDinamica->actualizado=temporal_get_string_time();
						         log_info(logB,"\tLRU actualizado: %s ...", unaDinamica->actualizado);

								 ////printf("Actual: %s \n",unaDinamica->actualizado);
								 //ENVIO
								 enviar_paquete(paquete,unSuscriptor->socket );
								 log_trace(logB,"\tMensaje enviado a socket: %d", unSuscriptor->socket );
								 //RESPUESTA?
								 int operacion=-1;
								 operacion=recibir_operacion(unSuscriptor->socket);

								 log_trace(logB,"\tMensaje: %d %s RECIBIDO (%d)", elMensaje->id,operacion==10?"":"-NO-",operacion);
								 if(operacion==RECIBIDO)
								 {
										t_list *lista=recibir_paquete(unSuscriptor->socket);
										list_destroy_and_destroy_elements(lista,free);
									 check->recibido=true;
									 envios++;
								 }
								 else
								 {
									 log_warning(logB,"\tSuscriptor se pone en desconectado: %d", unSuscriptor->id );
									 unSuscriptor->conectado=false;
								 }

            	             }
    	         }
    	         else
    	         {
    	         	envios++;
    	         }
    	         i++;
      }
	}

	return posicionM+1;

//	if(envios==tamanio &&  tamanio>0)
//    {
//	    //printf("\n\t\t Mensaje: %d debe ser eliminado \n",(int) elMensaje->id);
//	    int posicion=obtener_particion_utilizada(elMensaje->data);
//
//		particion_ * dinamica=(particion_*)list_get(dinamicas,posicion);
//
//
//		mensaje * aEliminar=(mensaje*)list_remove(unaCola->mensajes,posicionM);
//
//        list_destroy_and_destroy_elements(aEliminar->check,free);
//        aEliminar->data=NULL;
//        free(aEliminar);
//		dinamica-> libre=true;
//		free(dinamica->timestamp); dinamica->timestamp=NULL;
//		free(dinamica->actualizado);dinamica->actualizado=NULL;
//
//         //dumpCache();
//
//         //printf("\n\tSe consolida comenzando desde la posicion %d \n", posicion);
//
//         consolidar(true);
//
//		 //dumpCache();
//
//		return posicionM;
//    }
//	else
//    {
//        ////printf("\t\t\Mensaje: %d no debe ser eliminado \n", elMensaje->id);
//        return posicionM+1;
//    }
}

void  completar_nombre(void * nombre, int tamanio)
{
	char *descartable="a";
	memcpy(nombre+tamanio,descartable+1,1);
}

void distribuidor(elemento_ tipoM)
{
    //sem_wait(&binario);
  //  sem_wait(&mutex_distribuidor);
    int i;
    uint32_t longitud;
    void* numero=malloc(4);
    char*nombre=NULL;
    int tipoMensaje=(int)tipoM;

    switch (tipoMensaje)
    {
        case NEW:
            sem_wait(&mutex_new);
            sem_wait(&mutex_particiones);
            //printf("\nDISTRIBUYENDO NEW...\n");
            i=0;
            while(list_size(new_->mensajes)>i)
            {
//                //printf("\n\t\t Mensaje numero: %d\n", i+1);
                mensaje *unMensaje = (mensaje *) list_get(new_->mensajes, i);
                t_paquete *paqueteNew = crear_paquete(NEW_POKEMON);
                //ID_MENSAJE
                //printf("\t ID: %d\n", unMensaje->id);
                agregar_a_paquete(paqueteNew, &unMensaje->id, 4);
                //NOMBRE
                memcpy(numero, unMensaje->data, 4);
                longitud = *(uint32_t *) numero;
//                //printf("\t\tLA LONGITUD********************* (e.e) (e.e): %u\n", longitud);
                nombre = malloc(longitud+1);
                memcpy(nombre, unMensaje->data + 4, longitud);
                completar_nombre(nombre,longitud);
//                //printf("\t\tNombre: %s\n", (char*)nombre);

                ////printf("\t\tDISTRIBUYENDO NEW STRLEN %s = %d\n", nombre, strlen(nombre));
                agregar_a_paquete(paqueteNew, nombre, longitud+1);
                //X
                memcpy(numero, unMensaje->data + 4 + longitud, 4);
                ////printf("\t\tX: %d\n", *(int*)numero);
                agregar_a_paquete(paqueteNew, numero, 4);
                //Y
                memcpy(numero, unMensaje->data + 4 + longitud + 4, 4);
                ////printf("\t\tY: %d\n", *(int*)numero);
                agregar_a_paquete(paqueteNew, numero, 4);
                //CANTIDAD
                memcpy(numero, unMensaje->data + 4 + longitud + 4 + 4, 4);
                ////printf("\t\tCantidad: %d\n", *(int*)numero);
                agregar_a_paquete(paqueteNew, numero, 4);

                // distribuir(unMensaje, paqueteNew, new_->suscriptores);
                i = distribuir(i, paqueteNew, new_);
                eliminar_paquete(paqueteNew);
                free(nombre);
            }
            sem_post(&mutex_particiones);
            sem_post(&mutex_new);
            break;
        case APPEARED:
            sem_wait(&mutex_appeared);
            sem_wait(&mutex_particiones);
            //printf("\nDISTRIBUYENDO APPEARED...\n");
            i=0;
            while(list_size(appeared_->mensajes)>i)
            {
//                //printf("\n\t\t Mensaje numero: %d\n", i+1);
                mensaje *unMensaje= (mensaje*)list_get(appeared_->mensajes,i);
                t_paquete* paqueteAppeared= crear_paquete(APPEARED_POKEMON);
                //ID CORRELATIVO
                agregar_a_paquete(paqueteAppeared,&unMensaje->idc,4);
                //NOMBRE
                memcpy(numero,unMensaje->data,4);
                longitud=*(uint32_t*)numero;
//                //printf("\t\tLA LONGITUD********************* (e.e) (e.e): %u\n", longitud);
                nombre=malloc(longitud+1);
                memcpy(nombre, unMensaje->data+4, longitud);
                completar_nombre(nombre,longitud);
//                 //printf("\t\tNombre: %s\n", (char*)nombre);
                ////printf("\t\tDISTRIBUYENDO APPEARED STRLEN %s = %d\n", nombre, strlen(nombre));
                agregar_a_paquete(paqueteAppeared,nombre,longitud+1);
                //X
                memcpy(numero,unMensaje->data+4+longitud,4);
                ////printf("\t\tX: %d\n", *(int*)numero);
                agregar_a_paquete(paqueteAppeared,numero,4);
                //Y
                memcpy(numero,unMensaje->data+4+longitud+4,4);
                ////printf("\t\tY: %d\n", *(int*)numero);
                agregar_a_paquete(paqueteAppeared,numero,4);

                // distribuir(unMensaje, paqueteAppeared, appeared_->suscriptores);
                i= distribuir(i, paqueteAppeared, appeared_);
                eliminar_paquete(paqueteAppeared);
                free(nombre);
            }
            sem_post(&mutex_particiones);
            sem_post(&mutex_appeared);
            break;
        case CATCH:
            sem_wait(&mutex_catch);
            sem_wait(&mutex_particiones);
            //printf("\nDISTRIBUYENDO CATCH...\n");
            i=0;
            while(list_size(catch_->mensajes)>i)
            {
//                //printf("\n\t\t Mensaje numero: %d\n", i+1);
                mensaje *unMensaje= (mensaje*)list_get(catch_->mensajes,i);
                t_paquete* paqueteCatch= crear_paquete(CATCH_POKEMON);
                //ID_MENSAJE
                //printf("\t Id: %d\n", unMensaje->id);
                agregar_a_paquete(paqueteCatch,&unMensaje->id,4);
                //NOMBRE
                memcpy(numero,unMensaje->data,4);
                longitud=*(uint32_t*)numero;
//                //printf("\t\tLA LONGITUD********************* (e.e) (e.e): %u\n", longitud);
                nombre=malloc(longitud+1);
                memcpy(nombre, unMensaje->data+4, longitud);
                completar_nombre(nombre,longitud);
//                 //printf("\t\tNombre: %s\n", (char*)nombre);
                agregar_a_paquete(paqueteCatch,nombre,longitud+1);
                //X
                memcpy(numero,unMensaje->data+4+longitud,4);
                ////printf("\t\tX: %d\n", *(int*)numero);
                agregar_a_paquete(paqueteCatch,numero,4);
                //Y
                memcpy(numero,unMensaje->data+4+longitud+4,4);
                ////printf("\t\tY: %d\n", *(int*)numero);
                agregar_a_paquete(paqueteCatch,numero,4);

                // distribuir(unMensaje, paqueteCatch, catch_->suscriptores);
                i=  distribuir(i, paqueteCatch, catch_);
                eliminar_paquete(paqueteCatch);
                free(nombre);
            }
            sem_post(&mutex_particiones);
            sem_post(&mutex_catch);
            break;
        case CAUGHT:
            sem_wait(&mutex_caught);
            sem_wait(&mutex_particiones);
            //printf("\nDISTRIBUYENDO CAUGHT...\n");
            i=0;
            while(list_size(caught_->mensajes)>i)
            {
//                //printf("\n\t\t Mensaje numero: %d\n", i+1);
                mensaje *unMensaje= (mensaje*)list_get(caught_->mensajes,i);

                t_paquete* paquetecaught= crear_paquete(CAUGHT_POKEMON);
                //ID CORRELATIVO
                //printf("\t Id Correlativo: %d\n", unMensaje->idc);
                agregar_a_paquete(paquetecaught,&unMensaje->idc,4);
                ////printf("\t\tIDC: %d\n",unMensaje->idc);;
                //OK(1)/FAIL(0)
                memcpy(numero,unMensaje->data,4);
                agregar_a_paquete(paquetecaught,numero,4);
                ////printf("\t\tOK/fail: %d\n", *(int*)numero);

                //distribuir(unMensaje, paquetecaught, caught_->suscriptores);
                i=distribuir(i, paquetecaught, caught_);
                eliminar_paquete(paquetecaught);
            }
            sem_post(&mutex_particiones);
            sem_post(&mutex_caught);
            break;
        case GET:
            sem_wait(&mutex_get);
            sem_wait(&mutex_particiones);
            //printf("\nDISTRIBUYENDO GET...\n");
            i=0;
            while(list_size(get_->mensajes)>i)
            {
//                //printf("\n\t\t Mensaje numero: %d\n", i+1);
                mensaje *unMensaje= (mensaje*)list_get(get_->mensajes,i);

                t_paquete* paqueteget= crear_paquete(GET_POKEMON);
                //ID_MENSAJE
                //printf("\t Id: %d\n", unMensaje->id);
                agregar_a_paquete(paqueteget,&unMensaje->id,4);
                //NOMBRE
                memcpy(numero,unMensaje->data,4);
                longitud=*(uint32_t*)numero;
//                //printf("\t\tLA LONGITUD********************* (e.e) (e.e): %u\n", longitud);
                nombre=malloc(longitud+1);
                memcpy(nombre, unMensaje->data+4, longitud);
                completar_nombre(nombre,longitud);
//                //printf("\t\tNombre: %s\n", (char*)nombre);
                ////printf("\t\tDISTRIBUYENDO GET STRLEN %s = %d\n", nombre, strlen(nombre));
                agregar_a_paquete(paqueteget,nombre,longitud+1);


                //distribuir(unMensaje, paqueteget, get_->suscriptores);
                i=distribuir(i, paqueteget, get_);
                eliminar_paquete(paqueteget);
                free(nombre);
            }
            sem_post(&mutex_particiones);
            sem_post(&mutex_get);
            break;
        case LOCALIZED:
            sem_wait(&mutex_localized);
            sem_wait(&mutex_particiones);
            //printf("\nDISTRIBUYENDO LOCALIZED...\n");
            i=0;
            while(list_size(localized_->mensajes)>i)
            {
//                //printf("\n\t\t Mensaje numero: %d\n", i+1);
                mensaje *unMensaje= (mensaje*)list_get(localized_->mensajes,i);
                t_paquete* paquetelocalized= crear_paquete(LOCALIZED_POKEMON);

                //ID CORRELATIVO
                //printf("\t Id Correlativo: %d\n", unMensaje->idc);
                agregar_a_paquete(paquetelocalized,&unMensaje->idc,4);
                //NOMBRE
                memcpy(numero,unMensaje->data,4);
                longitud=*(uint32_t*)numero;
//                //printf("\t\tLA LONGITUD********************* (e.e) (e.e): %u\n", longitud);
                nombre=malloc(longitud+1);
                memcpy(nombre, unMensaje->data+4, longitud);
                completar_nombre(nombre,longitud);
//                //printf("\t\tNombre: %s\n", (char*)nombre);
                ////printf("\t\tDISTRIBUYENDO LOCALIZED, ID: %d, STRLEN (%s) = %d\n", unMensaje->id,nombre, strlen(nombre));
                agregar_a_paquete(paquetelocalized,nombre,longitud+1);
                //cantidad pares
                memcpy(numero,unMensaje->data+4+longitud,4);
                ////printf("\t\tcantidad Pares: %d\n", *(int*)numero);
                agregar_a_paquete(paquetelocalized,numero,4);
                int pares =0;
                int cantidad=*(int*)numero;
                int desplazar=4+longitud + 4;
                while(cantidad>pares)
                {
                    memcpy(numero,unMensaje->data+desplazar,4);
                    ////printf("\t\tX: %d", *(int*)numero);
                    agregar_a_paquete(paquetelocalized,numero,4);
                    desplazar +=4;
                    memcpy(numero,unMensaje->data+desplazar,4);
                    ////printf("\t\tY: %d\n", *(int*)numero);
                    agregar_a_paquete(paquetelocalized,numero,4);
                    desplazar +=4;
                    pares ++;

                }
                //distribuir(unMensaje, paquetelocalized, localized_->suscriptores);
                i=distribuir(i, paquetelocalized, localized_);
                eliminar_paquete(paquetelocalized);
                free(nombre);
            }
            sem_post(&mutex_particiones);
            sem_post(&mutex_localized);
            break;
    }
   // sem_post(&mutex_distribuidor);
//    free(nombre);
    free(numero);
    sem_post(&distribuidorD);
    sem_post(&agregarSuscriptor);
}

void levantar_configuracion()
{
     ip=leer_config("IP",archivoDeConfiguracion);
     puerto=leer_config("PUERTO",archivoDeConfiguracion);
     tamanioM=(uint32_t)atoi(leer_config("TAMANO_MEMORIA",archivoDeConfiguracion));
     tamanioMinPar=(uint32_t)atoi(leer_config("TAMANO_MINIMO_PARTICION",archivoDeConfiguracion));
     algoritmoM=leer_config("ALGORITMO_MEMORIA",archivoDeConfiguracion);
     algoritmoR=leer_config("ALGORITMO_REEMPLAZO",archivoDeConfiguracion);
     algoritmoPL=leer_config("ALGORITMO_PARTICION_LIBRE",archivoDeConfiguracion);
     frecuenciaC=(uint32_t)atoi(leer_config("FRECUENCIA_COMPACTACION",archivoDeConfiguracion));
     archivoDeLog =leer_config("LOG_FILE",archivoDeConfiguracion);
     servidor=iniciar_servidor(ip,puerto);

    printf("Esperando clientes, IP: %s, PUERTO: %s, ID:%d \n", ip, puerto, servidor);

    printf("\n\ttamanioM: %u\n\ttamanioMP: %u\n\talgortimoM: %s\n\talgortimoR: %s\n\talgoritmoPL: %s\n\tfrecuenciaC: %u\n\tarchivoDeLog: %s\n"
            ,tamanioM, tamanioMinPar,algoritmoM, algoritmoR, algoritmoPL, frecuenciaC,archivoDeLog);

    puts("------------------------------------------------------------------");
}

void inicializar_colas()
{
	//printf("Inicializando colas...\n");
    colas=list_create();
    new_ =(cola*)malloc(sizeof(cola)) ;
    appeared_ =(cola*)malloc(sizeof(cola)) ;
    catch_ =(cola*)malloc(sizeof(cola)) ;
    caught_ =(cola*)malloc(sizeof(cola)) ;
    get_ =(cola*)malloc(sizeof(cola)) ;;
    localized_ =(cola*)malloc(sizeof(cola)) ;

    list_add(colas,(void*)new_);
    list_add(colas,(void*)appeared_);
    list_add(colas,(void*)catch_);
    list_add(colas,(void*)caught_);
    list_add(colas,(void*)get_);
    list_add(colas,(void*)localized_);

    int i=0;

    while(6>i)
    {
        cola * unaCola = (cola*)list_get(colas,i);
        unaCola->mensajes=list_create();
        unaCola->suscriptores=list_create();
        i++;
    }
}

void particiones_dinamicas()
{
  dinamicas=list_create();
  particion_ * cabeza=(particion_*)malloc(sizeof(particion_));
  cabeza->timestamp=NULL;
  cabeza->actualizado=NULL;
  cabeza->tamanio=tamanioM;
  cabeza->libre=true;
  cabeza->inicio=cache;
  list_add(dinamicas,(void*)cabeza);
}

void particionar_cache()
{
  if(strcmp("BS",algoritmoM)==0)
  {
//	  buddy_system();
      particiones_dinamicas();
  }else
  {
	  if(strcmp("PD",algoritmoM)==0)
	     {
	   	  particiones_dinamicas();
	     }
	  else
	  {
		  log_error(logB,"EL ALGORITMO NO EXISTE: particionar_cache.");
	  }
  }
}

void inicializar_cache()
{
	//printf("Inicializando cache...");
    cache=(void*)malloc(tamanioM);
    particionar_cache();
    //printf("\t INICIO:%d\n", (int)cache);
}

int redondear(int numero)
{
	double elevado=0;
	int i=1;
	int retorno;
	if(strcmp(algoritmoM,"BS")==0)
	{
		while(numero>(int)elevado)
		{
			elevado=pow((double)2,(double)i);
			i++;
		}
		retorno=(int)elevado;
	}
	else
	{
		retorno=numero;
	}


	if (retorno<tamanioMinPar)
    {
	    return tamanioMinPar;
    }
	return retorno;

}

void inicializar_semaforos()
{
    //sem_init(emaforoAInicializar, 1:Procesos/0:hilos, valorInicial);
    //sem_init(sem_t *mutex_alloc, int pshared, unsigned int value);

    sem_init(&mutex_distribuidor, 0, 1);
    sem_init(&mutex_new, 0, 1);
    sem_init(&mutex_appeared, 0, 1);
    sem_init(&mutex_caught, 0, 1);
    sem_init(&mutex_catch, 0, 1);
    sem_init(&mutex_get, 0, 1);
    sem_init(&mutex_localized, 0, 1);
    sem_init(&mutex_colas, 0, 1);
    sem_init(&mutex_victima, 0, 1);
    sem_init(&mutex_suscriptor_ID, 0, 1);
    sem_init(&mutex_mensaje_ID, 0, 1);
    sem_init(&binarioA, 0, 0);
    sem_init(&binarioB, 0, 0);
    sem_init(&binarioC, 0, 0);
    sem_init(&binarioD, 0, 0);
    sem_init(&binarioE, 0, 0);
    sem_init(&binarioF, 0, 0);
    sem_init(&binarioG, 0, 0);
    sem_init(&binarioH, 0, 0);
    sem_init(&binarioI, 0, 0);
    sem_init(&binarioJ, 0, 0);
    sem_init(&binario, 0, 0);
    sem_init(&agregarSuscriptor,0,1);
    sem_init(&distribuidorD,0,1);

    //creamos la lista de semaforos siguiendo el orden de la lista de colas.
    semaforos=list_create();
    list_add(semaforos,&mutex_new);
    list_add(semaforos,&mutex_appeared);
    list_add(semaforos,&mutex_caught);
    list_add(semaforos,&mutex_catch);
    list_add(semaforos,&mutex_get);
    list_add(semaforos,&mutex_localized);

    sem_init(&mutex_particiones, 0, 1);
}

void inicializar_broker()
{
    levantar_configuracion();
    logB=log_create(archivoDeLog, "Broker", 1, LOG_LEVEL_TRACE);
    logDUMP=log_create("log_dump_cache.log", "Broker", 1, LOG_LEVEL_TRACE);
    inicializar_colas();
    inicializar_cache();
    inicializar_semaforos();

    contador_mensaje=0;
    contador_suscriptor=0;
}

void liberar_colas()
{

}

void finalizar_broker()
{
    liberar_colas();
    free(cache);
    liberar_servidor(servidor);
}

void mostrar_elementos(elemento_ laCola, elemento_ param)
{
    int i, count, longitud;
    t_list *mensajes=NULL;
    void* numero=malloc(4);
    void* nombre=NULL;
    cola* cola=NULL;
    mensaje* unMensaje=NULL;
    int parametro= (int)param;
    int cola1=laCola;
    switch(parametro)
    {
        case MENSAJES:
            switch(cola1)
            {
                case NEW:
                    sem_wait(&mutex_new);
                    mensajes= new_->mensajes;
                    i=0;
                    count=list_size(mensajes);
                    //printf("\n\tMensajes NEW_POKEMON, longitud: %d.\n",count);
                    while(count>i)
                    {
                        unMensaje=(mensaje*)list_get(mensajes, i);
                        //printf("\n\t\tID_MENSAJE: %d\n", unMensaje->id);
                        //falta mostrar el check
                        memcpy(numero, unMensaje->data, 4);
                        longitud=*(int*)numero;
                        nombre=malloc(longitud+1);
                        //printf("\t\tLongitud: %d\n", longitud);
                        memcpy(nombre, unMensaje->data+4, longitud);
                        //printf("\t\tNombre: %s\n", (char*)nombre);
                        memcpy(numero, unMensaje->data+4+longitud, 4);
                        //printf("\t\tX: %d\n", *(int*)numero);
                        memcpy(numero, unMensaje->data+4+longitud+4, 4);
                        //printf("\t\tY: %d\n", *(int*)numero);
                        memcpy(numero, unMensaje->data+4+longitud+4+4, 4);
                        //printf("\t\tCantidad: %d\n", *(int*)numero);
                        i++;
                    }
                    sem_post(&mutex_new);
                    break;
                case APPEARED:
                    sem_wait(&mutex_appeared);
                    mensajes= appeared_->mensajes;
                    i=0;
                    count=list_size(mensajes);
                    //printf("\n\tMensajes APPEARED_POKEMON, longitud: %d.\n",count);
                    while(count>i)
                    {
                        unMensaje=(mensaje*)list_get(mensajes, i);
                        //printf("\n\t\tID: %d\n", unMensaje->id);
                        //falta mostrar el check
                        memcpy(numero, unMensaje->data, 4);
                        longitud=*(int*)numero;
                        nombre=malloc(longitud+1);
                        //printf("\t\tLongitud: %d\n", longitud);
                        memcpy(nombre, unMensaje->data+4, longitud);
                        //printf("\t\tNombre: %s\n", (char*)nombre);
                        memcpy(numero, unMensaje->data+4+longitud, 4);
                        //printf("\t\tX: %d\n", *(int*)numero);
                        memcpy(numero, unMensaje->data+4+longitud+4, 4);
                        //printf("\t\tY: %d\n", *(int*)numero);

                        //printf("\t\tIDC: %d\n", unMensaje->idc);
                        i++;
                    }
                    sem_post(&mutex_appeared);
                    break;
                case CATCH:
                    sem_wait(&mutex_catch);
                    mensajes= catch_->mensajes;
					i=0;
					count=list_size(mensajes);
					//printf("\n\tMensajes CATCH_POKEMON, longitud: %d.\n",count);
					while(count>i)
					{
						unMensaje=(mensaje*)list_get(mensajes, i);
						//printf("\n\t\tID_MENSAJE: %d\n", unMensaje->id);
						//falta mostrar el check
						memcpy(numero, unMensaje->data, 4);
						longitud=*(int*)numero;
						nombre=malloc(longitud+1);
						//printf("\t\tLongitud: %d\n", longitud);
						memcpy(nombre, unMensaje->data+4, longitud);
						//printf("\t\tNombre: %s\n", (char*)nombre);
						memcpy(numero, unMensaje->data+4+longitud, 4);
						//printf("\t\tX: %d\n", *(int*)numero);
						memcpy(numero, unMensaje->data+4+longitud+4, 4);
						//printf("\t\tY: %d\n", *(int*)numero);
						i++;
                	 }
                    sem_post(&mutex_catch);
                    break;
                case CAUGHT:
                    sem_wait(&mutex_caught);
				    mensajes= caught_->mensajes;
					i=0;
					count=list_size(mensajes);
					//printf("\n\tMensajes CAUGHT_POKEMON, longitud: %d.\n",count);
					while(count>i)
					{
						unMensaje=(mensaje*)list_get(mensajes, i);
						//printf("\n\t\tID_MENSAJE: %d", unMensaje->id);
                        //printf("\n\t\tID_CORRELATIVO: %d\n", unMensaje->idc);
						//falta mostrar el check
						memcpy(numero, unMensaje->data, 4);

						//printf("\t\tEstado: %u, %s\n", *(uint32_t*)numero, *(uint32_t*)numero?"atrapado":"no atrapado");

						i++;
					 }
                    sem_post(&mutex_caught);
                	break;
                case GET:
                    sem_wait(&mutex_get);
                    mensajes= get_->mensajes;
 					i=0;
 					count=list_size(mensajes);
 					//printf("\n\tMensajes GET_POKEMON, longitud: %d.\n",count);
 					while(count>i)
 					{
						unMensaje=(mensaje*)list_get(mensajes, i);
						//printf("\n\t\tID_MENSAJE: %d\n", unMensaje->id);
						//falta mostrar el check
						memcpy(numero, unMensaje->data, 4);
						longitud=*(int*)numero;
						nombre=malloc(longitud+1);
						//printf("\t\tLongitud: %d\n", longitud);
						memcpy(nombre, unMensaje->data+4, longitud);
						//printf("\t\tNombre: %s\n", (char*)nombre);

 						i++;
 					 }
                    sem_post(&mutex_get);
                	break;
                case LOCALIZED:
                    sem_wait(&mutex_localized);
  				    mensajes= localized_->mensajes;
  					i=0;
  					count=list_size(mensajes);
  					//printf("\n\tMensajes LOCALIZED_POKEMON, longitud: %d.\n",count);
  					while(count>i)
  					{
 						unMensaje=(mensaje*)list_get(mensajes, i);
 						//printf("\n\t\tID_MENSAJE: %d\n", unMensaje->id);
 						//falta mostrar el check
 						memcpy(numero, unMensaje->data, 4);
 						longitud=*(int*)numero;
 						nombre=malloc(longitud+1);
 						//printf("\t\tLongitud: %d\n", longitud);
 						memcpy(nombre, unMensaje->data+4, longitud);
 						//printf("\t\tNombre: %s\n", (char*)nombre);
 						memcpy(numero, unMensaje->data+4+longitud, 4);
 						//printf("\t\tCantidad de pares: %d\n", *(int*)numero);
 						int desplazo=4 + longitud + 4;
 						int g=0;
 						int iteraciones=*(int*)numero;
 						while(iteraciones>g)
 						{
 							memcpy(numero, unMensaje->data + desplazo,4);
 							desplazo+=4;
 							//printf("\t\tX: %d  ", *(int*)numero);
 							memcpy(numero, unMensaje->data + desplazo,4);
 							desplazo+=4;
 							//printf("Y: %d\n", *(int*)numero);
 							g++;

 						}

  						i++;
  					 }
                    sem_post(&mutex_localized);
                	break;

            }
            break;
        case SUSCRIPTORES:
            switch(cola1)
            {
                case NEW: cola=new_; break;
                case APPEARED: cola=appeared_; break;
                case CATCH: cola=catch_; break;
                case CAUGHT: cola=caught_; break;
                case GET: cola=get_; break;
                case LOCALIZED: cola=localized_; break;
            }

            i=0;
            count=list_size(cola->suscriptores);
            //printf("\n\t\tEl tamaño de la lista es: %d \n", count);
            suscriptor *unSuscriptor;
            while(count !=0)
            {
                unSuscriptor=(suscriptor*)list_get(cola->suscriptores,i);
                //printf ("\t\tPosicion %d, ID: %d. socket:%d \n", i, unSuscriptor->id, unSuscriptor->socket);
                count --;
                i++;
            }
            break;
        case TODO:
            mostrar_elementos(cola1, SUSCRIPTORES);
            mostrar_elementos(cola1, MENSAJES);
            break;
    }
    free(numero);
    free(nombre);
}

void *gestionarCliente(void* socketCliente)
{
	int socket=(int) socketCliente;
	uint32_t idMensaje;

    int operacion=0;
	t_list* lista;
	while(1)
	{
		operacion=-1;
		operacion = recibir_operacion(socket);
       // //printf("Operacion recibida:%d\n",operacion);
		switch(operacion)
		{
            case ENLAZAR:
                lista = recibir_paquete(socket);
                int idSuscriptor= *(int*)lista->head->data;
                list_destroy_and_destroy_elements(lista,free);

                if(idSuscriptor==0)
                {
                    idSuscriptor=generar_id_cliente();
                    log_trace(logB,"\tENLAZAR: Socket: %d Recibimos id 0, generamos el id: %d",socket, idSuscriptor);
                    t_paquete* paquete_id_cliente= crear_paquete(SUSCRIPTOR_ID);
                    agregar_a_paquete(paquete_id_cliente, &idSuscriptor, sizeof(int));
                    enviar_paquete(paquete_id_cliente, socket);
                    eliminar_paquete(paquete_id_cliente);
                }
                else
                {
                	log_trace(logB,"\tREENLACE:  Socket: %d, id: %d" , socket, idSuscriptor);
                    t_paquete* paquete_id_cliente= crear_paquete(REENLAZADO);
                    enviar_paquete(paquete_id_cliente, socket);
                    eliminar_paquete(paquete_id_cliente);
                }


                break;
                case SUSCRIBIR:case SUSCRIPTOR:
                lista = recibir_paquete(socket);
                //************AVISAR A OTROS MODULOS QUE POSICION 0 ES IDSUSCRIPTOR, POSICION 1 ES COLA
                idSuscriptor=*(int*)list_get(lista, 0);
                int cola= *(int*)list_get(lista, 1);
                list_destroy_and_destroy_elements(lista,free);
                switch(cola)
                {
                    case NEW_POKEMON:
                    	sem_wait(&agregarSuscriptor);
                    	log_trace(logB,"\tSUSCRIBIR: NEW_POKEMON  (Enum: %d, Socket: %d, id: %d)",cola,socket,idSuscriptor);

                        sem_wait(&mutex_new);
                        sem_post(&binarioA);
                        agregar_suscriptor(idSuscriptor ,socket, new_);

                        sem_post(&mutex_new);
                        sem_post(&binario);
                        sem_wait(&distribuidorD);
                        distribuidor(NEW);
                        ////mostrar_elementos(NEW, SUSCRIPTORES);
                        return 0;

                        break;
                      case APPEARED_POKEMON:
                    	  sem_wait(&agregarSuscriptor);
                    	log_trace(logB,"\tSUSCRIBIR: APPEARED_POKEMON   (Enum: %d, Socket: %d, id: %d)",cola,socket,idSuscriptor);
                        sem_wait(&mutex_appeared);
                        sem_post(&binarioA);
                        agregar_suscriptor(idSuscriptor ,socket, appeared_);

                        sem_post(&mutex_appeared);
                        sem_post(&binario);
                        sem_wait(&distribuidorD);
                        distribuidor(APPEARED);
                        ////mostrar_elementos(APPEARED, SUSCRIPTORES);

                        return 0;
                        break;
                    case CATCH_POKEMON:
                    	sem_wait(&agregarSuscriptor);
                    	log_trace(logB,"\tSUSCRIBIR: CATCH_POKEMON   (Enum: %d, Socket: %d, id: %d)",cola,socket,idSuscriptor);
                        sem_wait(&mutex_catch);
                        sem_post(&binarioA);
                        agregar_suscriptor(idSuscriptor ,socket, catch_);

                        sem_post(&mutex_catch);
                        sem_post(&binario);
                        sem_wait(&distribuidorD);
                        distribuidor(CATCH);
                        ////mostrar_elementos(CATCH, SUSCRIPTORES);
                        return 0;

                        break;
                    case CAUGHT_POKEMON:
                    	sem_wait(&agregarSuscriptor);
                    	log_trace(logB,"\tSUSCRIBIR: CAUGHT_POKEMON   (Enum: %d, Socket: %d, id: %d)",cola,socket,idSuscriptor);
                        sem_wait(&mutex_caught);
                        sem_post(&binarioA);
                        agregar_suscriptor(idSuscriptor ,socket, caught_);

                        sem_post(&mutex_caught);
                        sem_post(&binario);
                        sem_wait(&distribuidorD);
                        distribuidor(CAUGHT);
                        ////mostrar_elementos(CAUGHT, SUSCRIPTORES);
                        return 0;

                        break;
                    case GET_POKEMON:
                    	sem_wait(&agregarSuscriptor);
                    	log_trace(logB,"\tSUSCRIBIR: GET_POKEMON   (Enum: %d, Socket: %d, id: %d)",cola,socket,idSuscriptor);
                        sem_wait(&mutex_get);
                        sem_post(&binarioA);
                        agregar_suscriptor(idSuscriptor ,socket, get_);

                        sem_post(&mutex_get);
                        sem_post(&binario);
                        sem_wait(&distribuidorD);
                        distribuidor(GET);
                        ////mostrar_elementos(GET, SUSCRIPTORES);
                        return 0;

                        break;
                    case LOCALIZED_POKEMON:
                    	sem_wait(&agregarSuscriptor);
                    	log_trace(logB,"\tSUSCRIBIR: LOCALIZED_POKEMON   (Enum: %d, Socket: %d, id: %d)",cola,socket,idSuscriptor);
                        sem_wait(&mutex_localized);
                        sem_post(&binarioA);
                        agregar_suscriptor(idSuscriptor ,socket, localized_);

                        sem_post(&mutex_localized);
                        sem_post(&binario);
                        sem_wait(&distribuidorD);
                        distribuidor(LOCALIZED);
                        ////mostrar_elementos(LOCALIZED, SUSCRIPTORES);
                        return 0;

                        break;
                    default: log_error(logB,"\tSUSCRIBIR: Cola no encontrada.  (Enum: %d, Socket: %d, id: %d)",cola,socket,idSuscriptor); return 0;
                        break;
                }
                break;
//            case SUSCRIPTOR:
//                lista = recibir_paquete(socket);
//                //printf("\n\tSUSCRIPTOR: GAMEBOY   (cola: %s, Socket: %d, id: %d)\n",lista->head->data,socket,idSuscriptor);
//                //printf("\t\tCola de mensajes: %s\n", lista->head->data);
//                //printf("\t\tTiempo: %u\n", *(int*)lista->head->next->data);
//                break;
            case NEW_POKEMON:
            	 sem_wait(&agregarSuscriptor);
                lista = recibir_paquete(socket);
                //El idMensaje esta hardcodeado en agregar_mensaje();
                sem_post(&binarioE);
                idMensaje= agregar_mensaje(NEW_POKEMON, lista);
                list_destroy_and_destroy_elements(lista,free);
                t_paquete* idNew= crear_paquete(MENSAJE_ID);

				agregar_a_paquete(idNew, &idMensaje, 4);
				enviar_paquete(idNew, socket);
				eliminar_paquete(idNew);
                sem_post(&binario);
                distribuidor(NEW);

                break;
            case APPEARED_POKEMON:
            	sem_wait(&agregarSuscriptor);
                lista = recibir_paquete(socket);
                sem_post(&binarioE);
                //El idMensaje esta hardcodeado en agregar_mensaje();
                idMensaje=   agregar_mensaje(APPEARED_POKEMON, lista);
                list_destroy_and_destroy_elements(lista,free);
                t_paquete* idAppeared= crear_paquete(MENSAJE_ID);
                agregar_a_paquete(idAppeared, &idMensaje, 4);
                enviar_paquete(idAppeared, socket);
                eliminar_paquete(idAppeared);
                sem_post(&binario);
                distribuidor(APPEARED);

                break;
            case CATCH_POKEMON:
            	sem_wait(&agregarSuscriptor);
                lista = recibir_paquete(socket);
                sem_post(&binarioE);
                //El idMensaje esta hardcodeado en agregar_mensaje();
                idMensaje= agregar_mensaje(CATCH_POKEMON, lista);
                list_destroy_and_destroy_elements(lista,free);
                t_paquete* idCatch= crear_paquete(MENSAJE_ID);
                agregar_a_paquete(idCatch, &idMensaje, 4);
                enviar_paquete(idCatch, socket);
                eliminar_paquete(idCatch);
                sem_post(&binario);
                distribuidor(CATCH);

                break;
            case CAUGHT_POKEMON:
            	sem_wait(&agregarSuscriptor);
                lista = recibir_paquete(socket);
                sem_post(&binarioE);
                //El idMensaje esta hardcodeado en agregar_mensaje();
                idMensaje=   agregar_mensaje(CAUGHT_POKEMON, lista);
                list_destroy_and_destroy_elements(lista,free);
       		    t_paquete* idCaught= crear_paquete(MENSAJE_ID);
                agregar_a_paquete(idCaught, &idMensaje,4);
                enviar_paquete(idCaught, socket);
                eliminar_paquete(idCaught);
                sem_post(&binario);
                distribuidor(CAUGHT);

                break;
            case GET_POKEMON:
            	sem_wait(&agregarSuscriptor);
                lista = recibir_paquete(socket);
                sem_post(&binarioE);
                //El idMensaje esta hardcodeado en agregar_mensaje();
                idMensaje=    agregar_mensaje(GET_POKEMON, lista);
                list_destroy_and_destroy_elements(lista,free);
                t_paquete* idGet= crear_paquete(MENSAJE_ID);
                agregar_a_paquete(idGet, &idMensaje,4);
                enviar_paquete(idGet, socket);
                eliminar_paquete(idGet);
                sem_post(&binario);
                distribuidor(GET);

                break;
            case LOCALIZED_POKEMON:
            	sem_wait(&agregarSuscriptor);
                lista = recibir_paquete(socket);
                sem_post(&binarioE);
                //El idMensaje esta hardcodeado en agregar_mensaje();
                idMensaje=   agregar_mensaje(LOCALIZED_POKEMON, lista);
                list_destroy_and_destroy_elements(lista,free);
       		    t_paquete* idLocalized= crear_paquete(MENSAJE_ID);
                agregar_a_paquete(idLocalized, &idMensaje,4);
                enviar_paquete(idLocalized, socket);
                eliminar_paquete(idLocalized);

                sem_post(&binario);
                distribuidor(LOCALIZED);

                break;
            case DESENLAZAR: //cambiar por desconexion, se depurara cuando un team complete sus objetivos
            	lista = recibir_paquete(socket);
            	log_trace(logB,"\tEl cliente %d se desenlazo. Liberando todas las estructuras asociadas...",(*(int*)list_get(lista,0)));
                depurar_suscriptor((*(int*)list_get(lista,0)));
                ////mostrar_elementos(APPEARED, SUSCRIPTORES);
                list_destroy_and_destroy_elements(lista,free);
                liberar_cliente(socket);
                //printf("------------------------------------------------------------------\n");
                return 0;
            case -1:
            	log_warning(logB,"\tDesconexion de socket: %d ...",socket);
    		    liberar_cliente(socket);
			    return 0;
			    break;
            default: log_error(logB,"\tOPERACION DESCONOCIDA: gestionar_cliente (%d).", operacion);
            break;
		}
	}
}
//para llamar al sistema killall -s SIGUSR1 broker
void signalHandler(int signalNum)
{
  if (signalNum == SIGUSR1)
  {
	log_trace(logB,"\tDUMP CACHE\n");
    dumpCache();
  }
}

int main(int cantidad,char* argumentos[])
{

	archivoDeConfiguracion = (char*)malloc(30);
	memcpy(archivoDeConfiguracion, argumentos[1], strlen(argumentos[1]) + 1);

    inicializar_broker();

	while (1)
	{
		/* Call in main as follows */
		signal(SIGUSR1, signalHandler);
		int cliente=-1;
		cliente = esperar_cliente(servidor);

		if(cliente!=-1)
		{
			pthread_create(&thread0, NULL, gestionarCliente,(void*) cliente);
			pthread_detach(thread0);

//			pthread_join(thread0,NULL);
//		    liberar_cliente(cliente);
		}
	}
    pthread_join(thread0,NULL);
  liberar_servidor(servidor);
}
