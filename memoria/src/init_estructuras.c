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

    log_info(logger_memoria,"TAM_PAGINA: %d", TAM_PAGINA);
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

static void iniciar_memoria_usuario()
{
    memoria_espacio_usuario = malloc(TAM_MEMORIA);
	if(memoria_espacio_usuario == NULL)
    {
			log_error(logger_memoria, "Fallo Malloc");
	    	exit(1);
	}

    int cantidad_marcos = TAM_MEMORIA / TAM_PAGINA;
    log_info(logger_memoria, "Cantidad de marcos: %d", cantidad_marcos);
    listado_marcos = list_create();
    for(int i = 0; i < cantidad_marcos; i++)
    {
        t_marco* marcos = malloc(sizeof(t_marco));
        marcos->id = i;
        marcos->pid = -1;
        marcos->pagina_asignada = -1;
        marcos->bit_de_uso = 0;
        list_add(listado_marcos, marcos);
    }
}

//TODO: mover a donde corresponda
void eliminar_espacio_de_usuario()
{
    free(memoria_espacio_usuario);
}

//TODO: ver donde inicializar y si es necesario el static
static void iniciar_bitmap(int cantidad_marcos)
{
    void *bitmap_memoria_usuario = malloc(cantidad_marcos / 8);
    bitmap = bitarray_create_with_mode(bitmap_memoria_usuario, cantidad_marcos / 8, LSB_FIRST);
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

    //TAM_PAG = TAM_MARCO
    iniciar_bitmap(cant_paginas);

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
    log_info(logger_memoria, "Creacion de Tabla de Paginas PID: <%d>", proceso->id);
}

void init_memoria(char* path_config)
{
    init_log();
    iniciar_configuracion(path_config);
    iniciar_estructuras();
    iniciar_semaforos();
    // iniciar_espacio_de_usuario();
    // iniciar_tabla_de_pagina();
    iniciar_mutex();
    iniciar_memoria_usuario();
}