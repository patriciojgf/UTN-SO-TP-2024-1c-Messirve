#ifndef PCB_H
#define PCB_H


#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include "estructuras.h"

typedef enum estado {
    NEW,
    READY,
    EXEC,
    BLOCKED,
    EXIT,
} t_estado;

typedef struct {
    char* nombre;
    int instancias;
    t_queue* bloqueados; //esta lista la vamos a usar para saber que procesos estan bloqueados por este recurso
}t_recurso;

typedef struct{
    int pid; //Número de la próxima instrucción a ejecutar.
    int program_counter; //Número de la próxima instrucción a ejecutar.
    int quantum; //Unidad de tiempo utilizada por el algoritmo de planificación VRR.
    t_registros_cpu* registros_cpu; //Registros de la CPU.
    t_list* recursos_asignados; // va a ser una lista de t_recurso
    t_list* archivos_abiertos; // va a ser una lista de t_archivo
    t_estado estado_actual; //ref 20222c
    t_estado estado_anterior;   
} t_pcb;


#endif // PCB_H