#ifndef CONX_H
#define CONX_H

#include "configuracion_cpu.h"
#include "init_estructuras.h"
#include "instrucciones.h"

extern t_contexto* contexto_cpu;


void init_conexiones();
void gestionar_conexion_memoria();
void gestionar_conexion_interrupt();
void gestionar_conexion_dispatch();

void atender_peticiones_memoria();
void atender_peticiones_interrupt();
void atender_peticiones_dispatch();
void atender_peticiones_interrupt();

#endif /*CONX_H*/