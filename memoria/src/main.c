#include "main.h"

int tamanio_pagina(){
    return atoi(config_get_string_value(config_memoria,"TAM_PAGINA"));
}

/* -------------------------------------Iniciar Memoria -----------------------------------------------*/
    int main(int argc, char **argv){
    config_memoria = iniciar_config(argv[1]);
    logger_memoria=iniciar_logger("memoria.log","MEMORIA");

    int socket_servidor, socket_cliente, socket_kernel, socket_cpu, socket_io;

    log_protegido_mem(string_from_format("Iniciando Conexiones..."));
    char *puerto = config_get_string_value(config_memoria, "PUERTO_ESCUCHA");
    log_protegido_mem(string_from_format("Iniciando servidor en puerto: %s", puerto));
    socket_servidor = iniciar_servidor(puerto);    
    if (socket_servidor == -1)
    {
        log_error(logger_memoria, "ERROR - No se pudo crear el servidor");
        return EXIT_FAILURE;
    }
    log_protegido_mem(string_from_format("Servidor listo para recibir clientes"));
    pthread_t hilo_kernel, hilo_cpu, hilo_io;
    for (int i = 0; i < 3; i++)
    {
        log_protegido_mem(string_from_format("Esperando Cliente..."));
        socket_cliente = esperar_cliente(socket_servidor);

        int cod_op = recibir_operacion(socket_cliente);

        switch (cod_op)
        {
        case KERNEL:
            log_protegido_mem(string_from_format("Se conecto el KERNEL"));
            socket_kernel = socket_cliente;
            pthread_create(&hilo_kernel, NULL, (void *)conectarKernel, &socket_kernel);
            break;
        case CPU:
            log_protegido_mem(string_from_format("Se conecto el CPU"));
            socket_cpu = socket_cliente;
            pthread_create(&hilo_cpu, NULL, (void *)conectarCpu, &socket_cpu);
            break;
        case IO: 
            log_protegido_mem(string_from_format("Se conecto el IO"));
            socket_io = socket_cliente;
            pthread_create(&hilo_io, NULL, (void *)conectarIO, &socket_io);
            break; 
        case STDIN:
            log_protegido_mem(string_from_format("Se conecto el STDIN"));
            break;
        case STDOUT:
            log_protegido_mem(string_from_format("Se conecto el STDOUT"));
            break;
        case GENERICA:
            log_protegido_mem(string_from_format("Se conecto una interfaz generica"));
            break;
        case DIALFS:
            log_protegido_mem(string_from_format("Se conecto el DIALFS"));
            break;            
        default:
            log_protegido_mem(string_from_format("No reconozco ese codigo"));
            break;
        }
    }
    pthread_join(hilo_kernel, NULL);
    return 0;
}


/* ------------------------------------Conexion Kernel ----------------------------------------------*/
int conectarKernel(int* socket_kernel){
    log_protegido_mem(string_from_format("Enviando mensaje de confirmacion..."));

    bool confirmacion = true;
    send(*socket_kernel, &confirmacion, sizeof(bool), 0);
    log_protegido_mem(string_from_format("Mensaje enviado"));
    int size;
    void *buffer;

    while (1){
        //int pid;
        log_protegido_mem(string_from_format("Esperando peticiones de KERNEL..."));
        int cod_kernel = recibir_operacion(*socket_kernel);

        switch (cod_kernel){
        case MENSAJE:
            recibir_mensaje(*socket_kernel,logger_memoria);
            break;
        // case INICIAR:
        //     //int size=0;
        //     buffer = recibir_buffer(&size, *socket_kernel);
        //     int pid=0;
        //     double size_proceso;
        //     int size_path=0;
        //     char* path;
        //     int desplazamiento = 0;

        //     memcpy(&pid, buffer +desplazamiento, sizeof(int));
        //     desplazamiento+=sizeof(int);
        //     memcpy(&size_proceso, buffer +desplazamiento, sizeof(double));
        //     desplazamiento+=sizeof(double);
        //     memcpy(&size_path,buffer +desplazamiento, sizeof(int));
        //     desplazamiento+=sizeof(int);
        //     path=malloc(size_path);
        //     memcpy(path, buffer +desplazamiento, size_path);

        //     printf("PATH: %s\n",path);
            
        //     FILE *f;
	    //     if (!(f = fopen(path, "r"))) {
        //         log_error(logger_memoria, "No se encontro el archivo de instrucciones");
        //         return -1;
	    //     }
        //     free(path);
        //     t_proceso* proceso_nuevo = agregar_proceso_instrucciones(f,pid);
            
        //     crear_tabla_de_paginas(size_proceso,proceso_nuevo);
        //     pedir_bloques_swap(proceso_nuevo);
           
        //     bool confirmacion = recibir_pos_swap(proceso_nuevo);
        //     send(*socket_kernel, &confirmacion, sizeof(bool), 0); //devuelve 1 (confirmado) o 0(error)
        //     break;
        // case LIBERAR_ESPACIO_PROCESO:
        //     int pid_a_liberar=0;

        //     buffer  = recibir_buffer(&size, *socket_kernel);
        //     memcpy(&pid_a_liberar, buffer , sizeof(int));

        //     log_protegido_mem(string_from_format("Eliminacion de Proceso PID: <%d>", pid_a_liberar));

        //     liberar_memoria_espacio_usuario(pid_a_liberar);
        // break;
        // case PAGE_FAULT:  // kernel avisa que hay que resolverlo            
	    // int p_id=0;
        //     int pag=0;
    
        //     buffer  = recibir_buffer(&size, *socket_kernel);
        //     memcpy(&p_id, buffer, sizeof(int));
        //     memcpy(&pag, buffer + sizeof(int), sizeof(int));
        //     resolver_page_fault(p_id,pag);
        //     enviar_mensaje("OK",*socket_kernel);
        //     break;
        case -1:
            log_error(logger_memoria,"El KERNEL se desconecto");
            // break;
            return EXIT_FAILURE;
        default:
            log_warning(logger_memoria, "Operacion desconocida.");
            // break;
            return EXIT_FAILURE;
        }
        free(buffer);
    }
    return 0;
}

/* ------------------------------------Conexion CPU ----------------------------------------------*/
int conectarCpu(int* socket_cpu){
    log_protegido_mem(string_from_format("Enviando mensaje de confirmacion..."));
    int tam_pagina = tamanio_pagina();
    send(*socket_cpu, &tam_pagina, sizeof(int), 0);
    log_protegido_mem(string_from_format("Tamanio de pagina enviado"));

    while (1){
        void *buffer;
        log_protegido_mem(string_from_format("Esperando peticiones de CPU..."));
        int cod_cpu = recibir_operacion(*socket_cpu);
        int size=0;
        int pid=0;
        switch (cod_cpu)
        {
            case MENSAJE:
                recibir_mensaje(*socket_cpu,logger_memoria);
                // break;
                return EXIT_FAILURE;
            default:
                log_warning(logger_memoria, "Operacion desconocida.");
                // break;
                return EXIT_FAILURE;
        }
    }
    return 0;
}

int conectarIO(int* socket_io){
    log_protegido_mem(string_from_format("Enviando mensaje de confirmacion..."));

    bool confirmacion = true;
    send(*socket_io, &confirmacion, sizeof(bool), 0);
    log_protegido_mem(string_from_format("Mensaje enviado"));
    int size;
    void *buffer;

    while (1){
        //int pid;
        log_protegido_mem(string_from_format("Esperando peticiones de IO..."));
        int cod_kernel = recibir_operacion(*socket_io);

        switch (cod_kernel){
        case MENSAJE:
            recibir_mensaje(*socket_io,logger_memoria);
            break;
        case -1:
            log_error(logger_memoria,"El IO se desconecto");
            break;
        default:
            log_warning(logger_memoria, "Operacion desconocida.");
            break;
        }
        free(buffer);
    }
    return 0;
}

void log_protegido_mem(char *mensaje){
    // sem_wait(&mlog);
    log_info(logger_memoria, "%s", mensaje);
    // sem_post(&mlog);
    free(mensaje);
}