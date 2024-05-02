#ifndef ATENDER_CPU_H_
#define ATENDER_CPU_H_

#include "mem_estructuras.h"
#include "proceso.h"


/*IN*/
void atender_fetch_instruccion(int pid, int PC);


/*OUT*/
void envio_instruccion_a_cpu(char* instruccion);



#endif /* ATENDER_CPU_H_ */
