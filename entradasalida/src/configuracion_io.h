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
#include <dirent.h>

#define LOG_NAME "io.log"
#define PROCESS_NAME "IO"
#define MSG_ERROR "No se pudo crear correctamente. "

typedef struct{
    char* nombre;
    int tamano;
    char* path_nombre;
    t_config* metadata;
    int puntero_inicio;
    int cantidad_bloques;
} t_fs_archivo;

typedef struct {
    int tamano_bloque;
    int cantidad_bloques;
    int tamano_total_bloques;
    FILE* archivo_bloques;
    void* archivo_bloques_en_memoria;

    int tamano_bitmap;
    t_bitarray* bitmap;
    FILE* archivo_bitmap;
    void* archivo_bitmap_en_memoria;

    t_dictionary* fs_archivos;
} t_filesystem;

//FS
extern t_filesystem info_FS;

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
extern void* bitmap_void;
extern int tamanio_archivo_bloque;
extern int tamanio_archivo_bitmap;


#endif