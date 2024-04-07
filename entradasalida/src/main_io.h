#ifndef STATIC_MAIN_IO_H_
#define STATIC_MAIN_IO_H_

    #include <configuracion_io.h>

    int socket_memoria;

    t_log* logger_io;
    t_config* config_io;
    t_config_io* datos_io;

    pthread_t hilo_io;

    void conectarMemoria();

#endif