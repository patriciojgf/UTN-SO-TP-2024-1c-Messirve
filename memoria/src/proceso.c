#include "proceso.h"

static t_list* leer_archivo_instrucciones(char* path);
static void liberar_un_marco(int pid);
static int buscar_marco_libre();
static int asignar_marco_libre(int pid, int nro_pagina);

static int cantidad_marcos_libres(){
    int cantidad = 0;
    for(int i = 0; i < list_size(listado_marcos); i++){
        t_marco* marco = list_get(listado_marcos, i);
        if(marco->pid == -1){
            cantidad++;
        }
    }
    return cantidad;

}

static int buscar_marco_libre() {
    int indice_marco = 0;
    int nro_marco = -1; // inicializamos a -1 para indicar que no se encontró un marco libre

    // Iteramos sobre la lista de marcos
    while (indice_marco < list_size(listado_marcos)) {
        t_marco *marco = list_get(listado_marcos, indice_marco);
        if (marco->bit_de_uso == 0) {
            nro_marco = indice_marco;
            break;
        }
        indice_marco++;
    }

    // Devolvemos el índice del marco libre encontrado, o -1 si no se encontró ninguno
    return nro_marco;
}

static int asignar_marco_libre(int pid, int nro_pagina){
    int nro_marco = buscar_marco_libre();
    if(nro_marco == -1){
        log_info(logger_memoria, "No se encontro un marco libre");
        return -1;
    }
    t_marco* marco = list_get(listado_marcos, nro_marco);
    marco->pid = pid;
    marco->pagina_asignada = nro_pagina;
    marco->bit_de_uso = 1;

    t_pagina* pagina = malloc(sizeof(t_pagina));
    pagina->id = nro_pagina;
    pagina->marco = nro_marco;
    list_add(get_proceso_memoria(pid)->tabla_de_paginas, pagina);
    get_proceso_memoria(pid)->cantidad_de_paginas++;
    return nro_marco;
}

static void liberar_un_marco(int pid){
    // Iniciando la liberación de un marco
    t_proceso* proceso = get_proceso_memoria(pid);
    if (proceso == NULL || proceso->cantidad_de_paginas == 0) {
        log_error(logger_memoria, "No se puede liberar un marco: proceso no encontrado o sin páginas");
        return;
    }
    // Libero el último marco ocupado de su tabla de páginas
    t_pagina* pagina = list_get(proceso->tabla_de_paginas, proceso->cantidad_de_paginas - 1);
    if (pagina == NULL) {
        log_error(logger_memoria, "No se puede liberar un marco: página no encontrada");
        return;
    }
    // Libero el marco de la lista de marcos
    t_marco* marco = list_get(listado_marcos, pagina->marco);
    if (marco == NULL) {
        log_error(logger_memoria, "No se puede liberar un marco: marco no encontrado");
        return;
    }
    marco->pid = -1;
    marco->pagina_asignada = -1;
    marco->bit_de_uso = 0;

    // Libero la página de la tabla de páginas
    list_remove(proceso->tabla_de_paginas, proceso->cantidad_de_paginas - 1);
    proceso->cantidad_de_paginas--;
    // Elimino la página
    free(pagina);
}

int resize_proceso(int pid, int new_size) {
    int cantidad_de_paginas_anterior = get_proceso_memoria(pid)->cantidad_de_paginas;
    int old_size = cantidad_de_paginas_anterior * TAM_PAGINA;
    int marcos_a_asignar = (new_size + TAM_PAGINA - 1) / TAM_PAGINA - (cantidad_de_paginas_anterior);

    if(cantidad_marcos_libres()<marcos_a_asignar){
        log_error(logger_memoria, "No hay marcos suficientes <%d> para asignar al proceso PID <%d>, hay <%d>.", marcos_a_asignar, pid, cantidad_marcos_libres());
        return -1; // Error al asignar marco
    }
    // log_info(logger_memoria, "resize_proceso - PID <%d> - OLD_SIZE <%d> - MARCOS_A_ASIGNAR <%d>", pid, old_size, marcos_a_asignar);

    if (marcos_a_asignar > 0) {
        //log obligatorio
        // Ampliación de Proceso: “PID: <PID> - Tamaño Actual: <TAMAÑO_ACTUAL> - Tamaño a Ampliar: <TAMAÑO_A_AMPLIAR>” 
        log_info(logger_memoria, "PID: <%d> - Tamaño Actual: <%d> - Tamaño a Ampliar: <%d>", pid, old_size, (new_size-old_size));
        for (int i = 0; i < marcos_a_asignar; i++) {
            if (asignar_marco_libre(pid, old_size + i) == -1) {
                log_error(logger_memoria, "No se pudo asignar un marco libre al proceso PID <%d>.", pid);
                return -1; // Error al asignar marco
            }
        }
    } else if (marcos_a_asignar < 0) {
        // log obligatorio
        // Reducción de Proceso: “PID: <PID> - Tamaño Actual: <TAMAÑO_ACTUAL> - Tamaño a Reducir: <TAMAÑO_A_REDUCIR>” 
        log_info(logger_memoria, "PID: <%d> - Tamaño Actual: <%d> - Tamaño a Reducir: <%d>", pid, old_size, (-(old_size-new_size)));
        for (int i = 0; i < -marcos_a_asignar; i++) { // Cambiado para iterar correctamente
            liberar_un_marco(pid);
        }
    } else {
        log_info(logger_memoria, "El proceso PID <%d> ya tiene el tamaño deseado: <%d>", pid, old_size);
    }
    //log obligatorio
    // Creación / destrucción de Tabla de Páginas: “PID: <PID> - Tamaño: <CANTIDAD_PAGINAS>”
    return 0; // Éxito
}

int buscar_marco_por_pagina(int nro_pagina, int pid){
    t_proceso* proceso = get_proceso_memoria(pid);
    return get_marco_proceso(proceso, nro_pagina);
}

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

int get_marco_proceso(t_proceso* proceso, int nro_pagina){
    if(list_size(proceso->tabla_de_paginas) <= nro_pagina){
        log_info(logger_memoria, "PID<%d> - La pagina %d esta fuera de rango, tamanio tabla es: <%d>", proceso->id, nro_pagina, list_size(proceso->tabla_de_paginas));
        return -1;
    }
    t_pagina* pagina = list_get(proceso->tabla_de_paginas, nro_pagina);
    return pagina->marco;
}

t_proceso* crear_proceso(int pid, char* path_instrucciones) {
    t_proceso* proceso = malloc(sizeof(t_proceso));
    proceso->id = pid;
    proceso->path_instrucciones = path_instrucciones;
    proceso->instrucciones = NULL;
    proceso->tabla_de_paginas = list_create();
    proceso->cantidad_de_paginas = 0;
    proceso->instrucciones = leer_archivo_instrucciones(path_instrucciones);
    // Log obligatorio
    // Creación / destrucción de Tabla de Páginas: “PID: <PID> - Tamaño: <CANTIDAD_PAGINAS>”
    log_info(logger_memoria, "Creacion - PID: <%d> - Tamaño: <0>", pid);
    return proceso;
}

void finalizar_proceso(int pid) {
    t_proceso* proceso = get_proceso_memoria(pid);
    // Liberar marcos asociados al proceso
    for (int i = 0; i < list_size(listado_marcos); i++) {
        t_marco* marco = list_get(listado_marcos, i);
        if (marco->pid == pid) {
            marco->pid = -1;
            marco->pagina_asignada = -1;
            marco->bit_de_uso = 0;
        }
    }

    int cantidad_paginas_para_log = proceso->cantidad_de_paginas;
    // Liberar la tabla de páginas
    if (proceso->tabla_de_paginas != NULL) {
        while (!list_is_empty(proceso->tabla_de_paginas)) {
            list_remove_and_destroy_element(proceso->tabla_de_paginas, 0, free);
        }
        list_destroy(proceso->tabla_de_paginas);
    }
    // Log obligatorio
    // Creación / destrucción de Tabla de Páginas: “PID: <PID> - Tamaño: <CANTIDAD_PAGINAS>”
    log_info(logger_memoria, "Destruccion - PID: <%d> - Tamaño: <%d>", pid, cantidad_paginas_para_log);

    // Liberar instrucciones
    if (proceso->instrucciones != NULL) {
        while (!list_is_empty(proceso->instrucciones)) {
            list_remove_and_destroy_element(proceso->instrucciones, 0, free);
        }
        list_destroy(proceso->instrucciones);
    }
    free(proceso);
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
    //log_protegido_mem(string_from_format("leer_archivo_instrucciones"));
    char* full_path = string_new();
    string_append(&full_path, config_get_string_value(config_memoria, "PATH_INSTRUCCIONES"));
    string_append(&full_path, path);
    FILE* archivo = fopen(full_path, "r");
    if (archivo == NULL) {
        log_error(logger_memoria, "No se pudo abrir el archivo de instrucciones");
        return NULL;
    }
    free(full_path);    
	char *instruccion = NULL;
	size_t longitud = 0;
	int cant_instrucciones = 0;
    t_list* lista_instrucciones = list_create();
	while (getline(&instruccion, &longitud, archivo) != -1)
	{
		if (strcmp(instruccion, "\n"))
		{
			int longitud = strlen(instruccion);
			if (instruccion[longitud - 1] == '\n')
				instruccion[longitud - 1] = '\0';
			char* aux = malloc(strlen(instruccion)+1);
			strcpy(aux,instruccion);
			list_add(lista_instrucciones, aux);
			cant_instrucciones++;
		} 
	}
    fclose(archivo);
	free(instruccion);
    return lista_instrucciones;
}




/*--------------*/
/*IN*/
/*OUT*/
void confirmar_proceso_creado(){
    t_paquete* paquete = crear_paquete(INICIAR_PROCESO_MEMORIA_OK);    
    //log_protegido_mem(string_from_format("INICIAR_PROCESO_MEMORIA_OK"));
    //agrego texto "OK" al paquete
    char* mensajeOK = "OK";

    agregar_a_paquete(paquete,mensajeOK,strlen(mensajeOK)+1);
    enviar_paquete(paquete,socket_cliente_kernel);
    eliminar_paquete(paquete);
}

void confirmar_memoria_liberada(){
    t_paquete* paquete = crear_paquete(LIBERAR_ESTRUCTURAS_MEMORIA_OK);    
    char* mensajeOK = "OK";

    agregar_a_paquete(paquete,mensajeOK,strlen(mensajeOK)+1);
    enviar_paquete(paquete,socket_cliente_kernel);
    eliminar_paquete(paquete);
}