#include "atender_cpu.h"

static void _enviar_peticiones_io_gen(t_interfaz *interfaz);
static t_interfaz* _obtener_interfaz(char* nombre);
/*---------------------------------------------------------*/
void atender_cpu_exit(t_pcb* pcb, t_instruccion* instruccion){
    log_protegido_kernel(string_from_format("[atender_cpu_exit]: pid: %d", pcb->pid));
    
    pthread_mutex_lock(&mutex_plan_exec);
    proceso_exec = NULL;
    pthread_mutex_unlock(&mutex_plan_exec);
    
    pthread_mutex_lock(&mutex_plan_exit);
    list_add(lista_plan_exit, pcb);
    pthread_mutex_unlock(&mutex_plan_exit);
    
    pthread_mutex_lock(&mutex_procesos_planificados);
    cantidad_procesos_planificados++;
    pthread_mutex_unlock(&mutex_procesos_planificados);

    pthread_t hilo_plp; 
    pthread_create(&hilo_plp, NULL, (void*)planificador_lp_new_ready, NULL);
    pthread_detach(hilo_plp);
}

void atender_cpu_io_gen_sleep(t_pcb* pcb, t_instruccion* instruccion){
    //verifico que el nombre de la interfaz que viene en la instruccion exista en el listado lista_interfaz_socket
    char* nombre_interfaz = list_get(instruccion->parametros, 0);
    log_protegido_kernel(string_from_format("[atender_io_gen_sleep]: nombre interfaz: %s", nombre_interfaz));
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

        planificador_cp();

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

static void _enviar_peticiones_io_gen(t_interfaz *interfaz){
    log_protegido_kernel(string_from_format("[ENVIAR PETICION INTERFAZ IO GEN %s]: INICIADA ---- ESPERANDO ----", interfaz->nombre_io));

    //espero que haya algun pedido creado
    //sem_wait(&interfaz->semaforo);

    //envio pedido a la interfaz
    pthread_mutex_lock(&interfaz->mutex_cola_block);
    t_pedido_sleep* pedido = list_get(interfaz->cola_procesos, 0);
    pthread_mutex_unlock(&interfaz->mutex_cola_block);

    t_paquete* paquete_pedido = crear_paquete(IO_GEN_SLEEP);
    agregar_datos_sin_tamaño_a_paquete(paquete_pedido, &pedido->tiempo_sleep, sizeof(int));
    enviar_paquete(paquete_pedido, interfaz->socket);
    
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
        log_protegido_kernel(string_from_format("[_obtener_interfaz]: %s",interfaz_buscada->nombre_io));
        if(strcmp(nombre,interfaz_buscada->nombre_io)==0){
            pthread_mutex_unlock(&mutex_lista_interfaz);
            return interfaz_buscada;
        }
    }
    pthread_mutex_unlock(&mutex_lista_interfaz);
    return NULL;
}
