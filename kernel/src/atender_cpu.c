#include "atender_cpu.h"


void atender_io_gen_sleep(t_pcb* pcb, t_instruccion* instruccion){
    //verifico que el nombre de la interfaz que viene en la instruccion exista en el listado lista_interfaz_socket
    char* nombre_interfaz = instruccion->parametros[0];
    int tiempo_sleep = atoi(instruccion->parametros[1]);
    t_interfaz* interfaz = _obtener_interfaz(nombre_interfaz);
    if(interfaz !=NULL){
        // Añadir el pcb directamente a la lista de bloqueados
        list_add(lista_plan_blocked, pcb);
        // Limpio el puntero del proceso en ejecución, indicando que ya no está siendo ejecutado
        proceso_exec = NULL;
        
        _io_gen_envio_pedido();

    }
    else{
        //habria que enviar a exit?
        log_warning(logger_kernel,"falta implementar la respuesta");
        exit(EXIT_FAILURE);
    }
}

t_interfaz* _obtener_interfaz(char* nombre){
    pthread_mutex_lock(&mutex_lista_interfaz);
    for(int i=0; i<list_size(lista_interfaz_socket);i++){
        t_interfaz* interfaz_buscada = list_get(lista_interfaz_socket,i);
        if(strcmp(nombre,interfaz_buscada->nombre_io)==0){
            return interfaz_buscada;
        }
    }
    pthread_mutex_unlock(&mutex_lista_interfaz);
    return NULL;
}