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
        log_warning(logger_kernel, "_FIFO: ya planifique");
    }
    else {
        pthread_mutex_unlock(&mutex_plan_ready);
    }
}

static void _FIFO(){
    log_warning(logger_kernel, "_FIFO");
    pthread_mutex_lock(&mutex_plan_exec); //bloqueo la lista de exec para poder vaidar
    //if(list_is_empty(lista_plan_execute)){
    if(proceso_exec==NULL){
        //t_pcb* pcb_ready = list_remove(lista_plan_ready, 0); //saco el primer pcb de ready
        t_pcb* pcb_ready = NULL;

        //bloqueo la lista de ready para poder validar que haya pendientes
        //saco el pcb de ready para pasarlo a exec
        pthread_mutex_lock(&mutex_plan_ready);
        if(!list_is_empty(lista_plan_ready)){
            pcb_ready = list_remove(lista_plan_ready, 0); 
        }
        pthread_mutex_unlock(&mutex_plan_ready);

        if(pcb_ready != NULL){
            log_warning(logger_kernel, "pcb_ready != NULL");
            pcb_ready->estado_anterior = pcb_ready->estado_actual;
            pcb_ready->estado_actual = estado_EXEC;
            proceso_exec = pcb_ready;
            enviar_contexto_dispatch(proceso_exec); //envio el pcb a CPU
            
            //
        }
        else{
            log_warning(logger_kernel, "No hay procesos en READY");
        }

    pthread_mutex_unlock(&mutex_plan_exec);
    }
    else {
        log_protegido_kernel(string_from_format("Ya hay un proceso en EXEC - PID %d", proceso_exec->pid));
        pthread_mutex_unlock(&mutex_plan_exec);
    }
}

void desbloquar_proceso(int pid){
    log_protegido_kernel(string_from_format("[desbloquar_proceso] - PID %d", pid));
    pthread_mutex_lock(&mutex_plan_blocked);

    t_pcb* pcb_desbloqueado = buscar_pcb_por_pid(pid, lista_plan_blocked);    
    if(pcb_desbloqueado != NULL){
        log_protegido_kernel(string_from_format("Desbloqueo - PID %d con PC %d", pcb_desbloqueado->pid, pcb_desbloqueado->program_counter));
        pcb_desbloqueado->estado_anterior = pcb_desbloqueado->estado_actual;
        pcb_desbloqueado->estado_actual = estado_READY;
        list_remove_element(lista_plan_blocked, pcb_desbloqueado);

        pthread_mutex_lock(&mutex_plan_ready);
        list_add(lista_plan_ready, pcb_desbloqueado);
        pthread_mutex_unlock(&mutex_plan_ready);

        pthread_mutex_unlock(&mutex_plan_blocked);

        planificador_cp();
    }
    else{
        pthread_mutex_unlock(&mutex_plan_blocked);
        log_warning(logger_kernel, "No se encontro el proceso a desbloquear");
    }

}


t_pcb* buscar_pcb_por_pid(int pid_buscado, t_list* listado_pcb){
        
	t_pcb* un_pcb;
	bool __buscar_pcb(t_pcb* void_pcb){
		if(void_pcb->pid == pid_buscado){
			return true;
		} else {
			return false;
		}
	}
	if(list_any_satisfy(listado_pcb, (void*)__buscar_pcb)){
		un_pcb = list_find(listado_pcb, (void*)__buscar_pcb);
	}
	else{
		un_pcb = NULL;
	}
	return un_pcb;
}