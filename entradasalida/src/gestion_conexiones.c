#include "gestion_conexiones.h"

static void _handshake_cliente(int socket, char* nombre_destino);
static void _atender_peticiones_memoria();
static void _atender_peticiones_kernel();
static void _identifico_nombre(int socket);
// --------------------------------------------------------------------------//


// --------------------------------------------------------------------------//
// ------------- CONEXIONES E HILOS -----------------------------------------//
// --------------------------------------------------------------------------//
void init_conexiones(){    
    if((socket_cliente_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA)) <= 0){
        log_error(logger_io, "[INIT CONEXIONES]: No se ha podido crear la conexion con memoria");
        exit(EXIT_FAILURE);
    }  
    if((socket_cliente_kernel = crear_conexion(IP_KERNEL, PUERTO_KERNEL)) <= 0){
        log_error(logger_io, "[INIT CONEXIONES]: No se ha podido crear la conexion con kernel");
        exit(EXIT_FAILURE);
    }
}

/*MEMORIA*/
void gestionar_conexion_memoria(){
    log_protegido_io(string_from_format("[GESTION CON MEMORIA]: ---- SOY CLIENTE ----"));
    _handshake_cliente(socket_cliente_memoria, "MEMORIA");
    log_protegido_io(string_from_format("[GESTION CON MEMORIA]: ---- MEMORIA CONECTADO ----"));
    _identifico_nombre(socket_cliente_memoria);
    pthread_create(&hilo_gestionar_memoria, NULL, (void*) _atender_peticiones_memoria, NULL);
	pthread_detach(hilo_gestionar_memoria);
}
/*KERNEL*/
void gestionar_conexion_kernel(){
    log_protegido_io(string_from_format("[GESTION CON KERNEL]: ---- SOY CLIENTE ----"));
    _handshake_cliente(socket_cliente_kernel, "KERNEL");
    log_protegido_io(string_from_format("[GESTION CON KERNEL]: ---- KERNEL CONECTADO ----"));
    _identifico_nombre(socket_cliente_kernel);
    pthread_create(&hilo_gestionar_kernel, NULL, (void*) _atender_peticiones_kernel, NULL);
	pthread_join(hilo_gestionar_kernel, NULL);
}

// --------------------------------------------------------------------------//
// ------------- CONEXIONES E HILOS ---FIN-----------------------------------//
// --------------------------------------------------------------------------//


// --------------------------------------------------------------------------//
// --------------------------------------------------------------------------//
// --------------------------------------------------------------------------//
// --------------------------------------------------------------------------//
// --------------------------------------------------------------------------//


// --------------------------------------------------------------------------//
// ------------- FUNCIONES DE LOGICA POR MODULO------------------------------//
// --------------------------------------------------------------------------//

static void _atender_peticiones_memoria(){
    log_warning(logger_io,"FALTA IMPLEMENTAR ateneder peticiones memoria");
    log_protegido_io(string_from_format("[ATENDER MEMORIA]: ---- ESPERANDO OPERACION ----"));
    while(1){
        int cod_op = recibir_operacion(socket_cliente_memoria);
        switch(cod_op){
            case MENSAJE:
                recibir_mensaje(socket_cliente_memoria, logger_io);
                break;
            case -1:
                log_error(logger_io,"El MEMORIA se desconecto");
                // break;
                exit(EXIT_FAILURE);
            default:
                log_warning(logger_io, "Memoria: Operacion desconocida.");
                // break;
                exit(EXIT_FAILURE);
        }
    }
}

static void _atender_peticiones_kernel(){
    log_protegido_io(string_from_format("[ATENDER KERNEL]: ---- ESPERANDO OPERACION ----"));
    while(1){
        int cod_op = recibir_operacion(socket_cliente_kernel);
        switch(cod_op){
            case MENSAJE:
                recibir_mensaje(socket_cliente_kernel, logger_io);
                break;
            case IO_GEN_SLEEP:
                log_protegido_io(string_from_format("[ATENDER KERNEL]: ---- OPERACION SLEEP ----"));
                int size=0;
                void *buffer = recibir_buffer(&size, socket_cliente_kernel);           
                int tiempo_sleep;
                memcpy(&tiempo_sleep, buffer, sizeof(int));
                log_protegido_io(string_from_format("[ATENDER KERNEL]: ---- OPERACION SLEEP: tiempo %d ----",tiempo_sleep));
                usleep(tiempo_sleep*1000);
                //le doy el ok a kernel
                log_protegido_io(string_from_format("[ATENDER KERNEL]: ---- VOY A ENVIAR EL CODIGO: %d ----",IO_GEN_SLEEP));
                t_paquete* paquete_pedido = crear_paquete(IO_GEN_SLEEP);
                enviar_paquete(paquete_pedido, socket_cliente_kernel);

                log_protegido_io(string_from_format("[ATENDER KERNEL]: ---- OPERACION SLEEP: fin ----",tiempo_sleep));

                break;
            case -1:
                log_error(logger_io,"El KERNEL se desconecto");
                // break;
                exit(EXIT_FAILURE);
            default:
                log_warning(logger_io, "Kernel: Operacion desconocida.");
                // break;
                exit(EXIT_FAILURE);
        }
    }
}


// --------------------------------------------------------------------------//
// ------------- FUNCIONES DE LOGICA POR MODULO ---- FIN --------------------//
// --------------------------------------------------------------------------//


// --------------------------------------------------------------------------//
// --------------------------------------------------------------------------//
// --------------------------------------------------------------------------//
// --------------------------------------------------------------------------//
// --------------------------------------------------------------------------//


// --------------------------------------------------------------------------//
// ------------- FUNCIONES AUXILIARES----------------------------------------//
// --------------------------------------------------------------------------//
static void _identifico_nombre(int socket){
    t_paquete* paquete = crear_paquete(NUEVA_IO);
    agregar_a_paquete(paquete, nombre_interfaz, strlen(nombre_interfaz)+1);
	enviar_paquete(paquete, socket);
	eliminar_paquete(paquete);
}


static int _get_handshake() {
    if (strcmp(TIPO_INTERFAZ, "GENERICA") == 0) {
        return HANDSHAKE_IO_GEN;
    } else if (strcmp(TIPO_INTERFAZ, "STDIN") == 0) {
        return HANDSHAKE_IO_STDIN;
    } else if (strcmp(TIPO_INTERFAZ, "STDOUT") == 0) {
        return HANDSHAKE_IO_STDOUT;
    } else if (strcmp(TIPO_INTERFAZ, "DIALFS") == 0) {
        return HANDSHAKE_IO_DIALFS;
    } else {
        log_error(logger_io, "TIPO DE INTERFAZ NO RECONOCIDO: %s", TIPO_INTERFAZ);
        exit(EXIT_FAILURE);
    }
}

static void _handshake_cliente(int socket, char* nombre_destino){
    int resultado_hs = handshake_cliente(_get_handshake(),socket);
    switch(resultado_hs){
        case HANDSHAKE_OK:
            log_protegido_io(string_from_format("[GESTION CONEXIONES]: Handshake con %s: OK\n", nombre_destino));
            break;
        default:
            log_error(logger_io, "ERROR EN HANDSHAKE: Operacion N* %d desconocida\n", resultado_hs);
            exit(EXIT_FAILURE);
            break;
    }    
}