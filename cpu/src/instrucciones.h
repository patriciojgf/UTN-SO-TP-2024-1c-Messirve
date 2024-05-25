#ifndef INSTRUCCIONES_H
#define INSTRUCCIONES_H

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>
#include <pthread.h>
#include <semaphore.h>
#include "configuracion_cpu.h"

extern int pid;
extern int tam_pag;
extern int program_counter;
extern int socket_memoria, socket_servidor_dispatch;
extern t_registros_cpu registros_cpu;
extern bool flag_ejecucion, flag_interrupt;
extern t_log* logger_cpu;
extern sem_t mlog;
extern t_contexto* contexto_cpu;

void f_exit();
char* fetch_instruccion();
t_instruccion* decodificar_instruccion(char* instruccion);
t_instruccion* execute_instruccion(t_instruccion* instruccion);
void devolver_contexto(int motivo, t_instruccion* instruccion);


#endif // INSTRUCCIONES_H