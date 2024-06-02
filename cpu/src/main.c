#include <main_cpu.h>

int main(int argc, char* argv[]) {
	init_cpu(argv[1]);   
    init_conexiones();
    gestionar_conexion_memoria();   //cliente   detach
    gestionar_conexion_interrupt(); //servidor  detach
    gestionar_conexion_dispatch();  //servidor  join
    finalizar_log(logger_cpu);
    finalizar_config(config_cpu);

    return EXIT_SUCCESS;
}