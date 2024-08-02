#include "gestion_conexiones.h"

static void _handshake_cliente(int socket, char* nombre_destino);
// static void _atender_peticiones_memoria();
static void _atender_peticiones_kernel();
static void _identifico_nombre(int socket);
static void _lectura_consola(int size_lectura, char* buffer);

static char* fs_write_envio_pedido_memoria(t_solicitud_fs_rw* solicitud_recibida);
static void recibir_solicitud_y_nombre_archivo(t_solicitud_fs_rw** solicitud_recibida, char** nombre_archivo);

static void log_hexdump(t_log* logger, const char* tag, const void* data, size_t size) {
    char* hexdump = malloc(size * 2 + 1); // Cada byte se representa con 2 caracteres hexadecimales y un carácter nulo al final.
    if (hexdump == NULL) {
        log_error(logger, "Error allocating memory for hexdump");
        return;
    }

    const unsigned char* bytes = (const unsigned char*)data;
    for (size_t i = 0; i < size; i++) {
        sprintf(hexdump + i * 2, "%02x", bytes[i]);
    }

    hexdump[size * 2] = '\0'; // Null-terminate the string.
    log_info(logger, "%s <%s>", tag, hexdump);
    free(hexdump);
}
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
        t_solicitud_fs_rw* solicitud_recibida_fs_rw;
        int mensajeOK =1;
        int size=0;
        t_paquete* paquete_para_kernel;
        int pid =0;
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
                //log obligatorio
                //Todos - Operación: “PID: <PID> - Operacion: <OPERACION_A_REALIZAR>”
                log_info(logger_io,"PID: <%d> - Operacion <STDOUT_WRITE>", solicitud_recibida_kernel->pid);
                log_info(logger_io,"Texto leido de memoria: <%s>",mensaje_recibido_de_memoria);
                log_hexdump(logger_io, "resultado_memoria", mensaje_recibido_de_memoria, size_mensaje_recibido_de_memoria);
                free(buffer_mensaje);
                free(mensaje_recibido_de_memoria); 

                paquete_para_kernel = crear_paquete(IO_STDOUT_WRITE);
                agregar_datos_sin_tamaño_a_paquete(paquete_para_kernel,&mensajeOK,sizeof(int));
                enviar_paquete(paquete_para_kernel, socket_cliente_kernel);
                eliminar_paquete(paquete_para_kernel);                                    
                liberar_solicitud_io(solicitud_recibida_kernel);                
                break;

            case IO_STDIN_READ:
                // Recibir la solicitud que contiene las direcciones de memoria y el tamaño a leer
                solicitud_recibida_kernel = recibir_solicitud_io(socket_cliente_kernel);
                if (solicitud_recibida_kernel == NULL) {
                    fprintf(stderr, "Error al recibir la solicitud IO\n");
                    continue;
                }        
                // Reservar memoria para la entrada y leer desde la consola
                char* input_text = malloc(solicitud_recibida_kernel->size_solicitud + 1);  // +1 para el carácter nulo
                //log obligatorio
                //Todos - Operación: “PID: <PID> - Operacion: <OPERACION_A_REALIZAR>”
                log_info(logger_io,"PID: <%d> - Operacion <STDIN_READ>", solicitud_recibida_kernel->pid);
                _lectura_consola(solicitud_recibida_kernel->size_solicitud, input_text);
                // Llenar los datos de memoria en la solicitud con el texto leído
                llenar_datos_memoria(solicitud_recibida_kernel, input_text);
                // Enviar la solicitud completada hacia donde se necesite procesar
                enviar_solicitud_io(socket_cliente_memoria, solicitud_recibida_kernel,IO_STDIN_READ);
                // Limpiar
                free(input_text);
                liberar_solicitud_io(solicitud_recibida_kernel);
                //espero handshake ok
                int cod_hand = recibir_operacion(socket_cliente_memoria);
                int size_temp =0;
                void* buffer_temp = recibir_buffer(&size_temp, socket_cliente_memoria);
                free(buffer_temp);
                //envio el ok a kernel
                if (cod_hand == IO_STDIN_READ){                    
                    paquete_para_kernel = crear_paquete(IO_STDIN_READ);
                    agregar_datos_sin_tamaño_a_paquete(paquete_para_kernel,&mensajeOK,sizeof(int));
                    enviar_paquete(paquete_para_kernel, socket_cliente_kernel);
                    eliminar_paquete(paquete_para_kernel);     
                    log_info(logger_io,"IO_STDIN_READ: OK enviado a kernel");              
                }
                else{
                    log_error(logger_io,"falla en conexion con memoria");
                }
                break;
            case IO_FS_WRITE:
                log_info(logger_io, "Iniciando [IO_FS_WRITE]");
                usleep(TIEMPO_UNIDAD_TRABAJO*1000);
                char* nombre_archivo_fs_write = NULL;
                recibir_solicitud_y_nombre_archivo(&solicitud_recibida_fs_rw, &nombre_archivo_fs_write);
                log_info(logger_io,"nombre_archivo_fs_write <%s>",nombre_archivo_fs_write);
                char* resultado_memoria = fs_write_envio_pedido_memoria(solicitud_recibida_fs_rw);
                //el tamaño del mensaje esta en solicitud_recibida_fs_rw->size_solicitud
                log_info(logger_io, "resultado_memoria <%s>", resultado_memoria);               
                log_hexdump(logger_io, "resultado_memoria", resultado_memoria, solicitud_recibida_fs_rw->size_solicitud);

                log_info(logger_io, "PID: <%d> - Operacion: <IO_FS_WRITE>", solicitud_recibida_fs_rw->pid);
                // void* datos = malloc(size); 
                if(escribir_archivo(nombre_archivo_fs_write, solicitud_recibida_fs_rw->puntero_archivo, solicitud_recibida_fs_rw->size_solicitud , resultado_memoria) == -1){
                    log_error(logger_io, "PID: <%d> - Escribir Archivo: <%s> FALLO", pid, nombre_archivo_fs_write);
                    free(nombre_archivo_fs_write);
                    free(resultado_memoria);
                    exit(EXIT_FAILURE);
                }
                free(resultado_memoria);
                liberar_solicitud_fs_rw(solicitud_recibida_fs_rw);
                free(nombre_archivo_fs_write);
                paquete_para_kernel = crear_paquete(IO_FS_WRITE);
                agregar_datos_sin_tamaño_a_paquete(paquete_para_kernel,&mensajeOK,sizeof(int));
                enviar_paquete(paquete_para_kernel, socket_cliente_kernel);
                eliminar_paquete(paquete_para_kernel); 
                break;  
            case IO_FS_READ:
                log_info(logger_io, "Iniciando [IO_FS_READ]");
                log_info(logger_io, "PID: <%d> - Operacion: <IO_FS_READ>", pid);
                usleep(TIEMPO_UNIDAD_TRABAJO*1000);

                char* nombre_archivo_fs_read = NULL;
                recibir_solicitud_y_nombre_archivo(&solicitud_recibida_fs_rw, &nombre_archivo_fs_read);

                char* lo_que_leimos_de_disco = malloc(solicitud_recibida_fs_rw->size_solicitud);
                if(leer_archivo(nombre_archivo_fs_read, solicitud_recibida_fs_rw->puntero_archivo, solicitud_recibida_fs_rw->size_solicitud, lo_que_leimos_de_disco) == -1){
                    log_error(logger_io, "PID: <%d> - Leer Archivo: <%s> FALLO", pid, nombre_archivo_fs_read);
                    free(nombre_archivo_fs_read);
                    free(lo_que_leimos_de_disco);
                    exit(EXIT_FAILURE);
                }
                //tomo lo leido del disco y se lo mando a memoria en una solicitud
                t_solicitud_io* solicitud_para_memoria_fs_read = convertir_fs_a_io(solicitud_recibida_fs_rw);
                llenar_datos_memoria(solicitud_para_memoria_fs_read, lo_que_leimos_de_disco);
                enviar_solicitud_io(socket_cliente_memoria, solicitud_para_memoria_fs_read,IO_FS_READ);

                //espero handshake ok
                int cod_fs_read = recibir_operacion(socket_cliente_memoria);
                int size_temp_fs_read =0;
                void* buffer_temp_fs_read = recibir_buffer(&size_temp_fs_read, socket_cliente_memoria);
                free(buffer_temp_fs_read);
                liberar_solicitud_io(solicitud_para_memoria_fs_read);
                
                //envio el ok a kernel
                if (cod_fs_read == IO_FS_READ){                    
                    paquete_para_kernel = crear_paquete(IO_STDIN_READ);
                    agregar_datos_sin_tamaño_a_paquete(paquete_para_kernel,&mensajeOK,sizeof(int));
                    enviar_paquete(paquete_para_kernel, socket_cliente_kernel);
                    eliminar_paquete(paquete_para_kernel);
                    log_info(logger_io,"IO_FS_READ: ok enviado a kernel");              
                }
                else{
                    //liberar memoria de t_solicitud_io
                    log_error(logger_io,"IO_FS_READ: falla en conexion con memoria");
                }
                liberar_solicitud_fs_rw(solicitud_recibida_fs_rw);
                free(nombre_archivo_fs_read);
                free(lo_que_leimos_de_disco);

                break;                 
            case IO_GEN_SLEEP:
                size=0;
                void *buffer = recibir_buffer(&size, socket_cliente_kernel);           
                int tiempo_sleep=0;
                int pid_gen_sleep=-1;
                memcpy(&pid_gen_sleep, buffer, sizeof(int));
                memcpy(&tiempo_sleep, buffer + sizeof(int), sizeof(int));
                //PATRICIO AGREGO FREE
                free(buffer);
                //log obligatorio
                //Todos - Operación: “PID: <PID> - Operacion: <OPERACION_A_REALIZAR>”
                log_info(logger_io,"PID: <%d> - Operacion <SLEEP>", pid_gen_sleep);
                log_info(logger_io,"Tiempo: <%d>", tiempo_sleep);
                usleep(tiempo_sleep*TIEMPO_UNIDAD_TRABAJO*1000);
                int handshake = IO_GEN_SLEEP;
                send(socket_cliente_kernel, &handshake, sizeof(handshake), 0);
                break;
            case IO_FS_CREATE:
                log_info(logger_io, "Iniciando [IO_FS_CREATE]");
                usleep(TIEMPO_UNIDAD_TRABAJO*1000);
                size=0;
                pid=0;
                int largo_nombre_archivo_fs_create =0;

                void *buffer_fs_create = recibir_buffer(&size, socket_cliente_kernel);
                memcpy(&pid, buffer_fs_create, sizeof(int));
                memcpy(&largo_nombre_archivo_fs_create, buffer_fs_create + sizeof(int), sizeof(int));
                char* nombre_archivo_create = malloc(largo_nombre_archivo_fs_create);
                memcpy(nombre_archivo_create, buffer_fs_create + sizeof(int) + sizeof(int), largo_nombre_archivo_fs_create);
                //PATRICIO AGREGO FREE
                free(buffer_fs_create);
                //log obligatorio
                //“PID: <PID> - Operacion: <OPERACION_A_REALIZAR>”
                log_info(logger_io, "PID: <%d> - Operacion: <IO_FS_CREATE>",pid);
                log_info(logger_io, "Archivo <%s>",nombre_archivo_create);
                if(crear_archivo(nombre_archivo_create)==-1){
                    log_error(logger_io, "PID: <%d> - Crear Archivo: <%s> FALLO", pid, nombre_archivo_create);
                }
                log_info(logger_io, "PID: <%d> - Crear Archivo: <%s>", pid, nombre_archivo_create);
                
                free(nombre_archivo_create);

                paquete_para_kernel = crear_paquete(IO_FS_CREATE);
                agregar_datos_sin_tamaño_a_paquete(paquete_para_kernel,&mensajeOK,sizeof(int));
                enviar_paquete(paquete_para_kernel, socket_cliente_kernel);
                eliminar_paquete(paquete_para_kernel);

                log_info(logger_io,"IO_FS_CREATE: ok enviado a kernel");   
                break;
            case IO_FS_DELETE:
                log_info(logger_io, "Iniciando [IO_FS_DELETE]");
                usleep(TIEMPO_UNIDAD_TRABAJO*1000);
                size=0;
                pid=0;
                int largo_nombre_archivo_fs_delete = 0;
                
                void *buffer_fs_delete = recibir_buffer(&size, socket_cliente_kernel);
                memcpy(&pid, buffer_fs_delete, sizeof(int));
                memcpy(&largo_nombre_archivo_fs_delete, buffer_fs_delete + sizeof(int), sizeof(int));
                char* nombre_archivo_delete = malloc(largo_nombre_archivo_fs_delete);
                memcpy(nombre_archivo_delete, buffer_fs_delete + sizeof(int) + sizeof(int), largo_nombre_archivo_fs_delete);
                //PATRICIO AGREGO FREE
                free(buffer_fs_delete);
                //log obligatorio
                //“PID: <PID> - Operacion: <OPERACION_A_REALIZAR>”
                log_info(logger_io, "PID: <%d> - Operacion: <IO_FS_DELETE>",pid);
                log_info(logger_io, "Archivo <%s>", nombre_archivo_create);
                if(liberar_bloques_de_archivo(nombre_archivo_delete) == -1){
                    log_error(logger_io, "PID: <%d> - Eliminar Archivo: <%s> FALLO", pid, nombre_archivo_delete);
                    exit(EXIT_FAILURE);
                }
                log_info(logger_io, "PID: <%d> - Eliminar Archivo: <%s>", pid, nombre_archivo_delete);
                free(nombre_archivo_delete);

                paquete_para_kernel = crear_paquete(IO_FS_DELETE);
                agregar_datos_sin_tamaño_a_paquete(paquete_para_kernel,&mensajeOK,sizeof(int));
                enviar_paquete(paquete_para_kernel, socket_cliente_kernel);
                eliminar_paquete(paquete_para_kernel); 

                log_info(logger_io,"IO_FS_DELETE: ok enviado a kernel");   
                break;
            case IO_FS_TRUNCATE:
                log_info(logger_io, "Iniciando [IO_FS_TRUNCATE]");
                usleep(TIEMPO_UNIDAD_TRABAJO*1000);

                size = 0;
                pid = 0;
                int largo_nombre_archivo_fs_truncate = 0;
                int tamano_byte = 0;
                
                // log_info(logger_io, "Recibiendo buffer de truncado");
                void *buffer_fs_truncate = recibir_buffer(&size, socket_cliente_kernel);
                // log_info(logger_io, "Buffer recibido, tamaño: %d", size);

                // log_info(logger_io, "Extrayendo PID del buffer");
                memcpy(&pid, buffer_fs_truncate, sizeof(int));
                // log_info(logger_io, "PID extraído: %d", pid);

                // log_info(logger_io, "Extrayendo largo del nombre del archivo del buffer");
                memcpy(&largo_nombre_archivo_fs_truncate, buffer_fs_truncate + sizeof(int), sizeof(int));
                // log_info(logger_io, "Largo del nombre del archivo: %d", largo_nombre_archivo_fs_truncate);

                char* nombre_archivo_truncate = malloc(largo_nombre_archivo_fs_truncate);
                if (nombre_archivo_truncate == NULL) {
                    log_error(logger_io, "Error al asignar memoria para el nombre del archivo");
                    free(buffer_fs_truncate);
                    exit(EXIT_FAILURE);
                }

                // log_info(logger_io, "Extrayendo nombre del archivo del buffer");
                memcpy(nombre_archivo_truncate, buffer_fs_truncate + sizeof(int) + sizeof(int), largo_nombre_archivo_fs_truncate);
                // log_info(logger_io, "Nombre del archivo: %s", nombre_archivo_truncate);

                // log_info(logger_io, "Extrayendo tamaño en bytes del buffer");
                memcpy(&tamano_byte, buffer_fs_truncate + sizeof(int) + sizeof(int) + largo_nombre_archivo_fs_truncate, sizeof(int));
                // log_info(logger_io, "Tamaño en bytes para truncar: %d", tamano_byte);

                // Log obligatorio
                log_info(logger_io, "PID: <%d> - Operacion: <IO_FS_TRUNCATE>", pid);
                // log_info(logger_io, "Archivo <%s>", nombre_archivo_truncate);

                // log_info(logger_io, "Truncando archivo");
                log_info(logger_io, "------------------------------------------------");
                log_info(logger_io, "truncar_archivo - nombre <%s> - tamano_byte <%d>",nombre_archivo_truncate,tamano_byte);
                listar_archivos();
                truncar_archivo(nombre_archivo_truncate, tamano_byte);
                // log_info(logger_io, "Archivo truncado");

                free(nombre_archivo_truncate);
                free(buffer_fs_truncate);  // Liberar buffer_fs_truncate después de su uso

                paquete_para_kernel = crear_paquete(IO_FS_TRUNCATE);
                agregar_datos_sin_tamaño_a_paquete(paquete_para_kernel, &mensajeOK, sizeof(int));
                enviar_paquete(paquete_para_kernel, socket_cliente_kernel);
                eliminar_paquete(paquete_para_kernel);

                log_info(logger_io, "IO_FS_TRUNCATE: ok enviado a kernel");
                log_info(logger_io, "fin - truncar_archivo - nombre <%s> - tamano_byte <%d>",nombre_archivo_truncate,tamano_byte);
                listar_archivos();
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

static void recibir_solicitud_y_nombre_archivo(t_solicitud_fs_rw** solicitud_recibida, char** nombre_archivo) {
    int size = 0;
    int largo_nombre_archivo = 0;
    *solicitud_recibida = recibir_solicitud_io_fs_rw(socket_cliente_kernel);
    if (*solicitud_recibida == NULL) {
        log_error(logger_io, "Error al recibir la solicitud IO");
        exit(EXIT_FAILURE);
    }    
    log_info(logger_io, "Recibi solicitud");

    int cod_nombre = recibir_operacion(socket_cliente_kernel);
    if(cod_nombre != IO_FS_WRITE && cod_nombre != IO_FS_READ){
        log_error(logger_io, "Error al recibir el nombre del archivo <%d>",cod_nombre );
        exit(EXIT_FAILURE);
    }
    log_info(logger_io, "Recibi nombre");

    void *buffer = recibir_buffer(&size, socket_cliente_kernel);                     
    memcpy(&largo_nombre_archivo, buffer, sizeof(int));
    *nombre_archivo = malloc(largo_nombre_archivo);
    memcpy(*nombre_archivo, buffer + sizeof(int), largo_nombre_archivo);
    free(buffer);
    log_info(logger_io, "Recibi nombre");
}

static char* fs_write_envio_pedido_memoria(t_solicitud_fs_rw* solicitud_recibida) {
    t_solicitud_io* solicitud_para_memoria = convertir_fs_a_io(solicitud_recibida);
    enviar_solicitud_io(socket_cliente_memoria, solicitud_para_memoria, IO_FS_WRITE);
    liberar_solicitud_io(solicitud_para_memoria);

    int cod_mensaje = recibir_operacion(socket_cliente_memoria);
    if (cod_mensaje == -1) {
        log_error(logger_io, "falla en conexion con memoria");
        return NULL;
    }
    int size = 0;
    void* buffer = recibir_buffer(&size, socket_cliente_memoria);
    int desplazamiento = 0;
    int size_mensaje_recibido=0;
    memcpy(&size_mensaje_recibido, buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);
    char* mensaje_recibido_de_memoria = malloc(size_mensaje_recibido);
    memcpy(mensaje_recibido_de_memoria, buffer + desplazamiento, size_mensaje_recibido);
    free(buffer);
    return mensaje_recibido_de_memoria;
}