#ifndef STATIC_CONSOLA_H_
#define STATIC_CONSOLA_H_

    #include <commons/collections/list.h>
    #include <commons/error.h>
    #include <commons/log.h>
    #include <commons/string.h>
    #include <utils/conexiones.h>
    // #include <utils/queues.h>
    #include <readline/history.h>
    #include <readline/readline.h>
    #include <signal.h>
    #include <stdlib.h>
    #include <pthread.h>
    #include <semaphore.h>
    
    extern sem_t mlog;
    extern t_log* logger_kernel;
    
    typedef struct comando
    {
        int cod_op;
        t_list* parametros; 
    }t_comando;

    void leer_consola(t_log* logger, int grado_multiprogramacion, int conexiones);
    void interpretar(t_comando* comando, char* leido);
    void log_protegido_kernel(char* mensaje);
    void _setup_parametros(t_comando* comando, char** leido_separado, int cod_op);
#endif