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
	gestionar_conexion_interrupt();
	gestionar_conexion_dispatch();

	pthread_t hilo_gestionar_conexion_io;
	pthread_create(&hilo_gestionar_conexion_io, NULL, (void*) gestionar_conexion_io, NULL);
	pthread_detach(hilo_gestionar_conexion_io);

	pthread_t hilo_planificador;
	pthread_create(&hilo_planificador, NULL, (void*)planificador_lp_new_ready, NULL);
	pthread_detach(hilo_planificador);

	//ver si esta bien
	pthread_t hilo_planificador_cp; //ver si esta bien aca
	pthread_create(&hilo_planificador_cp, NULL, (void*)planificador_cp, NULL);
	pthread_detach(hilo_planificador_cp);       	

	
	pthread_t t5;
	pthread_create(&t5, NULL, (void*) leerConsola, NULL);
	pthread_join(t5, NULL);


    return 0;
}


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



