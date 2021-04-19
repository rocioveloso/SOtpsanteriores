#include "utils.h"

t_config* g_config = NULL;

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);

	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);

	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

char *leer_config(char* campo, char* archivo)
{
	//Estoy no anda con team.
	if (g_config==NULL) {g_config = config_create(archivo);}
	//g_config = config_create(archivo);
	char*valor = config_get_string_value(g_config, campo);
	return valor;
}

int crear_conexion(char* ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente;
	if ((socket_cliente=socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol)) == -1)
	{
		fprintf(stderr, "Error al crear el socket\n");

		return -1;
	}

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
	{
		//printf("Imposible conectar con el host\n");
		free(server_info);
		return -1;
	}

	freeaddrinfo(server_info);
	return socket_cliente;
}

int iniciar_servidor(char* IP, char* PUERTO)
{
	int socket_servidor;
	bool reusabilidad_de_puertos = true;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(IP, PUERTO, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            puts("Imposible crear un socket\n");
        	freeaddrinfo(servinfo);
        	continue;
        }

        if (setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &reusabilidad_de_puertos, sizeof(int)) == -1) {
            puts("Imposible crear la reusabilidad de puertos \n");
        	close(socket_servidor);
              freeaddrinfo(socket_servidor);
          }

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1)
        {
        	printf("Imposible asignar el puerto al servidor\n");
        	freeaddrinfo(socket_servidor);
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);
    freeaddrinfo(servinfo);
    return socket_servidor;
}


int esperar_cliente(int socket_servidor)
{
	struct sockaddr_in dir_cliente;
	int tam_direccion = sizeof(struct sockaddr_in);
	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);
	return socket_cliente;
}

void liberar_cliente(int socket_cliente)
{	close(socket_cliente);
}

void liberar_servidor(int socket_servidor)
{	close(socket_servidor);
    config_destroy(g_config);
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(op_code operacion)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = operacion;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);
	send(socket_cliente, a_enviar, bytes, 0);
	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;

}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0){
		return cod_op;
	}else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

uint32_t recibir_numero(int socket_cliente)
{
	int size;
	uint32_t recibido;
	void * buffer;

	buffer = recibir_buffer(&size, socket_cliente);

	//printf("tamaño de buffer recibido:%d\n",size);
	uint32_t valor;
	memcpy(&valor, buffer, sizeof(uint32_t));
	//printf("recibir_numero: %u\n",valor);

	free(buffer);

	return valor;
}


