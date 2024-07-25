#include "kernel_pcb.h"

t_pcb* crear_pcb(char* path){
    t_pcb* pcb = malloc(sizeof(t_pcb));
    pthread_mutex_lock(&mutex_pid_proceso);
    pid_proceso++;
    pcb->pid = pid_proceso;
    pthread_mutex_unlock(&mutex_pid_proceso);
    // pcb->program_counter = 0;
    pcb->archivos_abiertos = list_create();
    pcb->recursos_asignados = list_create();
    pcb->path = string_duplicate(path);
    pcb->quantum = QUANTUM;
    inicializar_registros(&(pcb->registros_cpu));   
    //log obligatorio
    log_info(logger_kernel, "Se crea el proceso %d en NEW", pcb->pid);
    return pcb;
}

/*Funciones PCB - INICIO*/ 
static t_paquete* _empaquetar_contexto_cpu(t_pcb* pcb){
	t_paquete* paquete = crear_paquete(PCB);
	agregar_datos_sin_tamaño_a_paquete(paquete,&(pcb->pid),sizeof(int));
	// agregar_datos_sin_tamaño_a_paquete(paquete,&(pcb->program_counter),sizeof(int));
	empaquetar_registros_cpu(paquete, pcb->registros_cpu);
	return paquete;
}

void enviar_contexto_dispatch(t_pcb* pcb){
	t_paquete* paquete_a_enviar = _empaquetar_contexto_cpu(pcb);
	enviar_paquete(paquete_a_enviar, socket_dispatch);
	eliminar_paquete(paquete_a_enviar);
}
/*Funciones PCB - FIN*/