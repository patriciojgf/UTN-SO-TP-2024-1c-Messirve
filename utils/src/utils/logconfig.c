#include "logconfig.h"

t_log* iniciar_logger(char* nombreLog, char* proceso){

	t_log* nuevo_logger= log_create(nombreLog,proceso,1,LOG_LEVEL_INFO);
	if(nuevo_logger == NULL){
		printf("No pude crear el logger.\n");}
	return nuevo_logger;
}

t_config* iniciar_config(char* archivo){
	t_config* nuevo_config= config_create(archivo);
	if(nuevo_config == NULL){
		printf("No pude leer la config.\n");
		exit(2);
	}
	return nuevo_config;
}

void finalizar_log(t_log* log){
	if(log == NULL){
		error_show("Error al finalizar log.");
	}
	log_destroy(log);
}

void finalizar_config(t_config* config){
	if(config  == NULL){
		error_show("Error al finalizar config.");
	}
	config_destroy(config);
}