#ifndef ATENDER_CPU_H
#define ATENDER_CPU_H

#include "configuracion_kernel.h"
#include "init_estructuras.h"
#include "planificador_cp.h"
#include "planificador_lp.h"

typedef struct {
    t_pcb* pcb;
    sem_t semaforo_pedido_ok;
    t_interfaz* interfaz; // Agregar este campo
} t_pedido_stdin2;

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
void atender_cpu_io_fs_create(t_pcb* pcb, t_instruccion* instruccion);
void atender_cpu_io_fs_delete(t_pcb* pcb, t_instruccion* instruccion);
void atender_cpu_io_fs_truncate(t_pcb* pcb, t_instruccion* instruccion);
void atender_cpu_io_fs_read(t_pcb* pcb, t_instruccion* instruccion);
void atender_cpu_io_fs_write(t_pcb* pcb, t_instruccion* instruccion);
int preparar_enviar_solicitud_io(t_pcb* pcb, t_instruccion* instruccion);

#endif /*ATENDER_CPU_H*/