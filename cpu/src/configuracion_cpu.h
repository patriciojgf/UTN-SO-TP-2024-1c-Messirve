#ifndef STATIC_CONFIGURACION_CPU_H_  
#define STATIC_CONFIGURACION_CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <utils/logconfig.h>
#include <utils/conexiones.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>

#define LOG_NAME "cpu.log"
#define PROCESS_NAME "CPU"
#define MSG_ERROR "No se pudo crear correctamente. "

extern t_log* logger_cpu;
extern t_config* config_cpu;

//semaforos
extern sem_t mlog;
extern sem_t s_instruccion_actual;
extern sem_t s_signal_kernel;
extern sem_t s_fetch_espere_instruccion;

//mutex
extern pthread_mutex_t mutex_ejecucion_proceso;

//flags
extern bool flag_ejecucion,flag_interrupt;

//contexto
extern char* instruccion_actual;
extern int motivo_interrupt;
extern int pid_interrupt;

//------IP y PUERTOS----------
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* PUERTO_ESCUCHA_DISPATCH;
extern char* PUERTO_ESCUCHA_INTERRUPT;
extern int CANTIDAD_ENTRADAS_TLB;
extern char* ALGORITMO_TLB;

//------Sockets---------------
extern int socket_memoria;
extern int socket_servidor_dispatch;
extern int socket_servidor_interrupt;

extern int socket_cliente_dispatch;
extern int socket_cliente_interrupt;

//------Hilos Conexiones----------
extern pthread_t hilo_gestionar_memoria;
extern pthread_t hilo_gestionar_dispatch;
extern pthread_t hilo_gestionar_interrupt;


typedef struct config_cpu
{
    char* ip_memoria;
    char* puerto_memoria;
    char* puerto_escucha_dispatch;
    char* puerto_escucha_intrerrupt;
    int cantidad_entradas_tlb;
    char* algoritmo_tlb;
}t_config_cpu;

typedef struct{
    int pid; // Identificador del proceso.
    int program_counter; // Número de la próxima instrucción a ejecutar.
    t_registros_cpu registros_cpu; // Registros de la CPU.
} t_contexto;

t_config_cpu* iniciar_config_cpu(t_config* config_cpu);
void finalizar_config_cpu(t_config_cpu* config_cpu);

#endif