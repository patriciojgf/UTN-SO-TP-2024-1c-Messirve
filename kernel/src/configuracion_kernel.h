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

//conexiones
extern int socket_dispatch;

//config
extern t_log* logger_kernel; //main.h
extern t_config* config_kernel; //main.h

//semaforos
extern sem_t mlog; //main.h

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

//pcb
extern int pid_proceso;

//planificacion
extern int GRADO_MULTIPROGRAMACION;
extern int cantidad_procesos_planificados;
extern t_planificacion ALGORITMO_PLANIFICACION; //main.h
extern t_list* lista_plan_new; //main.h
extern t_list* lista_plan_ready; //main.h
extern t_list* lista_plan_execute; //main.h
extern t_list* lista_plan_blocked; //main.h
extern t_list* lista_plan_exit; //main.h
extern t_list* lista_recursos; //main.h
extern t_list* lista_archivos_abiertos; //main.h

//--- Estructuras -----
extern char** RECURSOS;
extern char** INSTANCIAS_RECURSOS;


#endif /* CONFIG_KERNEL_H */