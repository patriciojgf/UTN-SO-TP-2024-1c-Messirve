#include <dialfs.h>

static int obtener_bloques_libres(int inicio);
static void ocupar_bloque(int index);
static void liberar_bloque(int index);
static bool se_puede_agrandar(int inicio, int bloques_a_actualizar);

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

void truncar_archivo(t_solicitud_io* solicitud_io, char* nombre_archivo)
{
    char* path = concatenar_path(PATH_BASE_DIALFS, nombre_archivo);
    t_config* metada_aux = config_create(path); 
    if(metada_aux == NULL)
    {
        log_error(logger_io, "Error al leer archivo metadata.");
        return;
    }
    int tamanio_archivo_aux = config_get_int_value(metada_aux, "TAMANIO_ARCHIVO");
    int bloque_inicial_aux = config_get_int_value(metada_aux, "BLOQUE_INICIAL");

    int bloques_actuales = ceil(((double)tamanio_archivo_aux) / BLOCK_SIZE);
    if(!bloques_actuales)
    {
        bloques_actuales = 1;
    }
    
    int nuevo_bloques = ceil(((double)solicitud_io->datos_memoria->tamano) / BLOCK_SIZE);
    if(!nuevo_bloques)
    {
        nuevo_bloques = 1;
    }

    int bloques_a_actualizar = nuevo_bloques - bloques_actuales;
    if(bloques_a_actualizar > 0)
    {
        //agrandar
        int inicio = bloques_actuales + bloque_inicial_aux;

        if(!se_puede_agrandar(inicio, bloques_a_actualizar))
        {
            log_info(logger_io, "Compatación hace su magia...");  
            usleep(TIEMPO_UNIDAD_TRABAJO*1000);
            //TODO: implementar compatación
        }

        for(int i = inicio; i < inicio + bloques_a_actualizar; i++)
        {
            ocupar_bloque(i);
        }

        //TODO: ver el tema de compatación
    }
    else if(bloques_a_actualizar < 0)
    {
        //achicar
        bloques_a_actualizar = bloques_actuales - nuevo_bloques;
        int inicio =  bloque_inicial_aux + bloques_actuales - 1; 
        for(int i = inicio; i < inicio - bloques_a_actualizar; i++)
        {
            liberar_bloque(i);
        }
    }
    else
    {
        //TODO: que pasa si es cero
    }

    //actualizar metadata
    config_set_value(metada_aux, "TAMANIO_ARCHIVO", string_itoa(nuevo_bloques));
    free(path);
    config_destroy(metada_aux);
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

static bool se_puede_agrandar(int inicio, int bloques_a_actualizar)
{
    for(int i = inicio; i < inicio * bloques_a_actualizar; i++)
    {
        if(i >= BLOCK_SIZE)
        {
            return false;
        }
        if(bitarray_test_bit(bitmap_fs, i) == 1) //verifica que haya bloques contiguos libres
        {
            return false;
        }
    }
    return true; 
}

/******************************************************/
