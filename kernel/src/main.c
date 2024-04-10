#include "main.h"


// pthread_mutex_t mlog, mconexiones;

int conexiones;
int id_counter = 1;//PID DE LOS PROCESOS

/* -------------------------------------Iniciar Kernel -----------------------------------------------*/
int main(int argc, char **argv) {
	// sem_init(&planificadores,0,0);
	config_kernel = iniciar_config(argv[1]);
	logger_kernel=iniciar_logger("kernel.log","KERNEL");
	// pthread_mutex_init(&mconexiones,NULL);
	conexiones=0;

	pthread_t t1,t2,t3,t4,t5;
	// inicializar_estructuras();

    pthread_create(&t1, NULL, (void*) conectarMemoria, NULL);
	pthread_create(&t2, NULL, (void*) conectarCpuDispatch, NULL);
	pthread_create(&t3, NULL, (void*) conectarCpuInterrupt, NULL);
    // pthread_create(&t4, NULL, (void*) conectarFS, NULL);
	// pthread_create(&t5, NULL, (void*) leer_consola, NULL);



	// iniciar_hilos_estructuras();
	// pthread_join(t5, NULL);

	// free_estructuras();

	// log_destroy(logger_kernel);
 	// config_destroy(config_kernel);
	
	// liberar_conexion(socket_FS);

	//pthread_detach(t1);
	pthread_join(t1, NULL); //patricio:agrego esta linea momentaneamente para darle tiempo a que se conecte a memoria
	pthread_join(t2, NULL);
	pthread_join(t3, NULL);
	liberar_conexion(socket_memoria);
	liberar_conexion(socket_dispatch);
	liberar_conexion(socket_interrupt);
	printf("Hilo 1\n");
	// pthread_detach(t2);
    // pthread_detach(t3);
	// pthread_detach(t4);
	// sem_destroy(&planificadores);
    return 0;
}


/* ------------------------------------Conexion Mermoria --------------------------------------------*/
int conectarMemoria(){
	socket_memoria = -1;
	char* ip;
	char* puerto;

    ip= config_get_string_value(config_kernel,"IP_MEMORIA");
	puerto=config_get_string_value(config_kernel,"PUERTO_MEMORIA");
	log_protegido(string_from_format("MEMORIA:Conectando a memoria..."));

	while((socket_memoria = crear_conexion(ip, puerto)) <= 0){
		log_protegido(string_from_format("No se pudo establecer una conexion con la Memoria"));
        sleep(60);
	}

	int handshake = KERNEL;
	bool confirmacion;

	send(socket_memoria, &handshake, sizeof(int),0);		//no hace handshake
	recv(socket_memoria, &confirmacion, sizeof(bool), MSG_WAITALL);

	
	if(confirmacion)
		log_protegido(string_from_format("Conexion con memoria exitosa"));
	else
		log_protegido(string_from_format("ERROR: Handshake con memoria fallido"));

	// pthread_mutex_lock(&mconexiones);
	conexiones++;
	// pthread_mutex_unlock(&mconexiones);

return 0;

}


/*Conexiones*/
/* ------------------------------------Conexion CPU --------------------------------------------------*/
int conectarCpuDispatch(){
	char* ip 				= config_get_string_value(config_kernel,"IP_CPU");
	char* puerto_dispatch 	= config_get_string_value(config_kernel,"PUERTO_CPU_DISPATCH");

    socket_dispatch = crear_conexion(ip, puerto_dispatch);

    if (socket_dispatch <= 0){
        printf(" DISPATCH: No se pudo establecer una conexion con la CPU\n");
    }
    else{
		log_protegido(string_from_format("DISPATCH: Conexion con CPU exitosa"));
    }

	int handshake_dispatch = KERNEL_DISPATCH;
	bool confirmacion;
	send(socket_dispatch, &handshake_dispatch, sizeof(int),0); 
	recv(socket_dispatch, &confirmacion, sizeof(bool), MSG_WAITALL);

	if(confirmacion)
		log_protegido(string_from_format("Conexion de Modulo Dispatch con CPU exitosa"));
	else
		log_protegido(string_from_format("ERROR: Handshake de Modulo Dispatch con CPU fallido"));

	enviar_mensaje("SOY KERNEL DISPATCH",socket_dispatch);
	// pthread_mutex_lock(&mconexiones);
	conexiones++;
	// pthread_mutex_unlock(&mconexiones);
	
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
		log_protegido(string_from_format("INTERRUPT: Conexion con CPU exitosa"));
    }

	int handshake_interrupt = KERNEL_INTERRUPT;
	bool confirmacion;
	send(socket_interrupt, &handshake_interrupt, sizeof(int),0); 
	recv(socket_interrupt, &confirmacion, sizeof(bool), MSG_WAITALL);

	if(confirmacion)
		log_protegido(string_from_format("Conexion de Modulo Interrupt con CPU exitosa"));
	else
		log_protegido(string_from_format("ERROR: Handshake de Modulo Interrupt con CPU fallido"));

	enviar_mensaje("SOY KERNEL INTERRUPT",socket_interrupt);
	// pthread_mutex_lock(&mconexiones);
	conexiones++;
	// pthread_mutex_unlock(&mconexiones);
	return 0;
}