#include "planificador_cp.h"


static void _FIFO();

void planificador_cp(){
    log_warning(logger_kernel, "planificador_cp");
    pthread_mutex_lock(&mutex_plan_ready);
    if(!list_is_empty(lista_plan_ready)){
        log_warning(logger_kernel, "Falta setear ALGORITMO_PLANIFICACION al inciar el kernel");
        pthread_mutex_unlock(&mutex_plan_ready);
        //switch (ALGORITMO_PLANIFICACION)
        _FIFO();
    }
    else {
        pthread_mutex_unlock(&mutex_plan_ready);
    }
}

static void _FIFO(){
    log_warning(logger_kernel, "_FIFO");
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
            log_warning(logger_kernel, "pcb_ready != NULL");
            //cambio estado a EXEC
            pcb_ready->estado_anterior = pcb_ready->estado_actual;
            pcb_ready->estado_actual = estado_EXEC;
            list_add(lista_plan_execute, pcb_ready);
            //log_info(logger_kernel, "Se mueve el proceso %d de READY a EXEC", pcb_ready->pid);
            enviar_mensaje("SOY KERNEL DISPATCH",socket_dispatch);
            log_warning(logger_kernel, "list_add(lista_plan_execute, pcb_ready)");
            enviar_contexto_dispatch(pcb_ready); //envio el pcb a CPU
            log_warning(logger_kernel, "Agregar envio por dispatcher a CPU");
            enviar_mensaje("SOY KERNEL DISPATCH",socket_dispatch);
            //
        }
        else{
            log_warning(logger_kernel, "No hay procesos en READY");
        }

    pthread_mutex_unlock(&mutex_plan_exec);
    }
}