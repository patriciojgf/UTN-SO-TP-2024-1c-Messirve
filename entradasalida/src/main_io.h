#ifndef STATIC_MAIN_IO_H_
#define STATIC_MAIN_IO_H_

#include <configuracion_io.h>
#include <utils/conexiones.h>
#include "semaphore.h"
#include "init_estructuras.h"
#include "gestion_conexiones.h"


int socket_memoria;
int socket_servidor_kernel;

char* nombre_interfaz;

t_list* lista_interfaz_socket;

//----LOG y CONFIGS-----//
t_log* logger_io;
t_config* config_io;

//------CONFIGURACION, IP y PUERTOS----------
char* TIPO_INTERFAZ;
int TIEMPO_UNIDAD_TRABAJO;
char* IP_KERNEL;
char* PUERTO_KERNEL;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PATH_BASE_DIALFS;
int BLOCK_SIZE;
int BLOCK_COUNT;
int RETRASO_COMPACTACION;

//------Sockets----------
int socket_cliente_kernel;
int socket_cliente_memoria;

//------Hilos------------
pthread_t hilo_gestionar_memoria;
pthread_t hilo_gestionar_kernel;

//-----semaforos---------
sem_t sem_io_stdin_read_ok;

////----mutex
pthread_mutex_t mutex_lista_interfaz;

t_config_io* datos_io;

pthread_t hilo_kernel;
pthread_t hilo_memoria;

t_bitarray* bitmap_fs;

int conectarMemoria();
int conectarKernel();

#endif