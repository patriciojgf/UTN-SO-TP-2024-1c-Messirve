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

    log_info(logger_memoria,"TAM_PAGINA: %d", TAM_PAGINA);
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

//TODO: mover a donde corresponda
static void eliminar_espacio_de_usuario()
{
    free(memoria_espacio_usuario);
}

/*TODO
*
* revisar cant_paginas 
* revisar donde se va a ejecutar esta funcion
* agregar #include <math.h> donde corresponda
* 
*/
void iniciar_tabla_de_pagina(t_proceso* proceso)
{
    log_info(logger_memoria, "Inicializando tabla de pagina");
    // log_protegido_mem(string_from_format("PID: %d", proceso->id));
    // log_info(logger_memoria, "PID: %d", proceso->id);
    log_info(logger_memoria,"TAM_PAGINA: %d", TAM_PAGINA);
    int tam_pagina = TAM_PAGINA; 
    log_info(logger_memoria, "tam_pagina: %d", tam_pagina);
    
    // int cant_paginas = ceil((size_proceso/tam_pagina)); //TODO: ver que onda con cant_paginas
    int cant_paginas = TAM_MEMORIA/tam_pagina;
    log_info(logger_memoria, "cant_paginas: %d", cant_paginas);   

    // proceso->tabla_de_paginas = list_create(); => se inicializa en crear_proceso

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

//TODO: ver donde inicializar y si es necesario el static
static t_bitarray* iniciar_bitmap(int cantidad_marcos)
{
    void* bitmap_memoria_usuario = malloc (cantidad_marcos/8);
    return bitarray_create_with_mode(bitmap_memoria_usuario, cantidad_marcos/8, LSB_FIRST);
}

void init_memoria(char* path_config){
    init_log();
    iniciar_configuracion(path_config);
    iniciar_estructuras();
    iniciar_semaforos();
    iniciar_espacio_de_usuario();
    // iniciar_tabla_de_pagina();
}