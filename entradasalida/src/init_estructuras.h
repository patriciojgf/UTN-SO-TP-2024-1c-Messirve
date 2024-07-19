#ifndef INIT_ESTRUC_IO_H
#define INIT_ESTRUC_IO_H

#include <commons/bitarray.h>
#include <configuracion_io.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

char* concatenar_path(char* path, char* nombre_archivo);
bool crear_archivo_metadata(char *nombre_archivo, int bloque_inicial);
void log_protegido_io(char *mensaje);
void init_io(char *path_config, char *nombre_interfaz);

#endif /*INIT_ESTRUC_IO_H*/