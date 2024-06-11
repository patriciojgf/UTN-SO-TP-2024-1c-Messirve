#include "atender_cpu.h"

static void _enviar_peticiones_io_gen(t_interfaz *interfaz);
static t_interfaz* _obtener_interfaz(char* nombre);

/*---------------------------------------------------------*/
void atender_cpu_exit(t_pcb* pcb, char* motivo_exit){

    log_info(logger_kernel, "[CPU EXIT] PID <%d> Motivo <%s>", pcb->pid, motivo_exit);            
    //verifico si la planificacion esta activa.
    check_detener_planificador();   
    
    pthread_mutex_lock(&mutex_plan_exec);
    proceso_exec = NULL;
    pthread_mutex_unlock(&mutex_plan_exec);
    //sem_post(&sem_pcb_desalojado);
    sem_post(&sem_plan_exec_libre);//activo el planificador de corto plazo

    liberar_recursos_pcb(pcb);
    liberar_estructuras_memoria(pcb);

    pthread_mutex_lock(&mutex_plan_exit);
    list_add(lista_plan_exit, pcb);
    pthread_mutex_unlock(&mutex_plan_exit);
    
    pthread_mutex_lock(&mutex_procesos_planificados);
    cantidad_procesos_planificados--;
    pthread_mutex_unlock(&mutex_procesos_planificados);

    pthread_t hilo_plp; 
    pthread_create(&hilo_plp, NULL, (void*)planificador_lp_new_ready, NULL);
    pthread_detach(hilo_plp);
}

void atender_cpu_int_signal(t_pcb* pcb){
    atender_cpu_exit(pcb, string_from_format("Recurso no encontrado"));
}

void atender_cpu_signal(t_pcb* pcb, t_recurso* recurso){
    int mensajeOK ;
    log_info(logger_kernel, "[ATENDER CPU SIGNAL]");
    // log_info(logger_kernel, "[ATENDER CPU SIGNAL] PID <%d> Recurso <%s>", pcb->pid, recurso->nombre);    
    if (recurso != NULL) {
        // pthread_mutex_lock(&mutex_plan_exec);
        log_info(logger_kernel, "[ATENDER CPU SIGNAL] PID <%d> Recurso <%s> INSTANCIAS <%d>", pcb->pid, recurso->nombre, recurso->instancias);
        pthread_mutex_lock(&recurso->mutex_bloqueados);
        recurso->instancias++;
        if (recurso->instancias <= 0 && list_size(recurso->l_bloqueados) > 0) {
            log_info(logger_kernel, "[ATENDER CPU SIGNAL] PID <%d> Recurso <%s> DESBLOQUEANDO", pcb->pid, recurso->nombre);
            // Asigno el recurso al pcb
            t_pcb* pcb_desbloqueado = list_remove(recurso->l_bloqueados, 0);
            list_add(pcb_desbloqueado->recursos_asignados, recurso);

            // Mover el PCB desbloqueado a la lista de listos (READY)
            log_info(logger_kernel, "[ATENDER CPU SIGNAL] PID <%d> Recurso <%s> DESBLOQUEADO", pcb->pid, recurso->nombre);
            pthread_mutex_lock(&mutex_plan_ready);
            list_add(lista_plan_ready, pcb_desbloqueado);
            pthread_mutex_unlock(&mutex_plan_ready);
            log_info(logger_kernel, "[ATENDER CPU SIGNAL] PID <%d> Recurso <%s> DESBLOQUEADO", pcb->pid, recurso->nombre);
            sem_post(&sem_plan_ready);

            log_info(logger_kernel, "[ATENDER CPU SIGNAL] <PROCESO DESBLOQUEADO> PID <%d> por SIGNAL en recurso <%s>", pcb_desbloqueado->pid, recurso->nombre);
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
            log_info(logger_kernel, "[ATENDER CPU WAIT] lock mutex_bloqueados");
            pthread_mutex_lock(&recurso_encontrado->mutex_bloqueados);
            list_add(recurso_encontrado->l_bloqueados, pcb);
            pthread_mutex_unlock(&recurso_encontrado->mutex_bloqueados);
            log_info(logger_kernel, "[ATENDER CPU WAIT] unlock mutex_bloqueados");

            //agrego el pcb a la lista general de bloqueados

            log_info(logger_kernel, "[ATENDER CPU WAIT] lock mutex_plan_blocked");
            pthread_mutex_lock(&mutex_plan_blocked);
            list_add(lista_plan_blocked, pcb);
            pthread_mutex_unlock(&mutex_plan_blocked);
            log_info(logger_kernel, "[ATENDER CPU WAIT] unlock mutex_plan_blocked");

            //libero el cupo para planificar 
            log_info(logger_kernel, "[ATENDER CPU WAIT] lock mutex_plan_exec");
            pthread_mutex_lock(&mutex_plan_exec);
            proceso_exec = NULL;
            pthread_mutex_unlock(&mutex_plan_exec);
            log_info(logger_kernel, "[ATENDER CPU WAIT] unlock mutex_plan_exec");
            sem_post(&sem_plan_exec_libre);
            log_info(logger_kernel, "[ATENDER CPU WAIT] PID <%d> Recurso <%s> SIN INSTANCIAS", pcb->pid, nombre_recurso_wait);

        }
        else{// recurso existente y con instancias disponibles
            //agrego el recurso pedido al pcb
            // recurso_encontrado->pcb_asignado = proceso_exec;
            list_add(proceso_exec->recursos_asignados, recurso_encontrado);
            
            //muevo el proceso a ready
            pthread_mutex_lock(&mutex_plan_ready);
            list_add(lista_plan_ready, pcb);
            pthread_mutex_unlock(&mutex_plan_ready);
            sem_post(&sem_plan_ready);          

            //libero el espacio para ejecutar
            pthread_mutex_lock(&mutex_plan_exec);
            proceso_exec = NULL;
            pthread_mutex_unlock(&mutex_plan_exec);
            sem_post(&sem_plan_exec_libre);
            log_info(logger_kernel, "[ATENDER CPU WAIT] PID <%d> Recurso <%s> CON INSTANCIAS", pcb->pid, nombre_recurso_wait);
            
        }
    }
    else{
        atender_cpu_exit(pcb, string_from_format("Recurso %s no encontrado", nombre_recurso_wait));
    }
}

void atender_cpu_fin_quantum(t_pcb* pcb){
    //log_protegido_kernel(string_from_format("[atender_cpu_fin_quantum]: pid: %d", pcb->pid));
    pthread_mutex_lock(&mutex_plan_exec);
    proceso_exec = NULL;
    pthread_mutex_unlock(&mutex_plan_exec);
    //log_protegido_kernel(string_from_format("[atender_cpu_fin_quantum]: mutex_plan_exec"));
    //sem_post(&sem_pcb_desalojado);
    sem_post(&sem_plan_exec_libre);//activo el planificador de corto plazo

    pthread_mutex_lock(&mutex_plan_ready);
    list_add(lista_plan_ready, pcb);
    pthread_mutex_unlock(&mutex_plan_ready);
    //log_protegido_kernel(string_from_format("[atender_cpu_fin_quantum]: mutex_plan_ready"));
    sem_post(&sem_plan_ready);

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
        pthread_mutex_lock(&mutex_plan_blocked);
        list_add(lista_plan_blocked, pcb);
        pthread_mutex_unlock(&mutex_plan_blocked);

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

    //verifico si la planificacion esta activa.
    check_detener_planificador();    

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
            pthread_mutex_unlock(&mutex_lista_recursos);
			return recurso_encontrado;
		}
	}
    pthread_mutex_unlock(&mutex_lista_recursos);
	return NULL;
}

void liberar_estructuras_memoria(t_pcb* pcb){
    //verifico si la planificacion esta activa.
    check_detener_planificador();   

    // Envio peticion a memoria para liberar las estructuras de un proceso.
    t_paquete* paquete = crear_paquete(LIBERAR_ESTRUCTURAS_MEMORIA);
    agregar_datos_sin_tamaño_a_paquete(paquete, &pcb->pid, sizeof(int));
    enviar_paquete(paquete, socket_memoria);
    eliminar_paquete(paquete);
    sem_wait(&s_memoria_liberada_pcb);
}

void liberar_recursos_pcb(t_pcb* pcb){    
    //verifico si la planificacion esta activa.
    check_detener_planificador();   
    // Liberar cualquier recurso que realmente haya sido asignado al PCB.
    while (!list_is_empty(pcb->recursos_asignados)) {
        t_recurso* recurso_asignado = list_remove(pcb->recursos_asignados, 0);
        atender_cpu_signal(pcb, recurso_asignado); 
    }
    list_destroy(pcb->recursos_asignados);
    pcb->recursos_asignados = NULL; // Asegurar que la referencia en el PCB no apunte a una lista ya destruida.


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
}
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/* AUXILIARES FIN */
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/

