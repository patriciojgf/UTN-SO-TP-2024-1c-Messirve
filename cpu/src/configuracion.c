#include <configuracion_cpu.h>

t_config_cpu* iniciar_config_cpu(t_config* config_cpu){
    t_config_cpu* datos_cpu = malloc(sizeof(t_config_cpu));

    datos_cpu->ip_memoria = config_get_string_value(config_cpu, "IP_MEMORIA");
    datos_cpu->puerto_memoria = config_get_string_value(config_cpu, "PUERTO_MEMORIA");
    datos_cpu->puerto_escucha_dispatch = config_get_string_value(config_cpu, "PUERTO_ESCUCHA_DISPATCH");
    datos_cpu->puerto_escucha_intrerrupt = config_get_string_value(config_cpu, "PUERTO_ESCUCHA_INTERRUPT"); 
    datos_cpu->cantidad_entradas_tlb = config_get_int_value(config_cpu, "CANTIDAD_ENTRADAS_TLB");
    datos_cpu->algoritmo_tlb = config_get_string_value(config_cpu, "ALGORITMO_TLB");

    return datos_cpu;
}

// int inicializar_configuracion(t_log* logger_cpu, t_config* config_cpu, t_config_cpu* datos_cpu, char* config_name){
//     logger_cpu = iniciar_logger(LOG_NAME, PROCESS_NAME);

//     if(logger_cpu == NULL){
//         error_show("No se pudo crear correctamente. ");
//         return EXIT_FAILURE;
//     }

//     config_cpu = iniciar_config(config_name);
    
//     if(config_cpu == NULL){
//         log_error(logger_cpu, "No se pudo crear correctamente. ");
//         finalizar_log(logger_cpu);
//         return EXIT_FAILURE;
//     }

//     datos_cpu = iniciar_config_cpu(config_cpu);
//     return EXIT_SUCCESS;
// }