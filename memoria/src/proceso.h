#ifndef PROCESO_H
#define PROCESO_H

#include "mem_estructuras.h"
#include "init_estructuras.h"

t_proceso* crear_proceso(int pid, char* path_instrucciones);
int resize_proceso(int pid, int new_size);
void eliminar_proceso(t_proceso* proceso);

char* get_instruccion_proceso(t_proceso* proceso, int PC);
t_proceso* get_proceso_memoria(int pid);
int get_marco_proceso(t_proceso* proceso, int nro_pagina);
int buscar_marco_por_pagina(int nro_pagina, int pid);
void eliminar_proceso_estructuras(t_buffer* buffer);



/*OUT*/
void confirmar_proceso_creado();
void confirmar_memoria_liberada();


#endif // PROCESO_H