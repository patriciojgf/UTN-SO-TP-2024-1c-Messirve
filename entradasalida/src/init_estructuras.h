#ifndef INIT_ESTRUC_IO_H
#define INIT_ESTRUC_IO_H

#include <commons/bitarray.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <unistd.h>
#include <utils/conexiones.h>
#include <utils/logconfig.h>
#include <configuracion_io.h>

char* concatenar_path(char* path, char* nombre_archivo);
bool crear_archivo_metadata(char *nombre_archivo, int bloque_inicial);
void log_protegido_io(char *mensaje);
void init_io(char *path_config, char *nombre_interfaz);
int cantidad_bloques(int tamano_archivo, int tamano_bloque); //se usa tambien en dialfs

#endif /*INIT_ESTRUC_IO_H*/