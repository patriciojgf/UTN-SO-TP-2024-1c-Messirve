#include "planificador_cp.h"


static void _FIFO();

void planificador_cp(){
    pthread_mutex_lock(&mutex_plan_ready);
    if(!list_is_empty(lista_plan_ready)){
        log_warning(logger_kernel, "Falta setear ALGORITMO_PLANIFICACION al inciar el kernel");
        //switch (ALGORITMO_PLANIFICACION)
        _FIFO();

    }
    pthread_mutex_unlock(&mutex_plan_ready);
}

static void _FIFO(){
    pthread_mutex_lock(&mutex_plan_exec); //bloqueo la lista de exec para poder vaidar
    if(list_is_empty(lista_plan_execute)){
        t_pcb* pcb_ready = list_remove(lista_plan_ready, 0); //saco el primer pcb de ready

        //bloqueo la lista de ready para poder validar que haya pendientes
        //saco el pcb de ready para pasarlo a exec
        pthread_mutex_lock(&mutex_plan_ready);
        if(!list_is_empty(lista_plan_ready)){
            pcb_ready = list_remove(lista_plan_ready, 0); 
        }
        pthread_mutex_unlock(&mutex_plan_ready);

        if(pcb_ready != NULL){
            //cambio estado a EXEC
            //pcb_ready->estado = EXEC;
            list_add(lista_plan_execute, pcb_ready);
            //log_info(logger_kernel, "Se mueve el proceso %d de READY a EXEC", pcb_ready->pid);

            log_warning(logger_kernel, "Agregar envio por dispatcher a CPU");
            //
        }
        else{
            log_warning(logger_kernel, "No hay procesos en READY");
        }

    pthread_mutex_unlock(&mutex_plan_exec);
    }
}