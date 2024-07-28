#include "gestion_conexiones.h"

static void atender_peticiones_kernel(void *void_args);
static void atender_peticiones_cpu(void *void_args);
static void atender_peticiones_stdin(void *void_args);
static void atender_peticiones_stdout(void *void_args);
static void atender_peticiones_dialfs(void *void_args);
static void identificar_conexion_y_derivar(int socket_cliente, int cod_op);

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
    int *argumentos;
    switch(cod_op){
        case HANDSHAKE_KERNEL: {
            //log_protegido_mem(string_from_format("[HANDSHAKE KERNEL]: 3) ---- HANDSHAKE KERNEL RECIBIDO ----"));
			argumentos = malloc(sizeof(int));
			*argumentos = socket_cliente;
            socket_cliente_kernel = socket_cliente;
            pthread_create(&hilo_gestionar_kernel, NULL, (void*) atender_peticiones_kernel, argumentos);
            pthread_detach(hilo_gestionar_kernel);
            break;
        }
        case HANDSHAKE_CPU:{
            //log_protegido_mem(string_from_format("[HANDSHAKE KERNEL]: 3) ---- HANDSHAKE CPU RECIBIDO ----"));
            
            //Envio tamaño de pagina
            send(socket_cliente, &TAM_PAGINA, sizeof(int), 0);

			argumentos = malloc(sizeof(int));
			*argumentos = socket_cliente;
            socket_cliente_cpu = socket_cliente;

            //PATRICIO - agrego el envio de tamaño pagina antes de atender

            pthread_create(&hilo_gestionar_cpu, NULL, (void*) atender_peticiones_cpu, argumentos);
            pthread_detach(hilo_gestionar_cpu);
            break;
        }
        case HANDSHAKE_IO_GEN:
            //log_protegido_mem(string_from_format("[HANDSHAKE KERNEL]: 3) ---- HANDSHAKE CPU RECIBIDO ----"));
            break;
        case HANDSHAKE_IO_STDIN:        
            log_info(logger_memoria,"HANDSHAKE_IO_STDIN");
			recibir_operacion(socket_cliente);
            int size = 0;
            void *buffer = recibir_buffer(&size, socket_cliente);
            free(buffer);

            argumentos = malloc(sizeof(int));
			*argumentos = socket_cliente;            
            pthread_t hilo_gestionar_stdin;
            pthread_create(&hilo_gestionar_stdin, NULL, (void*) atender_peticiones_stdin, argumentos);
            pthread_detach(hilo_gestionar_stdin);

            break;
        case HANDSHAKE_IO_STDOUT:     
            log_info(logger_memoria,"HANDSHAKE_IO_STDOUT");
			recibir_operacion(socket_cliente);
            int size2 = 0;
            void *buffer2 = recibir_buffer(&size2, socket_cliente);
            free(buffer2);

            argumentos = malloc(sizeof(int));
			*argumentos = socket_cliente;            
            pthread_t hilo_gestionar_stdout;
            pthread_create(&hilo_gestionar_stdout, NULL, (void*) atender_peticiones_stdout, argumentos);
            pthread_detach(hilo_gestionar_stdout);

            break;

        case HANDSHAKE_IO_DIALFS: 
            log_info(logger_memoria,"HANDSHAKE_IO_DIALFS");
			recibir_operacion(socket_cliente);
            int size_dislfs = 0;
            void *buffer_dialfs = recibir_buffer(&size_dislfs, socket_cliente);
            free(buffer_dialfs);

            argumentos = malloc(sizeof(int));
			*argumentos = socket_cliente;            
            pthread_t hilo_gestionar_dialfs;
            pthread_create(&hilo_gestionar_dialfs, NULL, (void*) atender_peticiones_dialfs, argumentos);
            pthread_detach(hilo_gestionar_dialfs);
            break;
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
static void atender_peticiones_stdout(void *void_args){
    int* socket = (int*) void_args;
    while(1){
        int code_op = recibir_operacion(*socket);
        usleep(RETARDO_RESPUESTA*1000);
        if (code_op == IO_STDOUT_WRITE){
            t_solicitud_io* solicitud = recibir_solicitud_io(*socket);
            char* mensaje_respuesta_temp = NULL;
            int total_size = 0;
            for(int i=0; i<solicitud->cantidad_accesos; i++){
                log_info(logger_memoria,"cantidad de accesos: %d",solicitud->cantidad_accesos);
                void* dato_leido = mem_leer_dato_direccion_fisica(solicitud->datos_memoria[i].direccion_fisica, solicitud->datos_memoria[i].tamano, solicitud->pid);
                if(dato_leido == NULL){
                    free(mensaje_respuesta_temp);
                    exit(EXIT_FAILURE);
                }

                char* nuevo_resultado = realloc(mensaje_respuesta_temp, total_size + solicitud->datos_memoria[i].tamano +1); //+1 para el '\0'
                if (nuevo_resultado == NULL) {
                    // Manejo de error si realloc falla
                    free(dato_leido);
                    free(mensaje_respuesta_temp);
                    exit(EXIT_FAILURE);
                }
                mensaje_respuesta_temp = nuevo_resultado;
                memcpy(mensaje_respuesta_temp + total_size, dato_leido, solicitud->datos_memoria[i].tamano);
                total_size += solicitud->datos_memoria[i].tamano;
                mensaje_respuesta_temp[total_size] = '\0'; 
                free(dato_leido);                
            }

            t_paquete* paquete = crear_paquete(IO_STDOUT_WRITE);
            agregar_a_paquete(paquete, mensaje_respuesta_temp, strlen(mensaje_respuesta_temp)+1);
            enviar_paquete(paquete, *socket);
            eliminar_paquete(paquete);
            free(mensaje_respuesta_temp);
            liberar_solicitud_io(solicitud);

        }
        else if (code_op == -1){
            log_info(logger_memoria,"El IO_STDOUT_WRITE se desconecto");
            break;
        }
        else {
            log_error(logger_memoria,"atender_peticiones_stdout: Operacion desconocida");
            exit(EXIT_FAILURE);
        }
    }
}

static void atender_peticiones_stdin(void *void_args){
    int* socket = (int*) void_args;
    while(1){
        int code_op = recibir_operacion(*socket);
        usleep(RETARDO_RESPUESTA*1000);
        if (code_op == IO_STDIN_READ){
            t_solicitud_io* solicitud = recibir_solicitud_io(*socket);

            for(int i=0; i<solicitud->cantidad_accesos; i++){
                // log_info(logger_memoria,"escribir en direccion fisica <%d>",solicitud->datos_memoria[i].direccion_fisica);
                // log_info(logger_memoria,"los datos <%s>",solicitud->datos_memoria[i].datos);
                mem_escribir_dato_direccion_fisica(
                    solicitud->datos_memoria[i].direccion_fisica,   //direccion
                    solicitud->datos_memoria[i].datos,              //dato
                    // strlen(solicitud->datos_memoria[i].datos)+1,    //tamaño
                    solicitud->datos_memoria[i].tamano,
                    solicitud->pid);                                //pid

                // mem_leer_dato_direccion_fisica(solicitud->datos_memoria[i].direccion_fisica, strlen(solicitud->datos_memoria[i].datos)+1);
            }
            //liberar memoria de t_solicitud_io
            liberar_solicitud_io(solicitud);

            //confirmo a entradasalida
            int mensajeOK =1;
            t_paquete* paquete = crear_paquete(IO_STDIN_READ);
            agregar_datos_sin_tamaño_a_paquete(paquete,&mensajeOK,sizeof(int));
            enviar_paquete(paquete, *socket);
            eliminar_paquete(paquete);      
            //desconecto      
        }
        else if (code_op == -1){
            close(*socket);
            break;
        }
        else {
            log_error(logger_memoria,"atender_peticiones_stdin: Operacion desconocida");
            exit(EXIT_FAILURE);
        }
    }

}

static void atender_peticiones_dialfs(void *void_args){
    int* socket = (int*) void_args;
    while(1){
        int code_op = recibir_operacion(*socket);
        usleep(RETARDO_RESPUESTA*1000);
        if (code_op == IO_FS_CREATE){
            t_solicitud_io* solicitud = recibir_solicitud_io(*socket);

            //TODO: comento por el momento
            // for(int i=0; i<solicitud->cantidad_accesos; i++){
            //     // log_info(logger_memoria,"escribir en direccion fisica <%d>",solicitud->datos_memoria[i].direccion_fisica);
            //     // log_info(logger_memoria,"los datos <%s>",solicitud->datos_memoria[i].datos);
            //     mem_escribir_dato_direccion_fisica(
            //         solicitud->datos_memoria[i].direccion_fisica,   //direccion
            //         solicitud->datos_memoria[i].datos,              //dato
            //         // strlen(solicitud->datos_memoria[i].datos)+1,    //tamaño
            //         solicitud->datos_memoria[i].tamano,
            //         solicitud->pid);                                //pid

            //     // mem_leer_dato_direccion_fisica(solicitud->datos_memoria[i].direccion_fisica, strlen(solicitud->datos_memoria[i].datos)+1);
            // }
            //liberar memoria de t_solicitud_io
            liberar_solicitud_io(solicitud);

            //confirmo a entradasalida
            int mensajeOK =1;
            t_paquete* paquete = crear_paquete(IO_FS_CREATE);
            agregar_datos_sin_tamaño_a_paquete(paquete,&mensajeOK,sizeof(int));
            enviar_paquete(paquete, *socket);
            eliminar_paquete(paquete);      
            //desconecto      
        }
        else if(code_op == IO_FS_DELETE){
            t_solicitud_io* solicitud = recibir_solicitud_io(*socket);
            //liberar memoria de t_solicitud_io
            liberar_solicitud_io(solicitud);

            //confirmo a entradasalida
            int mensajeOK =1;
            t_paquete* paquete = crear_paquete(IO_FS_CREATE);
            agregar_datos_sin_tamaño_a_paquete(paquete,&mensajeOK,sizeof(int));
            enviar_paquete(paquete, *socket);
            eliminar_paquete(paquete);   
        }
        else if (code_op == -1){
            close(*socket);
            break;
        }
        else {
            log_error(logger_memoria,"atender_peticiones_stdin: Operacion desconocida");
            exit(EXIT_FAILURE);
        }
    }

}

static void atender_peticiones_cpu(void *void_args){
	int* socket = (int*) void_args;
    int PC, size, pid, pagina = 0;
    int desplazamiento = 0;
    int direccion_fisica = 0;
    void *buffer;
    while(1){
        desplazamiento = 0; size=0; pid=0; 
        int code_op = recibir_operacion(*socket);
        usleep(RETARDO_RESPUESTA*1000);
        switch (code_op) {
            case PEDIDO_MARCO:
                buffer = recibir_buffer(&size, *socket);
                memcpy(&pid, buffer, sizeof(int));
                memcpy(&pagina, buffer + sizeof(int), sizeof(int));
                int nro_marco = buscar_marco_por_pagina(pagina, pid);
                t_paquete* paquete_respuesta_marco = crear_paquete(PEDIDO_MARCO);
                agregar_datos_sin_tamaño_a_paquete(paquete_respuesta_marco,&nro_marco,sizeof(int));
                enviar_paquete(paquete_respuesta_marco, *socket);
                eliminar_paquete(paquete_respuesta_marco);
                break;                
            case MENSAJE:
                recibir_mensaje(*socket,logger_memoria);
                break;
            case FETCH_INSTRUCCION:               
                buffer = recibir_buffer(&size, *socket);
                memcpy(&pid, buffer, sizeof(int));
                memcpy(&PC, buffer + sizeof(int), sizeof(int));
                char* instruccion = get_instruccion_proceso(get_proceso_memoria(pid),PC);
                t_paquete* paquete = crear_paquete(FETCH_INSTRUCCION_RESPUESTA);
                agregar_a_paquete(paquete, instruccion, strlen(instruccion)+1);
                enviar_paquete(paquete, *socket);
                eliminar_paquete(paquete);
                break;
            case RESIZE:
                int new_size;
                buffer = recibir_buffer(&size, *socket);
                memcpy(&pid, buffer, sizeof(int));
                memcpy(&new_size, buffer + sizeof(int), sizeof(int));
                int respuesta_a_resize = resize_proceso(pid, new_size);
                t_paquete* paquete_respuesta_resize = crear_paquete(RESIZE);
                agregar_datos_sin_tamaño_a_paquete(paquete_respuesta_resize, &respuesta_a_resize, sizeof(int));
                enviar_paquete(paquete_respuesta_resize, *socket);
                eliminar_paquete(paquete_respuesta_resize);
                break;
            case LEER_MEMORIA:
                // log_info(logger_memoria, "[MEMORIA] - atender_peticiones_cpu - LEER_MEMORIA");
                int cantidad_bytes = 0;
                buffer = recibir_buffer(&size, *socket);
                memcpy(&pid, buffer, sizeof(int));
                desplazamiento += sizeof(int);
                memcpy(&direccion_fisica, buffer + desplazamiento, sizeof(int));
                desplazamiento += sizeof(int);
                memcpy(&cantidad_bytes, buffer + desplazamiento, sizeof(int));
                void* dato_leido_mem = mem_leer_dato_direccion_fisica(direccion_fisica, cantidad_bytes, pid);
                t_paquete* paquete_respuesta_leer_memoria = crear_paquete(LEER_MEMORIA);
                agregar_a_paquete(paquete_respuesta_leer_memoria, dato_leido_mem, cantidad_bytes);
                enviar_paquete(paquete_respuesta_leer_memoria, *socket);
                eliminar_paquete(paquete_respuesta_leer_memoria);
                break;
            case ESCRIBIR_MEMORIA:
                // log_info(logger_memoria, "[MEMORIA] - atender_peticiones_cpu - ESCRIBIR_MEMORIA");
                buffer = recibir_buffer(&size, *socket);
                // Leer PID
                memcpy(&pid, buffer, sizeof(int));   
                desplazamiento += sizeof(int); 
                // Leer dirección física
                memcpy(&direccion_fisica, buffer + desplazamiento, sizeof(int)); 
                desplazamiento += sizeof(int);
                // Leer cantidad de bytes a escribir
                memcpy(&cantidad_bytes, buffer + desplazamiento, sizeof(int));
                desplazamiento += sizeof(int);                
                // Preparar memoria para los datos a escribir
                void* dato_escrito = malloc(cantidad_bytes);
                // Copiar los datos a escribir en la memoria asignada
                memcpy(dato_escrito, buffer + desplazamiento, cantidad_bytes);

                // Realizar la escritura en memoria
                mem_escribir_dato_direccion_fisica(direccion_fisica, dato_escrito, cantidad_bytes, pid);
                free(dato_escrito);
                // Enviar respuesta de éxito
                t_paquete* paquete_respuesta_escribir_memoria = crear_paquete(ESCRIBIR_MEMORIA);
                int valor_ok = 1;
                agregar_a_paquete(paquete_respuesta_escribir_memoria, &valor_ok, sizeof(int));
                enviar_paquete(paquete_respuesta_escribir_memoria, *socket);
                eliminar_paquete(paquete_respuesta_escribir_memoria);
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
        int code_op = recibir_operacion(*socket);
        usleep(RETARDO_RESPUESTA*1000);
        switch (code_op) {
            case MENSAJE:
                recibir_mensaje(*socket,logger_memoria);
                break;       
            case INICIAR_PROCESO_MEMORIA:
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
                list_add(lista_procesos_en_memoria, proceso);
                confirmar_proceso_creado(); 
                free(path);
                break;
            case LIBERAR_ESTRUCTURAS_MEMORIA:
                pid=0; desplazamiento = 0; size = 0;
                buffer = recibir_buffer(&size, *socket);
                memcpy(&pid, buffer, sizeof(int));
                finalizar_proceso(pid);
                confirmar_memoria_liberada();
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



// --------------------------------------------------------------------------//
// ------------- FUNCIONES AUXILIARES POR MODULO -----------------------------//
// --------------------------------------------------------------------------//

// --------------------------------------------------------------------------//
// ------------- FUNCIONES AUXILIARES POR MODULO ---- FIN --------------------//
// --------------------------------------------------------------------------//
