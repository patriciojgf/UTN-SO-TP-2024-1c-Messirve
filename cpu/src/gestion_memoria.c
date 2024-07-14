#include "gestion_memoria.h"

/*----*/
static int tlb_buscar_pagina(int num_pagina);
static int tlb_eliminar_entrada();
static int tlb_busco_entrada_libre();
static void tlb_agregar_pagina(int pid, int pagina, int marco);
static int memoria_pido_marco(int pagina);
/*----*/

int mmu(int direccion_logica){
    int numero_pagina = direccion_logica / tamanio_pagina;
    int desplazamiento = direccion_logica % tamanio_pagina;
    int marco = -1;
    int direccion_fisica = -1;

    marco = tlb_buscar_pagina(numero_pagina);
    if(marco == -1){
        //log obligatorio: TLB Miss: “PID: <PID> - TLB MISS - Pagina: <NUMERO_PAGINA>”
        log_info(logger_cpu, "PID: %d - TLB MISS - Pagina: %d", contexto_cpu->pid, numero_pagina);

        //No esta presente en TLB
        //Busco en tabla de paginas
        marco = memoria_pido_marco(numero_pagina);
        if(marco == -1){
            //No esta presente en tabla de paginas
            //TODO
            log_warning(logger_cpu,"FALTA IMPLEMENTAR PAGE FAULT");
            return -1;
        } 
        else{
            //Actualizo TLB
            tlb_agregar_pagina(contexto_cpu->pid, numero_pagina, marco);
        }
    }
    direccion_fisica = marco * tamanio_pagina + desplazamiento;
    return direccion_fisica;
}




/*--------------------------------------------AUX TLB--------------------------------------------*/
//tlb_buscar_pagina: valida si existe la pagina en la tbl para el pid actual.
static int tlb_buscar_pagina(int num_pagina) {
    int marco = -1;
    // Busca ese pid y num_pagina en la estructura lista_tlb
    for (int i = 0; i < list_size(lista_tlb); i++) {
        t_fila_tlb* entrada = list_get(lista_tlb, i);
        if (entrada->pid == contexto_cpu->pid && entrada->pagina == num_pagina) {
            
            //log obligatorio TLB Hit: “PID: <PID> - TLB HIT - Pagina: <NUMERO_PAGINA>”
            log_info(logger_cpu, "PID: %d - TLB HIT - Pagina: %d", contexto_cpu->pid, num_pagina);

            marco = entrada->marco;
            // Actualiza el timestamp de último acceso si se usa LRU
            if(strcmp(ALGORITMO_TLB, "LRU") == 0){
                entrada->timestamp = time(NULL);
            }
            break;
        }
    }
    return marco;
}

//tlb_eliminar_entrada: elimina una entrada de la tlb siguiendo el algoritmo de reemplazo
static int tlb_eliminar_entrada(){
    // Busca la entrada con el timestamp más antiguo para reutilizar
    t_fila_tlb* entrada_a_reutilizar = NULL;
    int indice_a_reutilizar = -1;
    time_t min_timestamp = time(NULL);    
    for(int i = 0; i < list_size(lista_tlb); i++){
        t_fila_tlb* entrada = list_get(lista_tlb, i);
        if(entrada->timestamp < min_timestamp){
            min_timestamp = entrada->timestamp;
            entrada_a_reutilizar = entrada;
            indice_a_reutilizar = i;
        }
    }
    if (entrada_a_reutilizar) {
        entrada_a_reutilizar->pid = -1;  // Indica que la entrada está libre
        entrada_a_reutilizar->pagina = -1;
        entrada_a_reutilizar->marco = -1;
        entrada_a_reutilizar->timestamp = time(NULL);  // Actualizar el timestamp al momento actual
    }
    else{
        log_error(logger_cpu, "No se encontró ninguna entrada para reutilizar en la TLB");
        exit(EXIT_FAILURE);
    }
    return indice_a_reutilizar;
}

//tlb_busco_entrada_libre: valida algun pid -1 dentro de la tlb y retorna el indice
static int tlb_busco_entrada_libre(){
    int indice_entrada = -1;
    for(int i = 0; i < list_size(lista_tlb); i++){
        t_fila_tlb* entrada = list_get(lista_tlb, i);
        if(entrada->pid == -1){
            indice_entrada = i;
            break;
        }
    }
    return indice_entrada;
}

static void tlb_agregar_pagina(int pid, int pagina, int marco){
    int indice_entrada = tlb_busco_entrada_libre();
    if(indice_entrada != -1){
        indice_entrada = tlb_eliminar_entrada();
    }

    //piso los valores de la lista_tlb en el indice indice_entrada
    t_fila_tlb* entrada = list_get(lista_tlb, indice_entrada);
    entrada->pid = pid;
    entrada->pagina = pagina;
    entrada->marco = marco;
    if(strcmp(ALGORITMO_TLB, "LRU") == 0){
        entrada->timestamp = time(NULL); // Actualizamos el timestamp para LRU
    }
}

static int memoria_pido_marco(int pagina){
    //log obligatorio “PID: <PID> - OBTENER MARCO - Página: <NUMERO_PAGINA> - Marco: <NUMERO_MARCO>”.
    log_info(logger_cpu, "PID: %d - OBTENER MARCO - Página: %d", contexto_cpu->pid, pagina);
    t_paquete* paquete_a_enviar = crear_paquete(PEDIDO_MARCO);
    agregar_datos_sin_tamaño_a_paquete(paquete_a_enviar, &contexto_cpu->pid, sizeof(int));
    agregar_datos_sin_tamaño_a_paquete(paquete_a_enviar, &pagina, sizeof(int));
    enviar_paquete(paquete_a_enviar, socket_memoria);
    eliminar_paquete(paquete_a_enviar);    
    sem_wait(&s_pedido_marco);
    return respuesta_memoria;
}
