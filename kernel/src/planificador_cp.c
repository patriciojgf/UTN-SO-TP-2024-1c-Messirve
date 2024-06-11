#include "planificador_cp.h"

// static void _FIFO();
static void _esperar_liberar_quantum(t_pcb* pcb);
static void _esperar_vrr(t_pcb* pcb);
static void _quantum_wait(t_pcb* pcb);
// static void _RR();

/*------------------------------------------------------------------------------------------------------*/

void check_detener_planificador(){
    pthread_mutex_lock(&mutex_detener_planificacion);
    if(planificacion_detenida == 1){
        log_info(logger_kernel,"se activa DETENER_PLANIFICACION");
        sem_wait(&sem_planificacion_activa);
    }
    pthread_mutex_unlock(&mutex_detener_planificacion);
}


void planificador_cp(){
    //log_protegido_kernel(string_from_format("[planificador_cp]"));
    while(1){
        sem_wait(&sem_plan_ready);
        sem_wait(&sem_plan_exec_libre); //espero que haya lugar para ejecutar     
        
        //verifico si la planificacion esta activa.
        check_detener_planificador();           

        pthread_mutex_lock(&mutex_plan_ready);
        pthread_mutex_lock(&mutex_plan_ready_vrr);
        if(!list_is_empty(lista_plan_ready) || !list_is_empty(lista_plan_ready_vrr)){   
            pthread_mutex_lock(&mutex_plan_exec); //bloqueo el acceso al proceso_exec hasta que quede planificado el nuevo
            if(proceso_exec==NULL){
                t_pcb* pcb_ready = NULL;
                //busco el proximo pcb a ejecutar
                if(ALGORITMO_PLANIFICACION==VRR){
                    //valido si hay prioridad
                    if(!list_is_empty(lista_plan_ready_vrr)){
                        pcb_ready = list_remove(lista_plan_ready_vrr, 0); //saco el pcb de ready para pasarlo a exec
                        log_info(logger_kernel,"[PLANIFICADOR_CP] - hay PCB en lista ready vrr - quantum restante: %d",pcb_ready->quantum);
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

void desbloquar_proceso(int pid){
    //log_protegido_kernel(string_from_format("[desbloquar_proceso] - PID %d", pid));
    pthread_mutex_lock(&mutex_plan_blocked);

    t_pcb* pcb_desbloqueado = buscar_pcb_por_pid(pid, lista_plan_blocked);    
    if(pcb_desbloqueado != NULL){
        //log_protegido_kernel(string_from_format("Desbloqueo - PID %d con PC %d", pcb_desbloqueado->pid, pcb_desbloqueado->program_counter));
        pcb_desbloqueado->estado_anterior = pcb_desbloqueado->estado_actual;
        pcb_desbloqueado->estado_actual = estado_READY;
        list_remove_element(lista_plan_blocked, pcb_desbloqueado);

        pthread_mutex_lock(&mutex_plan_ready);
        list_add(lista_plan_ready, pcb_desbloqueado);
        pthread_mutex_unlock(&mutex_plan_ready);
        sem_post(&sem_plan_ready);

        pthread_mutex_unlock(&mutex_plan_blocked);

        //planificador_cp();
        //sem_post(&sem_plan_exec_libre); //activo el planificador de corto plazo
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
    log_warning(logger_kernel,"_quantum_wait");
    log_warning(logger_kernel,"_quantum disponible: %d", pcb->quantum);
    usleep(pcb->quantum*1000);
    log_warning(logger_kernel,"fin de quantum");
    //log_protegido_kernel(string_from_format("[_quantum_wait] - PID %d", pcb->pid));
    envio_interrupcion(pcb->pid, FIN_QUANTUM);
}

static void _esperar_liberar_quantum(t_pcb* pcb){
    log_warning(logger_kernel,"hilo_esperar_quantum");
    //armo hilo pthread_create llamando a _quantum_wait y pasandole pcb como parametro
    pthread_create(&hilo_esperar_quantum, NULL, (void*)_quantum_wait, pcb);
    pthread_detach(hilo_esperar_quantum);

    sem_wait(&sem_pcb_desalojado);
    
    log_warning(logger_kernel,"libero quantum, ya se desalojo el proceso");
    pthread_cancel(hilo_esperar_quantum);
}

static void _esperar_vrr(t_pcb* pcb){
    log_warning(logger_kernel,"_esperar_vrr");
    t_temporal* quantum_usado = temporal_create();
    _esperar_liberar_quantum(pcb);
    temporal_stop(quantum_usado);
    int ms_quantum_usado = temporal_gettime(quantum_usado);
    temporal_destroy(quantum_usado);
    if(ms_quantum_usado < QUANTUM){
        pcb->quantum = QUANTUM-ms_quantum_usado;
    }
}