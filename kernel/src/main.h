#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <pthread.h> 
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <sys/types.h>
// #include <utils/queues.h>
#include <consola.h>
#include <init_estructuras.h>
#include <configuracion_kernel.h>
#include "gestion_conexiones.h"

int conexiones;
int planificacion_detenida = 1;

t_log* logger_kernel;
t_config* config_kernel;

t_pcb* proceso_en_exec;

//-----Planificacion---
sem_t planificadores;
int GRADO_MULTIPROGRAMACION;
t_planificacion ALGORITMO_PLANIFICACION;
int QUANTUM;



//------IP y PUERTOS----------
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* IP_CPU;
char* PUERTO_CPU_DISPATCH;
char* PUERTO_CPU_INTERRUPT;
char* PUERTO_ESCUCHA;



//-----HILOS CONEXIONES ------//
pthread_t hilo_cpu_dispatch;

pthread_t hilo_gestionar_memoria;
pthread_t hilo_gestionar_dispatch;
pthread_t hilo_gestionar_interrupt;

pthread_t hilo_esperar_quantum;

// ------ Listas ------
t_list* lista_plan_new;
t_list* lista_plan_ready;
t_list* lista_plan_ready_vrr; //main.h
// t_list* lista_plan_execute;
t_list* lista_plan_blocked;
t_list* lista_plan_exit;
//-----
t_list* lista_recursos;
t_list* lista_archivos_abiertos;
t_list* lista_instrucciones_permitidas;

t_pcb* proceso_exec;

//--- Estructuras -----
char** RECURSOS;
char** INSTANCIAS_RECURSOS;


//armo estructura lista de tipo t_interfaz para guardar todas las que llegan
t_list* lista_interfaz_socket;
sem_t mlog, m_multiprogramacion, s_init_proceso_a_memoria, s_memoria_liberada_pcb;
sem_t sem_pcb_desalojado;
sem_t sem_plan_exec_libre;
sem_t sem_plan_ready;
sem_t sem_planificacion_activa;

// ------ PTHREAD_MUTEX ------

//semaforos para marcar conexiones existosas
sem_t s_conexion_memoria_ok;
sem_t s_conexion_cpu_i_ok;
sem_t s_conexion_cpu_d_ok;
sem_t s_conexion_interfaz;

//mutex
pthread_mutex_t mutex_conexiones;
//Listas planificador
pthread_mutex_t mutex_plan_new;
pthread_mutex_t mutex_plan_ready;
pthread_mutex_t mutex_plan_ready_vrr;
pthread_mutex_t mutex_plan_exec;
pthread_mutex_t mutex_plan_blocked;
pthread_mutex_t mutex_plan_exit;
pthread_mutex_t mutex_procesos_planificados;
pthread_mutex_t mutex_detener_planificacion;
//pcb
pthread_mutex_t mutex_pid_proceso;
//recursos
pthread_mutex_t mutex_lista_recursos;
//listas interfaces conectadas
pthread_mutex_t mutex_lista_interfaz;

//RR
pthread_mutex_t mutex_id_ejecucion;
int var_id_ejecucion;

//pcb
int pid_proceso = 0;

int socket_servidor_io;
int socket_IO;
int socket_memoria;
int socket_dispatch;
int socket_interrupt;
int socket_servidor;
bool ejecucion_pausada, volvio_pcb_cpu;
int tam_pag;
int cantidad_procesos_planificados = 0;

int conectarCpuDispatch();
int conectarCpuInterrupt();
int conectarMemoria();
int conectarIO();
int esperar_interfaz(int socket_servidor);
int conectarInterfaz();
// int leer_consola();
void inicializar_estructuras();
void free_estructuras();

/*----------------CONSOLA----------------*/
void leerConsola();

#endif
