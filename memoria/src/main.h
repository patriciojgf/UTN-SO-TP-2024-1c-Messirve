#ifndef MAIN_H
#define MAIN_H

#include "init_estructuras.h"
#include "configuracion_memoria.h"

#include "mem_estructuras.h"
#include "gestion_conexiones.h"
#include "atender_cpu.h"
#include "proceso.h"

//semaforos
sem_t mlog;

/*-----LOGS y CONFIG-----*/
t_log* logger_memoria;
t_config* config_memoria;
char* PUERTO_ESCUCHA;
int TAM_MEMORIA;
int TAM_PAGINA;
char* PATH_INSTRUCCIONES;
int RETARDO_RESPUESTA;


/*----CONEXIONES---------*/
pthread_t hilo_gestionar_cpu;
pthread_t hilo_gestionar_kernel;
//version nueva
int socket_servidor_escucha;
int socket_cliente_cpu;
int socket_cliente_kernel;
int socket_cliente_io;

//version vieja
int socket_peticion_FS;
int socket_servidor, socket_cliente, socket_kernel, socket_cpu, socket_io;


/*----PROCESOS e INSTRUCCIOENS----*/
t_list* lista_procesos_en_memoria;
t_list* lista_instrucciones;
t_list* lista_interfaz_socket;


////----mutex
pthread_mutex_t mutex_lista_interfaz;

/*----MEMORIA USUARIO-------------*/
void *memoria_espacio_usuario;

int conectarKernel(int* socket_kernel);
int conectarCpu(int* socket_cpu);
int conectarIO(int* socket_io);

#endif // MAIN_H