#ifndef INIT_ESTRUC_MEM_H
#define INIT_ESTRUC_MEM_H

#include <commons/bitarray.h>
#include <commons/error.h>
#include <configuracion_memoria.h>
#include <mem_estructuras.h>
#include <math.h> //TODO: remover sino se usa

// void log_protegido_mem(char* mensaje);
void eliminar_espacio_de_usuario();
void init_memoria(char* path_config);
void iniciar_tabla_de_pagina(t_proceso* proceso);

#endif/*INIT_ESTRUC_MEM_H*/