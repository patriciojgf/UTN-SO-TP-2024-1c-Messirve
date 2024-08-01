#include "init_estructuras.h"

static void init_log();

//--------

int cantidad_bloques(int tamano_archivo, int tamano_bloque) {
    if (tamano_archivo == 0) {
        return 1;
    }
    return (tamano_archivo + tamano_bloque - 1) / tamano_bloque;
}

static void init_log(){
    logger_io = iniciar_logger("io.log", "IO");
}

static void iniciar_configuracion(char* config_path){
    config_io = iniciar_config(config_path);
    if(config_io == NULL){
        log_error(logger_io,"No se pudo leer el archivo de configuracion");
        log_destroy(logger_io);
        config_destroy(config_io);
        exit(EXIT_FAILURE);
    }      
    TIPO_INTERFAZ = config_get_string_value(config_io, "TIPO_INTERFAZ");    

    if(strcmp(TIPO_INTERFAZ, "GENERICA") == 0){
        IP_MEMORIA = config_get_string_value(config_io, "IP_MEMORIA");
        IP_KERNEL = config_get_string_value(config_io, "IP_KERNEL");
        PUERTO_KERNEL = config_get_string_value(config_io, "PUERTO_KERNEL");
        TIEMPO_UNIDAD_TRABAJO = atoi(config_get_string_value(config_io, "TIEMPO_UNIDAD_TRABAJO"));
    }
    else if((strcmp(TIPO_INTERFAZ, "STDIN") == 0)|| (strcmp(TIPO_INTERFAZ, "STDOUT") == 0)){
        IP_KERNEL = config_get_string_value(config_io, "IP_KERNEL");
        PUERTO_KERNEL = config_get_string_value(config_io, "PUERTO_KERNEL");
        IP_MEMORIA = config_get_string_value(config_io, "IP_MEMORIA");
        PUERTO_MEMORIA = config_get_string_value(config_io, "PUERTO_MEMORIA");
    }
    else if(string_equals_ignore_case(TIPO_INTERFAZ, "DIALFS"))
    {
        TIEMPO_UNIDAD_TRABAJO = config_get_int_value(config_io, "TIEMPO_UNIDAD_TRABAJO");
        IP_KERNEL = config_get_string_value(config_io, "IP_KERNEL");
        PUERTO_KERNEL = config_get_string_value(config_io, "PUERTO_KERNEL");
        IP_MEMORIA = config_get_string_value(config_io, "IP_MEMORIA");
        PUERTO_MEMORIA = config_get_string_value(config_io, "PUERTO_MEMORIA");
        PATH_BASE_DIALFS = config_get_string_value(config_io, "PATH_BASE_DIALFS");
        BLOCK_SIZE = config_get_int_value(config_io, "BLOCK_SIZE");
        BLOCK_COUNT = config_get_int_value(config_io, "BLOCK_COUNT");
        RETRASO_COMPACTACION = config_get_int_value(config_io, "RETRASO_COMPACTACION");
    }
    else
    {
        log_error(logger_io, "Interfaz %s inválida.", TIPO_INTERFAZ); 
    }
}

static FILE* inicio_fs_crear_archivo_fisico(char* nombre_archivo, int tamano_archivo) {
    // Construye el nombre completo del archivo usando la ruta base y el nombre del archivo
    char *nombre_completo = malloc(strlen(PATH_BASE_DIALFS) + strlen(nombre_archivo) + 2); // +2 para el '/' y el '\0'
    sprintf(nombre_completo, "%s/%s", PATH_BASE_DIALFS, nombre_archivo);

    // Abre el archivo en modo de añadir binario y lectura/escritura
    FILE *archivo = fopen(nombre_completo, "ab+");
    if (!archivo) {
        log_error(logger_io, "Error al abrir o crear el archivo: %s", nombre_completo);
        free(nombre_completo);
        return NULL;
    }    
    // Obtiene el descriptor de archivo
    int fd = fileno(archivo);
    // Trunca el archivo al tamaño especificado
    if (ftruncate(fd, tamano_archivo) == -1) {
        log_error(logger_io, "Error al truncar el archivo: %s", nombre_completo);
        fclose(archivo);
        free(nombre_completo);
        return NULL;
    }    
    // Libera la memoria del nombre completo y retorna el archivo
    free(nombre_completo);
    return archivo;
}

static void* inicio_fs_mapear_archivo_en_memoria(FILE* archivo, int tamano_archivo) {
    // Obtiene el descriptor de archivo
    int fd = fileno(archivo);

    // Mapea el archivo en memoria con permisos de lectura y escritura
    void *archivo_mapeado = mmap(NULL, tamano_archivo, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (archivo_mapeado == MAP_FAILED) {
        log_error(logger_io, "Error al mapear el archivo en memoria");
        return NULL;
    }
    
    // Sincroniza la memoria mapeada con el archivo
    if (msync(archivo_mapeado, tamano_archivo, MS_SYNC) == -1) {
        log_error(logger_io, "Error al sincronizar la memoria mapeada");
        munmap(archivo_mapeado, tamano_archivo);
        return NULL;
    }
    
    // Retorna el puntero a la memoria mapeada
    return archivo_mapeado;
}

void crear_diccionario_nombres_archivos() {
    // Crear el diccionario que almacenará los datos de los archivos
    info_FS.fs_archivos = dictionary_create();    
    DIR* directorio;
    struct dirent* entrada;
    // Abrir el directorio especificado por PATH_BASE_DIALFS
    directorio = opendir(PATH_BASE_DIALFS);
    // Leer las entradas del directorio en un bucle
    while ((entrada = readdir(directorio)) != NULL) {
        // Verificar si la entrada es un archivo regular y es válido en el sistema de archivos
        if (entrada->d_type == DT_REG && strcmp(entrada->d_name, "bloques.dat") != 0 && strcmp(entrada->d_name, "bitmap.dat") != 0) {
            // Crear el nombre completo del archivo concatenando PATH_BASE_DIALFS con el nombre del archivo
            char* path_nombre = malloc(strlen(PATH_BASE_DIALFS) + strlen(entrada->d_name) + 2); // +2 para '/' y '\0'
            sprintf(path_nombre, "%s/%s", PATH_BASE_DIALFS, entrada->d_name);            
            // Asignar memoria para la estructura t_fs_archivo
            t_fs_archivo* datos_archivo = malloc(sizeof(t_fs_archivo));            
            // Inicializar la estructura t_fs_archivo
            datos_archivo->nombre = strdup(entrada->d_name);
            datos_archivo->path_nombre = path_nombre;
            datos_archivo->metadata = config_create(path_nombre);            
            // Verificar si la metadata se cargó correctamente
            if (datos_archivo->metadata) {
                datos_archivo->puntero_inicio = config_get_int_value(datos_archivo->metadata, "BLOQUE_INICIAL");
                datos_archivo->tamano = config_get_int_value(datos_archivo->metadata, "TAMANIO_ARCHIVO");
                datos_archivo->cantidad_bloques = cantidad_bloques(datos_archivo->tamano, info_FS.tamano_bloque); 
            } else {
                log_error(logger_io, "Error al cargar la metadata del archivo: %s", path_nombre);
                datos_archivo->puntero_inicio = -1;
                datos_archivo->cantidad_bloques = 0;
            }
            // Añadir los datos del archivo al diccionario fs_archivos
            dictionary_put(info_FS.fs_archivos, entrada->d_name, datos_archivo);
        }
    }
    // Cerrar el directorio después de procesar todas las entradas
    closedir(directorio);
}

static void inicio_FS(){
    // Inicializa bloques
    info_FS.tamano_total_bloques = BLOCK_SIZE * BLOCK_COUNT; // Calcula el tamaño total de los bloques (tamaño de un bloque por la cantidad de bloques)
    info_FS.tamano_bloque = BLOCK_SIZE;
    info_FS.cantidad_bloques = BLOCK_COUNT;
    info_FS.archivo_bloques = inicio_fs_crear_archivo_fisico("bloques.dat", info_FS.tamano_total_bloques); // Crea y abre el archivo de bloques físico con el tamaño total calculado
    info_FS.archivo_bloques_en_memoria = inicio_fs_mapear_archivo_en_memoria(info_FS.archivo_bloques, info_FS.tamano_total_bloques);// Mapea el archivo de bloques en memoria
    
    // Inicializa bitmap
    info_FS.tamano_bitmap = BLOCK_COUNT / 8; // Calcula el tamaño del bitmap (cantidad de bloques / 8 para obtener el tamaño en bytes)    
    info_FS.archivo_bitmap = inicio_fs_crear_archivo_fisico("bitmap.dat", info_FS.tamano_bitmap); // Crea y abre el archivo bitmap físico con el tamaño calculado
    info_FS.archivo_bitmap_en_memoria = inicio_fs_mapear_archivo_en_memoria(info_FS.archivo_bitmap, info_FS.tamano_bitmap); // Mapea el archivo bitmap en memoria
    info_FS.bitmap = bitarray_create_with_mode(info_FS.archivo_bitmap_en_memoria, info_FS.tamano_bitmap, LSB_FIRST); // Crea el bitarray utilizando el archivo bitmap mapeado en memoria

    info_FS.fs_archivos = dictionary_create(); // Crea el diccionario para los archivos del sistema de archivos 
    crear_diccionario_nombres_archivos();

}

static void iniciar_semaforos(){
    sem_init(&sem_io_stdin_read_ok,0,0);
}

static void iniciar_estructuras() {
    // lista_interfaz_socket = list_create();
    if(string_equals_ignore_case(TIPO_INTERFAZ, "DIALFS"))
    {
        if(PATH_BASE_DIALFS == NULL)
        {
            PATH_BASE_DIALFS = "/home/utnso/dialfs";
        }
        int valor = mkdir(PATH_BASE_DIALFS, 0777);        
        if(valor < 0)
        {
            log_info(logger_io, "Ya existe el directorio: (%s)", PATH_BASE_DIALFS);
        }
        inicio_FS();
    }
}

static void init_nombre_interfaz(char* nombre){
    if(nombre == NULL){
        log_error(logger_io, "No se ha ingresado el nombre de la interfaz");
        exit(EXIT_FAILURE);
    }
    nombre_interfaz = nombre;
}

void init_io(char* path_config, char* nombre){
    init_log();
    iniciar_configuracion(path_config);
    iniciar_estructuras();
    iniciar_semaforos();
    init_nombre_interfaz(nombre);
}