#include <main_io.h>

int main(int argc, char* argv[]) {
	logger_io = iniciar_logger(LOG_NAME, PROCESS_NAME);

    if(logger_io == NULL){
        error_show(MSG_ERROR);
        return EXIT_FAILURE;
    }

    config_io = iniciar_config(argv[1]);
    
    if(config_io == NULL){
        log_error(logger_io, MSG_ERROR);
        finalizar_log(logger_io);
        return EXIT_FAILURE;
    }

    datos_io = iniciar_config_io(config_io);
    log_info(logger_io, "Ip Memoria: %s - Puerto Memoria: %s", datos_io->ip_memoria, datos_io->puerto_memoria);
    printf("Tipo de interfaz: %s \n", datos_io->tipo_interfaz); //TODO: remover

    pthread_create(&hilo_io, NULL, (void*) conectarMemoria, NULL);
    pthread_join(hilo_io, NULL);

    finalizar_log(logger_io);
    finalizar_config_io(datos_io);
    finalizar_config(config_io);

    return EXIT_SUCCESS;
}

void conectarMemoria(){
    socket_memoria = -1;

    if((socket_memoria = crear_conexion(datos_io->ip_memoria, datos_io->puerto_memoria)) <= 0){
        log_error(logger_io, "No se ha podido conectar con la MEMORIA");
        return;
    }
    
    int identificador = IO;
    bool confirmacion;

    send(socket_memoria, &identificador, sizeof(int), 0);
    recv(socket_memoria, &confirmacion, sizeof(bool), MSG_WAITALL);

    if(!confirmacion){
        log_error(logger_io, "ERROR: Handshake con memoria fallido");
        return;
    }

    log_info(logger_io, "Conexion con memoria exitosa");
}
