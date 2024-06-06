#ifndef GES_CONEXIONES_H_
#define GES_CONEXIONES_H_

#include <configuracion_kernel.h>
#include "init_estructuras.h"
#include "atender_cpu.h"


void init_conexiones();

void gestionar_conexion_io();

void gestionar_conexion_memoria();
void atender_peticiones_memoria();

void gestionar_conexion_dispatch();
void atender_peticiones_dispatch();

void gestionar_conexion_interrupt();
void atender_peticiones_interrupt();

void envio_interrupcion(int pid, int motivo);

#endif /* GES_CONEXIONES_H_ */