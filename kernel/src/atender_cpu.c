#include "atender_cpu.h"

static void _enviar_peticiones_io_gen(t_interfaz *interfaz);
/*---------------------------------------------------------*/

t_interfaz* _obtener_interfaz(char* nombre){
    pthread_mutex_lock(&mutex_lista_interfaz);
    for(int i=0; i<list_size(lista_interfaz_socket);i++){
        t_interfaz* interfaz_buscada = list_get(lista_interfaz_socket,i);
        log_protegido_kernel(string_from_format("[_obtener_interfaz]: %s",interfaz_buscada->nombre_io));
        if(strcmp(nombre,interfaz_buscada->nombre_io)==0){
            return interfaz_buscada;
        }
    }
    pthread_mutex_unlock(&mutex_lista_interfaz);
    return NULL;
}

void atender_io_gen_sleep(t_pcb* pcb, t_instruccion* instruccion){
    //verifico que el nombre de la interfaz que viene en la instruccion exista en el listado lista_interfaz_socket
    char* nombre_interfaz = list_get(instruccion->parametros, 0);
    log_protegido_kernel(string_from_format("[atender_io_gen_sleep]: nombre interfaz: %s", nombre_interfaz));
    t_interfaz* interfaz = _obtener_interfaz(nombre_interfaz);
    if(interfaz !=NULL){
        t_pedido_sleep* pedido = malloc(sizeof(t_pedido_sleep));
        pedido->pcb = pcb;
        pedido->tiempo_sleep = atoi(list_get(instruccion->parametros, 1));
        sem_init(&pedido->semaforo_pedido_ok,0,0);
        list_add(lista_plan_blocked, pcb);// Añadir el pcb a la lista de bloqueados
        proceso_exec = NULL;// Limpio el puntero del proceso en ejecución, indicando que ya no está siendo ejecutado    
        log_protegido_kernel(string_from_format("[atender_io_gen_sleep]"));
		pthread_mutex_lock(&interfaz->mutex_cola_block);
        log_protegido_kernel(string_from_format("[atender_io_gen_sleep]"));
        list_add(interfaz->cola_procesos, pedido);    
		pthread_mutex_unlock(&interfaz->mutex_cola_block);
        //sem_post(&interfaz->semaforo);  // Notificar al hilo que maneja esta interfaz
        log_protegido_kernel(string_from_format("[atender_io_gen_sleep]"));
        _enviar_peticiones_io_gen(interfaz);
    }
    else{
        //habria que enviar a exit?
        log_warning(logger_kernel,"ver que hacer con el pcb si la interfaz no existe.");
        exit(EXIT_FAILURE);
    }
}

static void _enviar_peticiones_io_gen(t_interfaz *interfaz){
    log_protegido_kernel(string_from_format("[ENVIAR PETICION INTERFAZ IO GEN %s]: INICIADA ---- ESPERANDO ----", interfaz->nombre_io));

    //espero que haya algun pedido creado
    //sem_wait(&interfaz->semaforo);

    //envio pedido a la interfaz
    t_pedido_sleep* pedido = list_get(interfaz->cola_procesos, 0);
    t_paquete* paquete_pedido = crear_paquete(IO_GEN_SLEEP);
    agregar_datos_sin_tamaño_a_paquete(paquete_pedido, &pedido->tiempo_sleep, sizeof(int));
    enviar_paquete(paquete_pedido, interfaz->socket);
    log_protegido_kernel(string_from_format("[ENVIAR PETICION INTERFAZ IO GEN %s]: ENVIADA ---- ESPERANDO ----", interfaz->nombre_io));

    //espero el ok
    sem_wait(&pedido->semaforo_pedido_ok);
    log_protegido_kernel(string_from_format("[ENVIAR PETICION INTERFAZ IO GEN %s]: ENVIADA ---- ESPERANDO ----", interfaz->nombre_io));

    list_remove(interfaz->cola_procesos, 0);
    free(pedido);
    
}
