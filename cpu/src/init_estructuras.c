#include "init_estructuras.h"

static void init_log();

//-----------------------------------------------------------------------------//
void log_protegido_cpu(char* mensaje){
	sem_wait(&mlog);
	log_info(logger_cpu, "%s", mensaje);
	sem_post(&mlog);
	free(mensaje);
}

static void init_log(){
    logger_cpu = iniciar_logger("cpu.log", "CPU");
}

static void iniciar_configuracion(char* config_path){
    config_cpu = iniciar_config(config_path);  
    if(config_cpu == NULL){
        log_error(logger_cpu,"No se pudo leer el archivo de configuracion");
        log_destroy(logger_cpu);
        config_destroy(config_cpu);
        exit(2);
    }    
    IP_MEMORIA = config_get_string_value(config_cpu, "IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value(config_cpu, "PUERTO_MEMORIA"); 
    PUERTO_ESCUCHA_DISPATCH = config_get_string_value(config_cpu, "PUERTO_ESCUCHA_DISPATCH");
    PUERTO_ESCUCHA_INTERRUPT = config_get_string_value(config_cpu, "PUERTO_ESCUCHA_INTERRUPT"); 
    CANTIDAD_ENTRADAS_TLB = config_get_int_value(config_cpu, "CANTIDAD_ENTRADAS_TLB");
    ALGORITMO_TLB = config_get_string_value(config_cpu, "ALGORITMO_TLB");    
}

static void iniciar_semaforos(){
    sem_init(&mlog,0,1);
    sem_init(&s_instruccion_actual,0,0);
    sem_init(&s_signal_kernel,0,0);
    sem_init(&s_fetch_espere_instruccion,0,0);
}

static void iniciar_mutex(){
    pthread_mutex_init(&mutex_ejecucion_proceso,NULL);
}

static void iniciar_estructuras(){
    flag_ejecucion=false;
    flag_interrupt=false;
    contexto_cpu= malloc(sizeof(t_contexto));
}

void init_cpu(char* path_config){
    init_log();
    iniciar_configuracion(path_config);
    iniciar_estructuras();
    iniciar_semaforos();
    iniciar_mutex();
}