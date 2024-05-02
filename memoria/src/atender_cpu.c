#include "atender_cpu.h"

/*IN*/

void atender_fetch_instruccion(int pid, int PC){
    log_protegido_mem(string_from_format("Atendiendo pedido de instruccion PID: %d, PC: %d",pid,PC));
    t_proceso* proceso = get_proceso_memoria(pid);
    char* instruccion = get_instruccion_proceso(proceso,PC);
    envio_instruccion_a_cpu(instruccion);
}



/*OUT*/
void envio_instruccion_a_cpu(char* instruccion){
    enviar_mensaje(instruccion, socket_cpu);
}