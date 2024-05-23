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
#include "mem_estructuras.h"
#include "atender_cpu.h"
#include "proceso.h"

/*-----LOGS y CONFIG-----*/
t_log* logger_memoria;
t_config* config_memoria;

/*----CONEXIONES---------*/
int socket_peticion_FS;
int socket_servidor, socket_cliente, socket_kernel, socket_cpu, socket_io;

/*----PROCESOS e INSTRUCCIOENS----*/
t_list* lista_procesos_en_memoria;
t_list* lista_instrucciones;

/*----MEMORIA USUARIO-------------*/
void *memoria_espacio_usuario;

void log_protegido_mem(char* mensaje);
int conectarKernel(int* socket_kernel);
int conectarCpu(int* socket_cpu);
int conectarIO(int* socket_io);

#endif // MAIN_H