#ifndef PROCESO_H
#define PROCESO_H

#include "mem_estructuras.h"

t_proceso* crear_proceso(int pid, char* path_instrucciones);
void eliminar_proceso(t_proceso* proceso);

char* get_instruccion_proceso(t_proceso* proceso, int PC);
t_proceso* get_proceso_memoria(int pid);



#endif // PROCESO_H