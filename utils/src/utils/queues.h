#ifndef QUEUES_H
#define QUEUES_H

#include "estructuras.h"
#include "logconfig.h"
#include "conexiones.h"

extern t_pcb *proceso_exec;
extern int socket_dispatch, socket_interrupt,socket_memoria, socket_FS;
extern t_log *logger_kernel;
extern t_config* config_kernel;

void log_protegido(char *mensaje);

#endif// QUEUES_H