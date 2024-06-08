#include "gestion_conexiones.h"

static void _handshake_cliente_memoria(int socket, char* nombre_destino);
// static void _recibir_nuevo_contexto(void* buffer);
static void _recibir_nuevo_contexto(int socket);
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
    ////log_info(logger_cpu,"[GESTION CON MEMORIA]: ---- SOY CLIENTE ----");
    _handshake_cliente_memoria(socket_memoria, "MEMORIA");
    ////log_info(logger_cpu,"[GESTION CON MEMORIA]: ---- MEMORIA CONECTADO ----");

    pthread_create(&hilo_gestionar_memoria, NULL, (void*) atender_peticiones_memoria, NULL);
	pthread_detach(hilo_gestionar_memoria);
}
/*CPU_INTERRUPT*/
void gestionar_conexion_interrupt(){
    ////log_info(logger_cpu,"[GESTION CON INT]: ---- ESPERANDO CLIENTE ----");
    socket_cliente_interrupt = esperar_cliente(socket_servidor_interrupt);
    handshake_server(socket_cliente_interrupt);
    ////log_info(logger_cpu,"[GESTION CON INT]: ---- INTERRUPT CONECTADO ----");

    pthread_create(&hilo_gestionar_interrupt, NULL, (void*) atender_peticiones_interrupt, NULL);
	pthread_detach(hilo_gestionar_interrupt);//JOIN
}
/*CPU_DISPATCH*/
void gestionar_conexion_dispatch(){
    ////log_info(logger_cpu,"[GESTION CON DIS]: ---- ESPERANDO CLIENTE ----");
    socket_cliente_dispatch = esperar_cliente(socket_servidor_dispatch);
    handshake_server(socket_cliente_dispatch);
    ////log_info(logger_cpu,"[GESTION CON DIS]: ---- DISPATCH CONECTADO ----");

    pthread_create(&hilo_gestionar_dispatch, NULL, (void*) atender_peticiones_dispatch, NULL);
	pthread_join(hilo_gestionar_dispatch,NULL);//JOIN
}


// --------------------------------------------------------------------------//
// ------------- CONEXIONES E HILOS ---FIN-----------------------------------//
// --------------------------------------------------------------------------//




// --------------------------------------------------------------------------//
// ------------- FUNCIONES DE LOGICA POR MODULO------------------------------//
// --------------------------------------------------------------------------//
void atender_peticiones_interrupt(){
    ////log_info(logger_cpu,"[ATENDER INTERRUPT]: ---- ESPERANDO OPERACION ----");
    while(1){
        int cod_op = recibir_operacion(socket_cliente_interrupt);
        switch(cod_op){
            case FIN_QUANTUM:
		        pthread_mutex_lock(&mutex_ejecucion_proceso);
                log_info(logger_cpu,"[ATENDER INTERRUPT]: -- FIN_QUANTUM -- PID <%d> - PC<%d>",contexto_cpu->pid,contexto_cpu->program_counter);
                log_warning(logger_cpu,"fin de quantum");
                ////log_info(logger_cpu,"[ATENDER INTERRUPT]: ---- QUANTUM ----");
                int size=0;
                void *buffer = recibir_buffer(&size, socket_cliente_interrupt);
                motivo_interrupt=FIN_QUANTUM;
                memcpy(&(pid_interrupt), buffer , sizeof(int));    
                ////log_info(logger_cpu,"[ATENDER INTERRUPT]: ---- pid_interrupt: %d", pid_interrupt));
                ////log_info(logger_cpu,"[ATENDER INTERRUPT]: ---- contexto_cpu->pid: %d", contexto_cpu->pid));
                if(contexto_cpu->pid==pid_interrupt){
                    log_warning(logger_cpu,"flag_interrupt=true");
                    flag_interrupt=true;
                }
                else{
                    log_warning(logger_cpu,"flag_interrupt=false");
                    log_warning(logger_cpu,"pid: %d",pid_interrupt);
                    log_warning(logger_cpu,"pid_interrupt: %d",contexto_cpu->pid);

                }
                free(buffer);
		        pthread_mutex_unlock(&mutex_ejecucion_proceso);
                break;
            default:
                log_error(logger_cpu, "ERROR EN cod_op: Operacion N* %d desconocida\n", cod_op);
                exit(EXIT_FAILURE);
                break;
        }
    }
}

void atender_peticiones_dispatch(){
    //log_info(logger_cpu,"[ATENDER DISPATCH]: ---- ESPERANDO OPERACION ----");
    while(1){
        int cod_op = recibir_operacion(socket_cliente_dispatch);
        switch(cod_op){
            case PCB:
                //log_info(logger_cpu,"[ATENDER DISPATCH]: ---- PCB A EJECUTAR ----");
                _recibir_nuevo_contexto(socket_cliente_dispatch);
                log_info(logger_cpu,"[ATENDER DISPATCH]: -- PCB -- PID <%d> - PC<%d>",contexto_cpu->pid,contexto_cpu->program_counter);
                flag_ejecucion = true;             
                ejecutar_proceso(); 
                break;
            default:
                log_error(logger_cpu,"atender_peticiones_dispatch cod_op no reconocidos");
                exit(EXIT_FAILURE);
        }
    }
}

void atender_peticiones_memoria(){    
    while(1){
        //log_info(logger_cpu,"[ATENDER MEMORIA]: ---- ESPERANDO OPERACION ----");
        int cod_op = recibir_operacion(socket_memoria);
        //log_info(logger_cpu,"[ATENDER MEMORIA]: ---- COD OP ----");
        switch(cod_op){
            case FETCH_INSTRUCCION_RESPUESTA:
                log_info(logger_cpu,"[ATENDER MEMORIA]: -- FETCH_INSTRUCCION_RESPUESTA -- PID <%d> - PC<%d>",contexto_cpu->pid,contexto_cpu->program_counter);
                if (instruccion_actual != NULL) {
                    free(instruccion_actual);
                    instruccion_actual = NULL;
                }
                int size, tam_inst, desplazamiento = 0;
                void *buffer = recibir_buffer(&size, socket_memoria);
                memcpy(&tam_inst,buffer +desplazamiento, sizeof(int));;
                desplazamiento+=sizeof(int);
                instruccion_actual = malloc(tam_inst+1); 
                memcpy(instruccion_actual, buffer + desplazamiento, tam_inst);
                sem_post(&s_instruccion_actual);
                free(buffer);
                break;
            default:
                log_error(logger_cpu, "[atender_peticiones_memoria]: cod_op no identificado <%d>",cod_op);
                exit(EXIT_FAILURE);
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
            ////log_info(logger_cpu,"[GESTION CONEXIONES]: Handshake con %s: OK\n", nombre_destino));
            break;
        default:
            log_error(logger_cpu, "ERROR EN HANDSHAKE: Operacion N* %d desconocida\n", resultado_hs);
            exit(EXIT_FAILURE);
            break;
    }    
}

// static int _get_retardo(){
//     // log_protegido_mem(string_from_format("pedido de retardo");
//     return atoi(config_get_string_value(config_memoria,"RETARDO_RESPUESTA");
// }

/*DISPATCH*/
static void _recibir_nuevo_contexto(int socket){    
    //log_info(logger_cpu,"[DISPATCH]: Recibiendo contexto\n");
    int size=0;
    void *buffer = recibir_buffer(&size, socket_cliente_dispatch); 
    //contexto_cpu= malloc(sizeof(t_contexto));
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
