#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/config.h>
#include<commons/collections/list.h>
#include<commons/string.h>
#include<assert.h>
#include<signal.h>
#include<string.h>


typedef enum
{
	MENSAJE,
	PCB,
	DESALOJAR,
	KERNEL,
	KERNEL_INTERRUPT,
	KERNEL_DISPATCH,
	CPU,
	IO
    /*agregar el resto*/
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;


int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
void liberar_conexion(int socket_cliente);


int iniciar_servidor(char*);
int esperar_cliente(int);
void recibir_mensaje(int,t_log*);
void* recibir_buffer(int*, int);
void eliminar_paquete(t_paquete*);

int recibir_operacion(int);

#endif /* CONEXIONES_H_ */