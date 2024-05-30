#include <main_cpu.h>

int main(int argc, char* argv[]) {
	init_cpu(argv[1]);   
    init_conexiones();
    gestionar_conexion_memoria();   //cliente   detach
    gestionar_conexion_interrupt(); //servidor  detach
    gestionar_conexion_dispatch();  //servidor  join
    finalizar_log(logger_cpu);
    finalizar_config(config_cpu);

    return EXIT_SUCCESS;
}

int conectarMemoria(){
    socket_memoria = -1;

    char* ip = config_get_string_value(config_cpu, "IP_MEMORIA");
    char* puerto = config_get_string_value(config_cpu, "PUERTO_MEMORIA");
    log_protegido_cpu(string_from_format("Conectando con memoria en %s:%s", ip, puerto));
    
    if((socket_memoria = crear_conexion(ip, puerto)) <= 0){
        sem_wait(&mlog);
        log_error(logger_cpu, "No se ha podido conectar con la MEMORIA");
        sem_post(&mlog);
        return EXIT_FAILURE;
    }
    else{
        int identificador = CPU;

        send(socket_memoria, &identificador, sizeof(int), 0);
		recv(socket_memoria, &tam_pag, sizeof(int), MSG_WAITALL);
        
		log_protegido_cpu(string_from_format("Conexion con memoria exitosa"));
		log_protegido_cpu(string_from_format("TAMANIO PAGINA MEMORIA: %d",tam_pag));
    }
    return EXIT_SUCCESS;
}

int conectarKernelDispatch(){

    // char* puerto_dispatch = config_get_string_value(config_cpu, "PUERTO_ESCUCHA_DISPATCH");
    // log_protegido_cpu(string_from_format("Conectando con KERNEL-DISPATCH en puerto %s", puerto_dispatch));

    // socket_servidor_dispatch = iniciar_servidor(puerto_dispatch);
    // if(socket_servidor_dispatch == -1){
    //     sem_wait(&mlog);
    //     log_error(logger_cpu, "No se pudo crear el servidor para kernel dispatch");
    //     sem_post(&mlog);
    //     return EXIT_FAILURE;
    // }
    // log_protegido_cpu(string_from_format("Esperando Cliente..."));
    // socket_cliente_dispatch = esperar_cliente(socket_servidor_dispatch);
    // handshake_server(socket_cliente_dispatch);

    // log_protegido_cpu(string_from_format("Servidor KERNEL-DISPATCH: Servidor listo para recibir clientes\n"));
    // socket_cliente_dispatch = esperar_cliente(socket_servidor_dispatch);




    // if(KERNEL_DISPATCH == recibir_operacion(socket_cliente_dispatch)){
    //     log_protegido_cpu(string_from_format("Servidor KERNEL-DISPATCH: Enviando mensaje de confirmacion..."));
    //     bool confirmacion = true;
    //     send(socket_cliente_dispatch, &confirmacion, sizeof(bool), 0);
    //     log_protegido_cpu(string_from_format("Servidor KERNEL-DISPATCH: Mensaje de confirmacion enviado"));
    // }
    // else{
    //     sem_wait(&mlog);
    //     log_error(logger_cpu, "No se pudo conectar con kernel dispatch");
    //     sem_post(&mlog);
    //     return EXIT_FAILURE;
    // }

    // while (1)
    // {
    //     log_protegido_cpu(string_from_format("esperando conexion dispatch"));
    //     int cod_op = recibir_operacion(socket_dispatch);
    //     log_warning(logger_cpu, "Operacion recibida en socket_dispatch");
    //     switch (cod_op)
    //     {
    //         case MENSAJE:
    //             recibir_mensaje(socket_dispatch, logger_cpu);
    //             break;
    //         case PCB:
    //             log_warning(logger_cpu, "PCB");
    //             int size=0;
    //             void *buffer = recibir_buffer(&size, socket_dispatch);
    //             log_warning(logger_cpu, "recibir_buffer");
    //             _recibir_nuevo_contexto(buffer);
    //             log_warning(logger_cpu, "_recibir_nuevo_contexto");
    //             log_info(logger_cpu, "PCB recibido con pid %d", contexto_cpu->pid);
    //             flag_ejecucion = true;
    //             // pthread_t thread;
    //             // pthread_create(&thread, NULL, (void*)_ejecutar_proceso, NULL);
    //             // pthread_detach(thread);                
    //             _ejecutar_proceso();
    //             break;
    //         case -1:
    //             sem_wait(&mlog);
    //             log_error(logger_cpu, "Se ha desconectado el kernel dispatch");
    //             sem_post(&mlog);
    //             return EXIT_FAILURE;
    //         default:
    //             sem_wait(&mlog);
    //             log_error(logger_cpu, "Operacion desconocida");
    //             sem_post(&mlog);
    //             return EXIT_FAILURE;
    //     }

    // }
    return EXIT_SUCCESS;
}

int conectarKernelInterrupt(){
    char* puerto_interrupt = config_get_string_value(config_cpu, "PUERTO_ESCUCHA_INTERRUPT");
    log_protegido_cpu(string_from_format("Conectando con KERNEL-INTERRUPT en puerto %s", puerto_interrupt));

    socket_servidor_interrupt = iniciar_servidor(puerto_interrupt);
    if(socket_servidor_interrupt == -1){
        sem_wait(&mlog);
        log_error(logger_cpu, "No se pudo crear el servidor para KERNEL-INTERRUPT");
        sem_post(&mlog);
        return EXIT_FAILURE;
    }
    log_protegido_cpu(string_from_format("Servidor de KERNEL-INTERRUPT listo para recibir clientes"));
    socket_cliente_interrupt = esperar_cliente(socket_servidor_interrupt);
    if(KERNEL_INTERRUPT == recibir_operacion(socket_cliente_interrupt)){
        log_protegido_cpu(string_from_format("Conexion con KERNEL-INTERRUPT exitosa"));
        log_protegido_cpu(string_from_format("Enviando mensaje de confirmacion a KERNEL-INTERRUPT"));
        bool confirmacion = true;
        send(socket_cliente_interrupt, &confirmacion, sizeof(bool), 0);
        log_protegido_cpu(string_from_format("Mensaje de confirmacion enviado a KERNEL-INTERRUPT"));
    }
    else{
        sem_wait(&mlog);
        log_error(logger_cpu, "No se pudo conectar con KERNEL-INTERRUPT");
        sem_post(&mlog);
        return EXIT_FAILURE;
    }

    while (1)
    {
        int cod_op = recibir_operacion(socket_cliente_interrupt);
        switch (cod_op)
        {
            case MENSAJE:
                recibir_mensaje(socket_cliente_interrupt, logger_cpu);
                break;
            case DESALOJAR:
                log_protegido_cpu("Recibiendo PCB a desalojar");
                int size=0;
                void* buffer = recibir_buffer(&size, socket_cliente_interrupt);
                memcpy(&(motivo_interrupt), buffer, sizeof(int));
                memcpy(&(pid_interrupt), buffer + sizeof(int), sizeof(int));
                // log_protegido_cpu(string_from_format("Motivo de desalojo: %d", motivo_interrupt));
                if(pid_interrupt == pid){
                    flag_interrupt = true;
                }
                free(buffer);

                log_protegido_cpu("FALTAR IMPLEMENTAR DESALOJO");
                break;
            case -1:
                sem_wait(&mlog);
                log_error(logger_cpu, "Se ha desconectado el kernel interrupt");
                sem_post(&mlog);
                return EXIT_FAILURE;
            default:
                sem_wait(&mlog);
                log_warning(logger_cpu, "Operacion desconocida");
                sem_post(&mlog);
                break;
        }
    }
    return 0;
}






