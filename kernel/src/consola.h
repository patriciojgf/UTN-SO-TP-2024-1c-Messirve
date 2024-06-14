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


void procesar_comandos_consola();
void cambiar_multiprogramacion(int nuevo_grado_mult);

#endif