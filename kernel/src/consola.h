#ifndef STATIC_CONSOLA_H_
#define STATIC_CONSOLA_H_

#include <commons/collections/list.h>
#include <commons/error.h>
#include <commons/log.h>
#include <commons/string.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <signal.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "configuracion_kernel.h"
#include "kernel_pcb.h"
#include "planificador_lp.h"
#include "init_estructuras.h"
    
extern sem_t mlog;
extern t_log* logger_kernel;

typedef struct comando
{
    int cod_op;
    t_list* parametros; 
}t_comando;

void cambiar_multiprogramacion(int nuevo_grado_mult, sem_t m_multiprogramacion);
void leer_consola(sem_t m_multiprogramacion);
void imprimir_comandos_permitidos();
void interpretar(t_comando* comando, char* leido);
void _setup_parametros(t_comando* comando, char** leido_separado, int cod_op);
#endif