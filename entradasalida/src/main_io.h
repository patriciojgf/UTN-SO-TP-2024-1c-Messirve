#ifndef STATIC_MAIN_IO_H_
#define STATIC_MAIN_IO_H_

    #include <configuracion_io.h>
    #include <utils/conexiones.h>
    #include "semaphore.h"

    sem_t mlog;

    int socket_memoria;
    int socket_servidor_kernel;

    char* nombre_interfaz;

    t_log* logger_io;
    t_config* config_io;
    t_config_io* datos_io;

    pthread_t hilo_kernel;
    pthread_t hilo_memoria;

    void conectarMemoria();
    void conectarKernel();

#endif