#include <dialfs.h>

static void compactar(int pid, char* nombre_archivo);
static void ocupar_bloque(int index);
static int obtener_bloques_libres(int inicio);
static int obtener_bloques_ocupados(int inicio);
static void liberar_bloque(int index);
static bool se_puede_agrandar(int inicio, int bloques_a_actualizar);

void crear_archivo(char* nombre_archivo)
{
    log_info(logger_io, "Iniciando creación de archivo [%s]...", nombre_archivo);
    int bloque = obtener_bloques_libres(0);
    ocupar_bloque(bloque);
    if(!crear_archivo_metadata(nombre_archivo, bloque))
    {
        log_error(logger_io, "Error al crear el archivo metadata");
    }
}

int liberar_bloques_de_archivo(char* nombre_archivo)
{
    log_info(logger_io, "Eliminando archivo [%s]...", nombre_archivo);
    char* path = concatenar_path(PATH_BASE_DIALFS, nombre_archivo);
    t_config* metadata_aux = config_create(path);
    
    if(metadata_aux == NULL)
    {
        log_error(logger_io, "Error al eliminar el archivo metadata. ");
        return EXIT_FAILURE;
    }

    int bloque_inicial_aux = config_get_int_value(metadata_aux, "BLOQUE_INICIAL");
    int tamanio_archivo_aux = config_get_int_value(metadata_aux, "TAMANIO_ARCHIVO");
    int bloques_a_liberar = ceil(((double)tamanio_archivo_aux) / BLOCK_SIZE);

    //TODO: revisar
    if(!bloques_a_liberar)
    {
        bloques_a_liberar = 1;
    }
    
    for(int i = bloque_inicial_aux; i < bloque_inicial_aux + bloques_a_liberar; i++)
    {
        liberar_bloque(i);
    }
    log_info(logger_io, "Bloques liberados...");

    free(path);
    config_destroy(metadata_aux);
    return bloques_a_liberar;
}

void truncar_archivo(int pid, int tamano_bytes, char* nombre_archivo)
{
    log_info(logger_io, "PID: <%d> - Truncar Archivo: <%s> - Tamaño: <%d>", pid, nombre_archivo, tamano_bytes);

    char* path = concatenar_path(PATH_BASE_DIALFS, nombre_archivo);
    t_config* metadata_aux = config_create(path); 
    if(metadata_aux == NULL)
    {
        log_error(logger_io, "Error al leer archivo metadata.");
        return;
    }
    int tamanio_archivo_aux = config_get_int_value(metadata_aux, "TAMANIO_ARCHIVO");
    int bloque_inicial_aux = config_get_int_value(metadata_aux, "BLOQUE_INICIAL");

    int bloques_actuales = ceil(((double)tamanio_archivo_aux) / BLOCK_SIZE);
    if(!bloques_actuales)
    {
        bloques_actuales = 1;
    }
    
    int nuevo_bloques = ceil(tamano_bytes / BLOCK_SIZE);
    if(!nuevo_bloques)
    {
        nuevo_bloques = 1;
    }

    int bloques_a_actualizar = nuevo_bloques - bloques_actuales;
    log_info(logger_io, "Actualizando bloques...");
    if(bloques_a_actualizar > 0)
    {
        //agrandar
        int inicio = bloques_actuales + bloque_inicial_aux;

        if(!se_puede_agrandar(inicio, bloques_a_actualizar))
        {
            log_info(logger_io, "Compatación hace su magia...");  
            usleep(RETRASO_COMPACTACION * 1000);
            compactar(pid, nombre_archivo);
        }

        for(int i = inicio; i < inicio + bloques_a_actualizar; i++)
        {
            ocupar_bloque(i);
        }
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
        log_error(logger_io, "Error al actualizar: tamanio del bloques a actualizar es {%d}, caso no soportado.", bloques_a_actualizar);
    }

    //actualizar metadata
    log_info(logger_io, "Actualizando metadata...");
    config_set_value(metadata_aux, "TAMANIO_ARCHIVO", string_itoa(nuevo_bloques));
    config_save(metadata_aux);
    free(path);
    config_destroy(metadata_aux);
}

void* leer_archivo(char* nombre_archivo, int puntero, int tamanio)
{
    log_info(logger_io, "Leyendo archivo %s...", nombre_archivo); 
    char* path = concatenar_path(PATH_BASE_DIALFS, nombre_archivo);
    t_config* metadata_aux = config_create(path);
    int bloque_inicial = config_get_int_value(metadata_aux, "BLOQUE_INICIAL");
    int valor = bloque_inicial * BLOCK_SIZE;

    void* datos = malloc(tamanio);
    FILE* archivo = fopen(path, "r"); //TODO: revisar si le paso PATH o PATH_BASE_DIALFS
    fseek(archivo, valor + puntero, SEEK_SET);
    fread(datos, tamanio, 1, archivo);
    fclose(archivo);
    config_destroy(metadata_aux);
    return datos;
}

void escribir_archivo(void* datos, char* nombre_archivo, int puntero, int tamanio)
{
    log_info(logger_io, "Escribiendo archivo...");
    char* path = concatenar_path(PATH_BASE_DIALFS, nombre_archivo);
    t_config* metadata_aux = config_create(path);
    int bloque_inicial = config_get_int_value(metadata_aux, "BLOQUE_INICIAL");
    int valor = bloque_inicial * BLOCK_SIZE;

    // void* datos = malloc(tamanio);
    FILE* archivo = fopen(path, "r+"); //TODO: revisar si le paso PATH o PATH_BASE_DIALFS
    fseek(archivo, valor + puntero, SEEK_SET);
    fwrite(datos, tamanio, 1, archivo);
    fclose(archivo);
    config_destroy(metadata_aux);
}

/*************** FUNCIONES AUXILIARES *****************/

static void compactar(int pid, char* nombre_archivo)
{
    log_info(logger_io, "PID: <%d> - Inicio Compactación.", pid);
    int bloques = liberar_bloques_de_archivo(nombre_archivo);
    void* archivo_void = leer_archivo(nombre_archivo, 0, bloques * BLOCK_SIZE);
    int bloques_libre = obtener_bloques_libres(0);
    int bloques_ocupado = obtener_bloques_ocupados(bloques_libre);
    
    while(bloques_ocupado != -1)
    {
        bloques_libre = obtener_bloques_libres(bloques_libre);
        bloques_ocupado = obtener_bloques_ocupados(bloques_libre);
    }

    log_info(logger_io, "Actualizando metadata...");
    char* path = concatenar_path(PATH_BASE_DIALFS, nombre_archivo);
    t_config* metadata_aux = config_create(path);
    config_set_value(metadata_aux, "BLOQUE_INICIAL", string_itoa(bloques_libre));
    config_save(metadata_aux);
    config_destroy(metadata_aux);
    free(path);

    escribir_archivo(archivo_void, nombre_archivo, 0, bloques * BLOCK_SIZE);
    for(int i = bloques_libre; i < bloques_libre + bloques; i++)
    {
        ocupar_bloque(i);
    }
    free(archivo_void);

    log_info(logger_io, "PID: <%d> - Fin Compactación.", pid);
}

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

// static int obtener_puntero(int bloque_inicial)
// {
//     return bloque_inicial * BLOCK_SIZE;
// } 

/******************************************************/
