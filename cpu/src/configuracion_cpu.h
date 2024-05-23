#ifndef STATIC_CONFIGURACION_CPU_H_  
#define STATIC_CONFIGURACION_CPU_H_

#include <utils/logconfig.h>
#include <utils/conexiones.h>
#include <pthread.h>

#define LOG_NAME "cpu.log"
#define PROCESS_NAME "CPU"
#define MSG_ERROR "No se pudo crear correctamente. "

typedef struct config_cpu
{
    char* ip_memoria;
    char* puerto_memoria;
    char* puerto_escucha_dispatch;
    char* puerto_escucha_intrerrupt;
    int cantidad_entradas_tlb;
    char* algoritmo_tlb;
}t_config_cpu;



t_config_cpu* iniciar_config_cpu(t_config* config_cpu);
void finalizar_config_cpu(t_config_cpu* config_cpu);

#endif