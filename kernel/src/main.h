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

t_log* logger_kernel;
t_config* config_kernel;

//-----Planificacion---
sem_t planificadores;
int GRADO_MULTIPROGRAMACION;
t_planificacion ALGORITMO_PLANIFICACION;
// ------ Listas ------
t_list* lista_plan_new;
t_list* lista_plan_ready;
t_list* lista_plan_execute;
t_list* lista_plan_blocked;
t_list* lista_plan_exit;
//-----
t_list* lista_recursos;
t_list* lista_archivos_abiertos;
t_pcb* proceso_exec;

//--- Estructuras -----
char** RECURSOS;
char** INSTANCIAS_RECURSOS;


//armo estructura lista de tipo t_socket_interfaz para guardar todas las que llegan
t_list* lista_interfaz_socket;
sem_t mlog, m_multiprogramacion, s_init_proceso_a_memoria;

// ------ PTHREAD_MUTEX ------
//mutex
pthread_mutex_t mutex_conexiones;
//Listas planificador
pthread_mutex_t mutex_plan_new;
pthread_mutex_t mutex_plan_ready;
pthread_mutex_t mutex_plan_exec;
pthread_mutex_t mutex_plan_blocked;
pthread_mutex_t mutex_plan_exit;
pthread_mutex_t mutex_procesos_planificados;
//pcb
pthread_mutex_t mutex_pid_proceso;

//pcb
int pid_proceso = 0;

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
int nuevaInterfaz(int socket_cliente);
int esperar_interfaz(int socket_servidor);
int conectarInterfaz();
// int leer_consola();
void inicializar_estructuras();
void free_estructuras();


/*----------------CONSOLA----------------*/
void leerConsola();

#endif
