#ifndef ATENDER_CPU_H
#define ATENDER_CPU_H

#include "configuracion_kernel.h"
#include "init_estructuras.h"
#include "planificador_cp.h"
#include "planificador_lp.h"

void atender_cpu_io_gen_sleep(t_pcb* pcb, t_instruccion* instruccion);
void atender_cpu_exit(t_pcb* pcb, t_instruccion* instruccion);


#endif /*ATENDER_CPU_H*/