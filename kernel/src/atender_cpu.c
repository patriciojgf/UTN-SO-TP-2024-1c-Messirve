#include "atender_cpu.h"

static void _enviar_peticiones_io_gen(t_interfaz *interfaz);
static t_interfaz* _obtener_interfaz(char* nombre);

/*---------------------------------------------------------*/
void atender_cpu_int_finalizar_proceso(t_pcb* pcb){
    atender_cpu_exit(pcb, string_from_format("INTERRUPTED_BY_USER")); 
}


void atender_cpu_exit(t_pcb* pcb, char* motivo_exit){
 
    check_detener_planificador();   
    pthread_mutex_lock(&mutex_plan_exec);
    proceso_exec = NULL;
    pthread_mutex_unlock(&mutex_plan_exec);
    sem_post(&sem_plan_exec_libre);//activo el planificador de corto plazo

    mover_proceso_a_exit(pcb);
    //log obligatorio
    log_info(logger_kernel, "Finaliza el proceso <%d> - Motivo: <%s>", pcb->pid, motivo_exit);  
    


    // liberar_recursos_pcb(pcb);
    // liberar_estructuras_memoria(pcb);

    // pthread_mutex_lock(&mutex_plan_exit);
    // list_add(lista_plan_exit, pcb);
    // pthread_mutex_unlock(&mutex_plan_exit);
    
    // sem_post(&sem_multiprogramacion);
}

void atender_cpu_int_signal(t_pcb* pcb){
    atender_cpu_exit(pcb, string_from_format("INVALID_RESOURCE"));
}

void atender_cpu_signal(t_pcb* pcb, t_recurso* recurso){
    int mensajeOK ;
    log_info(logger_kernel, "[ATENDER CPU SIGNAL]");
    // log_info(logger_kernel, "[ATENDER CPU SIGNAL] PID <%d> Recurso <%s>", pcb->pid, recurso->nombre);    
    if (recurso != NULL) {
        // pthread_mutex_lock(&mutex_plan_exec);
        pthread_mutex_lock(&recurso->mutex_bloqueados);
        recurso->instancias++;
        if (recurso->instancias <= 0 && list_size(recurso->l_bloqueados) > 0) {
            // Asigno el recurso al pcb
            t_pcb* pcb_desbloqueado = list_remove(recurso->l_bloqueados, 0);
            list_add(pcb_desbloqueado->recursos_asignados, recurso);

            pthread_mutex_lock(&mutex_plan_blocked);
            list_remove_element(lista_plan_blocked, pcb_desbloqueado);
            pthread_mutex_unlock(&mutex_plan_blocked);

            // Mover el PCB desbloqueado a la lista de listos (READY)
            // pthread_mutex_lock(&mutex_plan_ready);
            // list_add(lista_plan_ready, pcb_desbloqueado);
            // pthread_mutex_unlock(&mutex_plan_ready);
            // sem_post(&sem_plan_ready);
            mover_proceso_a_ready(pcb_desbloqueado);
        }
        else{
            log_info(logger_kernel, "[ATENDER CPU SIGNAL] PID <%d> Recurso <%s> NO DESBLOQUEANDO", pcb->pid, recurso->nombre);
        }
        pthread_mutex_unlock(&recurso->mutex_bloqueados);

        // Contesto que se hizo el signal
        log_info(logger_kernel, "[ATENDER CPU SIGNAL] PID <%d> Recurso <%s> SIGNAL OK", pcb->pid, recurso->nombre);
        mensajeOK = 1;

        // pthread_mutex_lock(&mutex_plan_exec);
    }
    else{
        log_info(logger_kernel, "[ATENDER CPU SIGNAL] PID <%d> Recurso  NO ENCONTRADO", pcb->pid);  
        sem_post(&sem_pcb_desalojado);
        // envio_interrupcion(pcb->pid, INT_SIGNAL);
        //sem_post(&sem_pcb_desalojado);
        //atender_cpu_exit(pcb, string_from_format("Recurso %s no encontrado", recurso->nombre));
        mensajeOK = 0;
    }
        t_paquete* paquete_a_enviar = crear_paquete(SIGNAL);
        agregar_datos_sin_tamaño_a_paquete(paquete_a_enviar,&mensajeOK,sizeof(int));
        enviar_paquete(paquete_a_enviar,socket_dispatch);
        eliminar_paquete(paquete_a_enviar);
        if(mensajeOK){
            log_info(logger_kernel, "[ATENDER CPU SIGNAL] PID <%d> Recurso <%s> SIGNAL Enviado valor <%d>", pcb->pid, recurso->nombre, mensajeOK);
        }        
}

void atender_cpu_wait(t_pcb* pcb, t_instruccion* instruccion){
    char* nombre_recurso_wait = list_get(instruccion->parametros, 0);
    log_info(logger_kernel, "[ATENDER CPU WAIT] PID <%d> Recurso <%s>", pcb->pid, nombre_recurso_wait);
    t_recurso* recurso_encontrado = obtener_recurso(nombre_recurso_wait);
    log_info(logger_kernel, "[ATENDER CPU WAIT] PID recurso_encontrado");
    if(recurso_encontrado != NULL){
        recurso_encontrado->instancias--;
        if(recurso_encontrado->instancias < 0){// recurso sin instancias 
            //agrego el pcb a los bloqueados de ese recurso
            pthread_mutex_lock(&recurso_encontrado->mutex_bloqueados);
            list_add(recurso_encontrado->l_bloqueados, pcb);
            pthread_mutex_unlock(&recurso_encontrado->mutex_bloqueados);

            //agrego el pcb a la lista general de bloqueados

            // pthread_mutex_lock(&mutex_plan_blocked);
            // list_add(lista_plan_blocked, pcb);
            // pthread_mutex_unlock(&mutex_plan_blocked);
            // pcb->estado_anterior = pcb->estado_actual
            // pcb->estado_actual = estado_BLOCKED;
            mover_proceso_a_blocked(pcb, recurso_encontrado->nombre);

            //libero el cupo para planificar 
            pthread_mutex_lock(&mutex_plan_exec);
            proceso_exec = NULL;
            pthread_mutex_unlock(&mutex_plan_exec);
            sem_post(&sem_plan_exec_libre);

        }
        else{// recurso existente y con instancias disponibles
            //agrego el recurso pedido al pcb
            // recurso_encontrado->pcb_asignado = proceso_exec;
            list_add(proceso_exec->recursos_asignados, recurso_encontrado);
            
            //muevo el proceso a ready
            // pthread_mutex_lock(&mutex_plan_ready);
            // list_add(lista_plan_ready, pcb);
            // pthread_mutex_unlock(&mutex_plan_ready);
            // sem_post(&sem_plan_ready);   
            mover_proceso_a_ready(pcb);       

            //libero el espacio para ejecutar
            pthread_mutex_lock(&mutex_plan_exec);
            proceso_exec = NULL;
            pthread_mutex_unlock(&mutex_plan_exec);
            sem_post(&sem_plan_exec_libre);
            log_info(logger_kernel, "[ATENDER CPU WAIT] PID <%d> Recurso <%s> CON INSTANCIAS", pcb->pid, nombre_recurso_wait);
            
        }
    }
    else{
        atender_cpu_exit(pcb, "INVALID_RESOURCE");
    }
}

void atender_cpu_fin_quantum(t_pcb* pcb){
    // pthread_mutex_lock(&mutex_plan_ready);
    // list_add(lista_plan_ready, pcb);
    // pthread_mutex_unlock(&mutex_plan_ready);
    // sem_post(&sem_plan_ready);
    mover_proceso_a_ready(pcb);

    pthread_mutex_lock(&mutex_plan_exec);
    proceso_exec = NULL;
    pthread_mutex_unlock(&mutex_plan_exec);
    sem_post(&sem_plan_exec_libre);//activo el planificador de corto plazo
}

void atender_cpu_io_stdin_read(t_pcb* pcb, t_instruccion* instruccion){ 
    char* nombre_interfaz = list_get(instruccion->parametros, 0);
    t_interfaz* interfaz = _obtener_interfaz(nombre_interfaz);
    if(recibir_operacion(socket_dispatch) == IO_STDIN_READ){
        t_solicitud_io* solicitud_recibida_cpu = recibir_solicitud_io(socket_dispatch);
        
        t_pedido_stdin* pedido_en_espera = malloc(sizeof(t_pedido_stdin));
        pedido_en_espera->pcb = pcb;
        sem_init(&pedido_en_espera->semaforo_pedido_ok,0,0);

        mover_proceso_a_blocked(pcb, string_from_format("INTERFAZ %s", nombre_interfaz));

        pthread_mutex_lock(&mutex_plan_exec);
        proceso_exec = NULL;
        pthread_mutex_unlock(&mutex_plan_exec);
        sem_post(&sem_plan_exec_libre);

		pthread_mutex_lock(&interfaz->mutex_cola_block);
        list_add(interfaz->cola_procesos, pedido_en_espera);    
		pthread_mutex_unlock(&interfaz->mutex_cola_block);

        enviar_solicitud_io(interfaz->socket, solicitud_recibida_cpu);

        sem_wait(&pedido_en_espera->semaforo_pedido_ok);

        list_remove(interfaz->cola_procesos, 0);
        desbloquar_proceso(pedido_en_espera->pcb->pid);
        free(pedido_en_espera);
    }
    else{
        log_error(logger_kernel,"atender_cpu_io_stdin_read: error en la operacion recibida.");
    }
}

void atender_cpu_io_gen_sleep(t_pcb* pcb, t_instruccion* instruccion){
    
    log_info(logger_kernel,"[ATENDER CPU IO SLEEP]");
                
    char* nombre_interfaz = list_get(instruccion->parametros, 0);
    t_interfaz* interfaz = _obtener_interfaz(nombre_interfaz);
    if(interfaz !=NULL){
        t_pedido_sleep* pedido = malloc(sizeof(t_pedido_sleep));
        pedido->pcb = pcb;
        pedido->tiempo_sleep = atoi(list_get(instruccion->parametros, 1));
        sem_init(&pedido->semaforo_pedido_ok,0,0);

        // pthread_mutex_lock(&mutex_plan_blocked);
        // list_add(lista_plan_blocked, pcb);
        // pthread_mutex_unlock(&mutex_plan_blocked);
        mover_proceso_a_blocked(pcb, string_from_format("INTERFAZ %s", nombre_interfaz));

        pthread_mutex_lock(&mutex_plan_exec);
        proceso_exec = NULL;
        pthread_mutex_unlock(&mutex_plan_exec);

        //planificador_cp();
        sem_post(&sem_plan_exec_libre);//activo el planificador de corto plazo

		pthread_mutex_lock(&interfaz->mutex_cola_block);
        list_add(interfaz->cola_procesos, pedido);    
		pthread_mutex_unlock(&interfaz->mutex_cola_block);

        //sem_post(&interfaz->semaforo);  // Notificar al hilo que maneja esta interfaz
        _enviar_peticiones_io_gen(interfaz);
    }
    else{
        //habria que enviar a exit?
        log_warning(logger_kernel,"ver que hacer con el pcb si la interfaz no existe.");
        exit(EXIT_FAILURE);
    }
}





/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*AUXILIARES INTERFAZ*/
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
static void _enviar_peticiones_io_gen(t_interfaz *interfaz){
    //log_protegido_kernel(string_from_format("[ENVIAR PETICION INTERFAZ IO GEN %s]: INICIADA ---- ESPERANDO ----", interfaz->nombre_io));

    //espero que haya algun pedido creado
    //sem_wait(&interfaz->semaforo);

    //envio pedido a la interfaz
    pthread_mutex_lock(&interfaz->mutex_cola_block);
    t_pedido_sleep* pedido = list_get(interfaz->cola_procesos, 0);
    pthread_mutex_unlock(&interfaz->mutex_cola_block);

    t_paquete* paquete_pedido = crear_paquete(IO_GEN_SLEEP);
    agregar_datos_sin_tamaño_a_paquete(paquete_pedido, &pedido->tiempo_sleep, sizeof(int));
    enviar_paquete(paquete_pedido, interfaz->socket);
    eliminar_paquete(paquete_pedido);
    //espero el ok
    sem_wait(&pedido->semaforo_pedido_ok);

    list_remove(interfaz->cola_procesos, 0);
    desbloquar_proceso(pedido->pcb->pid);
    free(pedido);    
}

static t_interfaz* _obtener_interfaz(char* nombre){
    pthread_mutex_lock(&mutex_lista_interfaz);
    for(int i=0; i<list_size(lista_interfaz_socket);i++){
        t_interfaz* interfaz_buscada = list_get(lista_interfaz_socket,i);
        //log_protegido_kernel(string_from_format("[_obtener_interfaz]: %s",interfaz_buscada->nombre_io));
        if(strcmp(nombre,interfaz_buscada->nombre_io)==0){
            pthread_mutex_unlock(&mutex_lista_interfaz);
            return interfaz_buscada;
        }
    }
    pthread_mutex_unlock(&mutex_lista_interfaz);
    return NULL;
}
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*AUXILIARES INTERFAZ - FIN*/
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/

