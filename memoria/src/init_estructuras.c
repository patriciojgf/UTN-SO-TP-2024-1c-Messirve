#include "init_estructuras.h"

//-----------------------------------------------------------------------------//
void log_protegido_mem(char *mensaje){
    sem_wait(&mlog);
    log_info(logger_memoria, "%s", mensaje);
    sem_post(&mlog);
    free(mensaje);
}

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

static void iniciar_estructuras(){
    lista_procesos_en_memoria = list_create();
}

static void iniciar_espacio_de_usuario()
{
    memoria_espacio_usuario = malloc(TAM_MEMORIA);
    if(memoria_espacio_usuario == NULL)
    {
        error_show("Se produjo un error al iniciar espacio_usuario");
        exit(1);
    }
}

//TODO: revisar cant_paginas y donde se va a ejecutar esta funcion
static void iniciar_tabla_de_pagina(int size_proceso, t_proceso* proceso)
{
    int tam_pagina = TAM_PAGINA; 
    int cant_paginas = ceil((size_proceso/tam_pagina)); //TODO: ver que onda con cant_paginas

    proceso->tabla_de_paginas = list_create();

    for(int i = 0; i < cant_paginas; i++)
    {
        t_tabla_pagina* tdp = malloc(sizeof(t_tabla_pagina));
        tdp->marco = 0; 
        tdp->modificado = 0; 
        tdp->presencia = 0;

        list_add(proceso->tabla_de_paginas, tdp);
    }

    // proceso_nuevo->cant_paginas = cant_paginas; //TODO: debería agregar cant_pagina a t_proceso
    log_protegido_mem(string_from_format("Creacion de Tabla de Paginas PID: <%d>", proceso->id));
}


void init_memoria(char* path_config){
    init_log();
    iniciar_configuracion(path_config);
    iniciar_estructuras();
    iniciar_semaforos();
    iniciar_espacio_de_usuario();
    // iniciar_tabla_de_pagina();
}