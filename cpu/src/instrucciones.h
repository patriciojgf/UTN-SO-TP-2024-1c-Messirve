#ifndef INSTRUCCIONES_H
#define INSTRUCCIONES_H

#include "configuracion_cpu.h"
#include "init_estructuras.h"

extern int pid;
extern int tam_pag;
extern int program_counter;
extern int socket_memoria, socket_servidor_dispatch,socket_dispatch;
extern t_registros_cpu registros_cpu;
extern bool flag_ejecucion, flag_interrupt;
extern t_log* logger_cpu;
extern t_contexto* contexto_cpu;

typedef struct {
    void* direccion;
    size_t tamano;  // Tamaño del registro en bytes
} info_registro_cpu;

// char* fetch_instruccion();
void ejecutar_proceso();
char* recibir_instruccion(int socket_cliente);
void devolver_contexto_a_dispatch(int motivo, t_instruccion* instruccion);

#endif // INSTRUCCIONES_H