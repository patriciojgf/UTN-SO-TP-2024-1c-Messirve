#include <mmu.h>

/**************** MMU ****************/
static void _agregar_fila_tlb(int pid, int pagina, int marco);
static bool _comparar_referencias(fila_tlb* first_row, fila_tlb* second_row);
static void _consultar_frame_a_memoria(int socket_cliente_cpu);
static int _get_frame_from_tlb();

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
static int _get_frame_from_tlb(int pid, int pagina)
{
    // TODO: implementacion TLB HIT
    for(int i=0; i<CANTIDAD_ENTRADAS_TLB; i++)
    {
        fila_tlb* fila_tlb_aux = list_get(TLB, i); //TODO: declarar la variable afuera del for
        log_protegido_cpu(string_from_format("PID: %d",fila_tlb_aux->pid));
        log_protegido_cpu(string_from_format("Page: %d",fila_tlb_aux->page));
        // log_protegido_cpu(string_from_format("Ciclo cpu: %d",fila_tlb_aux->last_reference));

        if((fila_tlb_aux->pid == pid) && (fila_tlb_aux->page == pagina))
        {
            struct timeval time;
            gettimeofday(&time, NULL);
            fila_tlb_aux->last_reference = time;

            log_protegido_cpu(string_from_format("[TLB HIT] Frame: %d",fila_tlb_aux->frame));
            return fila_tlb_aux->frame;
        }
    }

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

/**************** TLB ****************/
void iniciar_tlb()
{
    log_protegido_cpu(string_from_format("[TLB] Inicializando TLB..."));
    for(int i=0; i<CANTIDAD_ENTRADAS_TLB; i++)
    {
        _agregar_fila_tlb(-1, -1, -1); //valores negativos primera vez para diferenciar
    }
}

void agregar_a_tbl(int pid, int pagina, int marco)
{
    log_protegido_cpu(string_from_format("CANTIDAD_ENTRADAS_TLB: %d", CANTIDAD_ENTRADAS_TLB));
    if(CANTIDAD_ENTRADAS_TLB == 0){
        log_protegido_cpu(string_from_format("La cantidad de entradas de la TLB es 0"));
        return;
    }
    else if(list_size(TLB) < CANTIDAD_ENTRADAS_TLB)
    {
        _agregar_fila_tlb(pid, pagina, marco);
    }
    else if(list_size(TLB) == CANTIDAD_ENTRADAS_TLB)
    {
        log_protegido_cpu(string_from_format("ALGORITMO_TLB: %s", ALGORITMO_TLB));
        if(string_equals_ignore_case(ALGORITMO_TLB, FIFO))
        {
            list_remove_and_destroy_element(TLB, 0, (void*)destroy_fila_TLB);
        }
        else if(string_equals_ignore_case(ALGORITMO_TLB, LRU))
        {
            log_protegido_cpu(string_from_format("Tamaño tlb %d previo al list_sort", list_size(TLB)));
            list_sort(TLB, (void*)_comparar_referencias); 
            log_protegido_cpu(string_from_format("Tamaño tlb %d", list_size(TLB)));
            list_remove(TLB, 0);
        }
        _agregar_fila_tlb(pid, pagina, marco);
    }
    else 
    {
        log_protegido_cpu(string_from_format("La lista TLB es mayor a las entradas de la tlb"));
    }
}


static bool _comparar_referencias(fila_tlb* first_row, fila_tlb* second_row)
{
    //tv_sec nos da los segundos desde 1970
    if(first_row->last_reference.tv_sec == second_row->last_reference.tv_sec){
        return (first_row->last_reference.tv_usec) < (second_row->last_reference.tv_usec);
    }
    return (first_row->last_reference.tv_sec) < (second_row->last_reference.tv_sec);
}

//TODO: se va a usar en otro lado?  
static void _agregar_fila_tlb(int pid, int pagina, int marco)
{
    fila_tlb* fila_tlb_aux = malloc(sizeof(fila_tlb));
    struct timeval time;
    gettimeofday(&time, NULL);

    fila_tlb_aux->pid = pid;
    fila_tlb_aux->page = pagina;
    fila_tlb_aux->frame = marco;
    fila_tlb_aux->init_time = time;
    fila_tlb_aux->last_reference = time; //cuando se carga por primera vez, el init_time sera igual al time de last_reference

    list_add(TLB, fila_tlb_aux);
}

void destroy_fila_TLB(fila_tlb* row_to_destroy) 
{
    free(row_to_destroy);
}

void log_tlb()
{
    log_protegido_cpu(string_from_format("[TLB] pid | pagina | marco"));
    log_protegido_cpu(string_from_format("TLB: "));
    for(int i = 0; i < list_size(TLB); i++)
    {
        fila_tlb* tlb_aux = list_get(TLB, i);
        log_protegido_cpu(string_from_format("%d | PID: %d | Pagina: %d | Marco: %d",
            i,
            tlb_aux->pid,
            tlb_aux->page,
            tlb_aux->frame
        ));
    }
}