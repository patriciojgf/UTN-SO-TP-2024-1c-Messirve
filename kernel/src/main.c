#include <main.h>

/* -------------------------------------Iniciar Kernel -----------------------------------------------*/
int main(int argc, char **argv) {

	/*--------- Cargo configuraciones y log--------*/
	init_kernel(argv[1]);

	/*--------- Inicio conexiones y estructuras--------*/
	init_conexiones();
	conexiones=0;

	/*---------- Hilos para planificadores --------------*/
	gestionar_conexion_memoria();
	log_info(logger_kernel,"[main IO]: gestionar_conexion_memoria");
	gestionar_conexion_interrupt();
	log_info(logger_kernel,"[main IO]: gestionar_conexion_interrupt");
	gestionar_conexion_dispatch();
	log_info(logger_kernel,"[main IO]: gestionar_conexion_dispatch");

	pthread_t hilo_gestionar_conexion_io;
	pthread_create(&hilo_gestionar_conexion_io, NULL, (void*) gestionar_conexion_io, NULL);
	pthread_detach(hilo_gestionar_conexion_io);

	
	log_info(logger_kernel,"[main IO]: leerConsola");
	pthread_t t5;
	pthread_create(&t5, NULL, (void*) leerConsola, NULL);
	pthread_join(t5, NULL);


    return 0;
}



/* ------------------------------------Conexiones--------------------------------------------*/
// int conectarMemoria(){
// 	socket_memoria = -1;
// 	char* ip;
// 	char* puerto;

//     ip= config_get_string_value(config_kernel,"IP_MEMORIA");
// 	puerto=config_get_string_value(config_kernel,"PUERTO_MEMORIA");
// 	log_info(logger_kernel,"MEMORIA:Conectando a memoria...");

// 	while((socket_memoria = crear_conexion(ip, puerto)) <= 0){
// 		log_info(logger_kernel,"No se pudo establecer una conexion con la Memoria");
// 	}

// 	int handshake = KERNEL;
// 	bool confirmacion;

// 	send(socket_memoria, &handshake, sizeof(int),0);		//no hace handshake
// 	recv(socket_memoria, &confirmacion, sizeof(bool), MSG_WAITALL);

	
// 	if(confirmacion)
// 		log_info(logger_kernel,"Conexion con memoria exitosa");
// 	else
// 		log_protegido_kernel(string_from_format("ERROR: Handshake con memoria fallido"));	

// 	/*---------- Hilos para atender conexiones --------------*/
// 	//pthread_t hilo_atender_memoria;
//     // pthread_create(&hilo_atender_memoria, NULL, (void*) atender_peticiones_memoria, NULL);
// 	// pthread_detach(hilo_atender_memoria);

// 	pthread_mutex_lock(&mutex_conexiones);
// 	conexiones++;
// 	pthread_mutex_unlock(&mutex_conexiones);

// return 0;

// }


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

// void _gestionar_peticiones_de_cpu_dispatch(){
// 	while(1){
// 		log_protegido_kernel(string_from_format("_gestionar_peticiones_de_cpu_dispatch"));
// 		int cod_op = recibir_operacion(socket_dispatch);
// 		log_protegido_kernel(string_from_format("Recibi operacion desde CPU DISPATCH"));
// 		switch (cod_op) {
// 			case CONTEXTO_EJECUCION:
// 				log_protegido_kernel(string_from_format("_gestionar_peticiones_de_cpu_dispatch: CONTEXTO_EJECUCION"));
// 				int motivo;
// 				t_instruccion* instrucciones=malloc(sizeof(t_instruccion));
// 				instrucciones->parametros =list_create();
// 				_recibir_contexto_cpu(proceso_exec, &motivo, instrucciones);
// 				switch(motivo){
// 					case EXIT:
// 						log_protegido_kernel(string_from_format("PID: <%d> - EXIT", proceso_exec->pid));
// 				}
// 				sleep(80);
// 				break;
// 		}
// 	}
	
// }

// void _atender_cpu_dispatch(){
// 	pthread_create(&hilo_cpu_dispatch, NULL, (void*)_gestionar_peticiones_de_cpu_dispatch, NULL);
// 	pthread_detach(hilo_cpu_dispatch);
// }

// int conectarCpuDispatch(){
// 	char* ip 				= config_get_string_value(config_kernel,"IP_CPU");
// 	char* puerto_dispatch 	= config_get_string_value(config_kernel,"PUERTO_CPU_DISPATCH");

//     socket_dispatch = crear_conexion(ip, puerto_dispatch);

//     if (socket_dispatch <= 0){
//         printf(" DISPATCH: No se pudo establecer una conexion con la CPU\n");
//     }
//     else{
// 		log_protegido_kernel(string_from_format("DISPATCH: Conexion con CPU exitosa"));
//     }

// 	int handshake_dispatch = KERNEL_DISPATCH;
// 	bool confirmacion;
// 	send(socket_dispatch, &handshake_dispatch, sizeof(int),0); 
// 	recv(socket_dispatch, &confirmacion, sizeof(bool), MSG_WAITALL);

// 	if(confirmacion)
// 		log_protegido_kernel(string_from_format("Conexion de Modulo Dispatch con CPU exitosa"));
// 	else
// 		log_protegido_kernel(string_from_format("ERROR: Handshake de Modulo Dispatch con CPU fallido"));
// 	enviar_mensaje("SOY KERNEL DISPATCH",socket_dispatch);
	
// 	_atender_cpu_dispatch();

// 	pthread_mutex_lock(&mutex_conexiones);
// 	conexiones++;
// 	pthread_mutex_unlock(&mutex_conexiones);
	
// return 0;
// }

// int conectarCpuInterrupt(){
// 	char* ip 				= config_get_string_value(config_kernel,"IP_CPU");
// 	char* puerto_interrupt 	= config_get_string_value(config_kernel,"PUERTO_CPU_INTERRUPT");

//     socket_interrupt = crear_conexion(ip, puerto_interrupt);

//     if (socket_interrupt <= 0){
//         printf(" INTERRUPT: No se pudo establecer una conexion con la CPU\n");
//     }
//     else{
// 		log_protegido_kernel(string_from_format("INTERRUPT: Conexion con CPU exitosa"));
//     }

// 	int handshake_interrupt = KERNEL_INTERRUPT;
// 	bool confirmacion;
// 	send(socket_interrupt, &handshake_interrupt, sizeof(int),0); 
// 	recv(socket_interrupt, &confirmacion, sizeof(bool), MSG_WAITALL);

// 	if(confirmacion)
// 		log_protegido_kernel(string_from_format("Conexion de Modulo Interrupt con CPU exitosa"));
// 	else
// 		log_protegido_kernel(string_from_format("ERROR: Handshake de Modulo Interrupt con CPU fallido"));

// 	enviar_mensaje("SOY KERNEL INTERRUPT",socket_interrupt);

// 	pthread_mutex_lock(&mutex_conexiones);
// 	conexiones++;
// 	pthread_mutex_unlock(&mutex_conexiones);

// 	return 0;
// }

/*-------------------------------------Consola----------------------------------*/

void leerConsola() {
	log_info(logger_kernel,"Esperando Cliente...");
	
    // while(conexiones < 4){/*ESPERA*/}

	sem_wait(&s_conexion_memoria_ok);
	sem_wait(&s_conexion_cpu_i_ok);
	sem_wait(&s_conexion_cpu_d_ok);
	sem_wait(&s_conexion_interfaz);

	// leer_consola(m_multiprogramacion);
	procesar_comandos_consola();
	printf("Finaliza consola...\n");
}



