#include <main_io.h>

void log_protegido_io(char* mensaje){
    sem_wait(&mlog);
    log_info(logger_io, "%s", mensaje);
    sem_post(&mlog);
    free(mensaje);
}

int main(int argc, char* argv[]) {
    sem_init(&mlog,0,1);
	logger_io = iniciar_logger(LOG_NAME, PROCESS_NAME);
    if(logger_io == NULL){
        error_show(MSG_ERROR);
        return EXIT_FAILURE;
    }

    config_io = iniciar_config(argv[1]);    
    if(config_io == NULL){
        sem_wait(&mlog);
        log_error(logger_io, MSG_ERROR);
        sem_post(&mlog);
        finalizar_log(logger_io);
        return EXIT_FAILURE;
    }

    nombre_interfaz = argv[2];
    if(nombre_interfaz == NULL){
        sem_wait(&mlog);
        log_error(logger_io, "No se ha ingresado el nombre de la interfaz");
        sem_post(&mlog);
        finalizar_log(logger_io);
        finalizar_config(config_io);
        return EXIT_FAILURE;
    }

    // datos_io = iniciar_config_io(config_io);
    // log_info(logger_io, "Ip Memoria: %s - Puerto Memoria: %s", datos_io->ip_memoria, datos_io->puerto_memoria);
    // printf("Tipo de interfaz: %s \n", datos_io->tipo_interfaz); //TODO: remover

    pthread_create(&hilo_memoria, NULL, (void*) conectarMemoria, NULL);
    pthread_create(&hilo_kernel, NULL, (void*) conectarKernel, NULL);
    pthread_join(hilo_memoria, NULL);
    pthread_join(hilo_kernel, NULL);
    sleep(30000);

    finalizar_log(logger_io);
    // finalizar_config_io(datos_io);
    finalizar_config(config_io);

    return EXIT_SUCCESS;
}

void conectarKernel(){
    socket_servidor_kernel = -1;
    char* ip_kernel = config_get_string_value(config_io, "IP_KERNEL");
    char* puerto_kernel = config_get_string_value(config_io, "PUERTO_KERNEL");
    int identificador = texto_to_cod_op(config_get_string_value(config_io, "TIPO_INTERFAZ"));
    
    
    if((socket_servidor_kernel = crear_conexion(ip_kernel, puerto_kernel)) <= 0){
        sem_wait(&mlog);
        log_error(logger_io, "No se ha podido conectar con el KERNEL");
        sem_post(&mlog);
        return;
    }
    //1. Envio tipo de interfaz
    send(socket_servidor_kernel, &identificador, sizeof(int), 0);
    log_protegido_io(string_from_format("Tipo de interfaz enviado a KERNEL"));

    //2. Envio el nombre de la interfaz
    enviar_mensaje(nombre_interfaz, socket_servidor_kernel);
    log_protegido_io(string_from_format("Nombre de interfaz enviado a KERNEL"));

}

void conectarMemoria(){
    socket_memoria = -1;
    char* ip_memoria = config_get_string_value(config_io, "IP_MEMORIA");
    char* puerto_memoria = config_get_string_value(config_io, "PUERTO_MEMORIA");
    int identificador = texto_to_cod_op(config_get_string_value(config_io, "TIPO_INTERFAZ"));


    if((socket_memoria = crear_conexion(ip_memoria, puerto_memoria)) <= 0){
        sem_wait(&mlog);
        log_error(logger_io, "No se ha podido conectar con la MEMORIA");
        sem_post(&mlog);
        return;
    }

    //1. Envio tipo de interfaz
    send(socket_memoria, &identificador, sizeof(int), 0);
    log_protegido_io(string_from_format("Tipo de interfaz enviado a MEMORIA"));

    //2. Envio el nombre de la interfaz
    enviar_mensaje(nombre_interfaz, socket_memoria);
    log_protegido_io(string_from_format("Nombre de interfaz enviado a MEMORIA"));
}
