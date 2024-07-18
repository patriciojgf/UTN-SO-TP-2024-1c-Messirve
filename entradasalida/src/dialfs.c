#include <dialfs.h>

static int obtener_bloques_libres(int inicio);
static void ocupar_bloque(int index);
static void liberar_bloque(int index);

void crear_archivo(char* nombre_archivo)
{
    int bloque = obtener_bloques_libres(0);
    ocupar_bloque(bloque);
    if(!crear_archivo_metadata(nombre_archivo, bloque))
    {
        error_show("Error al crear el archivo metadata");
    }
}

/*************** FUNCIONES AUXILIARES *****************/

static int obtener_bloques_libres(int inicio)
{
    for(int i = inicio; i < BLOCK_COUNT; i++)
    {
        if(!bitarray_test_bit(bitmap_fs, i))
        {
            return i;
        }
    }
    return -1;
}

static void ocupar_bloque(int index)
{
    bitarray_set_bit(bitmap_fs, index);
    msync(bitmap_void, tamanio_archivo_bitmap, index);
}

static void liberar_bloque(int index)
{
    bitarray_clean_bit(bitmap_fs, index);
    msync(bitmap_void, tamanio_archivo_bitmap, index);
}

/******************************************************/
