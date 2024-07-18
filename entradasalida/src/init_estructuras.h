#ifndef INIT_ESTRUC_IO_H
#define INIT_ESTRUC_IO_H

#include <commons/bitarray.h>
#include <configuracion_io.h>
#include <fcntl.h>
#include <unistd.h>

void log_protegido_io(char* mensaje);
void init_io(char* path_config, char* nombre_interfaz);
bool crear_archivo_metadata(char* nombre_archivo);

#endif/*INIT_ESTRUC_IO_H*/