#include "proceso.h"

static t_list* leer_archivo_instrucciones(char* path);

t_proceso* get_proceso_memoria(int pid) {
    bool _buscar_proceso_por_id(t_proceso* proceso) {
        return proceso->id == pid;
    }
    t_proceso* proceso = list_find(lista_procesos_en_memoria, (void*)_buscar_proceso_por_id);
    if (proceso == NULL) {
        log_error(logger_memoria, "No se encontro el proceso con id %d", pid);
        exit(EXIT_FAILURE);
    }
    return proceso;
}

char* get_instruccion_proceso(t_proceso* proceso, int PC){
    if(!(PC >= 0 && PC < list_size(proceso->instrucciones))){
        log_error(logger_memoria, "PID<%d> - El PC %d esta fuera de rango", proceso->id, PC);
        return NULL;
    }
    return list_get(proceso->instrucciones, PC);
}

t_proceso* crear_proceso(int pid, char* path_instrucciones) {
    t_proceso* proceso = malloc(sizeof(t_proceso));
    proceso->id = pid;
    proceso->path_instrucciones = path_instrucciones;
    proceso->instrucciones = NULL;
    proceso->tabla_de_paginas = list_create();
    proceso->cantidad_de_paginas = 0;

    proceso->instrucciones = leer_archivo_instrucciones(path_instrucciones);

    log_warning(logger_memoria, "FALTA IMPLEMENTAR CREAR TABLA DE PAGINAS");
    return proceso;
}

void eliminar_proceso(t_proceso* proceso) {
    log_warning(logger_memoria, "VALIDAR QUE SE ILIMINEN ASI)");
    free(proceso->path_instrucciones);
    list_destroy_and_destroy_elements(proceso->instrucciones, free);
    list_destroy_and_destroy_elements(proceso->tabla_de_paginas, free);
    free(proceso);
}






/*-----------------------------------------------------------------------------------------------------*/
static t_list* leer_archivo_instrucciones(char* path) {
    //config_memoria.path_instrucciones / archivo_instrucciones
    char* full_path = string_new();
    string_append(&full_path, config_get_string_value(config_memoria, "PATH_INSTRUCCIONES"));
    string_append(&full_path, "/");
    string_append(&full_path, path);
    FILE* archivo = fopen(full_path, "r");
    if (archivo == NULL) {
        log_error(logger_memoria, "No se pudo abrir el archivo de instrucciones");
        return NULL;
    }
    free(full_path);
    
    char* linea = malloc(sizeof(char) * 1024);
    size_t len = 0;
    t_list* lista_instrucciones = list_create();

    while((getline(&linea, &len, archivo)) != -1) {
        if (!(linea[0] == '\n' || linea[0] == '\0' || linea[0] == '#')) {
            list_add(lista_instrucciones, linea);
        }
    }
    return lista_instrucciones;
}