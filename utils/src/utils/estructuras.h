#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H_

#include <stdint.h>
#include <commons/collections/list.h>
#include <commons/temporal.h>
#include <stdlib.h>
#include <commons/log.h>
#include <assert.h>
#include <string.h>

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

typedef struct t_pcb 
{
    int id;
    int prioridad;
    int program_counter;
    t_temporal* tiempo_llegada;
    t_list* recursos_asignados;
    t_list* archivos_abiertos;
    t_registros_cpu registros_cpu;
}t_pcb;

#endif // ESTRUCTURAS_H