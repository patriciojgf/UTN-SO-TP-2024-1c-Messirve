#include "gestion_conexiones.h"

static void identificar_conexion_y_derivar(int socket_cliente, int cod_op);
static void atender_peticiones_kernel(void *void_args);
static void atender_peticiones_cpu(void *void_args);

// --------------------------------------------------------------------------//
// ------------- CONEXIONES E HILOS -----------------------------------------//
// --------------------------------------------------------------------------//
void init_conexiones(){
    socket_servidor_escucha = iniciar_servidor(PUERTO_ESCUCHA);
}

void gestionar_conexiones_entrantes(){
    while(1){
        log_protegido_mem(string_from_format("[GESTION CONEXION ENTRANTE]: 1) ---- ESPERANDO CLIENTE ----"));
        int socket_cliente_temp = esperar_cliente(socket_servidor_escucha);
        int cod_handshake = handshake_server(socket_cliente_temp);
        log_protegido_mem(string_from_format("[GESTION CONEXION ENTRANTE]: 2) ---- HANDSHAKE RECIBIDO ----"));
        identificar_conexion_y_derivar(socket_cliente_temp, cod_handshake);
    }
}

static void identificar_conexion_y_derivar(int socket_cliente, int cod_op){
    switch(cod_op){
        case HANDSHAKE_KERNEL: {
            log_protegido_mem(string_from_format("[HANDSHAKE KERNEL]: 3) ---- HANDSHAKE KERNEL RECIBIDO ----"));
			int *argumentos = malloc(sizeof(int));
			*argumentos = socket_cliente;
            socket_cliente_kernel = socket_cliente;
            pthread_create(&hilo_gestionar_kernel, NULL, (void*) atender_peticiones_kernel, argumentos);
            pthread_detach(hilo_gestionar_kernel);
            break;
        }
        case HANDSHAKE_CPU:{
            log_protegido_mem(string_from_format("[HANDSHAKE KERNEL]: 3) ---- HANDSHAKE CPU RECIBIDO ----"));

			int *argumentos = malloc(sizeof(int));
			*argumentos = socket_cliente;
            socket_cliente_cpu = socket_cliente;            
            pthread_create(&hilo_gestionar_cpu, NULL, (void*) atender_peticiones_cpu, argumentos);
            pthread_detach(hilo_gestionar_cpu);
            break;
        }
        case HANDSHAKE_IO_GEN:
            log_protegido_mem(string_from_format("[HANDSHAKE KERNEL]: 3) ---- HANDSHAKE CPU RECIBIDO ----"));
            break;
        case HANDSHAKE_IO_STDIN:
        case HANDSHAKE_IO_STDOUT:
        case HANDSHAKE_IO_DIALFS: 
        default:
            log_error(logger_memoria,"identificar_conexion_y_derivar: HANDSHAKE NO IDENTIFICADO, valor: %d", cod_op);
            exit(EXIT_FAILURE);
    }
}
// --------------------------------------------------------------------------//
// ------------- CONEXIONES E HILOS ---FIN-----------------------------------//
// --------------------------------------------------------------------------//



// --------------------------------------------------------------------------//
// ------------- FUNCIONES DE LOGICA POR MODULO------------------------------//
// --------------------------------------------------------------------------//
static void atender_peticiones_cpu(void *void_args){
	int* socket = (int*) void_args;
    int PC, size, pid = 0;
    void *buffer;
    while(1){
        log_protegido_mem(string_from_format("[ATENDER CPU]: Esperando operación."));
        int code_op = recibir_operacion(*socket);
        log_protegido_mem(string_from_format("[ATENDER CPU]: Operación recibida."));
        switch (code_op) {
            case MENSAJE:
                log_protegido_mem(string_from_format("[ATENDER CPU]: MENSAJE"));
                recibir_mensaje(*socket,logger_memoria);
                break;
            case FETCH_INSTRUCCION:
                log_protegido_mem(string_from_format("[ATENDER CPU]: FETCH_INSTRUCCION - PID: %d, PC: %d",pid,PC));                
                buffer = recibir_buffer(&size, *socket);
                memcpy(&pid, buffer, sizeof(int));
                memcpy(&PC, buffer + sizeof(int), sizeof(int));
                usleep(RETARDO_RESPUESTA * 1000);
                //atender_fetch_instruccion(pid,PC);
                //t_proceso* proceso = get_proceso_memoria(pid);
                char* instruccion = get_instruccion_proceso(get_proceso_memoria(pid),PC);
                t_paquete* paquete = crear_paquete(FETCH_INSTRUCCION_RESPUESTA);
                agregar_a_paquete(paquete, instruccion, strlen(instruccion)+1);
                enviar_paquete(paquete, *socket);
                log_protegido_mem(string_from_format("[ATENDER CPU]: INST ENVIADA - PID: %d, PC: %d",pid,PC));  
                eliminar_paquete(paquete);
                break;
            default:
                log_error(logger_memoria,"codigo desconocido %d",code_op);
                exit(EXIT_FAILURE);
        }
    }
}

static void atender_peticiones_kernel(void *void_args){
	int* socket = (int*) void_args;
    int size;
    void *buffer;
    while(1){
        log_protegido_mem(string_from_format("[ATENDER KERNEL]: Esperando operación."));
        int code_op = recibir_operacion(*socket);
        log_protegido_mem(string_from_format("[ATENDER KERNEL]: Operación recibida."));
        switch (code_op) {
            case MENSAJE:
                log_protegido_mem(string_from_format("[ATENDER KERNEL]: MENSAJE"));
                recibir_mensaje(*socket,logger_memoria);
                break;       
            case INICIAR_PROCESO_MEMORIA:
                log_protegido_mem(string_from_format("[ATENDER KERNEL]: INICIAR_PROCESO_MEMORIA"));
                buffer = recibir_buffer(&size, *socket); 
                int pid, size_path, desplazamiento = 0;
                char* path;
                memcpy(&pid, buffer +desplazamiento, sizeof(int));
                desplazamiento+=sizeof(int);
                memcpy(&size_path,buffer +desplazamiento, sizeof(int));
                desplazamiento+=sizeof(int);
                path=malloc(size_path);
                memcpy(path, buffer +desplazamiento, size_path);
                t_proceso* proceso = crear_proceso(pid,path);
                list_add(lista_procesos_en_memoria, proceso);
                confirmar_proceso_creado(); 
                break;    
            case -1:
                log_error(logger_memoria,"El KERNEL se desconecto");
                exit(EXIT_FAILURE);
            default:
                log_warning(logger_memoria, "Operacion desconocida:");
                log_warning(logger_memoria, "%d",code_op);
                exit(EXIT_FAILURE);
            }
            free(buffer);
    }
}

// --------------------------------------------------------------------------//
// ------------- FUNCIONES DE LOGICA POR MODULO ---- FIN --------------------//
// --------------------------------------------------------------------------//