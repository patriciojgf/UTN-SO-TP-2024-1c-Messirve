#ifndef PLANIFICADOR_LP_H_  
#define PLANIFICADOR_LP_H_

#include <pthread.h>
#include <semaphore.h>
#include "planificador_cp.h"

//ver si quedan aca



void planificador_lp_nuevo_proceso(t_pcb* nuevo_pcb);
void planificador_lp_new_ready();

#endif /* PLANIFICADOR_LP_H_ */