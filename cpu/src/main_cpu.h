#ifndef STATIC_MAIN_CPU_H_
#define STATIC_MAIN_CPU_H_
    
// #include <configuracion_cpu.h>
#include <instrucciones.h>
#include "configuracion_cpu.h"
#include "semaphore.h"
#include "gestion_conexiones.h"

//contexto
t_contexto* contexto_cpu;
char* instruccion_actual;
bool llego_interrupcion = 0;

//memoria
t_list* lista_tlb;
int tamanio_pagina;
int respuesta_memoria;
char* respuesta_memoria_char;

//semaforos
sem_t mlog;
sem_t s_instruccion_actual;
sem_t s_pedido_marco;
sem_t s_pedido_lectura_m;
sem_t s_pedido_escritura_m;
sem_t s_resize;
sem_t s_signal_kernel;
sem_t s_fetch_espere_instruccion;
sem_t sem_check_recibiendo_interrupcion;

//mutex
pthread_mutex_t mutex_ejecucion_proceso;
pthread_mutex_t mutex_check_recibiendo_interrupcion;

//conexiones
int socket_memoria;
int socket_servidor_dispatch;
int socket_servidor_interrupt;
int socket_cliente_dispatch;
int socket_cliente_interrupt;

//hilos atender conexiones
//pthread_t hilo_gestionar_dispatch;
pthread_t hilo_gestionar_memoria;
pthread_t hilo_gestionar_dispatch;
pthread_t hilo_gestionar_interrupt;

//int socket_dispatch;


int motivo_interrupt,pid_interrupt;
bool flag_ejecucion,flag_interrupt;
t_log* logger_cpu;
t_config* config_cpu;
t_config_cpu* datos_cpu;

//------CONFIGURACION, IP y PUERTOS----------
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA_DISPATCH;
char* PUERTO_ESCUCHA_INTERRUPT;
int CANTIDAD_ENTRADAS_TLB;
char* ALGORITMO_TLB;

int pid;
// int program_counter;

t_registros_cpu registros_cpu;

pthread_t hilo_cpu; 
pthread_t hilo_kernelDispatch;
pthread_t hilo_kernelInterrupt;

    
#endif
