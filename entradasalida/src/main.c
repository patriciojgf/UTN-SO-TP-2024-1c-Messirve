#include <main_io.h>
int main(int argc, char* argv[]) {
    init_io(argv[1],argv[2]);
    init_conexiones();
    if(strcmp(TIPO_INTERFAZ, "GENERICA") != 0){
        gestionar_conexion_memoria();
    }
    gestionar_conexion_kernel();

    finalizar_log(logger_io);
    // finalizar_config_io(datos_io);
    finalizar_config(config_io);

    return EXIT_SUCCESS;
}
