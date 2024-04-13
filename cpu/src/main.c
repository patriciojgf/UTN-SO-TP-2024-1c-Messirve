#include <main_cpu.h>

void log_protegido_cpu(char* mensaje){
	sem_wait(&mlog);
	log_info(logger_cpu, "%s", mensaje);
	sem_post(&mlog);
	free(mensaje);
}

int main(int argc, char* argv[]) {
	sem_init(&mlog,0,1);
	logger_cpu = iniciar_logger(LOG_NAME, PROCESS_NAME);
    if(logger_cpu == NULL){
        error_show(MSG_ERROR);
        return EXIT_FAILURE;
    }
    //log_info(logger_cpu, "Iniciando CPU");

    config_cpu = iniciar_config(argv[1]);    
    if(config_cpu == NULL){
        sem_wait(&mlog);
        log_error(logger_cpu, MSG_ERROR);
        sem_post(&mlog);
        finalizar_log(logger_cpu);
        return EXIT_FAILURE;
    }

    pthread_create(&hilo_cpu, NULL, (void*) conectarMemoria, NULL);
    pthread_create(&hilo_kernelDispatch,NULL,(void*) conectarKernelDispatch,NULL);
    pthread_create(&hilo_kernelInterrupt,NULL,(void*) conectarKernelInterrupt,NULL);

    pthread_join(hilo_cpu, NULL);
    pthread_join(hilo_kernelDispatch, NULL);
    pthread_join(hilo_kernelInterrupt, NULL);

    finalizar_log(logger_cpu);
    finalizar_config_cpu(datos_cpu);
    finalizar_config(config_cpu);

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
    socket_interrupt = esperar_cliente(socket_servidor_interrupt);
    if(KERNEL_INTERRUPT == recibir_operacion(socket_interrupt)){
        log_protegido_cpu(string_from_format("Conexion con KERNEL-INTERRUPT exitosa"));
        log_protegido_cpu(string_from_format("Enviando mensaje de confirmacion a KERNEL-INTERRUPT"));
        bool confirmacion = true;
        send(socket_interrupt, &confirmacion, sizeof(bool), 0);
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
        int cod_op = recibir_operacion(socket_interrupt);
        switch (cod_op)
        {
            case MENSAJE:
                recibir_mensaje(socket_interrupt, logger_cpu);
                break;
            case DESALOJAR:
                log_protegido_cpu("Recibiendo PCB a desalojar");
                int size=0;
                void* buffer = recibir_buffer(&size, socket_interrupt);
                memcpy(&(motivo_interrupt), buffer, sizeof(int));
                memcpy(&(pid_interrupt), buffer + sizeof(int), sizeof(int));
                log_protegido_cpu(string_from_format("Motivo de desalojo: %d", motivo_interrupt));

                log_protegido_cpu("FALTAR IMPLEMENTAR DESALOJO");
                free(buffer);
                break;
            case -1:
                sem_wait(&mlog);
                log_error(logger_cpu, "Se ha desconectado el kernel interrupt");
                sem_post(&mlog);
                return EXIT_FAILURE;
            default:
                sem_wait(&mlog);
                log_error(logger_cpu, "Operacion desconocida");
                sem_post(&mlog);
                return EXIT_FAILURE;
        }

    }

}

int conectarKernelDispatch(){
    char* puerto_dispatch = config_get_string_value(config_cpu, "PUERTO_ESCUCHA_DISPATCH");
    log_protegido_cpu(string_from_format("Conectando con KERNEL-DISPATCH en puerto %s", puerto_dispatch));

    socket_servidor_dispatch = iniciar_servidor(puerto_dispatch);
    if(socket_servidor_dispatch == -1){
        sem_wait(&mlog);
        log_error(logger_cpu, "No se pudo crear el servidor para kernel dispatch");
        sem_post(&mlog);
        return EXIT_FAILURE;
    }
    log_protegido_cpu(string_from_format("Servidor KERNEL-DISPATCH: esperando cliente"));
    socket_dispatch = esperar_cliente(socket_servidor_dispatch);
    if(KERNEL_DISPATCH == recibir_operacion(socket_dispatch)){
        log_protegido_cpu(string_from_format("Servidor KERNEL-DISPATCH: conexión exitosa"));
        bool confirmacion = true;
        send(socket_dispatch, &confirmacion, sizeof(bool), 0);
        log_protegido_cpu(string_from_format("Servidor KERNEL-DISPATCH: Mensaje de confirmacion enviado"));
    }
    else{
        sem_wait(&mlog);
        log_error(logger_cpu, "No se pudo conectar con kernel dispatch");
        sem_post(&mlog);
        return EXIT_FAILURE;
    }

    while (1)
    {
        int cod_op = recibir_operacion(socket_dispatch);
        switch (cod_op)
        {
            case MENSAJE:
                recibir_mensaje(socket_dispatch, logger_cpu);
                break;
            case PCB:
                recibir_pcb(socket_dispatch);
                break;
            case -1:
                sem_wait(&mlog);
                log_error(logger_cpu, "Se ha desconectado el kernel dispatch");
                sem_post(&mlog);
                return EXIT_FAILURE;
            default:
                sem_wait(&mlog);
                log_error(logger_cpu, "Operacion desconocida");
                sem_post(&mlog);
                return EXIT_FAILURE;
        }

    }
}

void conectarMemoria(){
    socket_memoria = -1;

    char* ip = config_get_string_value(config_cpu, "IP_MEMORIA");
    char* puerto = config_get_string_value(config_cpu, "PUERTO_MEMORIA");
    log_protegido_cpu(string_from_format("Conectando con memoria en %s:%s", ip, puerto));
    
    if((socket_memoria = crear_conexion(ip, puerto)) <= 0){
        sem_wait(&mlog);
        log_error(logger_cpu, "No se ha podido conectar con la MEMORIA");
        sem_post(&mlog);
        return;
    }
    else{
        int identificador = CPU;

        send(socket_memoria, &identificador, sizeof(int), 0);
        log_protegido_cpu(string_from_format("Enviando identificador de CPU a memoria"));
        log_protegido_cpu(string_from_format("Esperando respuesta de memoria"));
		recv(socket_memoria, &tam_pag, sizeof(int), MSG_WAITALL);
        
		log_protegido_cpu(string_from_format("Conexion con memoria exitosa"));
		log_protegido_cpu(string_from_format("TAMANIO PAGINA MEMORIA: %d",tam_pag));
    }
}

void recibir_pcb(int socket){
    int size=0;
    void* buffer = recibir_buffer(&size, socket);

    desempaquetar_pcb(buffer);
    free(buffer);
}

void desempaquetar_pcb(void *buffer){
    int desplazamiento = 0;

	memcpy(&(pid), buffer + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);
	// memcpy(&(program_counter), buffer + desplazamiento, sizeof(int));
	// desplazamiento += sizeof(int);
    memcpy(&(registros_cpu.PC), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
	memcpy(&(registros_cpu.AX), buffer + desplazamiento, sizeof(uint8_t));
	desplazamiento += sizeof(uint8_t);
    memcpy(&(registros_cpu.BX), buffer + desplazamiento, sizeof(uint8_t));
    desplazamiento += sizeof(uint8_t);
    memcpy(&(registros_cpu.CX), buffer + desplazamiento, sizeof(uint8_t));
    desplazamiento += sizeof(uint8_t);
    memcpy(&(registros_cpu.DX), buffer + desplazamiento, sizeof(uint8_t));
    desplazamiento += sizeof(uint8_t);
    memcpy(&(registros_cpu.EAX), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&(registros_cpu.EBX), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&(registros_cpu.ECX), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&(registros_cpu.EDX), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&(registros_cpu.SI), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&(registros_cpu.DI), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
}