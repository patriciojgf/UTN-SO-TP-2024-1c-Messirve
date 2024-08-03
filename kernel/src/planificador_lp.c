#include "planificador_lp.h"

/*-------------------------------------------*/
//Nuevo planificador

// Función para mover PCB a READY
// Se usa en el planificador LP y cuando los procesos se desbloqueen
void mover_proceso_a_ready(t_pcb* pcb) {
        check_detener_planificador();
        pcb->estado_anterior = pcb->estado_actual;
        pcb->estado_actual = estado_READY;
        
        //Log obligatorio
        log_info(logger_kernel, "Cambio de Estado: PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb->pid, estado_string(pcb->estado_anterior), estado_string(pcb->estado_actual));
        

        pthread_mutex_lock(&mutex_plan_ready);  
        pthread_mutex_lock(&mutex_plan_ready_vrr);

        //valido si estamos en VRR
        if (ALGORITMO_PLANIFICACION==VRR && pcb->quantum < QUANTUM && pcb->quantum != -1) {
            list_add(lista_plan_ready_vrr, pcb);
            //Log obligatorio
            char* pids = listado_pids(lista_plan_ready_vrr);
            log_info(logger_kernel,"Cola Ready Prioridad: %s", pids);
        }
        else{ 
            list_add(lista_plan_ready, pcb);
            //Log obligatorio
            char* pids = listado_pids(lista_plan_ready);
            log_info(logger_kernel,"Cola Ready: %s", pids);
        }
        // cantidad_procesos_planificados++;  // Incrementar contador de procesos planificados
        sem_post(&sem_plan_ready);

        pthread_mutex_unlock(&mutex_plan_ready_vrr);       
        pthread_mutex_unlock(&mutex_plan_ready);
        
}

void mover_proceso_a_exit(t_pcb* pcb){
    check_detener_planificador();
    liberar_recursos_pcb(pcb);
    if (!pcb->estado_actual == estado_NEW){
        liberar_estructuras_memoria(pcb);
    }
    pcb->estado_anterior = pcb->estado_actual;
    pcb->estado_actual = estado_EXIT;
    log_info(logger_kernel, "Cambio de Estado: PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb->pid, estado_string(pcb->estado_anterior), estado_string(pcb->estado_actual));
        
    pthread_mutex_lock(&mutex_plan_exit);
    list_add(lista_plan_exit, pcb);
    pthread_mutex_unlock(&mutex_plan_exit);
    if (pcb->estado_anterior==estado_READY){
        sem_wait(&sem_plan_ready);
        sem_post(&sem_multiprogramacion);
    }
    if (pcb->estado_anterior==estado_EXEC||pcb->estado_anterior==estado_BLOCKED){
        sem_post(&sem_multiprogramacion);
    }
}

void mover_proceso_a_blocked(t_pcb* pcb, char* motivo){
    check_detener_planificador();
    pcb->estado_anterior = pcb->estado_actual;
    pcb->estado_actual = estado_BLOCKED;
    pthread_mutex_lock(&mutex_plan_blocked);
    list_add(lista_plan_blocked, pcb);

    //Log obligatorio
    log_info(logger_kernel, "PID: <%d> - Bloqueado por: <%s>", pcb->pid, motivo);

    //Log obligatorio
    log_info(logger_kernel, "Cambio de Estado: PID: %d - Estado Anterior: %s - Estado Actual: %s", pcb->pid, estado_string(pcb->estado_anterior), estado_string(pcb->estado_actual));
    // PATRICIO AGREGO FREE - Liberar la memoria del motivo
    // free(motivo);
    pthread_mutex_unlock(&mutex_plan_blocked);
}


void desbloquar_proceso(int pid){
    pthread_mutex_lock(&mutex_plan_blocked);
    t_pcb* pcb_desbloqueado = buscar_pcb_por_pid(pid, lista_plan_blocked);    
    if(pcb_desbloqueado != NULL){
        // pcb_desbloqueado->estado_anterior = pcb_desbloqueado->estado_actual;
        // pcb_desbloqueado->estado_actual = estado_READY;
        list_remove_element(lista_plan_blocked, pcb_desbloqueado);
        mover_proceso_a_ready(pcb_desbloqueado);
        // pthread_mutex_lock(&mutex_plan_ready);
        // list_add(lista_plan_ready, pcb_desbloqueado);
        // pthread_mutex_unlock(&mutex_plan_ready);
        // sem_post(&sem_plan_ready);

        pthread_mutex_unlock(&mutex_plan_blocked);

        //planificador_cp();
        //sem_post(&sem_plan_exec_libre); //activo el planificador de corto plazo
    }
    else{
        pthread_mutex_unlock(&mutex_plan_blocked);
        log_warning(logger_kernel, "No se encontro el proceso a desbloquear");
    }

}

void planificador_lp_new_ready() {
    while (1) {
        sem_wait(&sem_plan_new);            // Esperar a que haya nuevos procesos en la cola NEW.
        sem_wait(&sem_multiprogramacion);   // Esperar a que haya capacidad bajo el límite de multiprogramación.
        check_detener_planificador();       // Verificar si la planificación debe pausarse.

        // pthread_mutex_lock(&mutex_procesos_planificados); // Bloquear el mutex de procesos planificados.
        pthread_mutex_lock(&mutex_plan_new); // Bloquear el mutex de la lista NEW.

        t_pcb* pcb_en_new = NULL;        
        if (!list_is_empty(lista_plan_new)) {
            pcb_en_new = list_remove(lista_plan_new, 0); // Obtener el primer proceso en NEW por FIFO.
            enviar_proceso_a_memoria(pcb_en_new, pcb_en_new->path);
            sem_wait(&s_init_proceso_a_memoria);  // Esperar confirmación de memoria
            mover_proceso_a_ready(pcb_en_new);
        }
        else {
            sem_post(&sem_multiprogramacion);
            // pthread_mutex_unlock(&mutex_procesos_planificados); // Desbloquear el mutex de procesos planificados.
            log_warning(logger_kernel,"Fallo en planificaicon new-ready, no hay proceso en new");
        }

        pthread_mutex_unlock(&mutex_plan_new); // Desbloquear el mutex de la lista NEW.
        // pthread_mutex_unlock(&mutex_procesos_planificados); // Desbloquear el mutex de procesos planificados.
    }
}


void planificador_lp_nuevo_proceso(t_pcb* nuevo_pcb) {
    if (nuevo_pcb != NULL) {
        pthread_mutex_lock(&mutex_plan_new);
        list_add(lista_plan_new, nuevo_pcb);
        nuevo_pcb->estado_actual = estado_NEW;
        sem_post(&sem_plan_new);
        pthread_mutex_unlock(&mutex_plan_new);

        // pthread_t hilo_planificador;
        // pthread_create(&hilo_planificador, NULL, (void*)planificador_lp_new_ready, NULL);
        // pthread_detach(hilo_planificador);
    }
}

void enviar_proceso_a_memoria(t_pcb* pcb, char* path){
    t_paquete* paquete_a_enviar = crear_paquete(INICIAR_PROCESO_MEMORIA);
    agregar_datos_sin_tamaño_a_paquete(paquete_a_enviar,&(pcb->pid),sizeof(int));
    agregar_a_paquete(paquete_a_enviar,path,strlen(path)+1);
    //
    enviar_paquete(paquete_a_enviar, socket_memoria);
    eliminar_paquete(paquete_a_enviar);
}



/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/* AUXILIARES RECURSOS */
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
t_recurso* obtener_recurso(char* recurso){
    pthread_mutex_lock(&mutex_lista_recursos);
	for(int i = 0; i<list_size(lista_recursos); i++){
		t_recurso* recurso_encontrado = list_get(lista_recursos,i);
		if(strcmp(recurso, recurso_encontrado->nombre) == 0){
            // log_info(logger_kernel,"recurso <%s> encontrado: <%s>", recurso, recurso_encontrado->nombre);
            pthread_mutex_unlock(&mutex_lista_recursos);
			return recurso_encontrado;
		}
        // else{
        //     log_error(logger_kernel,"recurso <%s> distinto a <%s>", recurso, recurso_encontrado->nombre);
        // }
	}
    pthread_mutex_unlock(&mutex_lista_recursos);
	return NULL;
}

void liberar_estructuras_memoria(t_pcb* pcb){
    //verifico si la planificacion esta activa.
    // check_detener_planificador();   

    // Envio peticion a memoria para liberar las estructuras de un proceso.
    t_paquete* paquete = crear_paquete(LIBERAR_ESTRUCTURAS_MEMORIA);
    agregar_datos_sin_tamaño_a_paquete(paquete, &pcb->pid, sizeof(int));
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);
    sem_wait(&s_memoria_liberada_pcb);
}

void liberar_recursos_pcb(t_pcb* pcb){  

    // Quita el PCB de la lista de bloqueados de cada recurso.
    pthread_mutex_lock(&mutex_lista_recursos);
    for (int i = 0; i < list_size(lista_recursos); i++) {
        t_recurso* recurso = list_get(lista_recursos, i);
        
        // Si el pcb esta en la lista de bloqueados del recurso, se quita y se suma instancia disponible.
        pthread_mutex_lock(&recurso->mutex_bloqueados);
        if (list_remove_element(recurso->l_bloqueados, pcb)) {
            pthread_mutex_unlock(&recurso->mutex_bloqueados);
            // atender_cpu_signal(pcb, recurso);
            recurso->instancias++;            
        }
        else{
            pthread_mutex_unlock(&recurso->mutex_bloqueados);
        }
    }
    pthread_mutex_unlock(&mutex_lista_recursos);

    // log_info(logger_kernel,"liberar_recursos_pcb");  
    // Liberar cualquier recurso que realmente haya sido asignado al PCB.
    while (!list_is_empty(pcb->recursos_asignados)) {
        // log_info(logger_kernel,"(!list_is_empty(pcb->recursos_asignados))");  
        t_recurso* recurso_asignado = list_remove(pcb->recursos_asignados, 0);
        atender_cpu_signal(pcb, recurso_asignado); 
    }
    list_destroy(pcb->recursos_asignados);
    pcb->recursos_asignados = NULL; // Asegurar que la referencia en el PCB no apunte a una lista ya destruida.

}
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/* AUXILIARES FIN */
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/* AUXILIARES ESTADOS */
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
// Función que obtiene una lista de PIDs en formato de cadena
char* listado_pids(t_list* lista) {
    // Calcular el tamaño máximo necesario para la cadena
    int tamaño_maximo = list_size(lista) * 12; // Considerando un máximo de 10 dígitos por PID, más coma y espacio
    char* pids = malloc(tamaño_maximo);
    if (!pids) {
        return NULL;
    }
    pids[0] = '\0'; // Inicializar cadena vacía

    strcat(pids, "["); // Agregar corchete de apertura

    for (int i = 0; i < list_size(lista); i++) {
        t_pcb* pcb = list_get(lista, i);
        char pid[12];
        sprintf(pid, "%d", pcb->pid); // Convertir PID a cadena

        strcat(pids, pid); // Agregar PID a la cadena principal

        if (i != list_size(lista) - 1) {
            strcat(pids, ","); // Agregar coma entre los PIDs
        }
    }

    strcat(pids, "]"); // Agregar corchete de cierre

    return pids; // Retornar la cadena resultante
}

void finalizar_proceso(int pid) {
    log_info(logger_kernel, "Finalizando proceso PID %d", pid);

    if (proceso_exec && proceso_exec->pid == pid) {

        pthread_mutex_lock(&mutex_finalizar_proceso);
        proceso_finalizando = true;
        pthread_mutex_unlock(&mutex_finalizar_proceso);

        sem_post(&sem_pcb_desalojado);
        log_info(logger_kernel, "Finalizando proceso en ejecución PID %d", pid);
        envio_interrupcion(pid, INT_FINALIZAR_PROCESO);
        return;
    }

    t_pcb* pcb = NULL;
    pthread_mutex_t* mutex_actual = NULL;
    // t_list* lista_actual = NULL;

    // Arreglo de listas y sus mutex para simplificar la búsqueda
    t_list* listas[] = {lista_plan_new, lista_plan_ready, lista_plan_ready_vrr, lista_plan_blocked};
    
    pthread_mutex_t* mutexes[] = {&mutex_plan_new, &mutex_plan_ready, &mutex_plan_ready_vrr, &mutex_plan_blocked};

    for (int i = 0; i < 4 ; i++) {
        // pthread_mutex_lock(mutexes[i]);
        pcb = buscar_pcb_por_pid(pid, listas[i]);
        if (pcb != NULL) {
            list_remove_element(listas[i], pcb); 
            mutex_actual = mutexes[i];
            // lista_actual = listas[i];
            break;
        }
        // pthread_mutex_unlock(mutexes[i]);
    }

    if (pcb) {
        pthread_mutex_unlock(mutex_actual);
        mover_proceso_a_exit(pcb);
        //log obligatorio
        log_info(logger_kernel, "Finaliza el proceso <%d> - Motivo: <INTERRUPTED_BY_USER>", pcb->pid);     

    } else {
        log_warning(logger_kernel, "No se encontró el proceso PID %d para finalizar", pid);
    }
}
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/* AUXILIARES ESTADOS */
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/