#ifndef ATENDER_CPU_H
#define ATENDER_CPU_H

#include "configuracion_kernel.h"
#include "init_estructuras.h"
#include "planificador_cp.h"
#include "planificador_lp.h"

// t_recurso* obtener_recurso(char* recurso);
// void liberar_recursos_pcb(t_pcb* pcb);
// void liberar_estructuras_memoria(t_pcb* pcb);

void atender_cpu_io_gen_sleep(t_pcb* pcb, t_instruccion* instruccion);
void atender_cpu_exit(t_pcb* pcb, char* motivo_exit);
void atender_cpu_fin_quantum(t_pcb* pcb);
void atender_cpu_wait(t_pcb* proceso_exec, t_instruccion* instruccion);
void atender_cpu_signal(t_pcb* pcb, t_recurso* recurso);
void atender_cpu_int_signal(t_pcb* pcb);
void atender_cpu_int_finalizar_proceso(t_pcb* pcb);
void atender_cpu_io_stdin_read(t_pcb* pcb, t_instruccion* instruccion);
void atender_cpu_io_stdout_write(t_pcb* pcb, t_instruccion* instruccion);

#endif /*ATENDER_CPU_H*/