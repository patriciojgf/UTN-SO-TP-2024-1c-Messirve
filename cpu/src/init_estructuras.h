#ifndef INIT_ESTRUCTURAS_H_
#define INIT_ESTRUCTURAS_H_

#include <configuracion_cpu.h>
// #include <mmu.h>

extern t_contexto* contexto_cpu;

void init_cpu(char* path_config);
void log_protegido_cpu(char* mensaje);

#endif /*INIT_ESTRUCTURAS_H_*/