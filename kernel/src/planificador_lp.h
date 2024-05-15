#ifndef PLANIFICADOR_LP_H_  
#define PLANIFICADOR_LP_H_

#include <utils/logconfig.h>
#include <pthread.h>
#include <semaphore.h>
#include "planificador_cp.h"

// ------ PTHREAD_MUTEX ------
//Listas planificador
pthread_mutex_t mutex_plan_new;
pthread_mutex_t mutex_plan_ready;
pthread_mutex_t mutex_plan_exec;
pthread_mutex_t mutex_plan_blocked;
pthread_mutex_t mutex_plan_exit;

pthread_mutex_t mutex_procesos_planificados; //mutex_core

// ------ Listas ------
t_list* lista_plan_new;
t_list* lista_plan_ready;
t_list* lista_plan_execute;
t_list* lista_plan_blocked;
t_list* lista_plan_exit;

//ver si quedan aca
int cantidad_procesos_planificados = 0;
extern t_log* logger_kernel;

//externo
extern int grado_multiprogramacion;


void planificador_lp_nuevo_proceso(t_pcb* nuevo_pcb);
void planificador_lp_new_ready();

#endif /* PLANIFICADOR_LP_H_ */