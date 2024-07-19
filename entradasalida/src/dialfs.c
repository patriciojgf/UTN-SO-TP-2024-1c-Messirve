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

void eliminar_archivo(char* nombre_archivo)
{
    char* path = concatenar_path(PATH_BASE_DIALFS, nombre_archivo);
    t_config* metadata_aux = config_create(path);
    
    if(metadata_aux == NULL)
    {
        log_error(logger_io, "Error al eliminar el archivo metadata. ");
        return;
    }

    int bloque_inicial_aux = config_get_int_value(metadata_aux, "BLOQUE_INICIAL");
    int tamanio_archivo_aux = config_get_int_value(metadata_aux, "TAMANIO_ARCHIVO");
    int bloques_a_liberar = ceil(tamanio_archivo_aux / BLOCK_SIZE);

    //TODO: revisar
    if(!bloques_a_liberar)
    {
        bloques_a_liberar = 1;
    }
    
    for(int i = bloque_inicial_aux; i < bloque_inicial_aux + bloques_a_liberar; i++)
    {
        liberar_bloque(i);
    }

    free(path);
    config_destroy(metadata_aux);
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

static int obtener_bloques_ocupados(int inicio)
{
    for(int i = inicio; i < BLOCK_COUNT; i++)
    {
        if(bitarray_test_bit(bitmap_fs, i))
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
