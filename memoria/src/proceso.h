#ifndef PROCESO_H
#define PROCESO_H

#include "mem_estructuras.h"

t_proceso* crear_proceso(int pid, char* path_instrucciones);
void eliminar_proceso(t_proceso* proceso);

char* get_instruccion_proceso(t_proceso* proceso, int PC);
t_proceso* get_proceso_memoria(int pid);

void eliminar_proceso_estructuras(t_buffer* buffer);


/*OUT*/
void confirmar_proceso_creado();
void confirmar_proceso_estructuras_eliminadas();


#endif // PROCESO_H