#include <main_cpu.h>

int main(int argc, char* argv[]) {
	logger_cpu = iniciar_logger(LOG_NAME, PROCESS_NAME);

    if(logger_cpu == NULL){
        error_show(MSG_ERROR);
        return EXIT_FAILURE;
    }

    config_cpu = iniciar_config(argv[1]);
    
    if(config_cpu == NULL){
        log_error(logger_cpu, MSG_ERROR);
        finalizar_log(logger_cpu);
        return EXIT_FAILURE;
    }

    datos_cpu = iniciar_config_cpu(config_cpu);

    log_info(logger_cpu, "Ip Memoria: %s - Puerto Memoria_ %s", datos_cpu->ip_memoria, datos_cpu->puerto_memoria);

    pthread_create(&hilo_cpu, NULL, (void*) conectarMemoria, NULL);
    pthread_join(hilo_cpu, NULL);

    finalizar_log(logger_cpu);
    finalizar_config_cpu(datos_cpu);
    finalizar_config(config_cpu);

    return EXIT_SUCCESS;
}

void conectarMemoria(){
    socket_memoria = -1;
    
    if((socket_memoria = crear_conexion(datos_cpu->ip_memoria, datos_cpu->puerto_memoria)) <= 0){
        log_error(logger_cpu, "No se ha podido conectar con la MEMORIA");
        return;
    }
    
    int identificador = CPU;
    bool confirmacion;

    send(socket_memoria, &identificador, sizeof(int), 0);
    recv(socket_memoria, &confirmacion, sizeof(bool), MSG_WAITALL);

    if(!confirmacion){
        log_error(logger_cpu, "ERROR: Handshake con memoria fallido");
        return;
    }

    log_info(logger_cpu, "Conexion con memoria exitosa");
}