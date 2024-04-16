#ifndef STATIC_CONSOLA_H_
#define STATIC_CONSOLA_H_

    #include <commons/collections/list.h>
    #include <commons/log.h>
    #include <commons/string.h>
    #include <utils/conexiones.h>
    #include <utils/queues.h>
    #include <readline/history.h>
    #include <readline/readline.h>
    #include <signal.h>
    #include <stdlib.h>
    
    typedef struct comando
    {
        int cod_op;
        t_list* parametros; 
    }t_comando;

    void leer_consola(t_log* logger, int grado_multiprogramacion, int conexiones);
    void interpretar(t_comando* comando, char* leido);

#endif