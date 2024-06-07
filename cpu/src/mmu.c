#include <mmu.h>

static int _get_frame_from_tlb();
static void _consultar_frame_a_memoria(int socket_cliente_cpu);

int MMU(int dir_logica)
{
    log_protegido_cpu(string_from_format("Obteniendo DF..."));
    int num_pagina = floor(dir_logica / TAM_PAG);
    int desplazamiento = dir_logica - num_pagina * TAM_PAG;

    log_protegido_cpu(string_from_format("Se busca en TLB..."));
    int frame = _get_frame_from_tlb();

    log_protegido_cpu(string_from_format("[MMU - Rta TLB] frame: %d", frame));

    // En caso que no encuentre en la TLB
    if (frame == -1)
    {
        log_protegido_cpu(string_from_format("Se busca en memoria..."));

        // Pedir pagina a MEMORIA, si la tiene, devuelve su marco, de lo contrario devuelve page fault
        int socket_cliente_cpu = 0; // TODO: ver de donde obtengo el socket de memoria
        _consultar_frame_a_memoria(socket_cliente_cpu);

        log_protegido_cpu(string_from_format("Esperando respuesta de memoria..."));
        sem_wait(&sem_control_peticion_marco_a_memoria);

        log_protegido_cpu(string_from_format("Se obtiene respuesta de memoria..."));
        if (frame >= 0)
        {
            // log_info(cpu_log_obligatorio, "PID: <%d> - OBTENER MARCO - Página: <%d> - Marco: <%d>", contexto->proceso_pid, num_pagina, marco);
            int dir_fisica = frame * TAM_PAG + desplazamiento;
            // contexto->proceso_ip = contexto->proceso_ip + 1;
            frame = dir_fisica;
        }
        // else
        // {
            //TODO: averiguar que ocurre en caso que encuentre, tira un PF?
            //https://github.com/sisoputnfrba/foro/issues/3814
        //     frame = -1;
        // }
    }
    // en caso que ocurre PF devolvemos -1
    log_protegido_cpu(string_from_format("[MMU - Rta MEMORIA] frame: %d", frame));

    return frame;
}

// TODO: ver que parametro le tengo que pasar
static int _get_frame_from_tlb()
{
    // TODO: implementacion TLB HIT

    // en caso de tlb miss
    log_protegido_cpu(string_from_format("[TLB MISS] La pagina no se encuentra en la TLB"));
    return -1;
}

static void _consultar_frame_a_memoria(int socket_cliente_cpu)
{
    t_paquete *paquete_a_enviar = crear_paquete(TAMANIO_PAGINA);
    // agregar_datos_sin_tamaño_a_paquete(paquete_a_enviar,&(TAM_PAGINA),sizeof(int));
    enviar_paquete(paquete_a_enviar, socket_cliente_cpu);
    eliminar_paquete(paquete_a_enviar);
}