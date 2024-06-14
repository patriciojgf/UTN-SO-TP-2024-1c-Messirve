#ifndef PLANIFICADOR_LP_H_  
#define PLANIFICADOR_LP_H_

#include <pthread.h>
#include <semaphore.h>
#include "planificador_cp.h"

void planificador_lp_nuevo_proceso(t_pcb* nuevo_pcb);
void planificador_lp_new_ready();
void enviar_proceso_a_memoria(t_pcb* pcb, char* path);

void desbloquar_proceso(int pid);

void mover_proceso_a_blocked(t_pcb* pcb, char* motivo);
void mover_proceso_a_ready(t_pcb* pcb);
void mover_proceso_a_exit(t_pcb* pcb);

t_recurso* obtener_recurso(char* recurso);
void liberar_estructuras_memoria(t_pcb* pcb);
void liberar_recursos_pcb(t_pcb* pcb);
void finalizar_proceso(int pid);


char* listado_pids(t_list* lista);

#endif /* PLANIFICADOR_LP_H_ */