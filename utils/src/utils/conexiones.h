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
#include"estructuras.h"

typedef enum
{
	//si agregan mas tipos de mensajes, agregarlos a la funcion texto_to_cod_op
	MENSAJE,
	PCB,
	CONTEXTO_EJECUCION,
	DESALOJAR,
	KERNEL,
	KERNEL_INTERRUPT,
	KERNEL_DISPATCH,
	CPU,
	IO,
	GENERICA,
	STDIN,
	STDOUT,
	DIALFS,
	INICIAR_PLANIFICACION,
	DETENER_PLANIFICACION,
	MULTIPROGRAMACION,
	INICIAR_PROCESO,
	FINALIZAR_PROCESO,
	PROCESO_ESTADO,
	EJECUTAR_SCRIPT,
	FETCH_INSTRUCCION,

/*MENSAJES - KERNEL-MEMRIA -INICIO*/
	INICIAR_PROCESO_MEMORIA,
	INICIAR_PROCESO_MEMORIA_OK,
/*MENSAJES - KERNEL-MEMRIA -FIN*/

/*INSTRUCCIONES - INICIO*/
	NO_RECONOCIDO,
	SET,
	MOV_IN,
	MOV_OUT,
	SUM,
	SUB,
	JNZ,
	RESIZE,
	COPY_STRING,
	WAIT,
	SIGNAL,
	IO_GEN_SLEEP,
	IO_STDIN_READ,
	IO_STDOUT_WRITE,
	IO_FS_CREATE,
	IO_FS_DELETE,
	IO_FS_TRUNCATE,
	IO_FS_WRITE,
	IO_FS_READ,
	EXIT
/*INSTRUCCIONES - FIN*/
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
char* leer_mensaje(int socket_cliente);
int recibir_operacion(int);

void agregar_socket_a_lista(t_list* lista, t_socket_interfaz* socket);
void quitar_socket_por_nombre(t_list* lista, char* nombre);
t_socket_interfaz* buscar_socket_por_nombre(t_list* lista, char* nombre);


/*Paquetes*/
void agregar_datos_sin_tamaño_a_paquete(t_paquete* paquete, void* datos, int size);
void eliminar_paquete(t_paquete*);
t_paquete *crear_paquete(int codigo_operacion);
void enviar_paquete(t_paquete* paquete, int socket_cliente);


/*Conexion con CPU + Paquetes - INICIO*/

void empaquetar_registros_cpu(t_paquete* paquete_contexto, t_registros_cpu registros_cpu);
void empaquetar_instruccion_cpu(t_paquete* paquete_contexto, t_instruccion* instruccion);
void empaquetar_contexto_cpu(t_paquete* paquete_contexto, t_instruccion* instruccion, int pid, t_registros_cpu registros_cpu);
void desempaquetar_contexto_cpu(t_paquete* paquete_contexto, t_instruccion* instruccion, int* pid, t_registros_cpu* registros_cpu);


#endif /* CONEXIONES_H_ */