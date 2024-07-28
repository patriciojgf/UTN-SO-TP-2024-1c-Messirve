#include "gestion_conexiones.h"

typedef struct {
    t_pcb* pcb;
    t_instruccion* instruccion;
} t_args_io;

static void _recibir_contexto_cpu(t_pcb *pcb, int* motivo, t_instruccion* instruccion);
static void _handshake_cliente_kernel(int socket, char* nombre_destino);
static char* _io_handshake_to_char(int handshake);
static void _gestionar_nueva_interfaz(void *void_args);
static void _atender_peticiones_io(t_interfaz *interfaz);
static void _agregar_a_lista_interfaces(t_interfaz *interfaz_nueva);

// static void* thread_atender_cpu_io_stdin_read(void* args) {
//     t_args_io* argumentos = (t_args_io*) args;
//     atender_cpu_io_stdin_read(argumentos->pcb, argumentos->instruccion);
//     free(argumentos);  
//     return NULL;
// }

// static void* thread_atender_cpu_io_stdin_write(void* args) {
//     t_args_io* argumentos = (t_args_io*) args;
//     atender_cpu_io_stdout_write(argumentos->pcb, argumentos->instruccion);
//     free(argumentos);  
//     return NULL;
// }

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
		//genero nueva conexion para cada IO
		int socket_io_nuevo = esperar_cliente(socket_servidor_io);

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
		//info_interfaz->tipo_io = _io_handshake_to_char(cod_tipo_io);
		info_interfaz->tipo_io = cod_tipo_io;
		info_interfaz->cola_procesos = list_create();
		pthread_mutex_init(&(info_interfaz->mutex_cola_block), NULL);
		sem_init(&info_interfaz->semaforo, 0, 0);		

		pthread_t hilo_interfaz;
		pthread_create(&hilo_interfaz, NULL, (void *) _gestionar_nueva_interfaz, info_interfaz);
		pthread_detach(hilo_interfaz);
		sem_post(&s_conexion_interfaz);
	}	
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

static void _gestionar_nueva_interfaz(void *void_args) {
    t_interfaz *interfaz_nueva = (t_interfaz *)void_args;
    _agregar_a_lista_interfaces(interfaz_nueva);
    // //log_protegido_kernel(string_from_format("[GESTION CONEXIONES]: Nueva interfaz %s conectada\n", interfaz_nueva->nombre_io));

    // Utilizar strcmp para comparar cadenas
    if (strcmp(_io_handshake_to_char(interfaz_nueva->tipo_io), "STDIN") == 0) {
        _atender_peticiones_io(interfaz_nueva);
    } else if (strcmp(_io_handshake_to_char(interfaz_nueva->tipo_io), "STDOUT") == 0) {
        _atender_peticiones_io(interfaz_nueva);
    } else if (strcmp(_io_handshake_to_char(interfaz_nueva->tipo_io), "GENERICA") == 0) {
        _atender_peticiones_io(interfaz_nueva);
    } else if (strcmp(_io_handshake_to_char(interfaz_nueva->tipo_io), "DIALFS") == 0) {
        _atender_peticiones_io(interfaz_nueva);
	}	
	 else {
        log_error(logger_kernel, "ERROR EN HANDSHAKE: Operacion de interfaz '%s' desconocida\n", _io_handshake_to_char(interfaz_nueva->tipo_io));
        exit(EXIT_FAILURE);
    }
}

/*----------------------Memoria---------------------------------------------------*/
void atender_peticiones_memoria(){
	while(1){
		int cod_op = recibir_operacion(socket_memoria);
		void* buffer_recibido;
		int size = 0;
		// //log_protegido_kernel(string_from_format("[ATENDER MEMORIA]: Recibi operacion"));
		switch(cod_op){
			case INICIAR_PROCESO_MEMORIA_OK:
		        // //log_protegido_kernel(string_from_format("[ATENDER MEMORIA]: INICIAR PROCESO"));
                size=0;
                buffer_recibido = recibir_buffer(&size, socket_memoria);
				sem_post(&s_init_proceso_a_memoria);
				free(buffer_recibido);
		        // //log_protegido_kernel(string_from_format("[ATENDER MEMORIA]: INICIAR PROCESO REALIZADO"));
				break;	
			case LIBERAR_ESTRUCTURAS_MEMORIA_OK:
		        // //log_protegido_kernel(string_from_format("[ATENDER MEMORIA]: FINALIZAR PROCESO"));
				size=0;
				buffer_recibido = recibir_buffer(&size, socket_memoria);
				sem_post(&s_memoria_liberada_pcb);
				free(buffer_recibido);
		        // //log_protegido_kernel(string_from_format("[ATENDER MEMORIA]: FINALIZAR PROCESO REALIZADO"));
				break;
			default:
		        // //log_protegido_kernel(string_from_format("[ATENDER MEMORIA]: Operacion no reconocida"));
				exit(EXIT_FAILURE);
		}
	}
}
/*----------------------Dispatch---------------------------------------------------*/
void atender_peticiones_dispatch(){
	while(1){
		// log_info(logger_kernel,"[atender_peticiones_dispatch]");
		int cod_op = recibir_operacion(socket_dispatch);
		// log_info(logger_kernel,"[atender_peticiones_dispatch] - recibir_operacion");
		// //log_protegido_kernel(string_from_format("[ATENDER DISPATCH]: Recibi operacion"));
		switch (cod_op) {
			case CONTEXTO_EJECUCION:
				// log_info(logger_kernel,"[atender_peticiones_dispatch] - CONTEXTO_EJECUCION");
				// log_info(logger_kernel,"[ATENDER DISPATCH]:CONTEXTO_EJECUCION]");
		        // //log_protegido_kernel(string_from_format("[ATENDER DISPATCH]: CONTEXTO_EJECUCION"));
				int motivo;
				t_instruccion* instrucciones=malloc(sizeof(t_instruccion));
				instrucciones->parametros =list_create();
				_recibir_contexto_cpu(proceso_exec, &motivo, instrucciones);	
				// log_info(logger_kernel,"[atender_peticiones_dispatch] - CONTEXTO_EJECUCION - _recibir_contexto_cpu");

				pthread_mutex_lock(&mutex_finalizar_proceso);
				if(proceso_finalizando && (motivo != INT_FINALIZAR_PROCESO)){
					// log_info(logger_kernel,"[ATENDER DISPATCH]:Recibi operacion pero estoy esperando FINALIZAR_PROCESO");
					// log_info(logger_kernel,"[atender_peticiones_dispatch] - CONTEXTO_EJECUCION - pthread_mutex_unlock(&mutex_finalizar_proceso)");
					
					//estoy esta bien?
					proceso_finalizando = false;
					
					pthread_mutex_unlock(&mutex_finalizar_proceso);	
					continue;	
				}
				pthread_mutex_unlock(&mutex_finalizar_proceso);		
				// log_info(logger_kernel,"[atender_peticiones_dispatch] - CONTEXTO_EJECUCION - pthread_mutex_unlock(&mutex_finalizar_proceso)");		
				switch(motivo){
					case INT_FINALIZAR_PROCESO:	
						// log_info(logger_kernel,"[atender_peticiones_dispatch] - CONTEXTO_EJECUCION - INT_FINALIZAR_PROCESO");	
						// log_info(logger_kernel,"[ATENDER DISPATCH]:PID: <%d> - INT_FINALIZAR_PROCESO", proceso_exec->pid);
						atender_cpu_int_finalizar_proceso(proceso_exec);
						pthread_mutex_lock(&mutex_finalizar_proceso);
        				proceso_finalizando = false;
        				pthread_mutex_unlock(&mutex_finalizar_proceso);
						break;
					case PAGE_FAULT:
						// log_info(logger_kernel,"[atender_peticiones_dispatch] - CONTEXTO_EJECUCION - PAGE_FAULT");	
						sem_post(&sem_pcb_desalojado);
						atender_cpu_exit(proceso_exec,"PAGE_FAULT");
						break;
					case OUT_OF_MEMORY:
						// log_info(logger_kernel,"[atender_peticiones_dispatch] - CONTEXTO_EJECUCION - OUT_OF_MEMORY");	
						sem_post(&sem_pcb_desalojado);
						atender_cpu_exit(proceso_exec,"OUT_OF_MEMORY");
						break;
					case EXIT:
						// log_info(logger_kernel,"[atender_peticiones_dispatch] - CONTEXTO_EJECUCION - EXIT");	
						// log_info(logger_kernel,"[ATENDER DISPATCH]:PID: <%d> - EXIT", proceso_exec->pid);
						//log_protegido_kernel(string_from_format("[ATENDER DISPATCH]:PID: <%d> - EXIT", proceso_exec->pid));						
						sem_post(&sem_pcb_desalojado);
						atender_cpu_exit(proceso_exec,"SUCCESS");
						break;
					case IO_GEN_SLEEP:
						// log_info(logger_kernel,"[atender_peticiones_dispatch] - CONTEXTO_EJECUCION - IO_GEN_SLEEP");	
						// log_info(logger_kernel,"[ATENDER DISPATCH]:PID: <%d> - IO_GEN_SLEEP", proceso_exec->pid);
						sem_post(&sem_pcb_desalojado);
						atender_cpu_io_gen_sleep(proceso_exec,instrucciones);
						break;
					case IO_STDIN_READ:
						// log_info(logger_kernel,"[atender_peticiones_dispatch] - CONTEXTO_EJECUCION - IO_STDIN_READ");
						sem_post(&sem_pcb_desalojado);
						// // Preparar los argumentos para el hilo
						// t_args_io* argumentos = malloc(sizeof(t_args_io));
						// argumentos->pcb = proceso_exec;
						// argumentos->instruccion = instrucciones;	
						// // Crear y lanzar el hilo					
						// pthread_t hilo_io_stdin_read;
						// pthread_create(&hilo_io_stdin_read, NULL, thread_atender_cpu_io_stdin_read, (void*) argumentos);
						// pthread_detach(hilo_io_stdin_read);	
						// sem_wait(&s_pedido_io_enviado);		
						if(preparar_enviar_solicitud_io(proceso_exec, instrucciones) == -1){
							atender_cpu_exit(proceso_exec,"INVALID_INTERFACE");
						}
						break;
					case IO_STDOUT_WRITE:
						// log_info(logger_kernel,"[atender_peticiones_dispatch] - CONTEXTO_EJECUCION - IO_STDOUT_WRITE");
						sem_post(&sem_pcb_desalojado);
						if(preparar_enviar_solicitud_io(proceso_exec, instrucciones) == -1){
							atender_cpu_exit(proceso_exec,"INVALID_INTERFACE");
						}
						break;
					case IO_FS_CREATE:
						sem_post(&sem_pcb_desalojado);
						if(atender_io_fs_create_delete(proceso_exec, instrucciones,IO_FS_CREATE) == -1){
							atender_cpu_exit(proceso_exec,"INVALID_INTERFACE");
						}
						break;
					case IO_FS_DELETE:
						sem_post(&sem_pcb_desalojado);
						if(atender_io_fs_create_delete(proceso_exec, instrucciones,IO_FS_DELETE) == -1){
							atender_cpu_exit(proceso_exec,"INVALID_INTERFACE");
						}
						break;
					case IO_FS_TRUNCATE:
						sem_post(&sem_pcb_desalojado);
						if(atender_io_fs_truncate(proceso_exec, instrucciones) == -1){
							atender_cpu_exit(proceso_exec,"INVALID_INTERFACE");
						}
						break;
					case IO_FS_WRITE:
						sem_post(&sem_pcb_desalojado);
						break;
					case IO_FS_READ:
						sem_post(&sem_pcb_desalojado);
						break;
					case FIN_QUANTUM:
						//log obligatorio
						//“PID: <PID> - Desalojado por fin de Quantum”
						log_info(logger_kernel,"PID: <%d> - Desalojado por fin de Quantum", proceso_exec->pid);
						sem_post(&sem_pcb_desalojado);
						atender_cpu_fin_quantum(proceso_exec);
						break;
					case WAIT:
						// log_info(logger_kernel,"[atender_peticiones_dispatch] - CONTEXTO_EJECUCION - WAIT");
						// log_info(logger_kernel,"[ATENDER DISPATCH]:PID: <%d> - WAIT", proceso_exec->pid);
						//log_protegido_kernel(string_from_format("[ATENDER DISPATCH]:PID: <%d> - WAIT", proceso_exec->pid));
						sem_post(&sem_pcb_desalojado);
						atender_cpu_wait(proceso_exec,instrucciones);
						break;
					case SIGNAL:
						// log_info(logger_kernel,"[atender_peticiones_dispatch] - CONTEXTO_EJECUCION - SIGNAL");
						// log_info(logger_kernel,"[ATENDER DISPATCH]:PID: <%d> - SIGNAL", proceso_exec->pid);
						//sem_post(&sem_pcb_desalojado); //no se desaloja
						atender_cpu_signal(proceso_exec,obtener_recurso(list_get(instrucciones->parametros, 0)));
						break;
					case INT_SIGNAL:
						// log_info(logger_kernel,"[atender_peticiones_dispatch] - CONTEXTO_EJECUCION - INT_SIGNAL");
						// log_info(logger_kernel,"[ATENDER DISPATCH]:PID: <%d> - INT_SIGNAL", proceso_exec->pid);
						atender_cpu_int_signal(proceso_exec);
						break;

					default:
                		log_error(logger_kernel, "[CONTEXTO_EJECUCION]: motivo no reconocido <%d> \n", cod_op);
				}
				//liberar memoria instrucciones
				for (int i=0; i < instrucciones->cantidad_parametros; i++){
					free(list_get(instrucciones->parametros, i));
				}
				free(instrucciones->parametros);
				break;
            default:
                log_error(logger_kernel, "ERROR EN PETICION DISPATCH: Operacion N* %d desconocida\n", cod_op);
                exit(EXIT_FAILURE);
		}
	}
}

void atender_peticiones_interrupt(){
	//log_protegido_kernel(string_from_format("[ATENDER INTERRUPT]: conectado."));
}
/*------------------------INTERFACES-----------------------------------------*/
/*STDIN*/

/*GENERICAS*/
static void _atender_peticiones_io(t_interfaz *interfaz){
		// log_info(logger_kernel,"[_atender_peticiones_io]");
		while(1){
    		// //log_protegido_kernel(string_from_format("[ATENDER INTERFAZ IO GEN %s]: INICIADA ---- ESPERANDO ----", interfaz->nombre_io));
			int cod_op = recibir_operacion(interfaz->socket);
			// log_info(logger_kernel,"[_atender_peticiones_io] - recibir_operacion");
			int size_temp =0;
			void* buffer_temp;
			switch (cod_op){
				case IO_GEN_SLEEP:	    
					// log_info(logger_kernel,"[_atender_peticiones_io] - IO_GEN_SLEEP");
    				// log_info(logger_kernel,"[ATENDER PETICION IO GEN]: SLEEP TERMINADO ");
					t_pedido_sleep* pedido = list_get(interfaz->cola_procesos, 0);
					sem_post(&pedido->semaforo_pedido_ok);
					//free(pedido);
					break;
				case IO_STDIN_READ:
					// log_info(logger_kernel,"[_atender_peticiones_io] - IO_STDIN_READ");
					// log_info(logger_kernel,"[ATENDER PETICION IO GEN]: IO_STDIN_READ TERMINADO ");
					size_temp =0;
					buffer_temp = recibir_buffer(&size_temp, interfaz->socket);
					free(buffer_temp);
					t_pedido_stdin* pedido_stdin = list_get(interfaz->cola_procesos, 0);
					sem_post(&pedido_stdin->semaforo_pedido_ok);
					break;
				case IO_STDOUT_WRITE:
					// log_info(logger_kernel,"[_atender_peticiones_io] - IO_STDOUT_WRITE");
					// log_info(logger_kernel,"[ATENDER PETICION IO GEN]: IO_STDOUT_WRITE TERMINADO ");
					size_temp =0;
					buffer_temp = recibir_buffer(&size_temp, interfaz->socket);
					free(buffer_temp);
					t_pedido_stdin* pedido_stdiout= list_get(interfaz->cola_procesos, 0);
					sem_post(&pedido_stdiout->semaforo_pedido_ok);
					break;
				case IO_FS_CREATE:
					size_temp =0;
					buffer_temp = recibir_buffer(&size_temp, interfaz->socket);
					free(buffer_temp);
					t_pedido_stdin* pedido_fs_create= list_get(interfaz->cola_procesos, 0);
					sem_post(&pedido_fs_create->semaforo_pedido_ok);
					break;
				case IO_FS_DELETE:
					size_temp =0;
					buffer_temp = recibir_buffer(&size_temp, interfaz->socket);
					free(buffer_temp);
					t_pedido_stdin* pedido_fs_delete= list_get(interfaz->cola_procesos, 0);
					sem_post(&pedido_fs_delete->semaforo_pedido_ok);
					break;
				case IO_FS_TRUNCATE:
					size_temp =0;
					buffer_temp = recibir_buffer(&size_temp, interfaz->socket);
					free(buffer_temp);
					t_pedido_stdin* pedido_fs_truncate= list_get(interfaz->cola_procesos, 0);
					sem_post(&pedido_fs_truncate->semaforo_pedido_ok);
					break;
				case IO_FS_WRITE:
					size_temp =0;
					buffer_temp = recibir_buffer(&size_temp, interfaz->socket);
					free(buffer_temp);
					t_pedido_stdin* pedido_fs_write= list_get(interfaz->cola_procesos, 0);
					sem_post(&pedido_fs_write->semaforo_pedido_ok);
					break;
				case IO_FS_READ:
					size_temp =0;
					buffer_temp = recibir_buffer(&size_temp, interfaz->socket);
					free(buffer_temp);
					t_pedido_stdin* pedido_fs_read= list_get(interfaz->cola_procesos, 0);
					sem_post(&pedido_fs_read->semaforo_pedido_ok);
					break;
				case -1:
					// log_info(logger_kernel,"[_atender_peticiones_io] - -1");
					// log_info(logger_kernel,"[ATENDER PETICION IO GEN]: INTERFAZ <%s> DESCONECTADA",interfaz->nombre_io);
					desconectar_interfaz(interfaz);
					pthread_exit(NULL);
				default:
					log_error(logger_kernel,"[ATENDER INTERFAZ IO GEN %s]: operacion desconocida - %d",interfaz->nombre_io, cod_op);
					exit(EXIT_FAILURE);
			}			
		}
}	

// static void _enviar_peticiones_io_gen(t_interfaz *interfaz){
//     //log_protegido_kernel(string_from_format("[ENVIAR PETICION INTERFAZ IO GEN %s]: INICIADA ---- ESPERANDO ----", interfaz->nombre_io));
// 	while (1) {
// 		//espero que haya algun pedido creado
//         sem_wait(&interfaz->semaforo);

// 		//envio pedido a la interfaz
// 		t_pedido_sleep* pedido = list_get(interfaz->cola_procesos, 0);
// 		t_paquete* paquete_pedido = crear_paquete(IO_GEN_SLEEP);
// 		agregar_datos_sin_tamaño_a_paquete(paquete_pedido, &pedido->tiempo_sleep, sizeof(int));
// 		enviar_paquete(paquete_pedido, interfaz->socket);

// 		//espero el ok
// 		sem_wait(&pedido->semaforo_pedido_ok);
// 		list_remove(interfaz->cola_procesos, 0);
// 		free(pedido);
//     }
// }



// --------------------------------------------------------------------------//
// ------------- FUNCIONES DE LOGICA POR MODULO ---- FIN --------------------//
// --------------------------------------------------------------------------//





// --------------------------------------------------------------------------//
// ------------- FUNCIONES AUXILIARES----------------------------------------//
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


static void _agregar_a_lista_interfaces(t_interfaz *interfaz_nueva) {
    pthread_mutex_lock(&mutex_lista_interfaz);  // Bloquear el mutex

    // Agregar la nueva interfaz al inicio de la lista
    list_add(lista_interfaz_socket, interfaz_nueva);

    pthread_mutex_unlock(&mutex_lista_interfaz);  // Desbloquear el mutex
}

static void _handshake_cliente_kernel(int socket, char* nombre_destino){
	//log_protegido_kernel(string_from_format("[GESTION CONEXIONES]: Inicio Handshake con %s\n", nombre_destino));
    int resultado_hs = handshake_cliente(HANDSHAKE_KERNEL,socket);
    switch(resultado_hs){
        case HANDSHAKE_OK:
            //log_protegido_kernel(string_from_format("[GESTION CONEXIONES]: Handshake con %s: OK\n", nombre_destino));
            break;
        default:
            log_error(logger_kernel, "ERROR EN HANDSHAKE: Operacion N* %d desconocida\n", resultado_hs);
            exit(EXIT_FAILURE);
            break;
    }    
}


/*----------------------Dispatch---------------------------------------------------*/
static void _recibir_contexto_cpu(t_pcb *pcb, int* motivo, t_instruccion* instruccion){
		//log_info(logger_kernel,"[_recibir_contexto_cpu]");
		int size;
		void* buffer = recibir_buffer(&size, socket_dispatch);
		int desplazamiento = 0;		
		memcpy(&(pcb->pid), buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);
		// memcpy(&(pcb->program_counter), buffer + desplazamiento, sizeof(int));
		// desplazamiento += sizeof(int);
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
		memcpy(&(pcb->registros_cpu.EDX), buffer + desplazamiento, sizeof(uint32_t));
		desplazamiento += sizeof(uint32_t);
		memcpy(&(pcb->registros_cpu.PC), buffer + desplazamiento, sizeof(uint32_t));
		desplazamiento += sizeof(uint32_t);
		memcpy(&(pcb->registros_cpu.SI), buffer + desplazamiento, sizeof(uint32_t));
		desplazamiento += sizeof(uint32_t);	
		memcpy(&(pcb->registros_cpu.DI), buffer + desplazamiento, sizeof(uint32_t));
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
		memcpy(motivo, buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);
		free(buffer);
}


// --------------------------------------------------------------------------//
// ------------- FUNCIONES AUXILIARES---------- FIN -------------------------//
// --------------------------------------------------------------------------//

void envio_interrupcion(int pid, int motivo){
	//log_protegido_kernel(string_from_format("[envio_interrupcion]"));
	t_paquete* paquete_interrupcion= crear_paquete(motivo);
	agregar_datos_sin_tamaño_a_paquete(paquete_interrupcion,&pid,sizeof(int));
	enviar_paquete(paquete_interrupcion,socket_interrupt);
	eliminar_paquete(paquete_interrupcion);	
}

void desconectar_interfaz(t_interfaz* interfaz){
	//elimino de la lista lista_interfaz_socket	
	pthread_mutex_lock(&mutex_lista_interfaz);
	list_remove_element(lista_interfaz_socket, interfaz);
	pthread_mutex_unlock(&mutex_lista_interfaz);

	pthread_mutex_lock(&mutex_plan_blocked);
	for(int i = 0; i < list_size(interfaz->cola_procesos); i++){
		t_pcb* pcb_desbloqueado = list_remove(interfaz->cola_procesos, 0);
		list_remove_element(lista_plan_blocked, pcb_desbloqueado);
		mover_proceso_a_ready(pcb_desbloqueado);
	}
	pthread_mutex_unlock(&mutex_plan_blocked);

	close(interfaz->socket);
	list_destroy_and_destroy_elements(interfaz->cola_procesos,free);
	free(interfaz->nombre_io);
	free(interfaz);
}