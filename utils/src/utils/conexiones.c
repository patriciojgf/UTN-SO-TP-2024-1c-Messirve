#include "conexiones.h"

/*------HANDSHAKE--------------*/
static void _send_handshake(int hs_origen, int socket){
    int handshake = hs_origen;
    send(socket, &handshake, sizeof(handshake), 0);
}

int handshake_cliente(int hs_origen, int socket){
	_send_handshake(hs_origen,socket);
	return recibir_operacion(socket);
}

int handshake_server(int socket) {
    int cod_op = recibir_operacion(socket);
    switch (cod_op) {
		case HANDSHAKE_CPU:
        case HANDSHAKE_KERNEL:
        case HANDSHAKE_MEMORIA:
        case HANDSHAKE_CPU_D:
        case HANDSHAKE_CPU_I:
        case HANDSHAKE_IO_GEN:
        case HANDSHAKE_IO_STDIN:
        case HANDSHAKE_IO_STDOUT:
        case HANDSHAKE_IO_DIALFS:
            {
				printf("Handshake recibido\n");
                int resp_ok = HANDSHAKE_OK;
                if (send(socket, &resp_ok, sizeof(resp_ok), 0) == -1) {
                    perror("Error sending handshake response");
                    return HANDSHAKE_ERROR;
                }
                return cod_op;
            }
        case -1:
            return HANDSHAKE_DESCONEXION;
        default:
            return HANDSHAKE_ERROR;
    }
}

/*-------------------------------------------CLIENTE----------------------------------------------------*/

void* serializar_paquete(t_paquete* paquete, int bytes){
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

int crear_conexion(char *ip, char* puerto){
	struct addrinfo hints, *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	// Ahora vamos a crear el socket.
	int socket_cliente = socket(server_info->ai_family,
            					server_info->ai_socktype,
								server_info->ai_protocol);
	

	// Ahora que tenemos el socket, vamos a conectarlo
	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
	{
		printf("Error en la conexión\n");
		socket_cliente = -1;
	}

	freeaddrinfo(server_info); 
	return socket_cliente;
}

/*-------------------------------------------------------Paquete-------------------------------------------------------*/

void enviar_mensaje(char* mensaje, int socket_cliente){
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2 * sizeof(int); // buff	
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

t_buffer *crear_buffer(){
	t_buffer *buffer = malloc(sizeof(t_buffer));
	assert(buffer != NULL);

	buffer->size = 0;
	buffer->stream = NULL;
	return buffer;
}

t_paquete *crear_paquete(int codigo_operacion){
	t_paquete *paquete = (t_paquete *)malloc(sizeof(t_paquete));

	assert(paquete != NULL);

	paquete->codigo_operacion = codigo_operacion;
	paquete->buffer = crear_buffer();
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio){
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void agregar_datos_sin_tamaño_a_paquete(t_paquete *paquete, void *valor, int bytes)
{
	t_buffer *buffer = paquete->buffer;

	// buffer->stream = realloc(buffer->stream, buffer->size + bytes);

	void* temp = realloc(buffer->stream, buffer->size + bytes);
	if (temp == NULL) {
		// Manejar el error de memoria aquí, por ejemplo, loguearlo y terminar la función.
		exit(EXIT_FAILURE);
	}
	buffer->stream = temp;


	memcpy(buffer->stream + buffer->size, valor, bytes);
	buffer->size += bytes;
}

void enviar_paquete(t_paquete* paquete, int socket_cliente){
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete){
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}


int recibir_operacion(int socket_cliente){
	int cod_op=0;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}


/*-------------------------------------------SERVIDOR----------------------------------------------------*/

int iniciar_servidor(char* puerto)
{
	// Quitar esta línea cuando hayamos terminado de implementar la funcion
//	assert(!"no implementado!");

	int socket_servidor;

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM; 
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);

	// Creamos el socket de escucha del servidor
	socket_servidor= socket(servinfo->ai_family,
							servinfo->ai_socktype,
							servinfo->ai_protocol);

	setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	// Asociamos el socket a un puerto
	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);
	// Escuchamos las conexiones entrantes
	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);

	return socket_servidor;
}


int esperar_cliente(int socket_servidor)
{
	// Aceptamos un nuevo cliente
	int socket_cliente = accept(socket_servidor, NULL, NULL);
	//log_info(logger, "Se conecto un cliente!");
	return socket_cliente;
}

char* leer_mensaje(int socket_cliente){
	int size=0;
	char* buffer = recibir_buffer(&size, socket_cliente);
	return buffer;
}

void recibir_mensaje(int socket_cliente, t_log* logger){
	int size=0;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s\n", buffer);
	free(buffer);
}

void* recibir_buffer(int* size, int socket_cliente){
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
} 

void liberar_conexion(int socket_cliente){
	close(socket_cliente);
}


void agregar_socket_a_lista(t_list* lista, t_interfaz* socket){
	list_add(lista, socket);
}

t_interfaz* buscar_socket_por_nombre(t_list* lista, char* nombre){
	t_interfaz* socket = list_find(lista, (void*)nombre);
	return socket;
}

void quitar_socket_por_nombre(t_list* lista, char* nombre){
	list_remove_by_condition(lista, (void*)nombre);
}

int texto_to_cod_op(char* texto){
	if(strcmp(texto, "MENSAJE") == 0)
		return MENSAJE;
	if(strcmp(texto, "PCB") == 0)
		return PCB;
	if(strcmp(texto, "DESALOJAR") == 0)
		return DESALOJAR;
	if(strcmp(texto, "KERNEL") == 0)
		return KERNEL;
	if(strcmp(texto, "KERNEL_INTERRUPT") == 0)
		return KERNEL_INTERRUPT;
	if(strcmp(texto, "KERNEL_DISPATCH") == 0)
		return KERNEL_DISPATCH;
	if(strcmp(texto, "CPU") == 0)
		return CPU;
	if(strcmp(texto, "IO") == 0)
		return IO;
	if(strcmp(texto, "GENERICA") == 0)
		return GENERICA;
	if(strcmp(texto, "STDIN") == 0)
		return STDIN;
	if(strcmp(texto, "STDOUT") == 0)
		return STDOUT;
	if(strcmp(texto, "DIALFS") == 0)
		return DIALFS;
	return -1;
}



/*Conexion con CPU + Paquetes - INICIO*/

void empaquetar_registros_cpu(t_paquete* paquete_contexto, t_registros_cpu registros_cpu){
	agregar_datos_sin_tamaño_a_paquete(paquete_contexto, &(registros_cpu.AX), sizeof(uint8_t));
	agregar_datos_sin_tamaño_a_paquete(paquete_contexto, &(registros_cpu.BX), sizeof(uint8_t));
	agregar_datos_sin_tamaño_a_paquete(paquete_contexto, &(registros_cpu.CX), sizeof(uint8_t));
	agregar_datos_sin_tamaño_a_paquete(paquete_contexto, &(registros_cpu.DX), sizeof(uint8_t));
	agregar_datos_sin_tamaño_a_paquete(paquete_contexto, &(registros_cpu.EAX), sizeof(uint32_t));
	agregar_datos_sin_tamaño_a_paquete(paquete_contexto, &(registros_cpu.EBX), sizeof(uint32_t));
	agregar_datos_sin_tamaño_a_paquete(paquete_contexto, &(registros_cpu.ECX), sizeof(uint32_t));
	agregar_datos_sin_tamaño_a_paquete(paquete_contexto, &(registros_cpu.EDX), sizeof(uint32_t));
	agregar_datos_sin_tamaño_a_paquete(paquete_contexto, &(registros_cpu.PC), sizeof(uint32_t));
	agregar_datos_sin_tamaño_a_paquete(paquete_contexto, &(registros_cpu.SI), sizeof(uint32_t));
	agregar_datos_sin_tamaño_a_paquete(paquete_contexto, &(registros_cpu.DI), sizeof(uint32_t));
}

void empaquetar_instruccion_cpu(t_paquete* paquete_contexto, t_instruccion* instruccion){
	agregar_datos_sin_tamaño_a_paquete(paquete_contexto, &(instruccion->identificador), sizeof(int));
	agregar_datos_sin_tamaño_a_paquete(paquete_contexto, &(instruccion->cantidad_parametros), sizeof(int));
	for(int i = 0; i < instruccion->cantidad_parametros; i++){
		char* parametro = list_get(instruccion->parametros, i);
		agregar_a_paquete(paquete_contexto, parametro, strlen(parametro) + 1);
	}
}

void empaquetar_contexto_cpu(t_paquete* paquete_contexto, t_instruccion* instruccion, int pid,t_registros_cpu registros_cpu, int motivo){
	agregar_datos_sin_tamaño_a_paquete(paquete_contexto, &(pid), sizeof(int));
	empaquetar_registros_cpu(paquete_contexto, registros_cpu);
	empaquetar_instruccion_cpu(paquete_contexto, instruccion);
	agregar_datos_sin_tamaño_a_paquete(paquete_contexto, &motivo, sizeof(int));
} 


void desempaquetar_contexto_cpu(t_paquete* paquete_contexto, t_instruccion* instruccion, int* pid, t_registros_cpu* registros_cpu){
	int desplazamiento = 0;
	memcpy(pid, paquete_contexto->buffer->stream + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	//Registros CPU
	memcpy(&(registros_cpu->AX), paquete_contexto->buffer->stream + desplazamiento, sizeof(uint8_t));
	desplazamiento += sizeof(uint8_t);
	memcpy(&(registros_cpu->BX), paquete_contexto->buffer->stream + desplazamiento, sizeof(uint8_t));
	desplazamiento += sizeof(uint8_t);
	memcpy(&(registros_cpu->CX), paquete_contexto->buffer->stream + desplazamiento, sizeof(uint8_t));
	desplazamiento += sizeof(uint8_t);
	memcpy(&(registros_cpu->DX), paquete_contexto->buffer->stream + desplazamiento, sizeof(uint8_t));
	desplazamiento += sizeof(uint8_t);
	memcpy(&(registros_cpu->EAX), paquete_contexto->buffer->stream + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(&(registros_cpu->EBX), paquete_contexto->buffer->stream + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(&(registros_cpu->ECX), paquete_contexto->buffer->stream + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(&(registros_cpu->PC), paquete_contexto->buffer->stream + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	memcpy(&(registros_cpu->SI), paquete_contexto->buffer->stream + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);
	
	//Instruccion
	memcpy(&(instruccion->identificador), paquete_contexto->buffer->stream + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(&(instruccion->cantidad_parametros), paquete_contexto->buffer->stream + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);
	for(int i = 0; i < instruccion->cantidad_parametros; i++){
		char* parametro;
		memcpy(&(parametro), paquete_contexto->buffer->stream + desplazamiento, sizeof(char*));
		desplazamiento += sizeof(char*);
		list_add(instruccion->parametros, parametro);
	}
	
}
/*Conexion con CPU + Paquetes - FIN*/

/*IO READ*/
t_paquete* empaquetar_solicitud_io(t_solicitud_io* solicitud, int motivo) {
    t_paquete* paquete = crear_paquete(motivo);
    agregar_datos_sin_tamaño_a_paquete(paquete, &(solicitud->pid), sizeof(int));
    agregar_datos_sin_tamaño_a_paquete(paquete, &(solicitud->size_solicitud), sizeof(uint32_t));
    agregar_datos_sin_tamaño_a_paquete(paquete, &(solicitud->cantidad_accesos), sizeof(uint32_t));

    for (int i = 0; i < solicitud->cantidad_accesos; i++) {
        agregar_datos_sin_tamaño_a_paquete(paquete, &(solicitud->datos_memoria[i].direccion_fisica), sizeof(uint32_t));

        agregar_datos_sin_tamaño_a_paquete(paquete, &(solicitud->datos_memoria[i].tamano), sizeof(uint32_t));
		
        agregar_a_paquete(paquete, solicitud->datos_memoria[i].datos, solicitud->datos_memoria[i].tamano);
		
    }
    return paquete;
}

void enviar_solicitud_io(int socket, t_solicitud_io* solicitud, int motivo) {
    t_paquete* paquete = empaquetar_solicitud_io(solicitud, motivo);
    enviar_paquete(paquete, socket);
    eliminar_paquete(paquete);
}

/* Función para recibir una solicitud de I/O */
t_solicitud_io* recibir_solicitud_io(int socket) {
    int size;
    void* buffer = recibir_buffer(&size, socket);
    int desplazamiento = 0;
    t_solicitud_io* solicitud = malloc(sizeof(t_solicitud_io));

    memcpy(&(solicitud->pid), buffer + desplazamiento, sizeof(int));
    desplazamiento += sizeof(int);
    memcpy(&(solicitud->size_solicitud), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);
    memcpy(&(solicitud->cantidad_accesos), buffer + desplazamiento, sizeof(uint32_t));
    desplazamiento += sizeof(uint32_t);

    solicitud->datos_memoria = malloc(solicitud->cantidad_accesos * sizeof(t_dato_memoria));
    for (int i = 0; i < solicitud->cantidad_accesos; i++) {
        memcpy(&(solicitud->datos_memoria[i].direccion_fisica), buffer + desplazamiento, sizeof(uint32_t));
        desplazamiento += sizeof(uint32_t);

        memcpy(&(solicitud->datos_memoria[i].tamano), buffer + desplazamiento, sizeof(uint32_t));
        desplazamiento += sizeof(uint32_t) + sizeof(int);

        solicitud->datos_memoria[i].datos = malloc(solicitud->datos_memoria[i].tamano);
        memcpy(solicitud->datos_memoria[i].datos, buffer + desplazamiento, solicitud->datos_memoria[i].tamano);
        desplazamiento += solicitud->datos_memoria[i].tamano;
    }

    free(buffer);
    return solicitud;
}



t_solicitud_io* crear_pedido_memoria(int pid, uint32_t size_solicitud) {
    t_solicitud_io* solicitud = malloc(sizeof(t_solicitud_io));
    if (solicitud != NULL) {
        solicitud->pid = pid;
        solicitud->size_solicitud = size_solicitud;
        solicitud->cantidad_accesos = 0;
        solicitud->datos_memoria = NULL;  // Inicialmente no hay datos
    }
    return solicitud;
}


// Función para agregar un dato de memoria a la solicitud existente
void agregar_a_pedido_memoria(t_solicitud_io* solicitud, char* dato, int size_dato, uint32_t direccion_fisica) {
    if (solicitud == NULL) {
        return;  // Seguridad para evitar la desreferencia de NULL
    }
    // Redimensionar el arreglo de datos de memoria
    size_t nuevo_tamano = solicitud->cantidad_accesos + 1;
    solicitud->datos_memoria = realloc(solicitud->datos_memoria, nuevo_tamano * sizeof(t_dato_memoria));
    if (solicitud->datos_memoria != NULL) {
        // Inicializar el nuevo t_dato_memoria
        // uint32_t size_dato = strlen(dato) + 1; // +1 para el carácter nulo
        solicitud->datos_memoria[solicitud->cantidad_accesos].direccion_fisica = direccion_fisica;  
        solicitud->datos_memoria[solicitud->cantidad_accesos].tamano = size_dato;
        solicitud->datos_memoria[solicitud->cantidad_accesos].datos = malloc(size_dato);
        if (solicitud->datos_memoria[solicitud->cantidad_accesos].datos != NULL) {
            memcpy(solicitud->datos_memoria[solicitud->cantidad_accesos].datos, dato, size_dato);
        }
		else{
			printf("Error al asignar memoria para datos_memorisa\n");
		}

        solicitud->cantidad_accesos = nuevo_tamano;  // Actualizar la cantidad de accesos
    } else {
        // Manejar error de memoria aquí si es necesario
        printf("Error al expandir la memoria para datos de memoria.\n");
    }
}


// Función para liberar un pedido de memoria completo
void eliminar_pedido_memoria(t_solicitud_io* solicitud) {
    if (solicitud != NULL) {
        // Primero liberar cada uno de los datos almacenados
        for (size_t i = 0; i < solicitud->cantidad_accesos; i++) {
            free(solicitud->datos_memoria[i].datos);  // Liberar los datos de cada acceso
        }
        // Luego liberar el arreglo de t_dato_memoria
        free(solicitud->datos_memoria);
        // Finalmente, liberar la estructura t_solicitud_io
        free(solicitud);
    }
}

// void llenar_datos_memoria(t_solicitud_io* solicitud, char* input_text) {
//     char* current_position = input_text;
//     for (int i = 0; i < solicitud->cantidad_accesos; ++i) {
//         t_dato_memoria* dato = &solicitud->datos_memoria[i];
//         printf("Dato %d: direccion_fisica: <%d>, tamano: <%d>\n", i, dato->direccion_fisica, dato->tamano);

//         // Aseguramos que la memoria está siendo correctamente asignada
//         dato->datos = (char*)malloc(dato->tamano + 1);  // +1 para el caracter nulo
//         if (dato->datos == NULL) {
//             fprintf(stderr, "Error al asignar memoria para datos_memoria\n");
//             exit(EXIT_FAILURE);
//         }

//         // Copiamos los datos y aseguramos el caracter nulo
//         memcpy(dato->datos, current_position, dato->tamano);
//         dato->datos[dato->tamano] = '\0';
//         printf("Dato %d: <%s>\n", i, dato->datos);

//         // Desplazamos la posición actual según el tamaño de los datos copiados
//         current_position += dato->tamano;
//     }
// }

void llenar_datos_memoria(t_solicitud_io* solicitud, char* input_text) {
    size_t size_solicitud = solicitud->size_solicitud;
    char* buffer = (char*)malloc(size_solicitud + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Error al asignar memoria para el buffer\n");
        exit(EXIT_FAILURE);
    }
    strncpy(buffer, input_text, size_solicitud);
    memset(buffer + strlen(input_text), ' ', size_solicitud - strlen(input_text));
    buffer[size_solicitud] = '\0';
    char* current_position = buffer;
    // printf("Total size_solicitud: %zu, buffer: <%s>\n", size_solicitud, buffer);
    for (int i = 0; i < solicitud->cantidad_accesos; ++i) {
        t_dato_memoria* dato = &solicitud->datos_memoria[i];
        // printf("Procesando Dato %d: tamano: %d\n", i, dato->tamano);
        dato->datos = (char*)malloc(dato->tamano + 1);
        if (dato->datos == NULL) {
            fprintf(stderr, "Error al asignar memoria para datos_memoria\n");
            free(buffer);
            exit(EXIT_FAILURE);
        }
        memcpy(dato->datos, current_position, dato->tamano);
        dato->datos[dato->tamano] = '\0';
        // printf("Dato %d: <%s>, current_position index: %ld\n", i, dato->datos, current_position - buffer);
        current_position += dato->tamano;
    }
    free(buffer);
}

void liberar_solicitud_io(t_solicitud_io* solicitud) {
    if (solicitud != NULL) {
        // Liberar cada conjunto de datos dentro del arreglo
        for (int i = 0; i < solicitud->cantidad_accesos; i++) {
            free(solicitud->datos_memoria[i].datos); // Liberar la memoria del dato individual
            solicitud->datos_memoria[i].datos = NULL; // Buenas prácticas: poner el puntero en NULL
        }

        // Liberar el arreglo de datos de memoria
        free(solicitud->datos_memoria);
        solicitud->datos_memoria = NULL;

        // Si la estructura t_solicitud_io misma fue asignada dinámicamente, liberarla aquí
        free(solicitud);
    }
}


/*IO READ - FIN*/