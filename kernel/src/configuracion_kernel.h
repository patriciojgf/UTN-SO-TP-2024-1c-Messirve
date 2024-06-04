#ifndef CONFIG_KERNEL_H
#define CONFIG_KERNEL_H
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <utils/logconfig.h>
#include <utils/conexiones.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <sys/types.h>

typedef enum{
    FIFO,
    RR,
    VRR
} t_planificacion;

// //---- Estructura para guardar informacion sobre la interfaz ----//
// typedef struct{
//     int socket;
//     char* tipo_io;
//     char* nombre_io;
//     t_list* cola_procesos; // Lista para encolar procesos que esperan IO_GEN_SLEEP
// 	pthread_mutex_t mutex_cola_block;
//     sem_t semaforo;// Semaforo para controlar el acceso a la cola de procesos
// } t_interfaz;

extern int conexiones;

//conexiones
extern int socket_dispatch;
extern int socket_memoria;
extern int socket_interrupt;
extern int socket_servidor_io;
extern t_list* lista_interfaz_socket;


extern t_pcb* proceso_exec;

//Hilos 
extern pthread_t hilo_gestionar_memoria;
extern pthread_t hilo_gestionar_dispatch;
extern pthread_t hilo_gestionar_interrupt;

//------IP y PUERTOS----------
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* IP_CPU;
extern char* PUERTO_CPU_DISPATCH;
extern char* PUERTO_CPU_INTERRUPT;
extern char* PUERTO_ESCUCHA;

//config
extern t_log* logger_kernel; //main.h
extern t_config* config_kernel; //main.h

//semaforos
extern sem_t mlog; //main.h
extern sem_t m_multiprogramacion;
extern sem_t s_init_proceso_a_memoria;

//semaforos para marcar conexiones existosas
extern sem_t s_conexion_memoria_ok;
extern sem_t s_conexion_cpu_d_ok;
extern sem_t s_conexion_cpu_i_ok;

//mutex
extern pthread_mutex_t mutex_conexiones; //main.h
//Listas planificador
extern pthread_mutex_t mutex_plan_new;
extern pthread_mutex_t mutex_plan_ready;
extern pthread_mutex_t mutex_plan_exec;
extern pthread_mutex_t mutex_plan_blocked;
extern pthread_mutex_t mutex_plan_exit;
extern pthread_mutex_t mutex_procesos_planificados; 
//pcb
extern pthread_mutex_t mutex_pid_proceso;
//listas interfaces conectadas
extern pthread_mutex_t mutex_lista_interfaz;

//pcb
extern int pid_proceso;

//planificacion
extern int GRADO_MULTIPROGRAMACION;
extern int cantidad_procesos_planificados;
extern t_planificacion ALGORITMO_PLANIFICACION; //main.h
extern t_list* lista_plan_new; //main.h
extern t_list* lista_plan_ready; //main.h
// extern t_list* lista_plan_execute; //main.h
extern t_list* lista_plan_blocked; //main.h
extern t_list* lista_plan_exit; //main.h
extern t_list* lista_recursos; //main.h
extern t_list* lista_archivos_abiertos; //main.h

//--- Estructuras -----
extern char** RECURSOS;
extern char** INSTANCIAS_RECURSOS;


#endif /* CONFIG_KERNEL_H */