#ifndef GESTION_MEM_H
#define GESTION_MEM_H

#include "init_estructuras.h"

int mmu(int direccion_logica);
char* leer_memoria(int direccion_logica, int cantidad_bytes);
int escribir_valor_en_memoria(int direccion_logica, int cantidad_bytes, char* valor);

#endif /*GESTION_MEM_H*/