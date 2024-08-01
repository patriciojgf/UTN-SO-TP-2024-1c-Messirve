#ifndef STATIC_DIALFS_H_
#define STATIC_DIALFS_H_

#include <commons/collections/dictionary.h>
#include <dirent.h>
#include <init_estructuras.h>
#include <sys/mman.h>

extern t_filesystem info_FS;
int crear_archivo(char* nombre_archivo);
int truncar_archivo(char* nombre_archivo, int nuevo_tamano);
void liberar_bloques_de_archivo(char* nombre_archivo);
void listar_archivos();
void escribir_archivo(char* nombre_archivo, int puntero, int tamanio, char* resultado_escritura);
void leer_archivo(char* nombre_archivo, int puntero, int tamanio);

#endif

/*************** FUNCIONES AUXILIARES *****************/
/******************************************************/