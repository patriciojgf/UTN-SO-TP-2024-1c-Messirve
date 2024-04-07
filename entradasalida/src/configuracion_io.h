#ifndef STATIC_CONFIGURACION_IO_H_
#define STATIC_CONFIGURACION_IO_H_

    #include <utils/logconfig.h>
    #include <utils/conexiones.h>
    #include <pthread.h>

    #define LOG_NAME "io.log"
    #define PROCESS_NAME "IO"
    #define MSG_ERROR "No se pudo crear correctamente. "

    typedef struct config_io
    {
        char* tipo_interfaz;
        int tiempo_unidad_trabajo;
        char* ip_kernel;
        char* puerto_kernel;
        char* ip_memoria;
        char* puerto_memoria;
        char* path_base;
        int block_size;
        int block_count;
    }t_config_io;

    t_config_io* iniciar_config_io(t_config* config_io);
    void finalizar_config_io(t_config_io* config_io);

#endif