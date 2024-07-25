#ifndef GESTION_MEM_USUARIO
#define GESTION_MEM_USUARIO
#include "configuracion_memoria.h"
#include "mem_estructuras.h"
void mem_escribir_dato_direccion_fisica(int dir_fisica, void* dato, int size,int pid);
void* mem_leer_dato_direccion_fisica(int dir_fisica, int size,int pid);

#endif /*GESTION_MEM_USUARIO*/