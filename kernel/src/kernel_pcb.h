#ifndef K_PCB_H
#define K_PCB_H

#include "configuracion_kernel.h"


t_pcb* crear_pcb();

//void enviar_pcb_cpu_dispatch(t_pcb* pcb);
void enviar_contexto_dispatch(t_pcb* pcb);

#endif/*K_PCB_H*/