#ifndef PLANIFICADOR_CP_H_  
#define PLANIFICADOR_CP_H_

#include <utils/logconfig.h>
#include <pthread.h>
#include <semaphore.h>
#include <utils/estructuras.h>

// ------ PTHREAD_MUTEX ------
extern pthread_mutex_t mutex_plan_new;
extern pthread_mutex_t mutex_plan_ready;
extern pthread_mutex_t mutex_plan_exec;
extern pthread_mutex_t mutex_plan_blocked;
extern pthread_mutex_t mutex_plan_exit;

// ------ Listas ------
extern t_list* lista_plan_new;
extern t_list* lista_plan_ready;
extern t_list* lista_plan_execute;
extern t_list* lista_plan_blocked;
extern t_list* lista_plan_exit;

//ver si quedan aca
extern t_log* logger_kernel;


//funciones
void planificador_cp();


#endif /* PLANIFICADOR_CP_H_ */