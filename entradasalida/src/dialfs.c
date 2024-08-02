#include <dialfs.h>

static int obtener_offset_de_bloque(int bloque) {
    return bloque * info_FS.tamano_bloque;
}

static void fs_archivo_destroy(t_fs_archivo* archivo_datos)
{
    free(archivo_datos->nombre);
    free(archivo_datos->path_nombre);
    free(archivo_datos);
}
// static int cantidad_bloques(int tamano_archivo, int tamano_bloque) {
//     if (tamano_archivo == 0) {
//         return 1;
//     }
//     return (tamano_archivo + tamano_bloque - 1) / tamano_bloque;
// }

void listar_archivos() {
    // Función auxiliar para imprimir la información del archivo desde la metadata
    void imprimir_archivo_desde_metadata(t_fs_archivo* archivo) {
        char* path_completo = malloc(strlen(PATH_BASE_DIALFS) + strlen(archivo->nombre) + 2);
        sprintf(path_completo, "%s/%s", PATH_BASE_DIALFS, archivo->nombre);

        t_config* metadata = config_create(path_completo);
        if (metadata != NULL) {
            int primer_bloque = config_get_int_value(metadata, "BLOQUE_INICIAL");
            int tamano_archivo = config_get_int_value(metadata, "TAMANIO_ARCHIVO");
            int cant_bloques = cantidad_bloques(tamano_archivo, BLOCK_SIZE);

            log_info(logger_io, "Archivo: %s, Primer bloque: %d, Cantidad de bloques: %d",
                     archivo->nombre, primer_bloque, cant_bloques);
            
            config_destroy(metadata);
        } else {
            log_error(logger_io, "Error al leer la metadata del archivo: %s", archivo->nombre);
        }

        free(path_completo);
    }

    // Función auxiliar para iterar sobre el diccionario y llamar a imprimir_archivo_desde_metadata
    void listar_archivos_aux(char* key, void* value) {
        t_fs_archivo* archivo = (t_fs_archivo*)value;
        imprimir_archivo_desde_metadata(archivo);
    }

    // Itera sobre el diccionario info_FS.fs_archivos y llama a listar_archivos_aux para cada archivo
    dictionary_iterator(info_FS.fs_archivos, listar_archivos_aux);

    // Imprime todos los bloques ocupados recorriendo el bitarray
    // log_info(logger_io, "Bloques ocupados:");
    // for (int i = 0; i < info_FS.cantidad_bloques; i++) {
    //     if (bitarray_test_bit(info_FS.bitmap, i)) {
    //         log_info(logger_io, "Bloque %d ocupado", i);
    //     }
    // }
}

static int reservar_primer_bloque_libre() {
    int indice_bloque = 0;
    // Busca el primer bloque disponible
    while (indice_bloque < info_FS.cantidad_bloques) {
        if (bitarray_test_bit(info_FS.bitmap, indice_bloque) == 0) {
            // log_info(logger_io, "Bloque libre encontrado en el índice <%d>", indice_bloque);
            // Marca el bloque como ocupado y sincroniza los cambios
            // log_info(logger_io, "Reservando el bloque <%d>", indice_bloque);
            bitarray_set_bit(info_FS.bitmap, indice_bloque);
            msync(info_FS.archivo_bitmap_en_memoria, info_FS.tamano_bitmap, MS_SYNC);
            return indice_bloque;
        }
        // log_info(logger_io, "El bloque <%d> está ocupado", indice_bloque);
        indice_bloque++;
    }
    // Verifica si se encontró un bloque libre
    if (indice_bloque >= info_FS.cantidad_bloques) {
        return -1; // No hay bloques libres disponibles
    }
    return indice_bloque;
}

static t_fs_archivo* inicializar_archivo_vacio(char* nombre_archivo, int indice_primer_bloque_libre, char* path_completo) {
    t_fs_archivo* archivo_datos = malloc(sizeof(t_fs_archivo));
    archivo_datos->nombre = strdup(nombre_archivo);
    archivo_datos->path_nombre = path_completo;
    archivo_datos->metadata = config_create(path_completo);
    archivo_datos->puntero_inicio = indice_primer_bloque_libre;
    archivo_datos->cantidad_bloques = 1;
    archivo_datos->tamano=0;
    char* puntero = string_itoa(indice_primer_bloque_libre);
    config_set_value(archivo_datos->metadata, "BLOQUE_INICIAL", puntero);
    char* tamano = string_itoa(0);
    config_set_value(archivo_datos->metadata, "TAMANIO_ARCHIVO", tamano);
    config_save(archivo_datos->metadata);
    free(puntero);
    free(tamano);

    return archivo_datos;
}

int crear_archivo(char* nombre_archivo) {
    int indice_primer_bloque_libre = reservar_primer_bloque_libre();
    if (indice_primer_bloque_libre == -1) {
        return -1; // No hay bloques libres disponibles
    }
    char* path_completo = malloc(strlen(PATH_BASE_DIALFS) + strlen(nombre_archivo) + 2);
    sprintf(path_completo, "%s/%s", PATH_BASE_DIALFS, nombre_archivo);

    // Crear el archivo si no existe
    FILE* archivo = fopen(path_completo, "a+");
    if (!archivo) {
        free(path_completo);
        return -1; // Error al crear el archivo
    }
    fclose(archivo);

    // Inicializar la estructura del archivo y agregar los datos
    t_fs_archivo* archivo_datos = inicializar_archivo_vacio(nombre_archivo, indice_primer_bloque_libre, path_completo);

    // Agregar la estructura del archivo al diccionario
    dictionary_put(info_FS.fs_archivos, nombre_archivo, archivo_datos);

    return 0;
}

void liberar_bloques(t_fs_archivo* archivo, void* buffer) {
    // Copia los bytes del archivo a una variable interna
    memcpy(buffer, info_FS.archivo_bloques_en_memoria + obtener_offset_de_bloque(archivo->puntero_inicio), archivo->tamano);
}

int liberar_bloques_de_archivo(char* nombre_archivo) {
    t_fs_archivo* archivo_datos = (t_fs_archivo*)dictionary_remove(info_FS.fs_archivos, nombre_archivo);
    if (!archivo_datos) {
        log_error(logger_io, "El archivo %s no existe.", nombre_archivo);
        return -1; // El archivo no existe
    }
    // Liberar los bloques ocupados por el archivo
    for (int i = archivo_datos->puntero_inicio; i < archivo_datos->puntero_inicio + archivo_datos->cantidad_bloques; i++) {
        // Pone en 0 el bit en la posición i del bitmap, indicando que el bloque i está ahora libre.
        bitarray_clean_bit(info_FS.bitmap, i);
    }
    // Escribo los cambios que se realizaron en memoria, en el archivo fisico (que fue mapeado a memoria en inicio_fs_mapear_archivo_en_memoria)
    msync(info_FS.archivo_bitmap_en_memoria, info_FS.tamano_bitmap, MS_SYNC);
    // Eliminar el archivo físico y su metadata
    remove(archivo_datos->path_nombre);
    config_destroy(archivo_datos->metadata);
    free(archivo_datos->nombre);
    free(archivo_datos->path_nombre);
    free(archivo_datos);

    log_info(logger_io, "Archivo %s eliminado correctamente.", nombre_archivo);
    return 0; // Archivo eliminado exitosamente
}

static void compactar_archivos(t_list* lista_archivos, int* offset_bloques) {
    // Ordena la lista según el bloque inicial sin usar comparator
    for (int i = 0; i < list_size(lista_archivos) - 1; i++) {
        for (int j = 0; j < list_size(lista_archivos) - i - 1; j++) {
            t_fs_archivo* archivo1 = list_get(lista_archivos, j);
            t_fs_archivo* archivo2 = list_get(lista_archivos, j + 1);
            if (archivo1->puntero_inicio > archivo2->puntero_inicio) {
                list_replace(lista_archivos, j, archivo2);
                list_replace(lista_archivos, j + 1, archivo1);
            }
        }
    }

    t_list_iterator* iterador = list_iterator_create(lista_archivos);
    while (list_iterator_has_next(iterador)) {
        t_fs_archivo* archivo = (t_fs_archivo*)list_iterator_next(iterador);
        void* buffer = malloc(archivo->tamano);
        liberar_bloques(archivo, buffer);
        memcpy(info_FS.archivo_bloques_en_memoria + obtener_offset_de_bloque(*offset_bloques), buffer, archivo->tamano);
        archivo->puntero_inicio = *offset_bloques;
        char* offset_bloques_str = string_itoa(*offset_bloques);
        config_set_value(archivo->metadata, "BLOQUE_INICIAL", offset_bloques_str);
        config_save(archivo->metadata);
        *offset_bloques += archivo->cantidad_bloques;
        free(buffer);
        free(offset_bloques_str);
    }
    list_iterator_destroy(iterador);
}

void mover_archivo_al_final(t_fs_archivo* archivo_a_mover, void* buffer, int offset_bloques) {
    // Mueve el archivo al final
    memcpy(info_FS.archivo_bloques_en_memoria + obtener_offset_de_bloque(offset_bloques), buffer, archivo_a_mover->tamano);
    // Actualiza la información del archivo
    archivo_a_mover->puntero_inicio = offset_bloques;
    char* offset_bloques_str = string_itoa(offset_bloques);
    config_set_value(archivo_a_mover->metadata, "BLOQUE_INICIAL", offset_bloques_str);
    config_save(archivo_a_mover->metadata);
    msync(info_FS.archivo_bloques_en_memoria, info_FS.tamano_total_bloques, MS_SYNC);
    free(offset_bloques_str);
}

void compactar(char* nombre_archivo_a_mover_al_final) {
    log_info(logger_io, "Inicio de compactación para el archivo: %s", nombre_archivo_a_mover_al_final);
    usleep(RETRASO_COMPACTACION*1000);

    t_fs_archivo* archivo_a_mover = dictionary_get(info_FS.fs_archivos, nombre_archivo_a_mover_al_final);
    if (archivo_a_mover == NULL) {
        log_error(logger_io, "El archivo %s no se encuentra en el sistema de archivos", nombre_archivo_a_mover_al_final);
        return;
    }

    log_info(logger_io, "Archivo a mover encontrado: %s", archivo_a_mover->nombre);
    void* buffer = malloc(archivo_a_mover->tamano);
    if (buffer == NULL) {
        log_error(logger_io, "Error al asignar memoria para el buffer");
        return;
    }
    liberar_bloques(archivo_a_mover, buffer);
    t_list* lista_archivos = dictionary_elements(info_FS.fs_archivos);
    list_remove_element(lista_archivos, (void*)archivo_a_mover);
    // Compactar archivos en orden de bloque inicial
    int offset_bloques = 0;
    compactar_archivos(lista_archivos, &offset_bloques);
    list_destroy(lista_archivos);
    mover_archivo_al_final(archivo_a_mover, buffer, offset_bloques);
    free(buffer);

    // Actualiza el bitmap
    for (int i = 0; i < archivo_a_mover->puntero_inicio + archivo_a_mover->cantidad_bloques; i++) {
        bitarray_set_bit(info_FS.bitmap, i);
    }
    msync(info_FS.archivo_bitmap_en_memoria, info_FS.tamano_bitmap, MS_SYNC);
}

static int actualizar_tamano_archivo(t_fs_archivo* archivo_datos, int nuevo_tamano) {
    archivo_datos->tamano = nuevo_tamano;
    char* tamano_str = string_itoa(nuevo_tamano);
    config_set_value(archivo_datos->metadata, "TAMANIO_ARCHIVO", tamano_str);
    config_save(archivo_datos->metadata);
    free(tamano_str);
    return 0;
}

static int achicar_archivo(t_fs_archivo* archivo_datos, int nuevo_tamano, int nueva_cantidad_bloques) {
    for (int i = archivo_datos->puntero_inicio + archivo_datos->cantidad_bloques - 1; i >= archivo_datos->puntero_inicio + nueva_cantidad_bloques; i--) {
        bitarray_clean_bit(info_FS.bitmap, i);
    }
    msync(info_FS.archivo_bitmap_en_memoria, info_FS.tamano_bitmap, MS_SYNC);

    archivo_datos->tamano = nuevo_tamano;
    archivo_datos->cantidad_bloques = nueva_cantidad_bloques;
    return actualizar_tamano_archivo(archivo_datos, nuevo_tamano);
}

static int agrandar_archivo(t_fs_archivo* archivo_datos, int nuevo_tamano, int nueva_cantidad_bloques, char* nombre_archivo) {
    bool necesita_compactacion = false;
    for (int i = archivo_datos->puntero_inicio + archivo_datos->cantidad_bloques; i < archivo_datos->puntero_inicio + nueva_cantidad_bloques && !necesita_compactacion; i++) {
        necesita_compactacion = bitarray_test_bit(info_FS.bitmap, i);
    }

    if (necesita_compactacion) {
        compactar(nombre_archivo);
    }

    for (int i = archivo_datos->puntero_inicio + archivo_datos->cantidad_bloques; i < archivo_datos->puntero_inicio + nueva_cantidad_bloques; i++) {
        bitarray_set_bit(info_FS.bitmap, i);
    }
    msync(info_FS.archivo_bitmap_en_memoria, info_FS.tamano_bitmap, MS_SYNC);

    archivo_datos->tamano = nuevo_tamano;
    archivo_datos->cantidad_bloques = nueva_cantidad_bloques;
    return actualizar_tamano_archivo(archivo_datos, nuevo_tamano);
}

int truncar_archivo(char* nombre_archivo, int nuevo_tamano) {
    t_fs_archivo* archivo_datos = dictionary_get(info_FS.fs_archivos, nombre_archivo);
    if (archivo_datos == NULL) {
        log_error(logger_io, "El archivo %s no existe.", nombre_archivo);
        return -1; // El archivo no existe
    }
    // int nueva_cantidad_bloques = (nuevo_tamano + info_FS.tamano_bloque - 1) / info_FS.tamano_bloque;
    int nueva_cantidad_bloques = cantidad_bloques(nuevo_tamano,info_FS.tamano_bloque);
    if (nueva_cantidad_bloques == archivo_datos->cantidad_bloques) {
        return actualizar_tamano_archivo(archivo_datos, nuevo_tamano);
    }
    if (nueva_cantidad_bloques < archivo_datos->cantidad_bloques) {
        return achicar_archivo(archivo_datos, nuevo_tamano, nueva_cantidad_bloques);
    }
    return agrandar_archivo(archivo_datos, nuevo_tamano, nueva_cantidad_bloques, nombre_archivo);
}

int leer_archivo(char* nombre_archivo, int puntero, int tamanio, char* buffer) {
    // Obtiene los datos del archivo desde el diccionario
    t_fs_archivo* archivo_datos = (t_fs_archivo*)dictionary_get(info_FS.fs_archivos, nombre_archivo);
    if(archivo_datos==NULL){
        log_error(logger_io, "El archivo %s no existe.", nombre_archivo);
        return -1; // El archivo no existe
    }    
    // Calcula el offset del bloque en la memoria mapeada
    int offset_bloques = obtener_offset_de_bloque(archivo_datos->puntero_inicio);
    
    // Copia los datos desde la memoria mapeada al buffer
    memcpy(buffer, info_FS.archivo_bloques_en_memoria + offset_bloques + puntero, tamanio);
    
    // Libera la estructura de datos del archivo (si es necesario)
    // fs_archivo_destroy(archivo_datos);
    
    // Retorna un valor indicando el éxito de la operación
    return 0; // Puedes cambiar esto para manejar errores si es necesario
}

int escribir_archivo(char* nombre_archivo, int puntero, int tamanio, char* resultado_escritura) {
    log_info(logger_io, "Iniciando escritura en el archivo <%s> - puntero <%d> - tamaño <%d>", nombre_archivo, puntero, tamanio);

    t_fs_archivo* archivo_datos = (t_fs_archivo*)dictionary_get(info_FS.fs_archivos, nombre_archivo);
    if (archivo_datos == NULL) {
        log_error(logger_io, "El archivo %s no existe.", nombre_archivo);
        return -1; // El archivo no existe
    }

    log_info(logger_io, "Archivo encontrado <%s>", archivo_datos->nombre);
    int offset_bloques = obtener_offset_de_bloque(archivo_datos->puntero_inicio);
    log_info(logger_io, "Offset de bloques calculado <%d>", offset_bloques);

    // Verifica si hay suficiente espacio para escribir
    int espacio_disponible = archivo_datos->cantidad_bloques * info_FS.tamano_bloque;
    log_info(logger_io, "Espacio disponible <%d>", espacio_disponible);
    if (tamanio > espacio_disponible) {
        log_error(logger_io, "No hay suficiente espacio en el archivo %s para escribir %d bytes.", nombre_archivo, tamanio);
        return -1;
    }

    // Escribe los datos en la memoria mapeada
    log_info(logger_io, "Copiando datos a la memoria mapeada");
    memcpy(info_FS.archivo_bloques_en_memoria + offset_bloques + puntero, resultado_escritura, tamanio);

    log_info(logger_io, "Sincronizando cambios con msync");
    msync(info_FS.archivo_bloques_en_memoria + offset_bloques + puntero, tamanio, MS_SYNC);


    log_info(logger_io, "Datos escritos en el archivo <%s> con éxito.", nombre_archivo);
    return 0;
}