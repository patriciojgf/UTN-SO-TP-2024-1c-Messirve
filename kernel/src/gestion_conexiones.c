#include "gestion_conexiones.h"


static void _recibir_contexto_cpu(t_pcb *pcb, int* motivo, t_instruccion* instruccion);
static void _handshake_cliente_kernel(int socket, char* nombre_destino);
static char* _io_handshake_to_char(int handshake);
static void _gestionar_nueva_interfaz(void *void_args);

void init_conexiones(){
	socket_servidor_io = iniciar_servidor(PUERTO_ESCUCHA);
    socket_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
    socket_dispatch = crear_conexion(IP_CPU, PUERTO_CPU_DISPATCH);
    socket_interrupt = crear_conexion(IP_CPU, PUERTO_CPU_INTERRUPT);
    lista_interfaz_socket = list_create();
}

// --------------------------------------------------------------------------//
// ------------- CONEXIONES E HILOS -----------------------------------------//
// --------------------------------------------------------------------------//
/*SERVER IO*/
void gestionar_conexion_io(){
	while(1) {
		log_protegido_kernel(string_from_format("[GESTION IO]: Esperando cliente"));
		//genero nueva conexion para cada IO
		int socket_io_nuevo = esperar_cliente(socket_servidor_io);
		log_protegido_kernel(string_from_format("[GESTION IO]: Recibi operacion"));

		//hago handshake y guardo el tipo de io.
		int cod_tipo_io = handshake_server(socket_io_nuevo);

		//busco el nombre que me llega en un paquete nuevo.
		recibir_operacion(socket_io_nuevo);		
        int size, tam_nombre, desplazamiento = 0;
		void *buffer = recibir_buffer(&size, socket_io_nuevo);
		memcpy(&tam_nombre,buffer +desplazamiento, sizeof(int));;
		desplazamiento+=sizeof(int);
		char* nombre_nueva_io = malloc(tam_nombre+1); 
		memcpy(nombre_nueva_io, buffer + desplazamiento, tam_nombre);

		// Crear estructura de interfaz para pasar por pthread_create
        t_interfaz *info_interfaz = malloc(sizeof(t_interfaz));
		info_interfaz->socket = socket_io_nuevo;
		info_interfaz->nombre_io = nombre_nueva_io;
		info_interfaz->tipo_io = _io_handshake_to_char(cod_tipo_io);

		pthread_t hilo_interfaz;
		pthread_create(&hilo_interfaz, NULL, (void *) _gestionar_nueva_interfaz, info_interfaz);
		pthread_detach(hilo_interfaz);
	}	
}

static void _gestionar_nueva_interfaz(void *void_args){
	t_interfaz *interfaz_nueva = (t_interfaz *)void_args;
	list_add(lista_interfaz_socket, interfaz_nueva);
	log_protegido_kernel(string_from_format("[GESTION CONEXIONES]: Nueva interfaz %s conectada\n", interfaz_nueva->nombre_io));
}


/*MEMORIA*/
void gestionar_conexion_memoria(){
    _handshake_cliente_kernel(socket_memoria, "MEMORIA");

    pthread_create(&hilo_gestionar_memoria, NULL, (void*) atender_peticiones_memoria, NULL);
	pthread_detach(hilo_gestionar_memoria);

	sem_post(&s_conexion_memoria_ok);
}
/*CPU_DISPATCH*/
void gestionar_conexion_dispatch(){
    _handshake_cliente_kernel(socket_dispatch, "DISPATCH");

    pthread_create(&hilo_gestionar_dispatch, NULL, (void*) atender_peticiones_dispatch, NULL);
	pthread_detach(hilo_gestionar_dispatch);

	sem_post(&s_conexion_cpu_d_ok);
}
/*CPU_INTERRUPT*/
void gestionar_conexion_interrupt(){
	_handshake_cliente_kernel(socket_interrupt, "INTERRUPT");

    pthread_create(&hilo_gestionar_interrupt, NULL, (void*) atender_peticiones_interrupt, NULL);
	pthread_detach(hilo_gestionar_interrupt);

	sem_post(&s_conexion_cpu_i_ok);
}

// --------------------------------------------------------------------------//
// ------------- CONEXIONES E HILOS ---FIN-----------------------------------//
// --------------------------------------------------------------------------//




// --------------------------------------------------------------------------//
// ------------- FUNCIONES DE LOGICA POR MODULO------------------------------//
// --------------------------------------------------------------------------//

static char* _io_handshake_to_char(int handshake) {
    switch (handshake) {
        case HANDSHAKE_IO_GEN:
            return "GENERICA";
        case HANDSHAKE_IO_STDIN:
            return "STDIN";
        case HANDSHAKE_IO_STDOUT:
            return "STDOUT";
        case HANDSHAKE_IO_DIALFS:
            return "DIALFS";
        default:
            log_error(logger_kernel, "TIPO DE INTERFAZ NO RECONOCIDO: %d", handshake);
            exit(EXIT_FAILURE);  // Considera retornar NULL o una cadena indicando error en lugar de salir.
    }
}

/*----------------------Memoria---------------------------------------------------*/
void atender_peticiones_memoria(){
	while(1){
		int cod_op = recibir_operacion(socket_memoria);
		void* buffer_recibido;
		log_protegido_kernel(string_from_format("[ATENDER MEMORIA]: Recibi operacion"));
		switch(cod_op){
			case INICIAR_PROCESO_MEMORIA_OK:
		        log_protegido_kernel(string_from_format("[ATENDER MEMORIA]: INICIAR PROCESO"));
                int size=0;
                buffer_recibido = recibir_buffer(&size, socket_memoria);
				sem_post(&s_init_proceso_a_memoria);
				free(buffer_recibido);
		        log_protegido_kernel(string_from_format("[ATENDER MEMORIA]: INICIAR PROCESO REALIZADO"));
				break;	
			default:
		        log_protegido_kernel(string_from_format("[ATENDER MEMORIA]: Operacion no reconocida"));
				exit(EXIT_FAILURE);
		}
	}
}
/*----------------------Dispatch---------------------------------------------------*/
void atender_peticiones_dispatch(){
	while(1){
		int cod_op = recibir_operacion(socket_dispatch);
		log_protegido_kernel(string_from_format("[ATENDER DISPATCH]: Recibi operacion"));
		switch (cod_op) {
			case CONTEXTO_EJECUCION:
		        log_protegido_kernel(string_from_format("[ATENDER DISPATCH]: CONTEXTO_EJECUCION"));
				int motivo;
				t_instruccion* instrucciones=malloc(sizeof(t_instruccion));
				instrucciones->parametros =list_create();
				_recibir_contexto_cpu(proceso_exec, &motivo, instrucciones);
				switch(motivo){
					case EXIT:
						log_protegido_kernel(string_from_format("[ATENDER DISPATCH]:PID: <%d> - EXIT", proceso_exec->pid));
				}
				break;
            default:
                log_error(logger_kernel, "ERROR EN HANDSHAKE: Operacion N* %d desconocida\n", cod_op);
                exit(EXIT_FAILURE);
		}
	}
}

void atender_peticiones_interrupt(){
	log_protegido_kernel(string_from_format("[ATENDER INTERRUPT]: por ahora solo se conectarme"));
}


// --------------------------------------------------------------------------//
// ------------- FUNCIONES DE LOGICA POR MODULO ---- FIN --------------------//
// --------------------------------------------------------------------------//





// --------------------------------------------------------------------------//
// ------------- FUNCIONES AUXILIARES----------------------------------------//
// --------------------------------------------------------------------------//

static void _handshake_cliente_kernel(int socket, char* nombre_destino){
	log_protegido_kernel(string_from_format("[GESTION CONEXIONES]: Inicio Handshake con %s\n", nombre_destino));
    int resultado_hs = handshake_cliente(HANDSHAKE_KERNEL,socket);
    switch(resultado_hs){
        case HANDSHAKE_OK:
            log_protegido_kernel(string_from_format("[GESTION CONEXIONES]: Handshake con %s: OK\n", nombre_destino));
            break;
        default:
			sleep(120);
            log_error(logger_kernel, "ERROR EN HANDSHAKE: Operacion N* %d desconocida\n", resultado_hs);
            exit(EXIT_FAILURE);
            break;
    }    
}


/*----------------------Dispatch---------------------------------------------------*/
static void _recibir_contexto_cpu(t_pcb *pcb, int* motivo, t_instruccion* instruccion){
		log_protegido_kernel(string_from_format("_recibir_contexto_cpu"));
		int size;
		void* buffer = recibir_buffer(&size, socket_dispatch);
		int desplazamiento = 0;		
		memcpy(&(pcb->pid), buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);
		//Registros CPU
		memcpy(&(pcb->registros_cpu.AX), buffer + desplazamiento, sizeof(uint8_t));
		desplazamiento += sizeof(uint8_t);
		memcpy(&(pcb->registros_cpu.BX), buffer + desplazamiento, sizeof(uint8_t));
		desplazamiento += sizeof(uint8_t);
		memcpy(&(pcb->registros_cpu.CX), buffer + desplazamiento, sizeof(uint8_t));
		desplazamiento += sizeof(uint8_t);
		memcpy(&(pcb->registros_cpu.DX), buffer + desplazamiento, sizeof(uint8_t));
		desplazamiento += sizeof(uint8_t);	
		memcpy(&(pcb->registros_cpu.EAX), buffer + desplazamiento, sizeof(uint32_t));
		desplazamiento += sizeof(uint32_t);
		memcpy(&(pcb->registros_cpu.EBX), buffer + desplazamiento, sizeof(uint32_t));
		desplazamiento += sizeof(uint32_t);
		memcpy(&(pcb->registros_cpu.ECX), buffer + desplazamiento, sizeof(uint32_t));
		desplazamiento += sizeof(uint32_t);
		memcpy(&(pcb->registros_cpu.PC), buffer + desplazamiento, sizeof(uint32_t));
		desplazamiento += sizeof(uint32_t);
		memcpy(&(pcb->registros_cpu.SI), buffer + desplazamiento, sizeof(uint32_t));
		desplazamiento += sizeof(uint32_t);		
		memcpy(&(instruccion->identificador ),buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);			
		memcpy(&(instruccion->cantidad_parametros),buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);		
		for (int i=0; i < instruccion->cantidad_parametros; i++){
			char* parametro;
			int size_parametro;
			memcpy(&(size_parametro), buffer + desplazamiento, sizeof(int));
			desplazamiento += sizeof(int);
			parametro=malloc(size_parametro);
			memcpy(parametro, buffer + desplazamiento, size_parametro);
			desplazamiento += size_parametro;
			list_add(instruccion->parametros, parametro);
		}
		free(buffer);
		//saco el motivo
		memcpy(&motivo, buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);
}


// --------------------------------------------------------------------------//
// ------------- FUNCIONES AUXILIARES---------- FIN -------------------------//
// --------------------------------------------------------------------------//