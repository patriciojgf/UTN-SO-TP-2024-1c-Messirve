#include <consola.h>

static void ejecutar_script(const char* path);

static void listar_los_pid_por_lista(){
    log_info(logger_kernel, "!------------------------!");
    log_info(logger_kernel, "Listado de PID por ESTADO");
    log_info(logger_kernel, "NEW: %s", listado_pids(lista_plan_new));
    log_info(logger_kernel, "READY: %s", listado_pids(lista_plan_ready));
    if(ALGORITMO_PLANIFICACION==VRR){
        log_info(logger_kernel, "READY Prioridad: %s", listado_pids(lista_plan_ready_vrr));
    }
    if(proceso_exec==NULL){
        log_info(logger_kernel, "EXEC: []");
    }
    else{
        log_info(logger_kernel, "EXEC: [%d]", proceso_exec->pid);
    }    
    log_info(logger_kernel, "BLOCKED: %s", listado_pids(lista_plan_blocked));
    log_info(logger_kernel, "EXIT: %s", listado_pids(lista_plan_exit));
    log_info(logger_kernel, "!------------------------!");
}



static void _detener_planificacion(){
    if(planificacion_detenida == 1){
        // log_info(logger_kernel, "DETENER_PLANIFICACION ya se encuentra activo");
    }
    else{
        planificacion_detenida = 1;
        check_detener_planificador();
    }
}

static void _iniciar_planificacion(){
    if(planificacion_detenida == 0){
        // log_info(logger_kernel, "INICIAR_PLANIFICACION ya se encuentra activo");
    }
    else{
        planificacion_detenida = 0;
        sem_post(&sem_planificacion_activa);
        // log_info(logger_kernel, "_iniciar_planificacion:sem_planificacion_activa");
    }    
}

static void _help()
{
    printf("----------- HELP -----------\n");
    printf("1. EJECUTAR_SCRIPT [PATH]\n");
    printf("2. INICIAR_PROCESO [PATH]\n");
    printf("3. FINALIZAR_PROCESO [PID]\n");
    printf("4. DETENER_PLANIFICACION\n");
    printf("5. INICIAR_PLANIFICACION\n");
    printf("6. MULTIPROGRAMACION [VALOR]\n");
    printf("7. PROCESO_ESTADO\n");
    printf("----------------------------\n");
}

static bool _check_instruccion(char* leido) {
    // Divide la cadena de entrada en partes utilizando espacios como delimitadores.
    char** comando_consola = string_split(leido, " ");
    
    // Calcula la cantidad de parámetros obteniendo el tamaño del array - 1 (excluyendo el nombre del comando).
    int cantidad_de_parametros = string_array_size(comando_consola) - 1;
    
    // Inicializa la variable para determinar si el comando es válido.
    bool comando_valido = false;

    // Crea un iterador para la lista de instrucciones definidas.
    t_list_iterator* iterador_carousel = list_iterator_create(lista_instrucciones_permitidas);
    
    // Itera a través de cada instrucción disponible para encontrar una coincidencia.
    while (list_iterator_has_next(iterador_carousel)) {
        t_instruccion_consola* instruccion = list_iterator_next(iterador_carousel);
        
        // Compara el nombre de la instrucción con el primer elemento del comando dividido.
        if (string_equals_ignore_case(instruccion->nombre , comando_consola[0])) {
            // Comprueba si la cantidad de parámetros coincide con la definición de la instrucción.
            if (instruccion->cantidad_parametros == cantidad_de_parametros) {
                // Marca el comando como válido y detiene la iteración ya que se encontró una coincidencia completa.
                comando_valido = true;
                printf("> ");
                break;
            } else {
                // Registra un aviso si el comando fue reconocido pero el número de parámetros no coincide.
                log_warning(logger_kernel, "Número de parámetros incorrecto para el comando %s", comando_consola[0]);
            }
        }
    }

    // Si después de la iteración el comando no es válido, registra un error.
    if (!comando_valido) {
        log_error(logger_kernel, "Comando no reconocido o parámetros incorrectos: %s", leido);
    }

    // Destruye el iterador para liberar recursos.
    list_iterator_destroy(iterador_carousel);
    
    // Libera el array de cadenas generado por `string_split`.
    string_array_destroy(comando_consola);

    // Retorna true si el comando es válido, false de lo contrario.
    return comando_valido;
}

static void _ejecutar_comando_validado(char* leido) {
    char** comando_consola = string_split(leido, " ");

    // Función para buscar la instrucción en la lista.
    bool _buscar_instruccion_consola(t_instruccion_consola* instruccion) {
        return string_equals_ignore_case(instruccion->nombre , comando_consola[0]);
    }

    // Encontrar la instrucción correspondiente.
    t_instruccion_consola* instruccion = list_find(lista_instrucciones_permitidas, (void*) _buscar_instruccion_consola);

    if (instruccion == NULL) {
        log_error(logger_kernel, "Instrucción no reconocida: %s", comando_consola[0]);
        string_array_destroy(comando_consola);
        return;
    }
    // Evaluar en el SWITCH CASE
    switch (instruccion->cod_identificador) {
        case EJECUTAR_SCRIPT:{
            ejecutar_script(comando_consola[1]);
            break;
    break;
        }
        case INICIAR_PROCESO: {
            t_pcb* pcb_inicializado = crear_pcb(comando_consola[1]);
            // log_info(logger_kernel, "El id del pcb es: %d", pcb_inicializado->pid);
            pthread_t hilo_iniciar_proceso;
            pthread_create(&hilo_iniciar_proceso, NULL, (void*)planificador_lp_nuevo_proceso, pcb_inicializado);
            pthread_detach(hilo_iniciar_proceso);
            break;
        }
        case FINALIZAR_PROCESO: {
            finalizar_proceso(atoi(comando_consola[1]));
            break;
        }
        case DETENER_PLANIFICACION:
            pthread_t hilo_deneter_planificacion;
            pthread_create(&hilo_deneter_planificacion, NULL, (void*)_detener_planificacion, NULL);
            pthread_detach(hilo_deneter_planificacion);            
            break;
        case INICIAR_PLANIFICACION:
            pthread_t hilo_iniciar_planificacion;
            pthread_create(&hilo_iniciar_planificacion, NULL, (void*)_iniciar_planificacion, NULL);
            pthread_detach(hilo_iniciar_planificacion);     
            break;
        case MULTIPROGRAMACION: {
            int nuevo_grado_mult = atoi(comando_consola[1]);
            // int grado_multiprogramacion_viejo = GRADO_MULTIPROGRAMACION;
            cambiar_multiprogramacion(nuevo_grado_mult);
            //log_protegido_kernel(string_from_format("Grado Anterior: <%d> - Grado Actual: <%d>",grado_multiprogramacion_viejo, nuevo_grado_mult));
            // log_info(logger_kernel, "Proximamente hace su magia...");
            break;
        }
        case PROCESO_ESTADO:{
            listar_los_pid_por_lista();    
            break;
        }
        case HELPER:{
            _help(); 
            break;            
        }
        default:
            log_warning(logger_kernel, "Operación no soportada.");
            break;
    }

    string_array_destroy(comando_consola);
}

void procesar_comandos_consola() {
    // Leer el primer comando.
    char* leido = readline("> ");
    // Continuar leyendo mientras la línea no esté vacía.
    while (leido != NULL && strcmp(leido, "") != 0 && !(string_equals_ignore_case(leido, "EXIT") || string_equals_ignore_case(leido, "SALIR"))) {
        // Validar y procesar el comando si es válido.
        if (_check_instruccion(leido)) {
            printf("Comando válido\n");
            _ejecutar_comando_validado(leido);
        } else {
            printf("Comando inválido o incorrecto\n");
        }
        add_history(leido);
        // Liberar la memoria asignada a la línea leída y leer la siguiente.
        free(leido);
        leido = readline("> ");
    }
    // Liberar la última lectura si es nula o vacía.
    if (leido) {
        free(leido);
    }
}

void cambiar_multiprogramacion(int nuevo_grado_mult)
{
    pthread_mutex_lock(&mutex_grado_multiprogramacion);  // Asegurar acceso atómico a GRADO_MULTIPROGRAMACION
    if (nuevo_grado_mult < GRADO_MULTIPROGRAMACION) {
        for (int i = GRADO_MULTIPROGRAMACION; i > nuevo_grado_mult; i--) {
            sem_wait(&sem_multiprogramacion);  // Reducir la capacidad permitida de multiprogramación
        }
    } else if (nuevo_grado_mult > GRADO_MULTIPROGRAMACION) {
        for (int i = GRADO_MULTIPROGRAMACION; i < nuevo_grado_mult; i++) {
            sem_post(&sem_multiprogramacion);  // Aumentar la capacidad permitida de multiprogramación
        }
    }
    GRADO_MULTIPROGRAMACION = nuevo_grado_mult;
    pthread_mutex_unlock(&mutex_grado_multiprogramacion);  // Liberar el mutex
}

static void ejecutar_script(const char* path) {
    FILE* archivo = fopen(path, "r");
    if (!archivo) {
        log_error(logger_kernel, "No se pudo abrir el archivo de script: %s", path);
        return;
    }

    char* linea = NULL;
    size_t longitud = 0;
    ssize_t leido;

    while ((leido = getline(&linea, &longitud, archivo)) != -1) {
        // Elimina el salto de línea al final de la línea leída
        if (linea[leido - 1] == '\n') {
            linea[leido - 1] = '\0';
        }
        
        // Procesa la línea como si fuera un comando ingresado en la consola
        if (_check_instruccion(linea)) {
            _ejecutar_comando_validado(linea);
        } else {
            printf("Comando inválido o incorrecto en el script\n");
        }
        sleep(1);
    }

    free(linea); // Liberar la línea leída
    fclose(archivo); // Cerrar el archivo
}