#ifndef STATIC_MAIN_CPU_H_
#define STATIC_MAIN_CPU_H_
    
#include <configuracion_cpu.h>
#include <instrucciones.h>
#include "semaphore.h"

sem_t mlog;

int socket_memoria;
int socket_servidor_dispatch;
int socket_dispatch;
int motivo_interrupt,pid_interrupt;
bool flag_ejecucion,flag_interrupt;

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

int conectarMemoria();
int conectarKernelDispatch();
int conectarKernelInterrupt();
void log_protegido_cpu(char* mensaje);
static void _recibir_pcb(int socket);
static void _desempaquetar_pcb(void *buffer);
static void _check_interrupt(t_instruccion* instruccion);
static void _ejecutar_proceso();
    
#endif
