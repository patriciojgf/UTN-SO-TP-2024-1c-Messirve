#ifndef STATIC_CONFIGURACION_MEMORIA_H_  
#define STATIC_CONFIGURACION_MEMORIA_H_

#include <utils/logconfig.h>
#include <utils/conexiones.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

//log y config
extern t_log* logger_memoria;
extern t_config* config_memoria;
extern char* PUERTO_ESCUCHA;
extern int TAM_MEMORIA;
extern int TAM_PAGINA;
extern char* PATH_INSTRUCCIONES;
extern int RETARDO_RESPUESTA;

//semaforos
extern sem_t mlog;

/*----CONEXIONES---------*/
extern int socket_servidor_escucha;
extern int socket_cliente_cpu;
extern int socket_cliente_kernel;
extern int socket_cliente_io;

/*----HILOS--------------*/
extern pthread_t hilo_gestionar_cpu;
extern pthread_t hilo_gestionar_kernel;


/*----PROCESOS e INSTRUCCIOENS----*/
extern t_list* lista_procesos_en_memoria;
extern t_list* lista_instrucciones;
extern t_list* lista_interfaz_socket;

////----mutex
extern pthread_mutex_t mutex_lista_interfaz;

#endif /*STATIC_CONFIGURACION_MEMORIA_H_*/