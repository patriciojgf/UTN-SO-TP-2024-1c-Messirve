#include "gestion_conexiones.h"

static void _handshake_cliente(int socket, char* nombre_destino);
// static void _atender_peticiones_memoria();
static void _atender_peticiones_kernel();
static void _identifico_nombre(int socket);
static void _lectura_consola(int size_lectura, char* buffer);
// --------------------------------------------------------------------------//


// --------------------------------------------------------------------------//
// ------------- CONEXIONES E HILOS -----------------------------------------//
// --------------------------------------------------------------------------//
void init_conexiones(){
    
    if(strcmp(TIPO_INTERFAZ, "GENERICA") != 0){
        if((socket_cliente_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA)) <= 0){
            log_error(logger_io, "[INIT CONEXIONES]: No se ha podido crear la conexion con memoria");
            exit(EXIT_FAILURE);
        }
    }
    if((socket_cliente_kernel = crear_conexion(IP_KERNEL, PUERTO_KERNEL)) <= 0){
        log_error(logger_io, "[INIT CONEXIONES]: No se ha podido crear la conexion con kernel");
        exit(EXIT_FAILURE);
    }
}

/*MEMORIA*/
void gestionar_conexion_memoria(){
    //log_protegido_io(string_from_format("[GESTION CON MEMORIA]: ---- SOY CLIENTE ----"));
    _handshake_cliente(socket_cliente_memoria, "MEMORIA");
    //log_protegido_io(string_from_format("[GESTION CON MEMORIA]: ---- MEMORIA CONECTADO ----"));
    _identifico_nombre(socket_cliente_memoria);
    // pthread_create(&hilo_gestionar_memoria, NULL, (void*) _atender_peticiones_memoria, NULL);
	// pthread_detach(hilo_gestionar_memoria);
}
/*KERNEL*/
void gestionar_conexion_kernel(){
    //log_protegido_io(string_from_format("[GESTION CON KERNEL]: ---- SOY CLIENTE ----"));
    _handshake_cliente(socket_cliente_kernel, "KERNEL");
    //log_protegido_io(string_from_format("[GESTION CON KERNEL]: ---- KERNEL CONECTADO ----"));
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

// static void _atender_peticiones_memoria(){
//     while(1){
//         int cod_op = recibir_operacion(socket_cliente_memoria);
//         switch(cod_op){
//             case MENSAJE:
//                 recibir_mensaje(socket_cliente_memoria, logger_io);
//                 break;
//             case IO_STDIN_READ:           
//                 sem_post(&sem_io_stdin_read_ok);
//                 break;
//             case -1:
//                 log_error(logger_io,"El MEMORIA se desconecto");
//                 // break;
//                 exit(EXIT_FAILURE);
//             default:
//                 log_warning(logger_io, "Memoria: Operacion desconocida.");
//                 // break;
//                 exit(EXIT_FAILURE);
//         }
//     }
// }

static void _atender_peticiones_kernel(){
    while(1){
        t_solicitud_io* solicitud_recibida_kernel;
        int mensajeOK =1;
        t_paquete* paquete_para_kernel;
        int cod_op = recibir_operacion(socket_cliente_kernel);
        switch(cod_op){
            case MENSAJE:
                recibir_mensaje(socket_cliente_kernel, logger_io);
                break;
            case IO_STDOUT_WRITE:
                log_info(logger_io,"IO_STDOUT_WRITE: Recibiendo solicitud de escritura de kernel");
                // Recibir la solicitud que contiene las direcciones de memoria y el tamaño a leer
                solicitud_recibida_kernel = recibir_solicitud_io(socket_cliente_kernel);
                log_info(logger_io,"IO_STDOUT_WRITE: solicitud_recibida_kernel");
                if (solicitud_recibida_kernel == NULL) {
                    fprintf(stderr, "Error al recibir la solicitud IO\n");
                    continue;
                }
                enviar_solicitud_io(socket_cliente_memoria, solicitud_recibida_kernel,IO_STDOUT_WRITE);
                log_info(logger_io,"IO_STDOUT_WRITE: enviar_solicitud_io");

                int cod_mensaje = recibir_operacion(socket_cliente_memoria);
                if(cod_mensaje == -1){
                    log_error(logger_io,"falla en conexion con memoria");
                    liberar_solicitud_io(solicitud_recibida_kernel);
                    continue;
                }
                int size_mensaje =0;
                void* buffer_mensaje = recibir_buffer(&size_mensaje, socket_cliente_memoria);
                
                char* mensaje_recibido_de_memoria;
                int size_mensaje_recibido_de_memoria;
                int desplazamiento = 0;
                memcpy(&(size_mensaje_recibido_de_memoria), buffer_mensaje + desplazamiento, sizeof(int));
                desplazamiento += sizeof(int);
                mensaje_recibido_de_memoria=malloc(size_mensaje_recibido_de_memoria);
                memcpy(mensaje_recibido_de_memoria, buffer_mensaje + desplazamiento, size_mensaje_recibido_de_memoria);

                log_info(logger_io,"Texto leido de memoria: <%s>",mensaje_recibido_de_memoria);
                free(buffer_mensaje);
                free(mensaje_recibido_de_memoria); 

                paquete_para_kernel = crear_paquete(IO_STDOUT_WRITE);
                agregar_datos_sin_tamaño_a_paquete(paquete_para_kernel,&mensajeOK,sizeof(int));
                enviar_paquete(paquete_para_kernel, socket_cliente_kernel);
                eliminar_paquete(paquete_para_kernel);                                    

                break;

            case IO_STDIN_READ:
                log_info(logger_io,"IO_STDIN_READ: Recibiendo solicitud de lectura de kernel");
                // Recibir la solicitud que contiene las direcciones de memoria y el tamaño a leer
                solicitud_recibida_kernel = recibir_solicitud_io(socket_cliente_kernel);
                if (solicitud_recibida_kernel == NULL) {
                    fprintf(stderr, "Error al recibir la solicitud IO\n");
                    continue;
                }        
                // Reservar memoria para la entrada y leer desde la consola
                char* input_text = malloc(solicitud_recibida_kernel->size_solicitud + 1);  // +1 para el carácter nulo
                _lectura_consola(solicitud_recibida_kernel->size_solicitud, input_text);
                // Llenar los datos de memoria en la solicitud con el texto leído
                llenar_datos_memoria(solicitud_recibida_kernel, input_text);
                // Enviar la solicitud completada hacia donde se necesite procesar
                enviar_solicitud_io(socket_cliente_memoria, solicitud_recibida_kernel,IO_STDIN_READ);
                // Limpiar
                free(input_text);
                log_info(logger_io,"IO_STDIN_READ: solicitud enviada a memoria");
                //espero handshake ok
                int cod_hand = recibir_operacion(socket_cliente_memoria);
                int size_temp =0;
                void* buffer_temp = recibir_buffer(&size_temp, socket_cliente_memoria);
                free(buffer_temp);
                log_info(logger_io,"IO_STDIN_READ: handshake recibido de memoria");
                //envio el ok a kernel
                if (cod_hand == IO_STDIN_READ){                    
                    paquete_para_kernel = crear_paquete(IO_STDIN_READ);
                    agregar_datos_sin_tamaño_a_paquete(paquete_para_kernel,&mensajeOK,sizeof(int));
                    enviar_paquete(paquete_para_kernel, socket_cliente_kernel);
                    eliminar_paquete(paquete_para_kernel);     
                    log_info(logger_io,"IO_STDIN_READ: handshake enviado a kernel");              
                }
                else{
                    //liberar memoria de t_solicitud_io
                    liberar_solicitud_io(solicitud_recibida_kernel);
                    log_error(logger_io,"falla en conexion con memoria");
                }
                break;
            case IO_GEN_SLEEP:
                int size=0;
                void *buffer = recibir_buffer(&size, socket_cliente_kernel);           
                int tiempo_sleep;
                memcpy(&tiempo_sleep, buffer, sizeof(int));
                usleep(tiempo_sleep*TIEMPO_UNIDAD_TRABAJO*1000);
                int handshake = IO_GEN_SLEEP;
                send(socket_cliente_kernel, &handshake, sizeof(handshake), 0);
                break;
            case -1:
                log_error(logger_io,"El KERNEL se desconecto");
                exit(EXIT_FAILURE);
            default:
                log_warning(logger_io, "Kernel: Operacion desconocida.");
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

static void _lectura_consola(int size_lectura, char* buffer) {
    // char prompt[50];  // Asegúrate de que el buffer del prompt sea suficientemente grande
    // snprintf(prompt, sizeof(prompt), "Ingrese un texto de hasta %d caracteres y presione Enter: ", size_lectura);

    char* input = readline("> ");;
    if (input) {
        strncpy(buffer, input, size_lectura);
        buffer[size_lectura] = '\0';  // Asegurarse de que el buffer esté correctamente terminado
        free(input);  // readline aloja memoria con malloc, así que debemos liberarla

        // Opcional: agregar a historial si la entrada no es vacía
        if (buffer[0] != '\0') {
            add_history(buffer);
        }
    }
}

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
            //log_protegido_io(string_from_format("[GESTION CONEXIONES]: Handshake con %s: OK\n", nombre_destino));
            break;
        default:
            log_error(logger_io, "ERROR EN HANDSHAKE: Operacion N* %d desconocida\n", resultado_hs);
            exit(EXIT_FAILURE);
            break;
    }    
}