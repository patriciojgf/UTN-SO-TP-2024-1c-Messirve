#ifndef STATIC_DIALFS_H_
#define STATIC_DIALFS_H_

#include <commons/collections/dictionary.h>
#include <dirent.h>
#include <configuracion_io.h>
#include <sys/mman.h>

extern t_filesystem info_FS;
int crear_archivo(char* nombre_archivo);
int truncar_archivo(char* nombre_archivo, int nuevo_tamano);
void listar_archivos();

#endif

/*************** FUNCIONES AUXILIARES *****************/
/******************************************************/