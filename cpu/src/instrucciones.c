#include "instrucciones.h"

static char* _recibir_instruccion(int socket_cliente);
static u_int8_t _get_identificador(char* identificador_txt);
static char* _get_nombre_instruccion(u_int8_t instruccion);
static u_int8_t _cantidad_parametros(u_int8_t identificador);
static t_list* _parametros_instruccion(char* instruccion, u_int8_t cantidad_parametros);
static void _mostrar_parametros(t_instruccion* instruccion, u_int8_t cantidad_parametros);

extern t_registros_cpu registros_cpu;

void log_protegido_cpu(char* mensaje){
	sem_wait(&mlog);
	log_info(logger_cpu, "%s", mensaje);
	sem_post(&mlog);
	free(mensaje);
}

char* fetch_instruccion(){
	log_protegido_cpu(string_from_format("PID: <%d> - FETCH - Program Counter: <%d>", contexto_cpu->pid, contexto_cpu->program_counter));
	t_paquete* paquete = crear_paquete(FETCH_INSTRUCCION);
	agregar_datos_sin_tamaño_a_paquete(paquete, &contexto_cpu->pid, sizeof(int));
	agregar_datos_sin_tamaño_a_paquete(paquete, &contexto_cpu->program_counter, sizeof(int));
	enviar_paquete(paquete, socket_memoria);
	char* instruccion = _recibir_instruccion(socket_memoria);
	printf("Instruccion: %s\n", instruccion);
	contexto_cpu->program_counter++;
	eliminar_paquete(paquete);
	return instruccion;
}

t_instruccion* decodificar_instruccion(char* instruccion){
	t_instruccion* inst_decodificada = malloc(sizeof(t_instruccion));
	//Se separa la instruccion en un array de strings
	char** instruccion_separada = string_split(instruccion, " ");
	//Se obtiene el identificador de la instruccion
	u_int8_t identificador = _get_identificador(instruccion_separada[0]);

	for (int i=0; i < string_array_size(instruccion_separada);i++){
		free(instruccion_separada[i]);
	}
	free(instruccion_separada);
	printf("Identificador: %d\n", identificador);

	inst_decodificada->identificador = identificador;
	inst_decodificada->cantidad_parametros = _cantidad_parametros(identificador);
	inst_decodificada->parametros = _parametros_instruccion(instruccion, inst_decodificada->cantidad_parametros);
	free(instruccion);
	return inst_decodificada;
}

t_instruccion* execute_instruccion(t_instruccion* instruccion){
	log_info(logger_cpu, "identificador de instruccion: %d", instruccion->identificador);
	switch (instruccion->identificador){
		case SET:
			_mostrar_parametros(instruccion, instruccion->cantidad_parametros);
			// set(instruccion);
			break;
		case MOV_IN:
			_mostrar_parametros(instruccion, instruccion->cantidad_parametros);
			// mov_in(instruccion);
			break;
		case MOV_OUT:
			_mostrar_parametros(instruccion, instruccion->cantidad_parametros);
			// mov_out(instruccion);
			break;
		case SUM:
			_mostrar_parametros(instruccion, instruccion->cantidad_parametros);
			// sum(instruccion);
			break;
		case SUB:
			_mostrar_parametros(instruccion, instruccion->cantidad_parametros);
			// sub(instruccion);
			break;
		case JNZ:
			_mostrar_parametros(instruccion, instruccion->cantidad_parametros);
			// jnz(instruccion);
			break;
		case RESIZE:
			_mostrar_parametros(instruccion, instruccion->cantidad_parametros);
			// resize(instruccion);
			break;
		case COPY_STRING:
			_mostrar_parametros(instruccion, instruccion->cantidad_parametros);
			// copy_string(instruccion);
			break;
		case WAIT:
			_mostrar_parametros(instruccion, instruccion->cantidad_parametros);
			// wait(instruccion);
			break;
		case SIGNAL:
			_mostrar_parametros(instruccion, instruccion->cantidad_parametros);
			// signal(instruccion);
			break;
		case IO_GEN_SLEEP:
			_mostrar_parametros(instruccion, instruccion->cantidad_parametros);
			f_io_gen_sleep(instruccion);
			break;
		case IO_STDIN_READ:
			_mostrar_parametros(instruccion, instruccion->cantidad_parametros);
			// io_stdin_read(instruccion);
			break;
		case IO_STDOUT_WRITE:
			_mostrar_parametros(instruccion, instruccion->cantidad_parametros);
			// io_stdout_write(instruccion);
			break;
		case IO_FS_CREATE:
			_mostrar_parametros(instruccion, instruccion->cantidad_parametros);
			// io_fs_create(instruccion);
			break;
		case IO_FS_DELETE:
			_mostrar_parametros(instruccion, instruccion->cantidad_parametros);
			// io_fs_delete(instruccion);
			break;
		case IO_FS_TRUNCATE:
			_mostrar_parametros(instruccion, instruccion->cantidad_parametros);
			// io_fs_truncate(instruccion);
			break;
		case IO_FS_WRITE:
			_mostrar_parametros(instruccion, instruccion->cantidad_parametros);
			// io_fs_write(instruccion);
			break;
		case IO_FS_READ:
			_mostrar_parametros(instruccion, instruccion->cantidad_parametros);
			// io_fs_read(instruccion);
			break;
		case EXIT:
			_mostrar_parametros(instruccion, instruccion->cantidad_parametros);
			f_exit();
			break;
		default:
			log_protegido_cpu("Instruccion desconocida. ");
			break;
	}
	return instruccion;
}

static char* _recibir_instruccion(int socket_cliente){
	int size=0;
	recibir_operacion(socket_cliente);
	return recibir_buffer(&size, socket_cliente);
}

static u_int8_t _get_identificador(char* identificador_txt){
	u_int8_t identificador = NO_RECONOCIDO;
	if(strcmp(identificador_txt, "SET") == 0){
		identificador = SET;
	}else if(strcmp(identificador_txt, "MOV_IN") == 0){
		identificador = MOV_IN;
	}else if(strcmp(identificador_txt, "MOV_OUT") == 0){
		identificador = MOV_OUT;
	}else if(strcmp(identificador_txt, "SUM") == 0){
		identificador = SUM;
	}else if(strcmp(identificador_txt, "SUB") == 0){
		identificador = SUB;
	}else if(strcmp(identificador_txt, "JNZ") == 0){
		identificador = JNZ;
	}else if(strcmp(identificador_txt, "RESIZE") == 0){
		identificador = RESIZE;
	}else if(strcmp(identificador_txt, "COPY_STRING") == 0){
		identificador = COPY_STRING;
	}else if(strcmp(identificador_txt, "WAIT") == 0){
		identificador = WAIT;
	}else if(strcmp(identificador_txt, "SIGNAL") == 0){
		identificador = SIGNAL;
	}else if(strcmp(identificador_txt, "IO_GEN_SLEEP") == 0){
		identificador = IO_GEN_SLEEP;
	}else if(strcmp(identificador_txt, "IO_STDIN_READ") == 0){
		identificador = IO_STDIN_READ;
	}else if(strcmp(identificador_txt, "IO_STDOUT_WRITE") == 0){
		identificador = IO_STDOUT_WRITE;
	}else if(strcmp(identificador_txt, "IO_FS_CREATE") == 0){
		identificador = IO_FS_CREATE;
	}else if(strcmp(identificador_txt, "IO_FS_DELETE") == 0){
		identificador = IO_FS_DELETE;
	}else if(strcmp(identificador_txt, "IO_FS_TRUNCATE") == 0){
		identificador = IO_FS_TRUNCATE;
	}else if(strcmp(identificador_txt, "IO_FS_WRITE") == 0){
		identificador = IO_FS_WRITE;
	}else if(strcmp(identificador_txt, "IO_FS_READ") == 0){
		identificador = IO_FS_READ;
	}else if(strcmp(identificador_txt, "EXIT") == 0){
		identificador = EXIT;
	}
	return identificador;
}

static char* _get_nombre_instruccion(u_int8_t instruccion){
	char* nombre = string_new();
	switch(instruccion){
		case SET:
			string_append(&nombre, "SET");
			break;
		case MOV_IN:
			string_append(&nombre, "MOV_IN");
			break;
		case MOV_OUT:
			string_append(&nombre, "MOV_OUT");
			break;
		case SUM:
			string_append(&nombre, "SUM");
			break;
		case SUB:
			string_append(&nombre, "SUB");
			break;
		case JNZ:
			string_append(&nombre, "JNZ");
			break;
		case RESIZE:
			string_append(&nombre, "RESIZE");
			break;
		case COPY_STRING:
			string_append(&nombre, "COPY_STRING");
			break;
		case WAIT:
			string_append(&nombre, "WAIT");
			break;
		case SIGNAL:
			string_append(&nombre, "SIGNAL");
			break;
		case IO_GEN_SLEEP:
			string_append(&nombre, "IO_GEN_SLEEP");
			break;
		case IO_STDIN_READ:
			string_append(&nombre, "IO_STDIN_READ");
			break;
		case IO_STDOUT_WRITE:
			string_append(&nombre, "IO_STDOUT_WRITE");
			break;
		case IO_FS_CREATE:
			string_append(&nombre, "IO_FS_CREATE");
			break;
		case IO_FS_DELETE:
			string_append(&nombre, "IO_FS_DELETE");
			break;
		case IO_FS_TRUNCATE:
			string_append(&nombre, "IO_FS_TRUNCATE");
			break;
		case IO_FS_WRITE:
			string_append(&nombre, "IO_FS_WRITE");
			break;
		case IO_FS_READ:
			string_append(&nombre, "IO_FS_READ");
			break;
		case EXIT:
			string_append(&nombre, "EXIT");
			break;
	}
	return nombre;
}

static u_int8_t _cantidad_parametros(u_int8_t identificador){
	u_int8_t cantidad = 0;
	switch(identificador){
		case SET:
			cantidad = 2;
			break;
		case MOV_IN:
			cantidad = 2;
			break;
		case MOV_OUT:
			cantidad = 2;
			break;
		case SUM:
			cantidad = 2;
			break;
		case SUB:
			cantidad = 2;
			break;
		case JNZ:
			cantidad = 2;
			break;
		case RESIZE:
			cantidad = 1;
			break;
		case COPY_STRING:
			cantidad = 1;
			break;
		case WAIT:
			cantidad = 1;
			break;
		case SIGNAL:
			cantidad = 1;
			break;
		case IO_GEN_SLEEP:
			cantidad = 2;
			break;
		case IO_STDIN_READ:
			cantidad = 3;
			break;
		case IO_STDOUT_WRITE:
			cantidad = 3;
			break;
		case IO_FS_CREATE:
			cantidad = 2;
			break;
		case IO_FS_DELETE:
			cantidad = 2;
			break;
		case IO_FS_TRUNCATE:
			cantidad = 3;
			break;
		case IO_FS_WRITE:
			cantidad = 5;
			break;
		case IO_FS_READ:
			cantidad = 5;
			break;
		case EXIT:
			cantidad = 0;
			break;
	}
	return cantidad;
} 

static t_list* _parametros_instruccion(char* instruccion, u_int8_t cantidad_parametros){
	t_list* parametros = list_create();
	char** vector_parametros = string_split(instruccion, " ");
	
	//Se agrega cada parametro a la lista
	for(int i = 1; i <= cantidad_parametros; i++){
		char* parametro = malloc(strlen(vector_parametros[i])+1);
		strcpy(parametro, vector_parametros[i]);
		list_add(parametros, parametro);
	}

	//Se libera el vector de parametros
	for(int i = 0; i < string_array_size(vector_parametros); i++){
		free(vector_parametros[i]);
	}
	free(vector_parametros);

	return parametros;
}

static void _mostrar_parametros(t_instruccion* instruccion, u_int8_t cantidad_parametros){
	char* nombre_instruccion = _get_nombre_instruccion(instruccion->identificador);
	// if(cantidad_parametros == 0){
	// 	char* mensaje_log = string_from_format("PID: <%d> - Ejecutando: <%s>",pid,nombre_instruccion);
	// 	log_protegido_cpu(mensaje_log);
	// 	free(mensaje_log);
	// }else if(cantidad_parametros == 1){
	// 	char* mensaje_log = string_from_format("PID: <%d> - Ejecutando: <%s, %s>",pid,nombre_instruccion,list_get(instruccion->parametros,0));
	// 	log_protegido_cpu(mensaje_log);
	// 	free(mensaje_log);		
	// }else if(cantidad_parametros == 2){
	// 	char* mensaje_log = string_from_format("PID: <%d> - Ejecutando: <%s, %s, %s>",pid,nombre_instruccion,list_get(instruccion->parametros,0),list_get(instruccion->parametros,1));
	// 	log_protegido_cpu(mensaje_log);
	// 	free(mensaje_log);
	// }else if(cantidad_parametros == 3){
	// 	char* mensaje_log = string_from_format("PID: <%d> - Ejecutando: <%s, %s, %s, %s>",pid,nombre_instruccion,list_get(instruccion->parametros,0),list_get(instruccion->parametros,1),list_get(instruccion->parametros,2));
	// 	log_protegido_cpu(mensaje_log);
	// 	free(mensaje_log);
	// }else if(cantidad_parametros == 4){
	// 	char* mensaje_log = string_from_format("PID: <%d> - Ejecutando: <%s, %s, %s, %s, %s>",pid,nombre_instruccion,list_get(instruccion->parametros,0),list_get(instruccion->parametros,1),list_get(instruccion->parametros,2),list_get(instruccion->parametros,3));
	// 	log_protegido_cpu(mensaje_log);
	// 	free(mensaje_log);
	// }else if(cantidad_parametros == 5){
	// 	char* mensaje_log = string_from_format("PID: <%d> - Ejecutando: <%s, %s, %s, %s, %s, %s>",pid,nombre_instruccion,list_get(instruccion->parametros,0),list_get(instruccion->parametros,1),list_get(instruccion->parametros,2),list_get(instruccion->parametros,3),list_get(instruccion->parametros,4));
	// 	log_protegido_cpu(mensaje_log);
	// 	free(mensaje_log);
	// }
	// free(nombre_instruccion);
	
	// Prepara el mensaje con el nombre de la instrucción
    char* mensaje_log = string_from_format("PID: <%d> - Ejecutando: <%s", pid, nombre_instruccion);
    
    // Agrega los parámetros al mensaje
    for (int i = 0; i < cantidad_parametros; i++) {
        char* parametro = list_get(instruccion->parametros, i);
        mensaje_log = string_from_format("%s, %s", mensaje_log, parametro);
    }
    
    // Cierra el mensaje
    mensaje_log = string_from_format("%s>", mensaje_log);
    
    // Loggea el mensaje
    log_protegido_cpu(mensaje_log);
    
	log_warning(logger_cpu, "Necesito hacer un free aca?");
}

void devolver_contexto(int motivo, t_instruccion* instruccion){
	t_paquete* paquete = crear_paquete(CONTEXTO_EJECUCION);
	empaquetar_contexto_cpu(paquete, instruccion, pid,registros_cpu);
	enviar_paquete(paquete, socket_servidor_dispatch);
	eliminar_paquete(paquete);
}

void f_exit(){ 
    flag_ejecucion = false;
    log_warning(logger_cpu, "Falta implementar devolver_contexto");
	//devolver_contexto(EXIT, inst);
}

void f_io_gen_sleep(t_instruccion* instruccion)
{
	log_protegido_cpu("Proximamente hace su magia"); 
	
	log_warning(logger_cpu, "cantidad de parametros: %d", list_size(instruccion->parametros));
	log_warning(logger_cpu, "parametro 1: %p", list_get(instruccion->parametros, 0));
	log_warning(logger_cpu, "parametro 2: %p", list_get(instruccion->parametros, 1));
}