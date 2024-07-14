#include "init_estructuras.h"

static void init_log();
static char* concatenar_path(char* path, char* nombre_archivo);

//--------

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
        PUERTO_MEMORIA = config_get_string_value(config_io, "PUERTO_MEMORIA");
        BLOCK_SIZE = atoi(config_get_string_value(config_io, "BLOCK_SIZE"));
        BLOCK_COUNT = atoi(config_get_string_value(config_io, "BLOCK_COUNT"));
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

static void iniciar_semaforos(){
    sem_init(&sem_io_stdin_read_ok,0,0);
}

static void crear_archivo_de_bloques()
{
    char* path = concatenar_path(PATH_BASE_DIALFS, "bloques");

    int file_descriptor = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if(file_descriptor == -1)
    {
        log_error(logger_io, "Error al abrir el archivo de bloques.");
        return;
    }

    int tamanio_archivo = BLOCK_SIZE * BLOCK_COUNT;
    if(ftruncate(file_descriptor, tamanio_archivo) == -1)
    {
        log_error(logger_io, "Error al truncar el archivo de bloques.");
        return;
    }

    close(file_descriptor);
    log_info(logger_io, "Archivo de bloques creado correctamente.");
}

static void crear_archivo_bitmap()
{
    int tamanio_archivo_bitmap = BLOCK_COUNT / 8;

    void *bitmap_memoria_usuario = malloc(tamanio_archivo_bitmap);
    bitmap_fs = bitarray_create_with_mode(bitmap_memoria_usuario, tamanio_archivo_bitmap, LSB_FIRST);

    char* path = concatenar_path(PATH_BASE_DIALFS, "bitmap");

    int file_descriptor = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if(file_descriptor == -1)
    {
        log_error(logger_io, "Error al abrir el archivo de bitmap.");
        return;
    }

    if(ftruncate(file_descriptor, tamanio_archivo_bitmap) == -1)
    {
        log_error(logger_io, "Error al truncar el archivo de bitmap.");
        return;
    }
}

static void iniciar_estructuras()
{
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
            log_warning(logger_io, "Ya existe el directorio: (%s)", PATH_BASE_DIALFS);
        }
        crear_archivo_de_bloques();
        crear_archivo_bitmap();
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

/*************** FUNCIONES AUXILIARES *****************/
static char* concatenar_path(char* path, char* nombre_archivo)
{
	char *unaPalabra = string_new();
	string_append(&unaPalabra, path);
	string_append(&unaPalabra, "/");
	string_append(&unaPalabra, nombre_archivo);
	string_append(&unaPalabra, ".dat");

    return unaPalabra;
}
/******************************************************/