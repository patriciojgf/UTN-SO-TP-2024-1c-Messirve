#include "gestion_memoria.h"

/*----*/
static int tlb_buscar_pagina(int num_pagina);
static int tlb_eliminar_entrada();
static int tlb_busco_entrada_libre();
static void tlb_agregar_pagina(int pid, int pagina, int marco);
static int memoria_pido_marco(int pagina);
/*----*/

static void mostar_entradas_tbl(){
    for(int i = 0; i < list_size(lista_tlb); i++){
        t_fila_tlb* entrada = list_get(lista_tlb, i);
        log_info(logger_cpu, "TLB - Entrada <%d> - PID <%d> - Pagina <%d> - Marco <%d> - TIME <%ld>", i, entrada->pid, entrada->pagina, entrada->marco, entrada->timestamp);
    }
}

static int min(int a, int b) {
    return (a < b) ? a : b;
}

/*----*/

int escribir_valor_en_memoria(int direccion_logica, int cantidad_bytes, char* valor) {
    log_info(logger_cpu,"escribir_valor_en_memoria - valor: %s", valor);
    log_info(logger_cpu,"escribir_valor_en_memoria - valor: %d", *valor);
    log_info(logger_cpu,"escribir_valor_en_memoria - valor: %u", *valor);

    int total_escrito = 0;

    while (total_escrito < cantidad_bytes) {
        int direccion_fisica = mmu(direccion_logica + total_escrito);
        if (direccion_fisica == -1) {
            return -1;
        }

        // Calcular cuánto escribir en este segmento
        int bytes_restantes = cantidad_bytes - total_escrito;
        int bytes_a_escribir = min(tamanio_pagina - (direccion_logica % tamanio_pagina), bytes_restantes);

        // Preparar y enviar el paquete de escritura
        t_paquete* paquete_a_enviar = crear_paquete(ESCRIBIR_MEMORIA);
        agregar_datos_sin_tamaño_a_paquete(paquete_a_enviar, &contexto_cpu->pid, sizeof(int));
        agregar_datos_sin_tamaño_a_paquete(paquete_a_enviar, &direccion_fisica, sizeof(int));
        agregar_datos_sin_tamaño_a_paquete(paquete_a_enviar, &bytes_a_escribir, sizeof(int));
        agregar_datos_sin_tamaño_a_paquete(paquete_a_enviar, valor + total_escrito, bytes_a_escribir);
        enviar_paquete(paquete_a_enviar, socket_memoria);
        eliminar_paquete(paquete_a_enviar);

        // Esperar la confirmación de escritura
        sem_wait(&s_pedido_escritura_m);
        total_escrito += bytes_a_escribir;
	//log obligario
	//Lectura/Escritura Memoria: “PID: <PID> - Acción: <LEER / ESCRIBIR> - Dirección Física: <DIRECCION_FISICA> - Valor: <VALOR LEIDO / ESCRITO>”.
	log_info(logger_cpu,"PID: <%d> - Acción: <ESCRIBIR> - Dirección Física: <%d> - Valor: <%d>", contexto_cpu->pid, direccion_fisica, *(valor+total_escrito));
    }
    return 0;
}

// char* leer_memoria(int direccion_logica, int cantidad_bytes){
//     int direccion_fisica = mmu(direccion_logica);
//     if(direccion_fisica == -1){
//         log_warning(logger_cpu, "falta implementar PAGE FAULT");
//         exit(EXIT_FAILURE);
//     }
//     t_paquete* paquete_a_enviar = crear_paquete(LEER_MEMORIA);
//     agregar_datos_sin_tamaño_a_paquete(paquete_a_enviar, &contexto_cpu->pid, sizeof(int));
//     agregar_datos_sin_tamaño_a_paquete(paquete_a_enviar, &direccion_fisica, sizeof(int));
//     agregar_datos_sin_tamaño_a_paquete(paquete_a_enviar, &cantidad_bytes, sizeof(int));
//     enviar_paquete(paquete_a_enviar, socket_memoria);
//     eliminar_paquete(paquete_a_enviar);
//     sem_wait(&s_pedido_lectura_m);
//     return respuesta_memoria_char;
// }

char* leer_memoria(int direccion_logica, int cantidad_bytes) {
    int total_leido = 0;
    char* resultado_final = malloc(cantidad_bytes + 1); // Asegurar espacio para el resultado final
    if (!resultado_final) {
        log_error(logger_cpu, "No se pudo asignar memoria para leer datos.");
        return NULL;
    }
    resultado_final[0] = '\0'; // Inicializar como cadena vacía

    while (total_leido < cantidad_bytes) {
        int direccion_fisica = mmu(direccion_logica + total_leido);
        if (direccion_fisica == -1) {
            free(resultado_final);
            return "-1";
        }

        // Calcular cuánto leer en este segmento
        int bytes_restantes = cantidad_bytes - total_leido;
        int bytes_a_leer = min(tamanio_pagina - (direccion_logica % tamanio_pagina), bytes_restantes);

        // Preparar y enviar el paquete de lectura
        t_paquete* paquete_a_enviar = crear_paquete(LEER_MEMORIA);
        agregar_datos_sin_tamaño_a_paquete(paquete_a_enviar, &contexto_cpu->pid, sizeof(int));
        agregar_datos_sin_tamaño_a_paquete(paquete_a_enviar, &direccion_fisica, sizeof(int));
        agregar_datos_sin_tamaño_a_paquete(paquete_a_enviar, &bytes_a_leer, sizeof(int));
        enviar_paquete(paquete_a_enviar, socket_memoria);
        eliminar_paquete(paquete_a_enviar);

        // Esperar la respuesta
        sem_wait(&s_pedido_lectura_m);
        //log obligatorio
        //Lectura/Escritura Memoria: “PID: <PID> - Acción: <LEER / ESCRIBIR> - Dirección Física: <DIRECCION_FISICA> - Valor: <VALOR LEIDO / ESCRITO>”.
	    log_info(logger_cpu,"PID: <%d> - Acción: <LEER> - Dirección Física: <%d> - Valor: <%d> \n", contexto_cpu->pid, direccion_fisica, *respuesta_memoria_char);
        
        // Asumiendo que respuesta_memoria_char contiene los bytes leídos
        memcpy(resultado_final + total_leido, respuesta_memoria_char, bytes_a_leer);
        total_leido += bytes_a_leer;
    }

    resultado_final[cantidad_bytes] = '\0'; // Asegurarse de que el resultado es una cadena válida
    return resultado_final;
}

int mmu(int direccion_logica){
    int numero_pagina = direccion_logica / tamanio_pagina;
    int desplazamiento = direccion_logica % tamanio_pagina;
    int marco = -1;
    int direccion_fisica = -1;
    //Si tbl tiene 0 entradas no busco
    if(CANTIDAD_ENTRADAS_TLB == 0){
        marco = memoria_pido_marco(numero_pagina);
    }
    else{
        marco = tlb_buscar_pagina(numero_pagina);
        if(marco == -1){
            //log obligatorio: TLB Miss: “PID: <PID> - TLB MISS - Pagina: <NUMERO_PAGINA>”
            log_info(logger_cpu, "PID: %d - TLB MISS - Pagina: %d", contexto_cpu->pid, numero_pagina);
            log_info(logger_cpu, "-------------");
            log_info(logger_cpu, "TLB a actualizar:");
            mostar_entradas_tbl();
            //No esta presente en TLB
            //Busco en tabla de paginas
            marco = memoria_pido_marco(numero_pagina);
            if(marco == -1){
                return -1;
            } 
            else{
                //Actualizo TLB
                tlb_agregar_pagina(contexto_cpu->pid, numero_pagina, marco);
                log_info(logger_cpu, "Actualice TLB:");
                mostar_entradas_tbl();
                log_info(logger_cpu, "-------------");
            }
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
static int tlb_eliminar_entrada() {
    if (list_is_empty(lista_tlb)) {
        log_error(logger_cpu, "La lista TLB está vacía");
        exit(EXIT_FAILURE); // Manejo del error en caso de que la lista esté vacía
    }

    // Inicializar con la primera entrada
    t_fila_tlb* entrada_a_reutilizar = list_get(lista_tlb, 0);
    
    int indice_a_reutilizar = 0;
    time_t min_timestamp = entrada_a_reutilizar->timestamp;

    for (int i = 1; i < list_size(lista_tlb); i++) {
        t_fila_tlb* entrada = list_get(lista_tlb, i);
        if (entrada->timestamp < min_timestamp) {
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
    } else {
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
    if(indice_entrada == -1){
        indice_entrada = tlb_eliminar_entrada();
    }

    //piso los valores de la lista_tlb en el indice indice_entrada
    t_fila_tlb* entrada = list_get(lista_tlb, indice_entrada);
    if(entrada->pagina != -1){
        log_info(logger_cpu,"Voy a reemplazar en TBL la pagina <%d> - marco <%d>", entrada->pagina, entrada->marco);
    }
    log_info(logger_cpu,"PID <%d> - Voy a agregar la entrada de la TLB en el índice <%d> - pagina <%d> - marco <%d>", pid, indice_entrada, pagina, marco);
    entrada->pid = pid;
    entrada->pagina = pagina;
    entrada->marco = marco;
    if(strcmp(ALGORITMO_TLB, "LRU") == 0){
        entrada->timestamp = time(NULL); // Actualizamos el timestamp para LRU
        log_info(logger_cpu,"TLB - Actualizo el timestamp al valor <%ld>", entrada->timestamp);
    }
}

static int memoria_pido_marco(int pagina){
    //log obligatorio “PID: <PID> - OBTENER MARCO - Página: <NUMERO_PAGINA> - Marco: <NUMERO_MARCO>”.
    log_info(logger_cpu, "PID: <%d> - OBTENER MARCO - Página: <%d>", contexto_cpu->pid, pagina);
    t_paquete* paquete_a_enviar = crear_paquete(PEDIDO_MARCO);
    agregar_datos_sin_tamaño_a_paquete(paquete_a_enviar, &contexto_cpu->pid, sizeof(int));
    agregar_datos_sin_tamaño_a_paquete(paquete_a_enviar, &pagina, sizeof(int));
    enviar_paquete(paquete_a_enviar, socket_memoria);
    eliminar_paquete(paquete_a_enviar);    
    sem_wait(&s_pedido_marco);
    return respuesta_memoria;
}
