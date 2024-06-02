#include "estructuras.h"

void inicializar_registros(t_registros_cpu* registros)
{
        registros->AX = 0;
        registros->BX = 0;
        registros->CX = 0;
        registros->DX = 0;
        registros->EAX = 0;
        registros->EBX = 0;
        registros->ECX = 0;
        registros->EDX = 0;
        registros->PC = 0;
        registros->SI = 0;
        registros->DI = 0;
}