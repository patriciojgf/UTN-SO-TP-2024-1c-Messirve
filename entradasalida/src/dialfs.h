#ifndef STATIC_DIALFS_H_
#define STATIC_DIALFS_H_

#include <configuracion_io.h>
#include <init_estructuras.h>
#include <math.h>
#include <sys/mman.h>

void crear_archivo(char* nombre_archivo);
int liberar_bloques_de_archivo(char* nombre_archivo);
void truncar_archivo(t_solicitud_io* solicitud_io, char* nombre_archivo);

#endif

/*************** FUNCIONES AUXILIARES *****************/
/******************************************************/