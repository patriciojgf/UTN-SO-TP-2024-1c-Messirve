#include "init_estructuras.h"

static t_planificacion algoritmo_planinifacion(char* algoritmo);
static void init_log();
static void iniciar_configuracion(char* config_path);
static void init_listas_planificacion();
static void init_semaforos();
static void init_pthread_mutex();
static void init_recursos();

static t_planificacion algoritmo_planinifacion(char* algoritmo){
    if(strcmp(algoritmo,"FIFO") == 0){
        return FIFO;
    }else if(strcmp(algoritmo,"RR") == 0){
        return RR;
    }else if(strcmp(algoritmo,"VRR") == 0){
        return VRR;
    }else{
        log_error(logger_kernel,"Algoritmo de planificacion invalido: %s",algoritmo);
        return -1;
    }
}

static void init_log(){
    logger_kernel = iniciar_logger("kernel.log","KERNEL");
}

static void iniciar_configuracion(char* config_path){
    config_kernel = iniciar_config(config_path);
    if(config_kernel == NULL){
        log_error(logger_kernel,"No se pudo leer el archivo de configuracion");
        log_destroy(logger_kernel);
        config_destroy(config_kernel);
        exit(2);
    }

    ALGORITMO_PLANIFICACION = algoritmo_planinifacion(config_get_string_value(config_kernel,"ALGORITMO_PLANIFICACION"));
    RECURSOS = config_get_array_value(config_kernel,"RECURSOS");
    INSTANCIAS_RECURSOS = config_get_array_value(config_kernel, "INSTANCIAS_RECURSOS"); 
    GRADO_MULTIPROGRAMACION = config_get_int_value(config_kernel, "GRADO_MULTIPROGRAMACION");
}

static void init_listas_planificacion(){
    log_warning(logger_kernel,"creando las listas");
    lista_plan_new = list_create();
    lista_plan_ready = list_create();
    lista_plan_execute = list_create();
    lista_plan_blocked = list_create();
    lista_plan_exit = list_create();

    lista_recursos = list_create();
    lista_archivos_abiertos = list_create();
}

static void init_semaforos(){
    sem_init(&mlog,0,1);
    sem_init(&m_multiprogramacion, 0, GRADO_MULTIPROGRAMACION);
    sem_init(&s_init_proceso_a_memoria,0,0);
}

static void init_pthread_mutex(){
    pthread_mutex_init(&mutex_conexiones,NULL);
    pthread_mutex_init(&mutex_plan_new,NULL);
    pthread_mutex_init(&mutex_plan_ready,NULL);
    pthread_mutex_init(&mutex_plan_exec,NULL);
    pthread_mutex_init(&mutex_plan_blocked,NULL);
    pthread_mutex_init(&mutex_plan_exit,NULL);
    pthread_mutex_init(&mutex_procesos_planificados,NULL);

    pthread_mutex_init(&mutex_pid_proceso,NULL);
    
}

static void init_recursos(){
    int i=0;
    while(RECURSOS[i] != NULL){
        t_recurso* nuevo_recurso = malloc(sizeof(t_recurso));
        nuevo_recurso->nombre = RECURSOS[i];
        nuevo_recurso->instancias = atoi(INSTANCIAS_RECURSOS[i]);
        nuevo_recurso->l_bloqueados = list_create();
        nuevo_recurso->pcb_asignado = NULL;
        pthread_mutex_init(&nuevo_recurso->mutex_bloqueados, NULL);
        
        list_add(lista_recursos,nuevo_recurso);
        i++;
    }
}

void init_kernel(char* path_config){
    init_log();
    iniciar_configuracion(path_config);
    init_listas_planificacion();
    init_semaforos();
    init_pthread_mutex();
    init_recursos();
}