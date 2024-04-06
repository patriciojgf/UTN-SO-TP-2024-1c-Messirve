#include "queues.h"

void log_protegido(char *mensaje){
	// pthread_mutex_lock(&mlog);
	log_info(logger_kernel, "%s", mensaje);
	// pthread_mutex_unlock(&mlog);
	free(mensaje);
}