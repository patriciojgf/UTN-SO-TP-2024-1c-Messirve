#ifndef STATIC_MAIN_CPU_H_
#define STATIC_MAIN_CPU_H_
    
#include <configuracion_cpu.h>
#include <utils/conexiones.h>
#include <utils/estructuras.h>
#include "semaphore.h"

sem_t mlog;

int socket_memoria;
int socket_servidor_dispatch, socket_servidor_interrupt;
int socket_dispatch, socket_interrupt;
int motivo_interrupt,pid_interrupt;

t_log* logger_cpu;
t_config* config_cpu;
t_config_cpu* datos_cpu;

int tam_pag;
int pid;
int program_counter;

t_registros_cpu registros_cpu;

pthread_t hilo_cpu; 
pthread_t hilo_kernelDispatch;
pthread_t hilo_kernelInterrupt;

void conectarMemoria();
int conectarKernelDispatch();
int conectarKernelInterrupt();
void log_protegido_cpu(char* mensaje);
void recibir_pcb(int socket);
void desempaquetar_pcb(void *buffer);
    
#endif
