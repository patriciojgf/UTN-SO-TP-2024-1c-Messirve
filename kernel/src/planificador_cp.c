#include "planificador_cp.h"

// static void _FIFO();
static void _esperar_liberar_quantum(t_pcb* pcb);
static void _esperar_vrr(t_pcb* pcb);
static void _quantum_wait(t_pcb* pcb);
// static void _RR();

/*------------------------------------------------------------------------------------------------------*/

char* estado_string(t_estado estado) {
    switch (estado) {
        case estado_NEW:
            return "NEW";
        case estado_READY:
            return "READY";
        case estado_EXEC:
            return "EXEC";
        case estado_BLOCKED:
            return "BLOCKED";
        case estado_EXIT:
            return "EXIT";
        default:
            return "UNKNOWN";
    }
}

void check_detener_planificador(){
    pthread_mutex_lock(&mutex_detener_planificacion);
    if(planificacion_detenida == 1){
        log_info(logger_kernel,"se activa DETENER_PLANIFICACION");
        sem_wait(&sem_planificacion_activa);
    }
    pthread_mutex_unlock(&mutex_detener_planificacion);
}


void planificador_cp(){
    proceso_exec = NULL;
    //log_protegido_kernel(string_from_format("[planificador_cp]"));
    while(1){
        // log_info(logger_kernel,"planificador_cp: while");
        sem_wait(&sem_plan_ready);
        // log_info(logger_kernel,"planificador_cp: sem_plan_ready");
        sem_wait(&sem_plan_exec_libre); //espero que haya lugar para ejecutar     
        // log_info(logger_kernel,"planificador_cp: sem_plan_exec_libre");
        
        //verifico si la planificacion esta activa.
        check_detener_planificador();

        pthread_mutex_lock(&mutex_plan_ready);    
        // log_info(logger_kernel,"planificador_cp: mutex_plan_ready");
        pthread_mutex_lock(&mutex_plan_ready_vrr);
        // log_info(logger_kernel,"planificador_cp: mutex_plan_ready_vrr");
        if(!list_is_empty(lista_plan_ready) || !list_is_empty(lista_plan_ready_vrr)){   
            pthread_mutex_lock(&mutex_plan_exec); //bloqueo el acceso al proceso_exec hasta que quede planificado el nuevo
            if(proceso_exec==NULL){
                t_pcb* pcb_ready = NULL;
                //busco el proximo pcb a ejecutar
                if(ALGORITMO_PLANIFICACION==VRR){
                    //valido si hay prioridad
                    if(!list_is_empty(lista_plan_ready_vrr)){
                        pcb_ready = list_remove(lista_plan_ready_vrr, 0); //saco el pcb de ready para pasarlo a exec
                    }
                    else{
                        pcb_ready = list_remove(lista_plan_ready, 0); //saco el pcb de ready para pasarlo a exec
                    }
                } else if ((ALGORITMO_PLANIFICACION==RR)||(ALGORITMO_PLANIFICACION==FIFO)){
                    pcb_ready = list_remove(lista_plan_ready, 0); //saco el pcb de ready para pasarlo a exec
                } else{
                    log_error(logger_kernel,"ALGORITMO_PLANIFICACION no reconocido");
                    exit(EXIT_FAILURE);
                }
                pthread_mutex_unlock(&mutex_plan_ready_vrr);         
                pthread_mutex_unlock(&mutex_plan_ready);

                //preparo PCB y envio a CPU
                if(pcb_ready != NULL){
                    pcb_ready->estado_anterior = pcb_ready->estado_actual;
                    pcb_ready->estado_actual = estado_EXEC;
                    
                    proceso_exec = pcb_ready;
                    //Log obligatorio
                    log_info(logger_kernel, "Cambio de Estado: PID: %d - Estado Anterior: %s - Estado Actual: %s", proceso_exec->pid, estado_string(proceso_exec->estado_anterior), estado_string(proceso_exec->estado_actual));
                    enviar_contexto_dispatch(proceso_exec); //envio el pcb a CPU
                    if(ALGORITMO_PLANIFICACION==RR){
                        _esperar_liberar_quantum(proceso_exec);
                    }
                    else if(ALGORITMO_PLANIFICACION==VRR){
                        _esperar_vrr(proceso_exec);
                    }
                }
                else{
                    log_error(logger_kernel,"[planificador_cp]: No hay procesos para ejecutar");
                    exit(EXIT_FAILURE);
                }
            }
        }
        else {
            pthread_mutex_unlock(&mutex_plan_ready_vrr);
            pthread_mutex_unlock(&mutex_plan_ready);
            log_error(logger_kernel,"[planificador_cp]: Hay nada en Ready para planificar");
            exit(EXIT_FAILURE);
        }    
        //log_protegido_kernel(string_from_format("[planificador_cp] desbloqueo mutex_plan_exec"));
        pthread_mutex_unlock(&mutex_plan_exec); //bloqueo el acceso al proceso_exec hasta que quede planificado el nuevo
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

/*--------------------------------------------*/

// static void _FIFO(){    
//     if(proceso_exec==NULL){
//         //log_protegido_kernel(string_from_format("[_FIFO]: Hay lugar para planificar"));
//         t_pcb* pcb_ready = NULL;        
        
//         pthread_mutex_lock(&mutex_plan_ready);//bloqueo la lista de ready para poder validar que haya pendientes
//         if(!list_is_empty(lista_plan_ready)){
//             pcb_ready = list_remove(lista_plan_ready, 0); //saco el pcb de ready para pasarlo a exec
//         }
//         pthread_mutex_unlock(&mutex_plan_ready);

//         if(pcb_ready != NULL){
//             //log_protegido_kernel(string_from_format("[_FIFO] - PCB a planificar: PID <%d>",pcb_ready->pid));
//             pcb_ready->estado_anterior = pcb_ready->estado_actual;
//             pcb_ready->estado_actual = estado_EXEC;
//             proceso_exec = pcb_ready;
//             enviar_contexto_dispatch(proceso_exec); //envio el pcb a CPU
//         }
//         else{
//             log_warning(logger_kernel, "No hay procesos en READY");
//         }
//     }
//     else {
//         log_error(logger_kernel,"[_FIFO]: Ya hay un proceso en EXEC : PID <%d>",proceso_exec->pid);
//         exit(EXIT_FAILURE);
//     }
// }

static void _quantum_wait(t_pcb* pcb){
    usleep(pcb->quantum*1000);
    envio_interrupcion(pcb->pid, FIN_QUANTUM);
}

static void _esperar_liberar_quantum(t_pcb* pcb){
    pthread_create(&hilo_esperar_quantum, NULL, (void*)_quantum_wait, pcb);
    pthread_detach(hilo_esperar_quantum);
    // log_info(logger_kernel,"_esperar_liberar_quantum");
    sem_wait(&sem_pcb_desalojado);
    pthread_cancel(hilo_esperar_quantum);
}

static void _esperar_vrr(t_pcb* pcb){
    t_temporal* quantum_usado = temporal_create();
    _esperar_liberar_quantum(pcb);
    temporal_stop(quantum_usado);
    int ms_quantum_usado = temporal_gettime(quantum_usado);
    temporal_destroy(quantum_usado);
    if(ms_quantum_usado < QUANTUM){
        pcb->quantum = QUANTUM-ms_quantum_usado;
    }
}