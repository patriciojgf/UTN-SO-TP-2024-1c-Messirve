#include <consola.h>

void log_protegido_kernel(char* mensaje)
{
	sem_wait(&mlog);
	log_info(logger_kernel, "%s", mensaje);
	sem_post(&mlog);
}

void leer_consola(t_log* logger, int grado_multiprogramacion, sem_t m_multiprogramacion)
{
    printf("Leyendo consola...\n");
    char* leido;
    leido = readline(">");

    while(!string_equals_ignore_case(leido, "EXIT") /*|| !string_equals_ignore_case(leido, "SALIR")*/)
    {			
        t_comando* comando = malloc(sizeof(t_comando));
        comando->parametros=list_create();

        add_history(leido);

        interpretar(comando, leido);
        printf("el codigo es: %d\n", comando->cod_op);
        switch (comando->cod_op)
        {
            case HELPER: 
                imprimir_comandos_permitidos();
                break;

            case INICIAR_PLANIFICACION:
                log_info(logger_kernel, "Proximamente Iniciando planificación...");
                break;

            case DETENER_PLANIFICACION:
                log_info(logger_kernel, "Proximamente Se detiene la planificación...");
                break;

            case MULTIPROGRAMACION:
                int nuevo_grado_mult = atoi(list_get(comando->parametros,0));
				int grado_multiprogramacion_viejo = grado_multiprogramacion;
                cambiar_multiprogramacion(nuevo_grado_mult, grado_multiprogramacion, m_multiprogramacion);
                log_protegido_kernel(string_from_format("Grado Anterior: <%d> - Grado Actual: <%d>",grado_multiprogramacion_viejo, nuevo_grado_mult));
                log_info(logger, "Proximamente hace su magia...");
                break;

            case INICIAR_PROCESO:
                t_pcb* pcb_inicializado = crear_pcb();
                //muestro el id del pcb
                log_info(logger_kernel, "El id del pcb es: %d", pcb_inicializado->pid);
                pthread_t hilo_iniciar_proceso;
                pthread_create(&hilo_iniciar_proceso, NULL, (void*)planificador_lp_nuevo_proceso, pcb_inicializado);
                pthread_detach(hilo_iniciar_proceso);

                log_info(logger_kernel, "Proximamente Iniciando proceso...");
                break;

            case FINALIZAR_PROCESO:
                log_info(logger, "Proximamente Finalizando proceso...");
                break;

            case PROCESO_ESTADO:
                log_info(logger, "Proximamente Estado del proceso...");
                break;

            case EJECUTAR_SCRIPT:
                log_info(logger, "Proximamente Ejecutando script...");
                break;

            default:
                log_error(logger, "No se pudo ejecutar: %s", leido);
                break;
        }

        free(leido);

        if (comando->parametros != NULL)
        {	
            for (int i = 0; i < list_size(comando->parametros); i++)
            {
                free(list_get(comando->parametros, i));
            }
        }

        list_destroy(comando->parametros);
        free(comando);

        leido = readline(">");
    }

    if(string_equals_ignore_case(leido, "EXIT") || string_equals_ignore_case(leido, "SALIR")){
        kill(0, SIGINT);
    }

    free(leido);
    pthread_exit(NULL);
    
}

// Ejecutar Script de Instrucciones: Se encargará de abrir un archivo de instrucciones el cual se encontrará en la máquina donde corra el Kernel contendrá 
// alguna de las siguientes 5 Instrucciones y que las deberá ejecutar una a una hasta finalizar el archivo.
// Nomenclatura: EJECUTAR_SCRIPT [PATH]

// Iniciar proceso: Se encargará de ejecutar un nuevo proceso en base a un archivo dentro del file system de linux que se encontrará en la máquina donde 
// corra la Memoria. Dicho mensaje se encargará de la creación del proceso (PCB) y dejará el mismo en el estado NEW.
// Nomenclatura: INICIAR_PROCESO [PATH]

// Finalizar proceso: Se encargará de finalizar un proceso que se encuentre dentro del sistema. Este mensaje se encargará de realizar las mismas operaciones 
// como si el proceso llegara a EXIT por sus caminos habituales (deberá liberar recursos, archivos y memoria).
// Nomenclatura: FINALIZAR_PROCESO [PID]

// Detener planificación: Este mensaje se encargará de pausar la planificación de corto y largo plazo. El proceso que se encuentra en ejecución NO es desalojado, 
// pero una vez que salga de EXEC se va a pausar el manejo de su motivo de desalojo. De la misma forma, los procesos bloqueados van a pausar su transición a la cola de Ready.
// Nomenclatura: DETENER_PLANIFICACION

// Iniciar planificación: Este mensaje se encargará de retomar (en caso que se encuentre pausada) la planificación de corto y largo plazo. En caso que la 
// planificación no se encuentre pausada, se debe ignorar el mensaje.
// Nomenclatura: INICIAR_PLANIFICACION

// Modificar el grado de multiprogramación del módulo: Se cambiará el grado de multiprogramación del sistema reemplazandolo por el valor indicado.
// Nomenclatura: MULTIPROGRAMACION [VALOR]

// Listar procesos por estado: Se encargará de mostrar por consola el listado de los estados con los procesos que se encuentran dentro de cada uno de ellos.
// Nomenclatura: PROCESO_ESTADO

void _setup_parametros(t_comando* comando, char** leido_separado, int cod_op)
{
    if(string_array_size(leido_separado) != 2)
    {
        error_show("Parametros incorrectos."); 
        return;
    }
    comando->cod_op = cod_op;
    char* parametro = malloc(strlen(leido_separado[1])+1);
    strcpy(parametro, leido_separado[1]);
    list_add(comando->parametros, parametro);
}

void imprimir_comandos_permitidos()
{
	printf("============= COMANDOS AUTORIZADOS ============\n");
	printf("EJECUTAR_SCRIPT [PATH]\n");
	printf("INICIAR_PROCESO [PATH]\n");
	printf("FINALIZAR_PROCESO [PID]\n");
	printf("DETENER_PLANIFICACION\n");
	printf("INICIAR_PLANIFICACION\n");
	printf("MULTIPROGRAMACION [VALOR]\n");
	printf("PROCESO_ESTADO\n");
	printf("===============================================\n");
}

void interpretar(t_comando* comando, char* leido)
{
	char**leido_separado = string_split(leido, " ");

	comando->cod_op = 500;

    if(string_equals_ignore_case(leido_separado[0], "HELPER")){
        comando->cod_op = HELPER;
    }

	if(string_equals_ignore_case(leido_separado[0],"INICIAR_PLANIFICACION")){ // Nomenclatura: INICIAR_PLANIFICACION
		comando->cod_op = INICIAR_PLANIFICACION;
	}

	if(string_equals_ignore_case(leido_separado[0],"DETENER_PLANIFICACION")) // Nomenclatura: DETENER_PLANIFICACION
	{
		comando->cod_op = DETENER_PLANIFICACION;
	}

	if(string_equals_ignore_case(leido_separado[0],"MULTIPROGRAMACION")) // Nomenclatura: MULTIPROGRAMACION [VALOR]
	{
        _setup_parametros(comando, leido_separado, MULTIPROGRAMACION);
	}

	if(string_equals_ignore_case(leido_separado[0],"INICIAR_PROCESO")) // Nomenclatura: INICIAR_PROCESO [PATH]
	{
        _setup_parametros(comando, leido_separado, INICIAR_PROCESO);
	}

	if(string_equals_ignore_case(leido_separado[0],"FINALIZAR_PROCESO")) // Nomenclatura: FINALIZAR_PROCESO [PID]
	{
        _setup_parametros(comando, leido_separado, FINALIZAR_PROCESO);
	}

	if(string_equals_ignore_case(leido_separado[0],"PROCESO_ESTADO")) // Nomenclatura: PROCESO_ESTADO
	{
		comando->cod_op = PROCESO_ESTADO;
	}

    if (string_equals_ignore_case(leido_separado[0], "EJECUTAR_SCRIPT")) // Nomenclatura: EJECUTAR_SCRIPT [PATH]
    {
        _setup_parametros(comando, leido_separado, EJECUTAR_SCRIPT);
    }
    

    for (int i = 0; leido_separado[i] != NULL; i++)
    {
        free(leido_separado[i]);
    }
    
	free(leido_separado);
}

void cambiar_multiprogramacion(int nuevo_grado_mult, int grado_multiprogramacion, sem_t m_multiprogramacion)
{
    if(nuevo_grado_mult < grado_multiprogramacion)
    {
        for(int i = nuevo_grado_mult; i < grado_multiprogramacion; i++)
        {
            sem_wait(&m_multiprogramacion); 
        }
    }
    else
    {
        for(int i = nuevo_grado_mult; i > grado_multiprogramacion; i++)
        {
            sem_post(&m_multiprogramacion); 
        }
    }

    grado_multiprogramacion = nuevo_grado_mult;
}