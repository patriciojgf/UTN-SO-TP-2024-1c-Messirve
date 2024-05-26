#include <main.h>

int conexiones;
//semaforos
// sem_t sem_sockets_interfaces;

// void log_protegido_kernel(char* mensaje){
// 	sem_wait(&mlog);
// 	log_info(logger_kernel, "%s", mensaje);
// 	sem_post(&mlog);
// }


/* -------------------------------------Iniciar Kernel -----------------------------------------------*/
int main(int argc, char **argv) {

	/*--------- Cargo configuraciones y log--------*/
	init_kernel(argv[1]);
	//config_kernel = iniciar_config(argv[1]);
	//logger_kernel=iniciar_logger("kernel.log","KERNEL");
	//GRADO_MULTIPROGRAMACION = config_get_int_value(config_kernel, "GRADO_MULTIPROGRAMACION");
	//sem_init(&mlog,0,1);
	//sem_init(&m_multiprogramacion, 0, GRADO_MULTIPROGRAMACION);
	//pthread_mutex_init(&mutex_conexiones,NULL);
	conexiones=0;

	/*--------- Inicio conexiones y estructuras--------*/
	pthread_t t1,t2,t3,t4,t5;

	/*---------- Hilos para planificadores --------------*/
	// inicializar_estructuras();
	lista_interfaz_socket = list_create();

    pthread_create(&t1, NULL, (void*) conectarMemoria, NULL);
	pthread_detach(t1); //ver si es correcto el detach aca
	pthread_create(&t2, NULL, (void*) conectarCpuDispatch, NULL);
	pthread_detach(t2);
	pthread_create(&t3, NULL, (void*) conectarCpuInterrupt, NULL);
	pthread_detach(t3);
    pthread_create(&t4, NULL, (void*) conectarInterfaz, NULL);
	pthread_detach(t4);
	pthread_create(&t5, NULL, (void*) leerConsola, NULL);

	/*------- Inicio los planificadores ----------------*/
	// iniciar_hilos_estructuras();

	
	/*------- Limpio listas, semaforos, etc ------------*/
	// free_estructuras();

	// log_destroy(logger_kernel);
 	// config_destroy(config_kernel);
	
	// liberar_conexion(socket_FS);
	pthread_join(t5, NULL);

	//pthread_detach(t1);
	// pthread_join(t1, NULL); //patricio:agrego esta linea momentaneamente para darle tiempo a que se conecte a memoria
	// pthread_join(t2, NULL);
	// pthread_join(t3, NULL);
	// pthread_join(t4, NULL);
	// pthread_join(t5, NULL);

	liberar_conexion(socket_memoria);
	liberar_conexion(socket_dispatch);
	liberar_conexion(socket_interrupt);
	liberar_conexion(socket_servidor);
	sem_destroy(&planificadores);
	// free_estructuras();
    return 0;
}


/*-------------------------------------Servidor para Interfaces------------------------------*/
int conectarInterfaz(){
	char* puerto_escucha = config_get_string_value(config_kernel, "PUERTO_ESCUCHA");
	socket_servidor = iniciar_servidor(puerto_escucha);
	if (socket_servidor == -1){
		log_error(logger_kernel, "ERROR - No se pudo crear el servidor");
		return EXIT_FAILURE;
	}
	log_protegido_kernel(string_from_format("Servidor listo para recibir clientes"));

	while(esperar_interfaz(socket_servidor) != -1){
		log_protegido_kernel(string_from_format("Esperando Cliente..."));
	}

	// pthread_mutex_lock(&mconexiones);
	// conexiones++;
	// pthread_mutex_unlock(&mconexiones);

	return 0;
}

int esperar_interfaz(int socket_servidor){
	int socket_cliente = esperar_cliente(socket_servidor);
	if(socket_cliente == -1){
		log_error(logger_kernel, "ERROR - No se pudo aceptar al cliente");
		return EXIT_FAILURE;
	}
	// int cod_op = recibir_operacion(socket_cliente);
	// printf("cod_op: %d\n", cod_op);
	//nuevo hilo
	pthread_t hilo_interfaz;
	pthread_create(&hilo_interfaz, NULL, (void *)nuevaInterfaz, (void*) socket_cliente);
	printf("Hilo Interfaz creado\n");
	pthread_detach(hilo_interfaz);
	printf("Hilo Interfaz detach\n");
	return 0;
}

int nuevaInterfaz(int socket_cliente){

	t_socket_interfaz* nueva_interfaz_socket = malloc(sizeof(t_socket_interfaz));
	nueva_interfaz_socket->nombre_interfaz = malloc(sizeof(char*));
	nueva_interfaz_socket->socket = socket_cliente;

	//1. Recibo el tipo de interfaz
	nueva_interfaz_socket->tipo_interfaz = recibir_operacion(socket_cliente);
	if(nueva_interfaz_socket->tipo_interfaz == -1){
		free(nueva_interfaz_socket);
		log_error(logger_kernel, "ERROR - No se pudo recibir el codigo de operacion");
		log_warning(logger_kernel, "Ver si hay que modificar este return para que falle");
		return EXIT_FAILURE;
	}

	//2. Recibo el nombre de la interfaz	
	int cod_op = recibir_operacion(socket_cliente);
	if(!(cod_op == MENSAJE))
		return EXIT_FAILURE;
	nueva_interfaz_socket->nombre_interfaz  = leer_mensaje(socket_cliente);

	if(nueva_interfaz_socket->nombre_interfaz == NULL){
		free(nueva_interfaz_socket->nombre_interfaz);
		free(nueva_interfaz_socket);
		log_error(logger_kernel, "ERROR - No se pudo recibir el nombre de la interfaz");
		return EXIT_FAILURE;
	}

	printf("Nombre de interfaz: %s\n", nueva_interfaz_socket->nombre_interfaz );

	switch (nueva_interfaz_socket->tipo_interfaz){
		case GENERICA:
			log_protegido_kernel(string_from_format("Se conecto la interfaz GENERICA %s", nueva_interfaz_socket->nombre_interfaz));
			// sem_wait(&sem_sockets_interfaces);
			agregar_socket_a_lista(lista_interfaz_socket, nueva_interfaz_socket);
			// sem_post(&sem_sockets_interfaces);
			//conectar con IO_GENERICA
			break;
		case DIALFS:
			log_protegido_kernel(string_from_format("Se conecto la interfaz DIALFS %s", nueva_interfaz_socket->nombre_interfaz));
			// sem_wait(&sem_sockets_interfaces);
			agregar_socket_a_lista(lista_interfaz_socket, nueva_interfaz_socket);
			// sem_post(&sem_sockets_interfaces);
			//conectar con IO_DIALFS
			break;
		case STDIN:
			log_protegido_kernel(string_from_format("Se conecto la interfaz STDIN %s", nueva_interfaz_socket->nombre_interfaz));
			// sem_wait(&sem_sockets_interfaces);
			agregar_socket_a_lista(lista_interfaz_socket, nueva_interfaz_socket);
			// sem_post(&sem_sockets_interfaces);
			//conectar con IO_STDIN
			break;
		case STDOUT:
			printf("STDOUT\n");
			log_protegido_kernel(string_from_format("Se conecto la interfaz STDOUT %s", nueva_interfaz_socket->nombre_interfaz));
			// sem_wait(&sem_sockets_interfaces);
			agregar_socket_a_lista(lista_interfaz_socket, nueva_interfaz_socket);
			// sem_post(&sem_sockets_interfaces);
			//conectar con IO_STDOUT
			break;
		default:
			free(nueva_interfaz_socket->nombre_interfaz);
			free(nueva_interfaz_socket);
			log_error(logger_kernel, "ERROR - No se pudo reconocer el codigo de operacion");
			break;
	}

	pthread_mutex_lock(&mutex_conexiones);
	conexiones++;
	pthread_mutex_unlock(&mutex_conexiones);

	// se agraga para que no siga tirando el warning
	return EXIT_SUCCESS;
}

/* ------------------------------------Conexiones--------------------------------------------*/
int conectarMemoria(){
	socket_memoria = -1;
	char* ip;
	char* puerto;

    ip= config_get_string_value(config_kernel,"IP_MEMORIA");
	puerto=config_get_string_value(config_kernel,"PUERTO_MEMORIA");
	log_protegido_kernel(string_from_format("MEMORIA:Conectando a memoria..."));

	while((socket_memoria = crear_conexion(ip, puerto)) <= 0){
		log_protegido_kernel(string_from_format("No se pudo establecer una conexion con la Memoria"));
        sleep(60);
	}

	int handshake = KERNEL;
	bool confirmacion;

	send(socket_memoria, &handshake, sizeof(int),0);		//no hace handshake
	recv(socket_memoria, &confirmacion, sizeof(bool), MSG_WAITALL);

	
	if(confirmacion)
		log_protegido_kernel(string_from_format("Conexion con memoria exitosa"));
	else
		log_protegido_kernel(string_from_format("ERROR: Handshake con memoria fallido"));	

	/*---------- Hilos para atender conexiones --------------*/
	pthread_t hilo_atender_memoria;
    pthread_create(&hilo_atender_memoria, NULL, (void*) atender_peticiones_memoria, NULL);
	pthread_detach(hilo_atender_memoria);

	pthread_mutex_lock(&mutex_conexiones);
	conexiones++;
	pthread_mutex_unlock(&mutex_conexiones);

return 0;

}

void _recibir_contexto_cpu(t_pcb *pcb, int* motivo, t_instruccion* instruccion){
		log_protegido_kernel(string_from_format("_recibir_contexto_cpu"));

		int size;
		log_protegido_kernel(string_from_format("_recibir_contexto_cpu"));
		void* buffer = recibir_buffer(&size, socket_dispatch);
		log_protegido_kernel(string_from_format("_recibir_contexto_cpu"));
		int desplazamiento = 0;		
		log_protegido_kernel(string_from_format("_recibir_contexto_cpu"));
		//saco el pid y lo copio aproceso_en_exec->pid
		memcpy(&(pcb->pid), buffer + desplazamiento, sizeof(int));
		log_protegido_kernel(string_from_format("_recibir_contexto_cpu"));
		desplazamiento += sizeof(int);
		log_protegido_kernel(string_from_format("_recibir_contexto_cpu"));

		//Registros CPU
		memcpy(&(pcb->registros_cpu.AX), buffer + desplazamiento, sizeof(uint8_t));
		log_protegido_kernel(string_from_format("_recibir_contexto_cpu"));
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
		log_protegido_kernel(string_from_format("_recibir_contexto_cpu"));
		
		memcpy(&(instruccion->identificador ),buffer + desplazamiento, sizeof(int));
		log_protegido_kernel(string_from_format("_recibir_contexto_cpu"));
		desplazamiento += sizeof(int);			
		memcpy(&(instruccion->cantidad_parametros),buffer + desplazamiento, sizeof(int));
		log_protegido_kernel(string_from_format("_recibir_contexto_cpu"));
		desplazamiento += sizeof(int);		
		for (int i=0; i < instruccion->cantidad_parametros; i++){
			char* parametro;
		log_protegido_kernel(string_from_format("_recibir_contexto_cpu"));
			int size_parametro;
			memcpy(&(size_parametro), buffer + desplazamiento, sizeof(int));
			desplazamiento += sizeof(int);
			parametro=malloc(size_parametro);
			memcpy(parametro, buffer + desplazamiento, size_parametro);
			desplazamiento += size_parametro;
		log_protegido_kernel(string_from_format("_recibir_contexto_cpu"));
			list_add(instruccion->parametros, parametro);
		log_protegido_kernel(string_from_format("_recibir_contexto_cpu"));
		}		
		log_protegido_kernel(string_from_format("_recibir_contexto_cpu"));
		free(buffer);	
		log_protegido_kernel(string_from_format("_recibir_contexto_cpu"));
		//saco el motivo
		memcpy(&motivo, buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);
}

// void _finalizar_proceso(t_pcb *pcb, int motivo)
// {
// 	int pid_proceso = pcb->id;
// 	pthread_mutex_lock(&mexit);
// 	queue_push(exit_queue, pcb);
// 	pthread_mutex_unlock(&mexit);
// 	sem_post(&procesos_en_exit);
// 	log_protegido(string_from_format("Finaliza el proceso <%d>- Motivo: <%s>", pid_proceso, get_motivo(motivo)));
// 	sem_post(&nivel_multiprogramacion);
// 	grado_multiprogramacion++;
// }

void _gestionar_peticiones_de_cpu_dispatch(){
	while(1){
		log_protegido_kernel(string_from_format("_gestionar_peticiones_de_cpu_dispatch"));
		int cod_op = recibir_operacion(socket_dispatch);
		log_protegido_kernel(string_from_format("Recibi operacion desde CPU DISPATCH"));
		switch (cod_op) {
			case CONTEXTO_EJECUCION:
				log_protegido_kernel(string_from_format("_gestionar_peticiones_de_cpu_dispatch: CONTEXTO_EJECUCION"));
				int motivo;
				t_instruccion* instrucciones=malloc(sizeof(t_instruccion));
				instrucciones->parametros =list_create();
				_recibir_contexto_cpu(proceso_exec, &motivo, instrucciones);
				switch(motivo){
					case EXIT:
						log_protegido_kernel(string_from_format("PID: <%d> - EXIT", proceso_exec->pid));
					case IO_GEN_SLEEP: 
						log_protegido_kernel("Intruccion IO_GEN_SLEEP"); 
				}
				sleep(80);
				break;
		}
	}
	
}

void _atender_cpu_dispatch(){
	pthread_create(&hilo_cpu_dispatch, NULL, (void*)_gestionar_peticiones_de_cpu_dispatch, NULL);
	pthread_detach(hilo_cpu_dispatch);
}

int conectarCpuDispatch(){
	char* ip 				= config_get_string_value(config_kernel,"IP_CPU");
	char* puerto_dispatch 	= config_get_string_value(config_kernel,"PUERTO_CPU_DISPATCH");

    socket_dispatch = crear_conexion(ip, puerto_dispatch);

    if (socket_dispatch <= 0){
        printf(" DISPATCH: No se pudo establecer una conexion con la CPU\n");
    }
    else{
		log_protegido_kernel(string_from_format("DISPATCH: Conexion con CPU exitosa"));
    }

	int handshake_dispatch = KERNEL_DISPATCH;
	bool confirmacion;
	send(socket_dispatch, &handshake_dispatch, sizeof(int),0); 
	recv(socket_dispatch, &confirmacion, sizeof(bool), MSG_WAITALL);

	if(confirmacion)
		log_protegido_kernel(string_from_format("Conexion de Modulo Dispatch con CPU exitosa"));
	else
		log_protegido_kernel(string_from_format("ERROR: Handshake de Modulo Dispatch con CPU fallido"));
	enviar_mensaje("SOY KERNEL DISPATCH",socket_dispatch);
	
	_atender_cpu_dispatch();

	pthread_mutex_lock(&mutex_conexiones);
	conexiones++;
	pthread_mutex_unlock(&mutex_conexiones);
	
return 0;
}

int conectarCpuInterrupt(){
	char* ip 				= config_get_string_value(config_kernel,"IP_CPU");
	char* puerto_interrupt 	= config_get_string_value(config_kernel,"PUERTO_CPU_INTERRUPT");

    socket_interrupt = crear_conexion(ip, puerto_interrupt);

    if (socket_interrupt <= 0){
        printf(" INTERRUPT: No se pudo establecer una conexion con la CPU\n");
    }
    else{
		log_protegido_kernel(string_from_format("INTERRUPT: Conexion con CPU exitosa"));
    }

	int handshake_interrupt = KERNEL_INTERRUPT;
	bool confirmacion;
	send(socket_interrupt, &handshake_interrupt, sizeof(int),0); 
	recv(socket_interrupt, &confirmacion, sizeof(bool), MSG_WAITALL);

	if(confirmacion)
		log_protegido_kernel(string_from_format("Conexion de Modulo Interrupt con CPU exitosa"));
	else
		log_protegido_kernel(string_from_format("ERROR: Handshake de Modulo Interrupt con CPU fallido"));

	enviar_mensaje("SOY KERNEL INTERRUPT",socket_interrupt);

	pthread_mutex_lock(&mutex_conexiones);
	conexiones++;
	pthread_mutex_unlock(&mutex_conexiones);

	return 0;
}

/*-------------------------------------Consola----------------------------------*/

void leerConsola() {
	printf("conexiones: %d \n", conexiones);
	printf("Arranca leer consona, esperando...\n");
    while(conexiones < 4){/*ESPERA*/}
	leer_consola(m_multiprogramacion);
	printf("Finaliza consola...\n");
}


/*----------------------Memoria---------------------------------------------------*/
void  atender_peticiones_memoria(){
	while(1){
		int cod_op = recibir_operacion(socket_memoria);
		void* buffer_recibido;
		log_protegido_kernel(string_from_format("Recibi operacion desde memoria"));
		switch(cod_op){
			case INICIAR_PROCESO_MEMORIA_OK:
                int size=0;
				log_protegido_kernel(string_from_format("INICIAR_PROCESO_MEMORIA_OK"));
                buffer_recibido = recibir_buffer(&size, socket_memoria);
				sem_post(&s_init_proceso_a_memoria);
				free(buffer_recibido);
				log_protegido_kernel(string_from_format("INICIAR_PROCESO_MEMORIA_OK"));
				break;	
			default:
				exit(EXIT_FAILURE);
				log_warning(logger_kernel, "Operacion no reconocida");
				log_warning(logger_kernel, "el valor de cod_op es: %d", cod_op);
		}
	}
}
