#include "instrucciones.h"

//static char* _recibir_instruccion(int socket_cliente);
static u_int8_t _get_identificador(char* identificador_txt);
static char* _get_nombre_instruccion(u_int8_t instruccion);
static u_int8_t _cantidad_parametros(u_int8_t identificador);
static t_list* _parametros_instruccion(char* instruccion, u_int8_t cantidad_parametros);
static void _mostrar_parametros(t_instruccion* instruccion, u_int8_t cantidad_parametros);
//instrucciones
static void _set(t_instruccion* instruccion);
static void _sum(t_instruccion* instruccion);
static void _sub(t_instruccion* instruccion);
static void _jnz(t_instruccion* instruccion);
static void _f_exit(t_instruccion *inst);
static info_registro_cpu _get_direccion_registro(char* string_registro);
//


//----
extern t_registros_cpu registros_cpu;

//---------------------------------------------------------------------------------------------------------------------//

static info_registro_cpu _get_direccion_registro(char* string_registro) {
    info_registro_cpu info = {0, 0};
    if (!strcmp(string_registro, "AX")) {
        info.direccion = &(contexto_cpu->registros_cpu.AX);
        info.tamano = sizeof(uint8_t);
    } else if (!strcmp(string_registro, "BX")) {
        info.direccion = &(contexto_cpu->registros_cpu.BX);
        info.tamano = sizeof(uint8_t);
    } else if (!strcmp(string_registro, "CX")) {
        info.direccion = &(contexto_cpu->registros_cpu.CX);
        info.tamano = sizeof(uint8_t);
    } else if (!strcmp(string_registro, "DX")) {
        info.direccion = &(contexto_cpu->registros_cpu.DX);
        info.tamano = sizeof(uint8_t);
    } else if (!strcmp(string_registro, "EAX")) {
        info.direccion = &(contexto_cpu->registros_cpu.EAX);
        info.tamano = sizeof(uint32_t);
    } else if (!strcmp(string_registro, "EBX")) {
        info.direccion = &(contexto_cpu->registros_cpu.EBX);
        info.tamano = sizeof(uint32_t);
    } else if (!strcmp(string_registro, "ECX")) {
        info.direccion = &(contexto_cpu->registros_cpu.ECX);
        info.tamano = sizeof(uint32_t);
    } else if (!strcmp(string_registro, "EDX")) {
        info.direccion = &(contexto_cpu->registros_cpu.EDX);
        info.tamano = sizeof(uint32_t);
    } else if (!strcmp(string_registro, "SI")) {
        info.direccion = &(contexto_cpu->registros_cpu.SI);
        info.tamano = sizeof(uint32_t);
    } else if (!strcmp(string_registro, "DI")) {
        info.direccion = &(contexto_cpu->registros_cpu.DI);
        info.tamano = sizeof(uint32_t);
    }
    return info;
}

char* recibir_instruccion(int socket_cliente){
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
	//log_protegido_cpu("_mostrar_parametros ");
	char* nombre_instruccion = _get_nombre_instruccion(instruccion->identificador);
	// if(cantidad_parametros == 0){
	// 	char* mensaje_log = string_from_format("PID: <%d> - Ejecutando: <%s>",pid,nombre_instruccion);
	// 	//log_protegido_cpu(mensaje_log);
	// 	free(mensaje_log);
	// }else if(cantidad_parametros == 1){
	// 	char* mensaje_log = string_from_format("PID: <%d> - Ejecutando: <%s, %s>",pid,nombre_instruccion,list_get(instruccion->parametros,0));
	// 	//log_protegido_cpu(mensaje_log);
	// 	free(mensaje_log);		
	// }else if(cantidad_parametros == 2){
	// 	char* mensaje_log = string_from_format("PID: <%d> - Ejecutando: <%s, %s, %s>",pid,nombre_instruccion,list_get(instruccion->parametros,0),list_get(instruccion->parametros,1));
	// 	//log_protegido_cpu(mensaje_log);
	// 	free(mensaje_log);
	// }else if(cantidad_parametros == 3){
	// 	char* mensaje_log = string_from_format("PID: <%d> - Ejecutando: <%s, %s, %s, %s>",pid,nombre_instruccion,list_get(instruccion->parametros,0),list_get(instruccion->parametros,1),list_get(instruccion->parametros,2));
	// 	//log_protegido_cpu(mensaje_log);
	// 	free(mensaje_log);
	// }else if(cantidad_parametros == 4){
	// 	char* mensaje_log = string_from_format("PID: <%d> - Ejecutando: <%s, %s, %s, %s, %s>",pid,nombre_instruccion,list_get(instruccion->parametros,0),list_get(instruccion->parametros,1),list_get(instruccion->parametros,2),list_get(instruccion->parametros,3));
	// 	//log_protegido_cpu(mensaje_log);
	// 	free(mensaje_log);
	// }else if(cantidad_parametros == 5){
	// 	char* mensaje_log = string_from_format("PID: <%d> - Ejecutando: <%s, %s, %s, %s, %s, %s>",pid,nombre_instruccion,list_get(instruccion->parametros,0),list_get(instruccion->parametros,1),list_get(instruccion->parametros,2),list_get(instruccion->parametros,3),list_get(instruccion->parametros,4));
	// 	//log_protegido_cpu(mensaje_log);
	// 	free(mensaje_log);
	// }
	// free(nombre_instruccion);
	
	// Prepara el mensaje con el nombre de la instrucción
    char* mensaje_log = string_from_format("PID: <%d> - Ejecutando: <%s", contexto_cpu->pid, nombre_instruccion);
    
    // Agrega los parámetros al mensaje
    for (int i = 0; i < cantidad_parametros; i++) {
        char* parametro = list_get(instruccion->parametros, i);
        mensaje_log = string_from_format("%s, %s", mensaje_log, parametro);
    }
    
    // Cierra el mensaje
    mensaje_log = string_from_format("%s>", mensaje_log);
    
    // Loggea el mensaje
    //log_protegido_cpu(mensaje_log);
    
	log_warning(logger_cpu, "Necesito hacer un free aca?");
}

void devolver_contexto_a_dispatch(int motivo, t_instruccion* instruccion){
	log_info(logger_cpu,"[DEVUELVO CONTEXTO]: -- PCB -- PID <%d> - PC<%d>",contexto_cpu->pid,contexto_cpu->program_counter);                
	//log_protegido_cpu(string_from_format("[devolver_contexto_a_dispatch]: ---- 1 ----"));
	t_paquete* paquete = crear_paquete(CONTEXTO_EJECUCION);
	agregar_datos_sin_tamaño_a_paquete(paquete, &contexto_cpu->pid, sizeof(int));
	agregar_datos_sin_tamaño_a_paquete(paquete, &contexto_cpu->program_counter, sizeof(int));
	//agrego registros_cpu al paquete
	empaquetar_registros_cpu(paquete, contexto_cpu->registros_cpu);
	empaquetar_instruccion_cpu(paquete, instruccion);
	//agrego motivo
	agregar_datos_sin_tamaño_a_paquete(paquete, &motivo, sizeof(int));
	enviar_paquete(paquete, socket_cliente_dispatch);
	eliminar_paquete(paquete);
	//free(contexto_cpu);
}


/*------------_INSTRUCCIONES---------------------------------------*/

//esto se hace directamente en el case del exe
// static void _io_gen_sleep(t_instruccion* instruccion)
// {
// 	devolver_contexto_a_dispatch(IO_GEN_SLEEP, instruccion);
//     flag_ejecucion = false;
// }

static void _set(t_instruccion* instruccion){
    char* nombre_registro = list_get(instruccion->parametros, 0);
    char* valor_string = list_get(instruccion->parametros, 1);
    uint32_t nuevo_valor = (uint32_t)atoi(valor_string); // Convierte el valor string a uint32_t una sola vez

    info_registro_cpu reg_info = _get_direccion_registro(nombre_registro);

    // Mostrar el valor actual del registro antes de la modificación
    if (reg_info.tamano == sizeof(uint8_t)) {
        *(uint8_t*)reg_info.direccion = (uint8_t)nuevo_valor; // Asigna el nuevo valor
    } else if (reg_info.tamano == sizeof(uint32_t)) {
        *(uint32_t*)reg_info.direccion = nuevo_valor; // Asigna el nuevo valor
    }
}


static void _sum(t_instruccion* instruccion){
    char *registro_destino = list_get(instruccion->parametros, 0);
    char *registro_origen = list_get(instruccion->parametros, 1);
    info_registro_cpu registro_des_info = _get_direccion_registro(registro_destino);
    info_registro_cpu registro_ori_info = _get_direccion_registro(registro_origen);

    // Tratar todo como uint8_t para simplificar
    uint8_t valor_destino = *(uint8_t*)registro_des_info.direccion;
    uint8_t valor_origen = *(uint8_t*)registro_ori_info.direccion;

    uint8_t resultado = valor_destino + valor_origen; // Suma como uint8_t

    // Asignar el resultado al registro de destino
    if (registro_des_info.tamano == sizeof(uint32_t)) {
        *(uint32_t*)registro_des_info.direccion = resultado; // Promoción automática a uint32_t
    } else {
        *(uint8_t*)registro_des_info.direccion = resultado;
    }
}

static void _sub(t_instruccion* instruccion){
    char *registro_destino = list_get(instruccion->parametros, 0);
    char *registro_origen = list_get(instruccion->parametros, 1);
    info_registro_cpu registro_des_info = _get_direccion_registro(registro_destino);
    info_registro_cpu registro_ori_info = _get_direccion_registro(registro_origen);
    // Tratar todo como uint8_t para simplificar
    uint8_t valor_destino = *(uint8_t*)registro_des_info.direccion;
    uint8_t valor_origen = *(uint8_t*)registro_ori_info.direccion;
    uint8_t resultado = valor_destino - valor_origen; // Resta como uint8_t
    // Asignar el resultado al registro de destino
    if (registro_des_info.tamano == sizeof(uint32_t)) {
        *(uint32_t*)registro_des_info.direccion = resultado; // Promoción automática a uint32_t
    } else {
        *(uint8_t*)registro_des_info.direccion = resultado;
    }			 
}

static void _jnz(t_instruccion* instruccion){
    char* registro_nombre = (char*)list_get(instruccion->parametros, 0);  // Asumiendo que esto devuelve un nombre de registro en forma de cadena
    info_registro_cpu dir_registro_info = _get_direccion_registro(registro_nombre);
    uint32_t registro_valor = *(uint32_t*)dir_registro_info.direccion;
    if (registro_valor != 0) {
        uint32_t nueva_pc = atoi(list_get(instruccion->parametros, 1));
        contexto_cpu->program_counter = nueva_pc;
        contexto_cpu->registros_cpu.PC = nueva_pc;
    }
}

static void _mov_int(t_instruccion* instruccion)
{
	log_info(logger_cpu, "[CPU] Se lee la instrucción MOV IN");
}

static void _mov_out(t_instruccion* instruccion)
{
	log_info(logger_cpu, "[CPU] Se lee la instrucción MOV OUT");
}

static void _resize(t_instruccion* instruccion)
{
	log_info(logger_cpu, "[CPU] Se lee la instrucción RESIZE");
}

static void _copy_string(t_instruccion* instruccion)
{
	log_info(logger_cpu, "[CPU] Se lee la instrucción COPY STRING");
}


static void _f_exit(t_instruccion *inst){ 
    flag_ejecucion = false;
	devolver_contexto_a_dispatch(EXIT, inst);
}

//CONTEXTO
void ejecutar_proceso(){
    // Continúa ejecutando mientras la bandera de ejecución esté activa.
    while (flag_ejecucion) {

		// Bloqueo el "hilo" del proceso, no quiero pedir esperar un fetch de memoria al mismo tiempo 
		// que estoy tratando un fin de QUANTUM
		pthread_mutex_lock(&mutex_ejecucion_proceso);

        // Enviar solicitud de instrucción.
        t_paquete* paquete = crear_paquete(FETCH_INSTRUCCION);
        int pid_a_enviar = contexto_cpu->pid;
        int pc_a_enviar = contexto_cpu->program_counter;
        agregar_datos_sin_tamaño_a_paquete(paquete, &pid_a_enviar, sizeof(int));
        agregar_datos_sin_tamaño_a_paquete(paquete, &pc_a_enviar, sizeof(int));
        enviar_paquete(paquete, socket_memoria);
        eliminar_paquete(paquete);
        log_info(logger_cpu,"[EJECUTAR PROCESO]: -- HICE FETCH -- PID <%d> - PC<%d>",contexto_cpu->pid,contexto_cpu->program_counter);
        sem_wait(&s_instruccion_actual);

        // Decodificar la instrucción actual.
        char** instruccion_separada = string_split(instruccion_actual, " ");
        u_int8_t identificador = _get_identificador(instruccion_separada[0]);
        //log_info(logger_cpu,"[Ejecutar Proceso]: Instrucción decodificada: %s", nombre_instruccion);

        t_instruccion* inst_decodificada = malloc(sizeof(t_instruccion));
        inst_decodificada->identificador = identificador;
        inst_decodificada->cantidad_parametros = _cantidad_parametros(identificador);
        inst_decodificada->parametros = _parametros_instruccion(instruccion_actual, inst_decodificada->cantidad_parametros);

        // Ejecutar la instrucción.

		int motivo_desalojo = -1;

        switch (inst_decodificada->identificador) {
            case EXIT: _f_exit(inst_decodificada); break;
            case SET: _set(inst_decodificada); break;
            case SUM: _sum(inst_decodificada); break;
            case SUB: _sub(inst_decodificada); break;
            case JNZ: _jnz(inst_decodificada); break;
			case MOV_IN: _mov_int(inst_decodificada); break;
			case MOV_OUT: _mov_out(inst_decodificada); break;
			case RESIZE: _resize(inst_decodificada); break;
			case COPY_STRING: _copy_string(inst_decodificada); break;
            // case IO_GEN_SLEEP: _io_gen_sleep(inst_decodificada); break;
			case IO_GEN_SLEEP:
				motivo_desalojo = IO_GEN_SLEEP;
				break;
			case WAIT:
				motivo_desalojo = WAIT;
				break;
			case SIGNAL:
				int motivo = SIGNAL;
				int mensajeOk = 0;
				int size = 0;
				t_paquete* paquete_signal = crear_paquete(CONTEXTO_EJECUCION);
				agregar_datos_sin_tamaño_a_paquete(paquete_signal, &contexto_cpu->pid, sizeof(int));
				agregar_datos_sin_tamaño_a_paquete(paquete_signal, &contexto_cpu->program_counter, sizeof(int));
				empaquetar_registros_cpu(paquete_signal, contexto_cpu->registros_cpu);
				empaquetar_instruccion_cpu(paquete_signal, inst_decodificada);
				agregar_datos_sin_tamaño_a_paquete(paquete_signal, &motivo, sizeof(int));
				enviar_paquete(paquete_signal, socket_cliente_dispatch);
				eliminar_paquete(paquete_signal);
				// sem_wait(&s_signal_kernel);
				//recibo respuesta del dispatch
				int codigo_op = recibir_operacion(socket_cliente_dispatch);
				log_info(logger_cpu,"[EJECUTAR PROCESO]:SIGNAL codigo_op <%d>",codigo_op);
				void* buffer_recibido = recibir_buffer(&size, socket_cliente_dispatch);
				memcpy((&mensajeOk),buffer_recibido,sizeof(int));
				log_info(logger_cpu,"[EJECUTAR PROCESO]:SIGNAL mensajeOk <%d>",mensajeOk);
				free(buffer_recibido);
				if(!mensajeOk){
					motivo_desalojo=INT_SIGNAL;
					log_info(logger_cpu,"[EJECUTAR PROCESO]: -- PID <%d> - PC<%d> - SIGNAL NO OK",contexto_cpu->pid,contexto_cpu->program_counter);
				}
				break;
            default: 
                log_error(logger_cpu,"[Ejecutar Proceso]: Instrucción no reconocida.");
				exit(EXIT_FAILURE);
                break;
        }
        contexto_cpu->program_counter++;
		contexto_cpu->registros_cpu.PC = contexto_cpu->program_counter;
		log_info(logger_cpu,"pthread_mutex_unlock(&mutex_ejecucion_proceso)1");
		pthread_mutex_unlock(&mutex_ejecucion_proceso);

		check_recibiendo_interrupcion();

		pthread_mutex_lock(&mutex_ejecucion_proceso);
		log_info(logger_cpu,"pthread_mutex_lock(&mutex_ejecucion_proceso)2");
		//Desalojos por instrucciones ejecutadas
		if(motivo_desalojo > -1){
			flag_ejecucion = false;
			devolver_contexto_a_dispatch(motivo_desalojo, inst_decodificada);
		}
        // Verificar interrupciones.
        if (flag_interrupt && flag_ejecucion) {
            //log_info(logger_cpu,"Hay interrupción, modificar el devolver contexto");
            devolver_contexto_a_dispatch(motivo_interrupt, inst_decodificada);
            flag_ejecucion = false;
        }
        flag_interrupt = false;

        // Limpiar recursos.
        for (int i = 0; i < list_size(inst_decodificada->parametros); i++) {
            char* parametro = list_get(inst_decodificada->parametros, i);
            free(parametro);
        }
        list_destroy(inst_decodificada->parametros);
        free(inst_decodificada);
        free(instruccion_actual);
		instruccion_actual=NULL; 
		pthread_mutex_unlock(&mutex_ejecucion_proceso);
		log_info(logger_cpu,"pthread_mutex_unlock(&mutex_ejecucion_proceso)2");

        //log_info(logger_cpu,"[Ejecutar Proceso]: Instrucción completada y recursos liberados.");
    }
}



// AUXILIARES
void check_recibiendo_interrupcion(){
    pthread_mutex_lock(&mutex_check_recibiendo_interrupcion);
    if(llego_interrupcion == 1){
        sem_wait(&sem_check_recibiendo_interrupcion);
    }
    pthread_mutex_unlock(&mutex_check_recibiendo_interrupcion);
}

void ejecutando_interrupcion_fin(){
    if(llego_interrupcion == 0){
    }
    else{
        llego_interrupcion = 0;
        sem_post(&sem_check_recibiendo_interrupcion);
    }    
}

void ejecutando_interrupcion(){
    if(llego_interrupcion == 1){
    }
    else{
        llego_interrupcion = 1;
        check_recibiendo_interrupcion();
    }
}