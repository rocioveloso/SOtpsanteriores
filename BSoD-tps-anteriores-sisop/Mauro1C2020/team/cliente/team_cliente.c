#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<commons/config.h>
#include <utils/utils.h>


int main()
{
	/*char* ip=leer_config("IP","team_cliente.config");
	char* puerto=leer_config("PUERTO","team_cliente.config");
*/

	char *ip= "127.0.0.2";
	char *puerto = "5002";

	int cliente = crear_conexion(ip, puerto);

	printf("\n\nEsperando clientes, IP: %s, PUERTO: %s, ID:%d \n", ip, puerto, cliente);
	puts("------------------------------------------------------------------");
	t_paquete* paquete = crear_paquete(SUSCRIBIR);
	int cola=(int) APPEARED_POKEMON;
	printf("ENVIO:%d\n", cola);
	agregar_a_paquete(paquete, &cola , sizeof(int));
	/*int numero = 2;
	agregar_a_paquete(paquete, &numero, sizeof(int));
	numero = 3;
	agregar_a_paquete(paquete, &numero, sizeof(int));
	numero = 56;
	agregar_a_paquete(paquete, &numero, sizeof(int));*/
	enviar_paquete(paquete, cliente);
	printf("SE ENVIO PAQUETE\n");
	eliminar_paquete(paquete);

	/*t_paquete* paquete_1 = crear_paquete(CADENA);
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
	eliminar_paquete(paquete_2);*/


	liberar_cliente(cliente);
	return 0;
}
