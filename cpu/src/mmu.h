#ifndef STATIC_MMU_H_  
#define STATIC_MMU_H_

#include <configuracion_cpu.h>
#include <init_estructuras.h>
#include <math.h>

void destroy_fila_TLB(fila_tlb* row_to_destroy);
void iniciar_tlb();
void log_tlb();
int MMU(int pid, int dir_logica);

#endif