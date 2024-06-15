#include "gestion_conexiones.h"

static void identificar_conexion_y_derivar(int socket_cliente, int cod_op);
static void atender_peticiones_kernel(void *void_args);
static void atender_peticiones_cpu(void *void_args);
static void _confirmar_memoria_liberada();
static void _enviar_tamanio_a_cpu(int socket_cliente_cpu);
static int _get_marco(int pid);

// --------------------------------------------------------------------------//
// ------------- CONEXIONES E HILOS -----------------------------------------//
// --------------------------------------------------------------------------//
void init_conexiones(){
    socket_servidor_escucha = iniciar_servidor(PUERTO_ESCUCHA);
}

void gestionar_conexiones_entrantes(){
    while(1){
        //log_protegido_mem(string_from_format("[GESTION CONEXION ENTRANTE]: 1) ---- ESPERANDO CLIENTE ----"));
        int socket_cliente_temp = esperar_cliente(socket_servidor_escucha);
        int cod_handshake = handshake_server(socket_cliente_temp);
        //log_protegido_mem(string_from_format("[GESTION CONEXION ENTRANTE]: 2) ---- HANDSHAKE RECIBIDO ----"));
        identificar_conexion_y_derivar(socket_cliente_temp, cod_handshake);
    }
}

static void identificar_conexion_y_derivar(int socket_cliente, int cod_op){
    switch(cod_op){
        case HANDSHAKE_KERNEL: {
            //log_protegido_mem(string_from_format("[HANDSHAKE KERNEL]: 3) ---- HANDSHAKE KERNEL RECIBIDO ----"));
			int *argumentos = malloc(sizeof(int));
			*argumentos = socket_cliente;
            socket_cliente_kernel = socket_cliente;
            pthread_create(&hilo_gestionar_kernel, NULL, (void*) atender_peticiones_kernel, argumentos);
            pthread_detach(hilo_gestionar_kernel);
            break;
        }
        case HANDSHAKE_CPU:{
            //log_protegido_mem(string_from_format("[HANDSHAKE KERNEL]: 3) ---- HANDSHAKE CPU RECIBIDO ----"));

			int *argumentos = malloc(sizeof(int));
			*argumentos = socket_cliente;
            socket_cliente_cpu = socket_cliente;

            _enviar_tamanio_a_cpu(socket_cliente_cpu);
   
            pthread_create(&hilo_gestionar_cpu, NULL, (void*) atender_peticiones_cpu, argumentos);
            pthread_detach(hilo_gestionar_cpu);
            break;
        }
        case HANDSHAKE_IO_GEN:
            //log_protegido_mem(string_from_format("[HANDSHAKE KERNEL]: 3) ---- HANDSHAKE CPU RECIBIDO ----"));
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
        //log_protegido_mem(string_from_format("[ATENDER CPU]: Esperando operación."));
        int code_op = recibir_operacion(*socket);
        usleep(RETARDO_RESPUESTA * 1000);

        //log_protegido_mem(string_from_format("[ATENDER CPU]: Operación recibida."));
        switch (code_op) {
            case MENSAJE:
                //log_protegido_mem(string_from_format("[ATENDER CPU]: MENSAJE"));
                recibir_mensaje(*socket,logger_memoria);
                break;
            case FETCH_INSTRUCCION:
                //log_protegido_mem(string_from_format("[ATENDER CPU]: FETCH_INSTRUCCION - PID: %d, PC: %d",pid,PC));                
                buffer = recibir_buffer(&size, *socket);
                memcpy(&pid, buffer, sizeof(int));
                memcpy(&PC, buffer + sizeof(int), sizeof(int));
                // usleep(RETARDO_RESPUESTA * 1000);
                //atender_fetch_instruccion(pid,PC);
                //t_proceso* proceso = get_proceso_memoria(pid);
                char* instruccion = get_instruccion_proceso(get_proceso_memoria(pid),PC);
                t_paquete* paquete = crear_paquete(FETCH_INSTRUCCION_RESPUESTA);
                agregar_a_paquete(paquete, instruccion, strlen(instruccion)+1);
                enviar_paquete(paquete, *socket);
                //log_protegido_mem(string_from_format("[ATENDER CPU]: INST ENVIADA - PID: %d, PC: %d",pid,PC));  
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
    int pid, size_path, desplazamiento = 0;
    while(1){
        //log_protegido_mem(string_from_format("[ATENDER KERNEL]: Esperando operación."));
        int code_op = recibir_operacion(*socket);
        log_info(logger_memoria, "[MEMORIA]: RETARDO_RESPUESTA: %d", RETARDO_RESPUESTA);
        usleep(RETARDO_RESPUESTA*1000);
        //log_protegido_mem(string_from_format("[ATENDER KERNEL]: Operación recibida."));
        switch (code_op) {
            case MENSAJE:
                //log_protegido_mem(string_from_format("[ATENDER KERNEL]: MENSAJE"));
                recibir_mensaje(*socket,logger_memoria);
                break;       
            case INICIAR_PROCESO_MEMORIA:
                //log_protegido_mem(string_from_format("[ATENDER KERNEL]: INICIAR_PROCESO_MEMORIA"));
                pid=0; size_path=0; desplazamiento = 0; size = 0;
                buffer = recibir_buffer(&size, *socket);
                char* path;
                memcpy(&pid, buffer +desplazamiento, sizeof(int));
                desplazamiento+=sizeof(int);
                memcpy(&size_path,buffer +desplazamiento, sizeof(int));
                desplazamiento+=sizeof(int);
                path=malloc(size_path);
                memcpy(path, buffer +desplazamiento, size_path);
                t_proceso* proceso = crear_proceso(pid,path);
                iniciar_tabla_de_pagina(proceso);
                list_add(lista_procesos_en_memoria, proceso);
                confirmar_proceso_creado(); 
                break;
             case LIBERAR_ESTRUCTURAS_MEMORIA:
                log_protegido_mem(string_from_format("[ATENDER KERNEL]: LIBERAR_ESTRUCTURAS_MEMORIA"));
                pid=0; desplazamiento = 0; size = 0;
                buffer = recibir_buffer(&size, *socket);
                memcpy(&pid, buffer, sizeof(int));
                log_warning(logger_memoria, "falta implementar liberar estructuras memoria");
                _confirmar_memoria_liberada();
                break;
            case OBTENER_MARCO:
                int _size, pid, _desplazamiento = 0;
                void *_buffer = recibir_buffer(&_size, *socket);
                memcpy(&pid, _buffer +_desplazamiento, sizeof(int));;
                log_info(logger_memoria, "[ATENDER MEMORIA]pid: %d\n", pid);
                log_info(logger_memoria, "PID: %d", pid);
                _get_marco(pid);
                _confirmar_memoria_liberada();
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

// --------------------------------------------------------------------------//
// ------------- FUNCIONES AUXILIARES POR MODULO -----------------------------//
// --------------------------------------------------------------------------//

static void _enviar_tamanio_a_cpu(int socket_cliente_cpu)
{
    t_paquete* paquete_a_enviar = crear_paquete(TAMANIO_PAGINA);
    agregar_datos_sin_tamaño_a_paquete(paquete_a_enviar,&(TAM_PAGINA),sizeof(int));
    enviar_paquete(paquete_a_enviar, socket_cliente_cpu);
    eliminar_paquete(paquete_a_enviar);
}

static void _confirmar_memoria_liberada(){
    log_warning(logger_memoria, "VER SI ES NECESARIO UN RETARDO");
    t_paquete* paquete = crear_paquete(LIBERAR_ESTRUCTURAS_MEMORIA_OK);    
    char* mensajeOK = "OK";

    agregar_a_paquete(paquete, mensajeOK, strlen(mensajeOK)+1);
    enviar_paquete(paquete, socket_cliente_kernel);
    eliminar_paquete(paquete);
}

static int _get_marco(int pid)
{
    t_proceso* proceso = get_proceso_memoria(pid);
    t_tabla_pagina* tdp = list_get(proceso->tabla_de_paginas, pid);

    if(tdp->presencia)
    {
        return tdp->marco;
    }

    return -1;
}

// --------------------------------------------------------------------------//
// ------------- FUNCIONES AUXILIARES POR MODULO ---- FIN --------------------//
// --------------------------------------------------------------------------//