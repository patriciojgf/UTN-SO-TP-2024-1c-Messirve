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

t_log* logger_memoria;
t_config* config_memoria;
int socket_peticion_FS;
t_list* procesos_memoria;
void *memoria;
// t_queue* cola_paginas;

void log_protegido_mem(char* mensaje);
int conectarKernel(int* socket_kernel);
int conectarCpu(int* socket_cpu);
int conectarIO(int* socket_io);

#endif // MAIN_H