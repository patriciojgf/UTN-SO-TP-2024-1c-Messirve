#include "init_estructuras.h"

static void init_log();

//-----------------------------------------------------------------------------//
void log_protegido_io(char* mensaje){
	sem_wait(&mlog);
	log_info(logger_io, "%s", mensaje);
	sem_post(&mlog);
	free(mensaje);
}

static void init_log(){
    logger_io = iniciar_logger("io.log", "IO");
}

static void iniciar_configuracion(char* config_path){
    config_io = iniciar_config(config_path);
    if(config_io == NULL){
        log_error(logger_io,"No se pudo leer el archivo de configuracion");
        log_destroy(logger_io);
        config_destroy(config_io);
        exit(EXIT_FAILURE);
    }      
    TIPO_INTERFAZ = config_get_string_value(config_io, "TIPO_INTERFAZ");
    TIEMPO_UNIDAD_TRABAJO = atoi(config_get_string_value(config_io, "TIEMPO_UNIDAD_TRABAJO"));
    IP_KERNEL = config_get_string_value(config_io, "IP_KERNEL");
    PUERTO_KERNEL = config_get_string_value(config_io, "PUERTO_KERNEL");
    IP_MEMORIA = config_get_string_value(config_io, "IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value(config_io, "PUERTO_MEMORIA");
    PATH_BASE_DIALFS = config_get_string_value(config_io, "PATH_BASE_DIALFS");
    BLOCK_SIZE = atoi(config_get_string_value(config_io, "BLOCK_SIZE"));
    BLOCK_COUNT = atoi(config_get_string_value(config_io, "BLOCK_COUNT"));
}

static void iniciar_semaforos(){
    sem_init(&mlog,0,1);
}

static void iniciar_estructuras(){
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