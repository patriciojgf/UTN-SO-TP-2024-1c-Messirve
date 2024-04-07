#include<configuracion_io.h>

t_config_io* iniciar_config_io(t_config* config_io){
    t_config_io* datos = malloc(sizeof(t_config_io));

    datos->tipo_interfaz = config_get_string_value(config_io, "TIPO_INTERFAZ");
    datos->tiempo_unidad_trabajo = config_get_int_value(config_io, "TIEMPO_UNIDAD_TRABAJO");
    datos->ip_kernel = config_get_string_value(config_io, "IP_KERNEL");
    datos->puerto_kernel = config_get_string_value(config_io, "PUERTO_KERNEL");
    datos->ip_memoria = config_get_string_value(config_io, "IP_MEMORIA");
    datos->puerto_memoria = config_get_string_value(config_io, "PUERTO_MEMORIA");
    datos->path_base = config_get_string_value(config_io, "PATH_BASE_DIALFS");
    datos->block_size = config_get_int_value(config_io, "BLOCK_SIZE");
    datos->block_count = config_get_int_value(config_io, "BLOCK_COUNT");

    return datos;
}

void finalizar_config_io(t_config_io* config_io){
    free(config_io);
}