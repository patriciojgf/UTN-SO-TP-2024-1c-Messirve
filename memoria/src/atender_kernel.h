#ifndef ATENDER_KERNEL_H
#define ATENDER_KERNEL_H

#include "mem_estructuras.h"

/*IN*/
void iniciar_estructura_proceso(t_buffer* buffer);
void eliminar_proceso_estructuras(t_buffer* buffer);


/*OUT*/
void confirmar_proceso_creado();
void confirmar_proceso_estructuras_eliminadas();

#endif // ATENDER_KERNEL_H