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
	//si agregan mas tipos de mensajes, agregarlos a la funcion texto_to_cod_op
	MENSAJE,
	PCB,
	DESALOJAR,
	KERNEL,
	KERNEL_INTERRUPT,
	KERNEL_DISPATCH,
	CPU,
	IO,
	GENERICA,
	STDIN,
	STDOUT,
	DIALFS
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

//defino la estructura t_list que guarde el socket y el nombre de la interfaz
typedef struct
{
	int socket;
	int tipo_interfaz;
	char* nombre_interfaz;
} t_socket_interfaz;


int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
void liberar_conexion(int socket_cliente);

int texto_to_cod_op(char* texto);


int iniciar_servidor(char*);
int esperar_cliente(int);
void recibir_mensaje(int,t_log*);
void* recibir_buffer(int*, int);
void eliminar_paquete(t_paquete*);
char* leer_mensaje(int socket_cliente);
int recibir_operacion(int);

void agregar_socket_a_lista(t_list* lista, t_socket_interfaz* socket);
void quitar_socket_por_nombre(t_list* lista, char* nombre);
t_socket_interfaz* buscar_socket_por_nombre(t_list* lista, char* nombre);


#endif /* CONEXIONES_H_ */