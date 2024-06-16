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
	//Conexiones
	//IMPORTANTE EL ORDEN, NO DESORDENAR LOS HANDSHAKE_.... - INICIO
	HANDSHAKE_KERNEL,
	HANDSHAKE_MEMORIA,
	HANDSHAKE_CPU,
	HANDSHAKE_CPU_D,
	HANDSHAKE_CPU_I,
	HANDSHAKE_IO_GEN,
	HANDSHAKE_IO_STDIN,
	HANDSHAKE_IO_STDOUT,
	HANDSHAKE_IO_DIALFS,
	HANDSHAKE_DESCONEXION,
	HANDSHAKE_OK,
	HANDSHAKE_ERROR,
	//IMPORTANTE EL ORDEN, NO DESORDENAR LOS HANDSHAKE_.... - FIN

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
	// INICIAR_PLANIFICACION,
	// DETENER_PLANIFICACION,
	// MULTIPROGRAMACION,
	// INICIAR_PROCESO,
	// FINALIZAR_PROCESO,
	// PROCESO_ESTADO,
	// EJECUTAR_SCRIPT,
	FETCH_INSTRUCCION,
	FETCH_INSTRUCCION_RESPUESTA,
	HELPER,
	NUEVA_IO,
/*MENSAJES - KERNEL-MEMORIA - INICIO*/
	INICIAR_PROCESO_MEMORIA,
	INICIAR_PROCESO_MEMORIA_OK,
	LIBERAR_ESTRUCTURAS_MEMORIA,
	LIBERAR_ESTRUCTURAS_MEMORIA_OK,
	OBTENER_MARCO,
/*MENSAJES - KERNEL-MEMORIA - FIN*/
/*MENSAJES - CPU-MEMORIA - INICIO*/
	TAMANIO_PAGINA,
	ENVIAR_MARCO,
/*MENSAJES - CPU-MEMORIA - FIN*/

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
	EXIT,
	FIN_QUANTUM,
	INT_SIGNAL,
	INT_FINALIZAR_PROCESO
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
// typedef struct
// {
// 	int socket;
// 	int tipo_interfaz;
// 	char* nombre_interfaz;
// } t_socket_interfaz;

//---- Estructura para guardar informacion sobre la interfaz ----//
typedef struct{
    int socket;
    int tipo_io;
    char* nombre_io;
    t_list* cola_procesos; // Lista para encolar procesos que esperando una interfaz
	pthread_mutex_t mutex_cola_block;
    sem_t semaforo;// Semaforo para controlar el acceso a la cola de procesos
} t_interfaz;

typedef struct {
    t_pcb* pcb;      // Puntero al PCB del proceso
    int tiempo_sleep; // Tiempo de sleep solicitado
	sem_t semaforo_pedido_ok;
} t_pedido_sleep;

typedef struct {
    t_pcb* pcb;      // Puntero al PCB del proceso
	sem_t semaforo_pedido_ok;
} t_pedido_stdin;

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

void agregar_socket_a_lista(t_list* lista, t_interfaz* socket);
void quitar_socket_por_nombre(t_list* lista, char* nombre);
t_interfaz* buscar_socket_por_nombre(t_list* lista, char* nombre);


/*Paquetes*/
void agregar_datos_sin_tamaño_a_paquete(t_paquete* paquete, void* datos, int size);
void eliminar_paquete(t_paquete*);
t_paquete *crear_paquete(int codigo_operacion);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void* serializar_paquete(t_paquete* paquete, int bytes);

/*Conexion con CPU + Paquetes - INICIO*/

void empaquetar_registros_cpu(t_paquete* paquete_contexto, t_registros_cpu registros_cpu);
void empaquetar_instruccion_cpu(t_paquete* paquete_contexto, t_instruccion* instruccion);
void empaquetar_contexto_cpu(t_paquete* paquete_contexto, t_instruccion* instruccion, int pid,t_registros_cpu registros_cpu, int motivo);
void desempaquetar_contexto_cpu(t_paquete* paquete_contexto, t_instruccion* instruccion, int* pid, t_registros_cpu* registros_cpu);


/*------HANDSHAKE--------------*/
//static void _send_handshake(int hs_origen, int socket);
int handshake_cliente(int hs_origen, int socket);
int handshake_server(int socket);

/*-------IO------------------*/
t_paquete* empaquetar_solicitud_io(t_solicitud_io* solicitud, int motivo);
void enviar_solicitud_io(int socket, t_solicitud_io* solicitud, int motivo);
t_solicitud_io* recibir_solicitud_io(int socket);
t_solicitud_io* crear_pedido_memoria(int pid, uint32_t size_solicitud);
void agregar_a_pedido_memoria(t_solicitud_io* solicitud, char* dato, uint32_t direccion_fisica);
void eliminar_pedido_memoria(t_solicitud_io* solicitud) ;
void llenar_datos_memoria(t_solicitud_io* solicitud, char* input_text);
#endif /* CONEXIONES_H_ */