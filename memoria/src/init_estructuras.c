#include "init_estructuras.h"

static void init_log(){
    logger_memoria = iniciar_logger("memoria.log", "MEMORIA");
}

static void iniciar_configuracion(char* config_path){
    config_memoria = iniciar_config(config_path);  
    if(config_memoria == NULL){
        log_error(logger_memoria,"No se pudo leer el archivo de configuracion");
        log_destroy(logger_memoria);
        config_destroy(config_memoria);
        exit(2);
    }
    PUERTO_ESCUCHA = config_get_string_value(config_memoria, "PUERTO_ESCUCHA");
    TAM_MEMORIA = atoi(config_get_string_value(config_memoria,"TAM_MEMORIA"));
    TAM_PAGINA = atoi(config_get_string_value(config_memoria,"TAM_PAGINA"));
    PATH_INSTRUCCIONES = config_get_string_value(config_memoria, "PATH_INSTRUCCIONES");
    RETARDO_RESPUESTA = atoi(config_get_string_value(config_memoria,"RETARDO_RESPUESTA"));
}

static void iniciar_semaforos(){
    sem_init(&mlog,0,1);
}

static void iniciar_mutex(){
    pthread_mutex_init(&mutex_lista_interfaz,NULL);
}

static void iniciar_estructuras(){
    lista_interfaz_socket = list_create();
    lista_procesos_en_memoria = list_create();
}


void init_memoria(char* path_config){
    init_log();
    iniciar_configuracion(path_config);
    iniciar_estructuras();
    iniciar_semaforos();
    iniciar_mutex();
}