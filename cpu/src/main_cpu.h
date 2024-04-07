#ifndef STATIC_MAIN_CPU_H_
#define STATIC_MAIN_CPU_H_
    
    #include <configuracion_cpu.h>

    int socket_memoria;

    t_log* logger_cpu;
    t_config* config_cpu;
    t_config_cpu* datos_cpu;

    pthread_t hilo_cpu;

    void conectarMemoria();
    
#endif
