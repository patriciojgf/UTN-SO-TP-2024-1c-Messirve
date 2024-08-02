#include "gestion_mem_usuario.h"

void mem_escribir_dato_direccion_fisica(int dir_fisica, void* dato, int size, int pid) {
    // log_obligatorio
    // Acceso a espacio de usuario: “PID: <PID> - Accion: <LEER / ESCRIBIR> - Direccion fisica: <DIRECCION_FISICA>” - Tamaño <TAMAÑO A LEER / ESCRIBIR>
    log_info(logger_memoria, "PID: <%d> - Accion: <ESCRIBIR> - Direccion fisica: <%d> - Tamaño <%d>", pid, dir_fisica, size);
    // memcpy(memoria_espacio_usuario + dir_fisica, dato, size);
    // Validar que la dirección y tamaño no excedan el espacio de memoria
    if (dir_fisica + size > TAM_MEMORIA || dir_fisica < 0) {
        log_error(logger_memoria, "Intento de escritura fuera de los límites de la memoria por PID: %d, dirección: %d, tamaño: %d", pid, dir_fisica, size);
        //return -1;
    }
    // Realizar la copia de datos al espacio de memoria
    memcpy(memoria_espacio_usuario + dir_fisica, dato, size);    
}

void* mem_leer_dato_direccion_fisica(int dir_fisica, int size, int pid) {
    // log_obligatorio
    // Acceso a espacio de usuario: “PID: <PID> - Accion: <LEER / ESCRIBIR> - Direccion fisica: <DIRECCION_FISICA>” - Tamaño <TAMAÑO A LEER / ESCRIBIR>
    log_info(logger_memoria, "PID: <%d> - Accion: <LEER> - Direccion fisica: <%d> - Tamaño <%d>", pid, dir_fisica, size);
    void* dato_leido = malloc(size);
    if (dato_leido == NULL) {
        // Manejo de error si malloc falla
        log_error(logger_memoria, "No se pudo asignar memoria para lectura desde dirección física.");
        return NULL;
    }
    memcpy(dato_leido, memoria_espacio_usuario + dir_fisica, size);
    // char *dato_leido_str = malloc(size + 1);
    // memcpy(dato_leido_str, dato_leido, size);
    // dato_leido_str[size] = '\0';
    // free(dato_leido_str);
    return dato_leido;
}

