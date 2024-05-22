#include "planificador_lp.h"

void planificador_lp_nuevo_proceso(t_pcb* nuevo_pcb){
    if(nuevo_pcb != NULL){
        //Agrego PCB a NEW
		pthread_mutex_lock(&mutex_plan_new);
		list_add(lista_plan_new, nuevo_pcb);
		//nuevo_pcb->estado = NEW; //falta agregar estado a t_pcb
		pthread_mutex_unlock(&mutex_plan_new);
		
        //log_info(logger_kernel, "Se crea el proceso %d en NEW", nuevo_pcb->pid);
    }
    pthread_t hilo_planificador; //ver si esta bien aca
    pthread_create(&hilo_planificador, NULL, (void*)planificador_lp_new_ready, NULL);
    pthread_detach(hilo_planificador);
}

void planificador_lp_new_ready(){
    t_pcb* pcb_en_new = NULL;

    pthread_mutex_lock(&mutex_procesos_planificados); //cantidad de procesos en el circuito
    pthread_mutex_lock(&mutex_plan_new); //mutex para lista NEW
    if(GRADO_MULTIPROGRAMACION > cantidad_procesos_planificados && !list_is_empty(lista_plan_new)){        
        pcb_en_new = list_remove(lista_plan_new, 0); //Busco el primer proceso en NEW por FIFO

        if(pcb_en_new != NULL){
            //Agregar el pedido de inicializar, y esperar respuesta de memoria
            log_warning(logger_kernel, "Agregar envio de proceso a memoria para inicializar estructuras");

            //Agrego PCB a READY
            pthread_mutex_lock(&mutex_plan_ready);
            //cambio estado a READY
            //pcb_en_new->estado = READY;
            list_add(lista_plan_ready, pcb_en_new);
            //log_info(logger_kernel, "Se mueve el proceso %d de NEW a READY", pcb_en_new->pid);
            cantidad_procesos_planificados++; //Aumento la cantidad de procesos en el circuito
            pthread_mutex_unlock(&mutex_plan_ready);
        }
    }
    pthread_mutex_unlock(&mutex_plan_new);
    pthread_mutex_unlock(&mutex_procesos_planificados);

}