#include "kernel_pcb.h"

t_pcb* crear_pcb(char* path){
    t_pcb* pcb = malloc(sizeof(t_pcb));

    pthread_mutex_lock(&mutex_pid_proceso);
    pid_proceso++;
    pcb->pid = pid_proceso;
    pthread_mutex_unlock(&mutex_pid_proceso);
    //muestro el id del pcb nuevo
    log_info(logger_kernel, "Se creo un nuevo PCB con PID: %d", pcb->pid);
    pcb->program_counter = 0;
    pcb->archivos_abiertos = list_create();
    pcb->recursos_asignados = list_create();
    pcb->path = string_duplicate(path);

    //TODO: validar si esto esta bien
    //pcb->registros_cpu = malloc(sizeof(t_registros_cpu));
    inicializar_registros(&(pcb->registros_cpu));   
    return pcb;
}

/*Funciones PCB - INICIO*/ 
static t_paquete* _empaquetar_contexto_cpu(t_pcb* pcb){
	t_paquete* paquete = crear_paquete(PCB);
	agregar_datos_sin_tamaño_a_paquete(paquete,&(pcb->pid),sizeof(int));
	agregar_datos_sin_tamaño_a_paquete(paquete,&(pcb->program_counter),sizeof(int));
	empaquetar_registros_cpu(paquete, pcb->registros_cpu);
	return paquete;
}

void enviar_contexto_dispatch(t_pcb* pcb){
    log_warning(logger_kernel,"Envio el contexto a dispatch");
	t_paquete* paquete_a_enviar = _empaquetar_contexto_cpu(pcb);
	enviar_paquete(paquete_a_enviar, socket_dispatch);
	eliminar_paquete(paquete_a_enviar);
}
/*Funciones PCB - FIN*/