#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<commons/config.h>
#include <utils/utils.h>


int main()
{
	char* ip=leer_config("IP","gamecard.config");
	char* puerto=leer_config("PUERTO","gamecard.config");

	int cliente = crear_conexion(ip, puerto);

	printf("\n\nEsperando clientes, IP: %s, PUERTO: %s, ID:%d \n", ip, puerto, cliente);
	puts("------------------------------------------------------------------");
	t_paquete* paquete = crear_paquete(CADENA);
	agregar_a_paquete(paquete, "hola", 5);
	enviar_paquete(paquete, cliente);
	printf("SE ENVIO PAQUETE\n");
	eliminar_paquete(paquete);

	t_paquete* paquete_1 = crear_paquete(CADENA);
	char* mensaje = (char*)malloc(6);
	char* hol = "holis";
	memcpy(mensaje, hol, 6);
	agregar_a_paquete(paquete_1, mensaje, 6);
	enviar_paquete(paquete_1, cliente);
	printf("SE ENVIO PAQUETE\n");
	eliminar_paquete(paquete_1);

	t_paquete* paquete_2 = crear_paquete(ENTERO);
	int* numero = (int*)malloc(sizeof(int));
	int tres = 3;
	memcpy(numero, &tres, sizeof(int));
	agregar_a_paquete(paquete_2, (void*)numero, sizeof(int));
	printf("MENSAJE_INT %d\n", *numero);
	enviar_paquete(paquete_2, cliente);
	printf("SE ENVIO PAQUETE\n");
	eliminar_paquete(paquete_2);


	liberar_conexion(cliente);
	return 0;
}
