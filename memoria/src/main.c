#include "main.h"

/* -------------------------------------Iniciar Memoria -----------------------------------------------*/
int main(int argc, char **argv){    
    init_memoria(argv[1]);   
    init_conexiones();
    gestionar_conexiones_entrantes();
    return 0;
}