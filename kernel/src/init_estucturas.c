#include "init_estructuras.h"

static t_planificacion algoritmo_planinifacion(char* algoritmo);
static void init_log();
static void iniciar_configuracion(char* config_path);
static void init_listas_planificacion();
static void init_semaforos();
static void init_pthread_mutex();
static void init_recursos();
static void init_variables_globales();

static void init_variables_globales(){
    var_id_ejecucion=0;
}

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
    QUANTUM = config_get_int_value(config_kernel, "QUANTUM");

    IP_MEMORIA = config_get_string_value(config_kernel,"IP_MEMORIA");
	PUERTO_MEMORIA = config_get_string_value(config_kernel,"PUERTO_MEMORIA");
	IP_CPU = config_get_string_value(config_kernel,"IP_CPU");
	PUERTO_CPU_DISPATCH = config_get_string_value(config_kernel,"PUERTO_CPU_DISPATCH");   
    PUERTO_CPU_INTERRUPT = config_get_string_value(config_kernel,"PUERTO_CPU_INTERRUPT");
    PUERTO_ESCUCHA = config_get_string_value(config_kernel,"PUERTO_ESCUCHA");
}

static void init_listas_planificacion(){
    log_warning(logger_kernel,"creando las listas");
    lista_plan_new = list_create();
    lista_plan_ready = list_create();
    lista_plan_ready_vrr = list_create();
    // lista_plan_execute = list_create();
    lista_plan_blocked = list_create();
    lista_plan_exit = list_create();

    lista_recursos = list_create();
    lista_archivos_abiertos = list_create();
}

static void init_semaforos(){
    sem_init(&sem_multiprogramacion, 0, GRADO_MULTIPROGRAMACION);
    sem_init(&s_init_proceso_a_memoria,0,0);
    sem_init(&s_conexion_memoria_ok,0,0);
    sem_init(&s_conexion_cpu_d_ok,0,0);
    sem_init(&s_conexion_cpu_i_ok,0,0);
    sem_init(&sem_pcb_desalojado,0,0);
    sem_init(&sem_plan_exec_libre,0,1);
    sem_init(&sem_plan_ready,0,0);
    sem_init(&sem_plan_new,0,0);
    sem_init(&sem_planificacion_activa,0,0);
    sem_init(&s_pedido_io_enviado,0,0);
}

static void init_pthread_mutex(){
    pthread_mutex_init(&mutex_conexiones,NULL);
    pthread_mutex_init(&mutex_plan_new,NULL);
    pthread_mutex_init(&mutex_plan_ready,NULL);
    pthread_mutex_init(&mutex_plan_exec,NULL);
    pthread_mutex_init(&mutex_plan_blocked,NULL);
    pthread_mutex_init(&mutex_plan_exit,NULL);
    pthread_mutex_init(&mutex_procesos_planificados,NULL);
    pthread_mutex_init(&mutex_detener_planificacion,NULL);
    pthread_mutex_init(&mutex_pid_proceso,NULL);
    pthread_mutex_init(&mutex_lista_interfaz,NULL);
    pthread_mutex_init(&mutex_id_ejecucion,NULL);  
    pthread_mutex_init(&mutex_lista_recursos,NULL);
    pthread_mutex_init(&mutex_grado_multiprogramacion,NULL); 
}

static void init_recursos(){
    int i=0;
    while(RECURSOS[i] != NULL){
        t_recurso* nuevo_recurso = malloc(sizeof(t_recurso));
        nuevo_recurso->nombre = RECURSOS[i];
        nuevo_recurso->instancias = atoi(INSTANCIAS_RECURSOS[i]);
        nuevo_recurso->l_bloqueados = list_create();
        // nuevo_recurso->pcb_asignado = NULL;
        pthread_mutex_init(&nuevo_recurso->mutex_bloqueados, NULL);
        
        list_add(lista_recursos,nuevo_recurso);
        i++;
    }
}

static void agrego_instruccion_permitida(t_list* listado_instrucciones, char* instruccion_nueva, t_codigo_consola codigo_inst, int cantidad_parametros){
    t_instruccion_consola* instruccion = malloc(sizeof(t_instruccion_consola));
    instruccion->cod_identificador = codigo_inst;
    instruccion->cantidad_parametros = cantidad_parametros;
    instruccion->nombre = instruccion_nueva;
    list_add(listado_instrucciones,instruccion);
}

static void init_instrucciones_consola(){
    lista_instrucciones_permitidas = list_create();
// Nomenclatura: EJECUTAR_SCRIPT [PATH]
// Nomenclatura: INICIAR_PROCESO [PATH]
// Nomenclatura: FINALIZAR_PROCESO [PID]
// Nomenclatura: DETENER_PLANIFICACION
// Nomenclatura: INICIAR_PLANIFICACION
// Nomenclatura: MULTIPROGRAMACION [VALOR]
    agrego_instruccion_permitida(lista_instrucciones_permitidas,"EJECUTAR_SCRIPT",EJECUTAR_SCRIPT,1);
    agrego_instruccion_permitida(lista_instrucciones_permitidas,"INICIAR_PROCESO",INICIAR_PROCESO,1);
    agrego_instruccion_permitida(lista_instrucciones_permitidas,"FINALIZAR_PROCESO",FINALIZAR_PROCESO,1);
    agrego_instruccion_permitida(lista_instrucciones_permitidas,"DETENER_PLANIFICACION",DETENER_PLANIFICACION,0);
    agrego_instruccion_permitida(lista_instrucciones_permitidas,"INICIAR_PLANIFICACION",INICIAR_PLANIFICACION,0);
    agrego_instruccion_permitida(lista_instrucciones_permitidas,"MULTIPROGRAMACION",MULTIPROGRAMACION,1);
    agrego_instruccion_permitida(lista_instrucciones_permitidas,"PROCESO_ESTADO",PROCESO_ESTADO,0);
    agrego_instruccion_permitida(lista_instrucciones_permitidas,"HELP",HELPER,0);
}

void init_kernel(char* path_config){
    init_log();
    iniciar_configuracion(path_config);
    init_listas_planificacion();
    init_semaforos();
    init_pthread_mutex();
    init_recursos();
    init_variables_globales();
    init_instrucciones_consola();
}
