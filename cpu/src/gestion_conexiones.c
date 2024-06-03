#include "gestion_conexiones.h"

static void _handshake_cliente_memoria(int socket, char* nombre_destino);
static void _check_interrupt(t_instruccion* instruccion);
static void _ejecutar_proceso();
static void _recibir_nuevo_contexto(void* buffer);
// static int _get_retardo();



// --------------------------------------------------------------------------//
// ------------- CONEXIONES E HILOS -----------------------------------------//
// --------------------------------------------------------------------------//
void init_conexiones(){
    
    if((socket_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA)) <= 0){
        log_error(logger_cpu, "[INIT CONEXIONES]: No se ha podido crear la conexion con memoria");
        exit(EXIT_FAILURE);
    }    
    socket_servidor_dispatch = iniciar_servidor(PUERTO_ESCUCHA_DISPATCH);
    socket_servidor_interrupt = iniciar_servidor(PUERTO_ESCUCHA_INTERRUPT);
}

/*MEMORIA*/
void gestionar_conexion_memoria(){
    log_protegido_cpu(string_from_format("[GESTION CON MEMORIA]: ---- SOY CLIENTE ----"));
    _handshake_cliente_memoria(socket_memoria, "MEMORIA");
    log_protegido_cpu(string_from_format("[GESTION CON MEMORIA]: ---- MEMORIA CONECTADO ----"));

    pthread_create(&hilo_gestionar_memoria, NULL, (void*) atender_peticiones_memoria, NULL);
	pthread_detach(hilo_gestionar_memoria);
}
/*CPU_INTERRUPT*/
void gestionar_conexion_interrupt(){
    log_protegido_cpu(string_from_format("[GESTION CON INT]: ---- ESPERANDO CLIENTE ----"));
    socket_cliente_interrupt = esperar_cliente(socket_servidor_interrupt);
    handshake_server(socket_cliente_interrupt);
    log_protegido_cpu(string_from_format("[GESTION CON INT]: ---- INTERRUPT CONECTADO ----"));
    log_warning(logger_cpu,"falta implementar interrupt");
}
/*CPU_DISPATCH*/
void gestionar_conexion_dispatch(){
    log_protegido_cpu(string_from_format("[GESTION CON DIS]: ---- ESPERANDO CLIENTE ----"));
    socket_cliente_dispatch = esperar_cliente(socket_servidor_dispatch);
    handshake_server(socket_cliente_dispatch);
    log_protegido_cpu(string_from_format("[GESTION CON DIS]: ---- DISPATCH CONECTADO ----"));

    pthread_create(&hilo_gestionar_dispatch, NULL, (void*) atender_peticiones_dispatch, NULL);
	pthread_join(hilo_gestionar_dispatch,NULL);//JOIN
}


// --------------------------------------------------------------------------//
// ------------- CONEXIONES E HILOS ---FIN-----------------------------------//
// --------------------------------------------------------------------------//




// --------------------------------------------------------------------------//
// ------------- FUNCIONES DE LOGICA POR MODULO------------------------------//
// --------------------------------------------------------------------------//
void atender_peticiones_dispatch(){
    log_protegido_cpu(string_from_format("[ATENDER DISPATCH]: ---- ESPERANDO OPERACION ----"));
    while(1){
        int cod_op = recibir_operacion(socket_cliente_dispatch);
        switch(cod_op){
            case PCB:
                log_protegido_cpu(string_from_format("[ATENDER DISPATCH]: ---- PCB A EJECUTAR ----"));
                int size=0;
                void *buffer = recibir_buffer(&size, socket_cliente_dispatch);
                _recibir_nuevo_contexto(buffer);
                flag_ejecucion = true;             
                _ejecutar_proceso(); 
        }
    }
}

void atender_peticiones_memoria(){
    log_protegido_cpu(string_from_format("[ATENDER MEMORIA]: ---- ESPERANDO OPERACION ----"));
    while(1){
        int cod_op = recibir_operacion(socket_memoria);
        switch(cod_op){
            case FETCH_INSTRUCCION_RESPUESTA:

                if (instruccion_actual != NULL) {
                    free(instruccion_actual);
                    instruccion_actual = NULL;
                }

                log_protegido_cpu(string_from_format("[ATENDER MEMORIA]: ---- DEVOLUCION FETCH_INSTRUCCION ----"));
                int size, tam_inst, desplazamiento = 0;
                void *buffer = recibir_buffer(&size, socket_memoria);
                memcpy(&tam_inst,buffer +desplazamiento, sizeof(int));;
                desplazamiento+=sizeof(int);
                //copio con memcpy a instruccion_actual
                instruccion_actual = malloc(tam_inst+1); 
                memcpy(instruccion_actual, buffer + desplazamiento, tam_inst);
                //instruccion_actual = recibir_instruccion(socket_memoria);
                sem_post(&s_instruccion_actual);
                break;
        }
    }
}

// --------------------------------------------------------------------------//
// ------------- FUNCIONES DE LOGICA POR MODULO ---- FIN --------------------//
// --------------------------------------------------------------------------//




// --------------------------------------------------------------------------//
// ------------- FUNCIONES AUXILIARES----------------------------------------//
// --------------------------------------------------------------------------//
static void _handshake_cliente_memoria(int socket, char* nombre_destino){
    int resultado_hs = handshake_cliente(HANDSHAKE_CPU,socket);
    switch(resultado_hs){
        case HANDSHAKE_OK:
            log_protegido_cpu(string_from_format("[GESTION CONEXIONES]: Handshake con %s: OK\n", nombre_destino));
            break;
        default:
            log_error(logger_cpu, "ERROR EN HANDSHAKE: Operacion N* %d desconocida\n", resultado_hs);
            exit(EXIT_FAILURE);
            break;
    }    
}

// static int _get_retardo(){
//     // log_protegido_mem(string_from_format("pedido de retardo"));
//     return atoi(config_get_string_value(config_memoria,"RETARDO_RESPUESTA"));
// }

/*DISPATCH*/
static void _recibir_nuevo_contexto(void* buffer){
    log_protegido_cpu(string_from_format("[DISPATCH]: Recibiendo contexto\n"));
    contexto_cpu= malloc(sizeof(t_contexto));
    int desplazamiento = 0;
    memcpy(&(contexto_cpu->pid), buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(&(contexto_cpu->program_counter), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&(contexto_cpu->registros_cpu.AX), buffer + desplazamiento, sizeof(uint8_t));
    desplazamiento += sizeof(uint8_t);
    memcpy(&(contexto_cpu->registros_cpu.BX), buffer + desplazamiento, sizeof(uint8_t));
    desplazamiento += sizeof(uint8_t);
    memcpy(&(contexto_cpu->registros_cpu.CX), buffer + desplazamiento, sizeof(uint8_t));
    desplazamiento += sizeof(uint8_t);
    memcpy(&(contexto_cpu->registros_cpu.DX), buffer + desplazamiento, sizeof(uint8_t));
    desplazamiento += sizeof(uint8_t);
    memcpy(&(contexto_cpu->registros_cpu.EAX), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&(contexto_cpu->registros_cpu.EBX), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&(contexto_cpu->registros_cpu.ECX), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&(contexto_cpu->registros_cpu.EDX), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&(contexto_cpu->registros_cpu.PC), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&(contexto_cpu->registros_cpu.SI), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&(contexto_cpu->registros_cpu.DI), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    free(buffer);
}

static void _ejecutar_proceso(){
    log_protegido_cpu(string_from_format("[DISPATCH]: EJECUTANDO PROCESO PID <%d>",contexto_cpu->pid));
    while (flag_ejecucion){
        fetch_instruccion();
        _check_interrupt(execute_instruccion(decodificar_instruccion()));
    }
}

static void _check_interrupt(t_instruccion* instruccion){
    if(flag_interrupt && flag_ejecucion){
        log_warning(logger_cpu, "hay interrupcion, modificar el devolver contexto");
        devolver_contexto_a_dispatch(motivo_interrupt, instruccion); //se envia el contetxto al kernel dispatch
        flag_ejecucion = false;
    }
    flag_interrupt = false;
    for (int i = 0; i < list_size(instruccion->parametros); i++){
        char* parametro = list_get(instruccion->parametros, i);
        free(parametro);
    }
    list_destroy(instruccion->parametros);
    free(instruccion);
    return;
}