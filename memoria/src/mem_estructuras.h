#ifndef MEM_ESTRUCTURAS_H_
#define MEM_ESTRUCTURAS_H_
#include <stdio.h>
#include <stdlib.h>
#include "pthread.h"
#include "semaphore.h"
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/temporal.h>
#include "commons/config.h"
#include <utils/conexiones.h>



/*-----LOGS y CONFIG-----*/
extern t_log* logger_memoria;
extern t_config* config_memoria;

/*----CONEXIONES---------*/
extern int socket_peticion_FS;
extern int socket_servidor, socket_cliente, socket_kernel, socket_cpu, socket_io;


/*----PROCESOS e INSTRUCCIOENS----*/
extern t_list* lista_procesos_en_memoria;
extern t_list* lista_instrucciones;
extern t_list* listado_marcos;

/*----MEMORIA USUARIO-------------*/
extern void *memoria_espacio_usuario;

typedef struct {
    int id;
    char* path_instrucciones;
    t_list* instrucciones;
    t_list* tabla_de_paginas;
    int cantidad_de_paginas;
}t_proceso;

//tabla de paginas
typedef struct {
    int id;
    int marco;
    int bit_de_validez;
}t_pagina;

typedef struct {
    int id;
    int pid;
    int pagina_asignada;
    int bit_de_uso;
}t_marco;

#endif /* MEM_ESTRUCTURAS_H_ */