#include "kernel_pcb.h"

t_pcb* crear_pcb(){
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

    //TODO: validar si esto esta bien
    //pcb->registros_cpu = malloc(sizeof(t_registros_cpu));
    inicializar_registros(&(pcb->registros_cpu));   
    return pcb;
}

void enviar_pcb_cpu_dispatch(t_pcb* pcb){
    log_info(logger_kernel, "enviar_pcb_cpu_dispatch");
    enviar_pcb_sin_listas_sin_estados(pcb, socket_dispatch);
    log_info(logger_kernel, "Se envio el PCB a dispatch con PID: %d a la CPU", pcb->pid);
}