#ifndef CONFIG_KERNEL_H
#define CONFIG_KERNEL_H
#include <utils/conexiones.h>
#include <utils/logconfig.h>
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <sys/types.h>

typedef enum{
    FIFO,
    RR,
    VRR
} t_planificacion;

//--- Consola -----
typedef enum{
    EJECUTAR_SCRIPT,
    INICIAR_PROCESO,
    FINALIZAR_PROCESO,
    DETENER_PLANIFICACION,
    INICIAR_PLANIFICACION,
    MULTIPROGRAMACION,
    PROCESO_ESTADO,
    HELPER
} t_codigo_consola;

typedef struct
{
    char* nombre;
    t_codigo_consola cod_identificador; //identificador de la instruccion
    int cantidad_parametros; //cantidad de parametros que tiene la instruccion
} t_instruccion_consola;

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
extern pthread_t hilo_esperar_quantum;

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
extern sem_t sem_multiprogramacion;
extern sem_t s_init_proceso_a_memoria; //se crearon las estructuras de memoria
extern sem_t s_memoria_liberada_pcb; //se liberaron las estructuras de memoria 

//semaforos para marcar conexiones existosas
extern sem_t s_conexion_memoria_ok;
extern sem_t s_conexion_cpu_d_ok;
extern sem_t s_conexion_cpu_i_ok;
extern sem_t s_conexion_interfaz; //para la primer interfaz

//planificacion
extern sem_t sem_pcb_desalojado;
extern sem_t s_pedido_io_enviado;
extern sem_t sem_plan_exec_libre;
extern sem_t sem_plan_ready;
extern sem_t sem_plan_new;
extern sem_t sem_planificacion_activa;

//mutex
extern pthread_mutex_t mutex_conexiones; //main.h
//Listas planificador
extern pthread_mutex_t mutex_plan_new;
extern pthread_mutex_t mutex_plan_ready;
extern pthread_mutex_t mutex_plan_ready_vrr;
extern pthread_mutex_t mutex_plan_exec;
extern pthread_mutex_t mutex_plan_blocked;
extern pthread_mutex_t mutex_plan_exit;
extern pthread_mutex_t mutex_procesos_planificados; 
extern pthread_mutex_t mutex_detener_planificacion;
extern pthread_mutex_t mutex_grado_multiprogramacion;
extern pthread_mutex_t mutex_finalizar_proceso;
//pcb
extern pthread_mutex_t mutex_pid_proceso;
//recursos
extern pthread_mutex_t mutex_lista_recursos;
//listas interfaces conectadas
extern pthread_mutex_t mutex_lista_interfaz;

//RR
extern pthread_mutex_t mutex_id_ejecucion;
extern int var_id_ejecucion;

//pcb
extern int pid_proceso;

//planificacion
extern int planificacion_detenida;
extern bool proceso_finalizando;

extern int GRADO_MULTIPROGRAMACION;
extern int cantidad_procesos_planificados;
extern t_planificacion ALGORITMO_PLANIFICACION; //main.h
extern int QUANTUM;
extern t_list* lista_plan_new; //main.h
extern t_list* lista_plan_ready; //main.h
extern t_list* lista_plan_ready_vrr; //main.h
// extern t_list* lista_plan_execute; //main.h
extern t_list* lista_plan_blocked; //main.h
extern t_list* lista_plan_exit; //main.h
extern t_list* lista_recursos; //main.h
extern t_list* lista_archivos_abiertos; //main.h
extern t_list* lista_instrucciones_permitidas; //consola.h

//--- Estructuras -----
extern char** RECURSOS;
extern char** INSTANCIAS_RECURSOS;

#endif /* CONFIG_KERNEL_H */