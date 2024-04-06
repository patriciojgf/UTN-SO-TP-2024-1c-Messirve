#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <utils/logconfig.h>
#include <utils/conexiones.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <utils/queues.h>


t_log* logger_kernel;
t_config* config_kernel;
t_pcb* proceso_exec;
sem_t planificadores;

int socket_IO;
int socket_memoria;
int socket_dispatch;
int socket_interrupt;
int id_counter;
int grado_multiprogramacion;
bool ejecucion_pausada, volvio_pcb_cpu;
int tam_pag;

int conectarCpuDispatch();
int conectarCpuInterrupt();
int conectarMemoria();
int conectarIO();

#endif
