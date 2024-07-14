#ifndef STATIC_CONFIGURACION_IO_H_
#define STATIC_CONFIGURACION_IO_H_

#include <commons/bitarray.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <utils/logconfig.h>
#include <utils/conexiones.h>

#define LOG_NAME "io.log"
#define PROCESS_NAME "IO"
#define MSG_ERROR "No se pudo crear correctamente. "


//---
extern t_log* logger_io;
extern t_config* config_io;
extern char* nombre_interfaz;

//------CONFIGURACION, IP y PUERTOS----------
extern char* TIPO_INTERFAZ;
extern int TIEMPO_UNIDAD_TRABAJO;
extern char* IP_KERNEL;
extern char* PUERTO_KERNEL;
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* PATH_BASE_DIALFS;
extern int BLOCK_SIZE;
extern int BLOCK_COUNT;
extern int RETRASO_COMPACTACION;


//------Sockets----------
extern int socket_cliente_kernel;
extern int socket_cliente_memoria;

//------Hilos------------
extern pthread_t hilo_gestionar_memoria;
extern pthread_t hilo_gestionar_kernel;

//-----semaforos---------
extern sem_t sem_io_stdin_read_ok;

extern t_bitarray* bitmap_fs;

typedef struct config_io
{
    char* tipo_interfaz;
    int tiempo_unidad_trabajo;
    char* ip_kernel;
    char* puerto_kernel;
    char* ip_memoria;
    char* puerto_memoria;
    char* path_base;
    int block_size;
    int block_count;
}t_config_io;

t_config_io* iniciar_config_io(t_config* config_io);
void finalizar_config_io(t_config_io* config_io);

#endif