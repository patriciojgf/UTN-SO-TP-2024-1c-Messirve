#ifndef STATIC_DIALFS_H_
#define STATIC_DIALFS_H_

#include <commons/collections/dictionary.h>
#include <configuracion_io.h>
#include <dirent.h>
#include <init_estructuras.h>
#include <math.h>
#include <sys/mman.h>

void crear_archivo(char* nombre_archivo);
void escribir_archivo(void* datos, char* nombre_archivo, int puntero, int tamanio);
void* leer_archivo(char* nombre_archivo, int puntero, int tamanio);
int liberar_bloques_de_archivo(char* nombre_archivo);
void truncar_archivo(int pid, int tamano_bytes, char* nombre_archivo);

#endif

/*************** FUNCIONES AUXILIARES *****************/
/******************************************************/