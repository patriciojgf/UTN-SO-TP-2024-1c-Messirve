#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H_

#include <stdint.h>
#include <commons/collections/list.h>
#include <commons/temporal.h>
#include <stdlib.h>
#include <commons/log.h>
#include <assert.h>
#include <string.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <semaphore.h>


typedef struct{
	// uint32_t AX,BX,CX,DX; 
    uint32_t PC;
    uint8_t AX;
    uint8_t BX;
    uint8_t CX;
    uint8_t DX;
    uint32_t EAX;
    uint32_t EBX;
    uint32_t ECX;
    uint32_t EDX;
    uint32_t SI;
    uint32_t DI;
} t_registros_cpu;

// typedef struct 
// {
//     int id;
//     int prioridad;
//     int program_counter;
//     t_temporal* tiempo_llegada;
//     t_list* recursos_asignados;
//     t_list* archivos_abiertos;
//     t_registros_cpu registros_cpu;
// }t_pcb;

typedef struct
{
    int identificador; //identificador de la instruccion
    int cantidad_parametros; //cantidad de parametros que tiene la instruccion
    t_list* parametros; //lista de parametros sin contar el identificador
}t_instruccion;

//PCB.h

typedef enum estado {
    estado_NEW,
    estado_READY,
    estado_EXEC,
    estado_BLOCKED,
    estado_EXIT,
} t_estado;


typedef struct t_pcb{
    int pid; //Número de la próxima instrucción a ejecutar.
    int program_counter; //Número de la próxima instrucción a ejecutar.
    int quantum; //Unidad de tiempo utilizada por el algoritmo de planificación VRR.
    t_registros_cpu registros_cpu; //Registros de la CPU.
    t_list* recursos_asignados; // va a ser una lista de t_recurso
    t_list* archivos_abiertos; // va a ser una lista de t_archivo
    t_estado estado_actual; //ref 20222c
    t_estado estado_anterior;   
    char* path;
} t_pcb;


typedef struct {
    char* nombre;
    int instancias;
    t_list* l_bloqueados; //esta lista la vamos a usar para saber que procesos estan bloqueados por este recurso
    pthread_mutex_t mutex_bloqueados;
}t_recurso;

void inicializar_registros(t_registros_cpu* registros);

#endif // ESTRUCTURAS_H