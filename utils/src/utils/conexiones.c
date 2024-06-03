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
	buffer->stream = realloc(buffer->stream, buffer->size + bytes);
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

