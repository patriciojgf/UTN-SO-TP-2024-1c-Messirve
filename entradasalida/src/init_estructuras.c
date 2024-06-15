#include "init_estructuras.h"

static void init_log();

//--------

static void init_log(){
    log_error(logger_io,"init_log");
}

static void iniciar_configuracion(char* config_path){
    log_error(logger_io,"iniciar_configuracion");
    config_io = iniciar_config(config_path);
    if(config_io == NULL){
        log_error(logger_io,"No se pudo leer el archivo de configuracion");
        log_destroy(logger_io);
        config_destroy(config_io);
        exit(EXIT_FAILURE);
    }      
    TIPO_INTERFAZ = config_get_string_value(config_io, "TIPO_INTERFAZ");    

    if(strcmp(TIPO_INTERFAZ, "GENERICA") == 0){
        IP_MEMORIA = config_get_string_value(config_io, "IP_MEMORIA");
        PUERTO_MEMORIA = config_get_string_value(config_io, "PUERTO_MEMORIA");
        BLOCK_SIZE = atoi(config_get_string_value(config_io, "BLOCK_SIZE"));
        BLOCK_COUNT = atoi(config_get_string_value(config_io, "BLOCK_COUNT"));
        IP_KERNEL = config_get_string_value(config_io, "IP_KERNEL");
        PUERTO_KERNEL = config_get_string_value(config_io, "PUERTO_KERNEL");
        TIEMPO_UNIDAD_TRABAJO = atoi(config_get_string_value(config_io, "TIEMPO_UNIDAD_TRABAJO"));
    }
    else if(strcmp(TIPO_INTERFAZ, "STDIN") == 0){
        IP_KERNEL = config_get_string_value(config_io, "IP_KERNEL");
        PUERTO_KERNEL = config_get_string_value(config_io, "PUERTO_KERNEL");
        IP_MEMORIA = config_get_string_value(config_io, "IP_MEMORIA");
        PUERTO_MEMORIA = config_get_string_value(config_io, "PUERTO_MEMORIA");
    }
}

static void iniciar_semaforos(){
    sem_init(&sem_io_stdin_read_ok,0,0);
}

static void iniciar_estructuras(){
    // lista_interfaz_socket = list_create();
}

static void init_nombre_interfaz(char* nombre){
    if(nombre == NULL){
        log_error(logger_io, "No se ha ingresado el nombre de la interfaz");
        exit(EXIT_FAILURE);
    }
    nombre_interfaz = nombre;
}

void init_io(char* path_config, char* nombre){
    init_log();
    iniciar_configuracion(path_config);
    iniciar_estructuras();
    iniciar_semaforos();
    init_nombre_interfaz(nombre);
}