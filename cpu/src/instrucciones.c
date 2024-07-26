#include "instrucciones.h"
//static char* _recibir_instruccion(int socket_cliente);
static u_int8_t _get_identificador(char* identificador_txt);
static u_int8_t _cantidad_parametros(u_int8_t identificador);
static t_list* _parametros_instruccion(char* instruccion, u_int8_t cantidad_parametros);
//instrucciones
static void _set(t_instruccion* instruccion);
static void _sum(t_instruccion* instruccion);
static void _sub(t_instruccion* instruccion);
static void _jnz(t_instruccion* instruccion);

static int _resize(t_instruccion* instruccion);
static void _copy_string(t_instruccion* instruccion);
static int _mov_in(t_instruccion* instruccion);
static int _mov_out(t_instruccion* instruccion);
static void _f_exit(t_instruccion *inst);
static info_registro_cpu _get_direccion_registro(char* string_registro);
static t_solicitud_io* _io_std(t_instruccion* instruccion);
static int calcular_bytes_segun_registro(char* registro);
//

static void mostrar_valores_registros_cpu(){
	
}

static int min(int a, int b) {
    return (a < b) ? a : b;
}
//----
extern t_registros_cpu registros_cpu;

static int leer_valor_registro_como_int(void* direccion, size_t tamano) {
    uint32_t valor = 0;
    if (tamano == 1) {
        valor = *(uint8_t*)direccion;
    } else if (tamano == 2) {
        valor = *(uint16_t*)direccion;
    } else if (tamano == 4) {
        valor = *(uint32_t*)direccion;
    }
    return (int)valor;  // Convertimos explícitamente a int para manejar la dirección como entero.
}

//CONTEXTO
void ejecutar_proceso(){
    // Continúa ejecutando mientras la bandera de ejecución esté activa.
    while (flag_ejecucion) {

		// Bloqueo el "hilo" del proceso, no quiero pedir esperar un fetch de memoria al mismo tiempo 
		// que estoy tratando un fin de QUANTUM
		pthread_mutex_lock(&mutex_ejecucion_proceso);
		t_solicitud_io* pedido_io_stdin_read;
		t_solicitud_io* pedido_io_stdout_write;

        // Enviar solicitud de instrucción.
        t_paquete* paquete = crear_paquete(FETCH_INSTRUCCION);
        int pid_a_enviar = contexto_cpu->pid;
        // int pc_a_enviar = contexto_cpu->program_counter;
        agregar_datos_sin_tamaño_a_paquete(paquete, &pid_a_enviar, sizeof(int));
        // agregar_datos_sin_tamaño_a_paquete(paquete, &pc_a_enviar, sizeof(int));
		int pc_a_enviar = contexto_cpu->registros_cpu.PC;
		log_info(logger_cpu,"PID: <%d> - FETCH - Program Counter: <%d>",contexto_cpu->pid,pc_a_enviar);
		agregar_datos_sin_tamaño_a_paquete(paquete, &pc_a_enviar, sizeof(int));
        enviar_paquete(paquete, socket_memoria);
        eliminar_paquete(paquete);
        sem_wait(&s_instruccion_actual);


		//log obligatorio: “PID: <PID> - FETCH - Program Counter: <PROGRAM_COUNTER>”.
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
		contexto_cpu->registros_cpu.PC++;

        switch (inst_decodificada->identificador) {
            case EXIT: _f_exit(inst_decodificada); break;
            case SET: _set(inst_decodificada); break;
            case SUM: _sum(inst_decodificada); break;
            case SUB: _sub(inst_decodificada); break;
            case JNZ: _jnz(inst_decodificada); break;
			case COPY_STRING: _copy_string(inst_decodificada); break;
			case MOV_IN: 
				if(_mov_in(inst_decodificada)==-1){
					motivo_desalojo = PAGE_FAULT;
					break;				
				}
				break;
			case MOV_OUT: 
				if(_mov_out(inst_decodificada)==-1){
					motivo_desalojo = PAGE_FAULT;
					break;				
				}
				break;
			// case MOV_OUT: _mov_out(inst_decodificada); break;
			case RESIZE: 
				if(_resize(inst_decodificada)==-1){
					motivo_desalojo = OUT_OF_MEMORY;
				}
				break;
			case IO_GEN_SLEEP:
				motivo_desalojo = IO_GEN_SLEEP;
				break;
			case IO_STDOUT_WRITE:
				// IO_STDOUT_WRITE Int3 BX EAX
				// (Interfaz, Registro Dirección, Registro Tamaño): 
				// Esta instrucción solicita al Kernel que mediante la interfaz seleccionada, 
				// se lea desde la posición de memoria indicada por la Dirección Lógica almacenada 
				// en el Registro Dirección, un tamaño indicado por el Registro Tamaño y se imprima por pantalla.
	
				//muevo la logica a una funcion propia
				pedido_io_stdout_write=_io_std(inst_decodificada);
				if(pedido_io_stdout_write == NULL){
					motivo_desalojo = PAGE_FAULT;
				} else{
					motivo_desalojo = IO_STDOUT_WRITE;
				}				
				break;

			case IO_STDIN_READ:
				//IO_STDIN_READ Int2 EAX AX
				// IO_STDIN_READ (Interfaz, Registro Dirección, Registro Tamaño): 
				// Esta instrucción solicita al Kernel que mediante la interfaz ingresada se 
				// lea desde el STDIN (Teclado) un valor cuyo tamaño está delimitado por el valor 
				// del Registro Tamaño y el mismo se guarde a partir de la Dirección Lógica almacenada 
				// en el Registro Dirección.

				//muevo la logica a una funcion propia
				pedido_io_stdin_read=_io_std(inst_decodificada);
				if(pedido_io_stdin_read == NULL){
					motivo_desalojo = PAGE_FAULT;
				} else{
					// motivo_desalojo = IO_STDOUT_WRITE;		
					motivo_desalojo = IO_STDIN_READ;
				}
				// pedido_io_stdin_read=_io_std(inst_decodificada);
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
				// agregar_datos_sin_tamaño_a_paquete(paquete_signal, &contexto_cpu->program_counter, sizeof(int));
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
					log_info(logger_cpu,"[EJECUTAR PROCESO]: -- PID <%d> - PC<%d> - SIGNAL NO OK",contexto_cpu->pid,contexto_cpu->registros_cpu.PC);
				}
				break;
            default: 
                log_error(logger_cpu,"[Ejecutar Proceso]: Instrucción no reconocida.");
				exit(EXIT_FAILURE);
                break;
        }
        // contexto_cpu->program_counter++;
		// contexto_cpu->registros_cpu.PC = contexto_cpu->program_counter;
		pthread_mutex_unlock(&mutex_ejecucion_proceso);

		check_recibiendo_interrupcion();

		pthread_mutex_lock(&mutex_ejecucion_proceso);

		//Desalojos por instrucciones ejecutadas
		if(motivo_desalojo > -1){
			flag_ejecucion = false;
			devolver_contexto_a_dispatch(motivo_desalojo, inst_decodificada);
			if (motivo_desalojo == IO_STDIN_READ){
				enviar_solicitud_io(socket_cliente_dispatch, pedido_io_stdin_read,motivo_desalojo);
			}
			if (motivo_desalojo == IO_STDOUT_WRITE){
				enviar_solicitud_io(socket_cliente_dispatch, pedido_io_stdout_write,motivo_desalojo);
			}
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

        //log_info(logger_cpu,"[Ejecutar Proceso]: Instrucción completada y recursos liberados.");
    }
}


////
void devolver_contexto_a_dispatch(int motivo, t_instruccion* instruccion){
	log_info(logger_cpu,"[DEVUELVO CONTEXTO]: -- PCB -- PID <%d> - PC<%d>",contexto_cpu->pid,contexto_cpu->registros_cpu.PC);                
	//log_protegido_cpu(string_from_format("[devolver_contexto_a_dispatch]: ---- 1 ----"));
	t_paquete* paquete = crear_paquete(CONTEXTO_EJECUCION);
	agregar_datos_sin_tamaño_a_paquete(paquete, &contexto_cpu->pid, sizeof(int));
	// agregar_datos_sin_tamaño_a_paquete(paquete, &contexto_cpu->program_counter, sizeof(int));
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

// static t_solicitud_io* _io_std(t_instruccion* instruccion){
// 		char *registro_direccion2 = list_get(instruccion->parametros, 1);
// 		char *registro_tamano2 = list_get(instruccion->parametros, 2);
// 		info_registro_cpu registro_dir_info2 = _get_direccion_registro(registro_direccion2);
// 		info_registro_cpu registro_tam_info2 = _get_direccion_registro(registro_tamano2);
// 		uint8_t valor_direccion_logica2 = *(uint8_t*)registro_dir_info2.direccion;
// 		uint8_t valor_tamano_a_escribir2 = *(uint8_t*)registro_tam_info2.direccion;
// 		//tomar la direccion logica y calcular la o las fisicas necesarias para escribir el tamano especificado.
// 		//por cada direccion fisica hacer un "agregar_a_pedido_memoria" con esa direccion, el parametro "prueba1" puede tener
// 		//cualquier valor aca porque se va a completar con datos de la interfaz despues.

// 		uint32_t dir_prueba2 = 99; //cuadno este la traduccion cambiar
// 		uint32_t tam_prueba2 = 20; //esto seria el tamaño de pagina (max)

// 		t_solicitud_io* pedido_io;
// 		pedido_io = crear_pedido_memoria(contexto_cpu->pid,tam_prueba2);
// 		agregar_a_pedido_memoria(pedido_io, "prueba1",dir_prueba2);
// 		agregar_a_pedido_memoria(pedido_io, "prueba2",dir_prueba2);
// 		return pedido_io;
// }
static void _copy_string(t_instruccion* instruccion){
	// COPY_STRING 8
	// COPY_STRING (Tamaño): Toma del string apuntado por el registro SI y copia la 
	// cantidad de bytes indicadas en el parámetro tamaño a la posición de memoria apuntada por el registro DI. 
	log_info(logger_cpu, "------------------------------------------COPY STRING------------------------------------------");

	int tamano_a_copiar = atoi(list_get(instruccion->parametros, 0));//Tamaño	
	info_registro_cpu registro_SI_info = _get_direccion_registro("SI");//String apuntado por registro SI
	info_registro_cpu registro_DI_info = _get_direccion_registro("DI");//String apuntado por registro SI
	int direccion_logica_origen 	= leer_valor_registro_como_int(registro_SI_info.direccion, registro_SI_info.tamano	);
	int direccion_logica_destino 	= leer_valor_registro_como_int(registro_DI_info.direccion, registro_DI_info.tamano	);

	//log obligatorio “PID: <PID> - Ejecutando: <INSTRUCCION> - <PARAMETROS>”.
	log_info(logger_cpu,"PID: <%d> - Ejecutando: <COPY_STRING> - <%d>",contexto_cpu->pid, tamano_a_copiar);

	//solicito a memoria leer esa direccion, y espero la respuesta
	char* dato_leido  = leer_memoria(direccion_logica_origen, tamano_a_copiar);
	//escribo el valor que me devolvio memoria en el registro correspondiente
	escribir_valor_en_memoria(direccion_logica_destino,tamano_a_copiar, dato_leido);	
	free(dato_leido);

}

static int _mov_in(t_instruccion* instruccion){
	// MOV_IN EDX ECX
	// (Registro Datos, Registro Dirección): 
	// Lee el valor de memoria correspondiente a la Dirección Lógica que se encuentra 
	// en el Registro Dirección y lo almacena en el Registro Datos.
	log_info(logger_cpu, "--------------------------------------------MOV IN---------------------------------------------");

	//Busco los registros de la instruccion en formato char
	char* registro_datos = list_get(instruccion->parametros, 0);
	char* registro_direccion = list_get(instruccion->parametros, 1);

	//log obligatorio “PID: <PID> - Ejecutando: <INSTRUCCION> - <PARAMETROS>”.
	log_info(logger_cpu,"PID: <%d> - Ejecutando: <MOV_IN> - <%s> - <%s>",contexto_cpu->pid, registro_datos, registro_direccion);

	//Consigo la informacon de esos registros (direccion + tamanio)
    info_registro_cpu registro_datos_info = _get_direccion_registro(registro_datos);
    info_registro_cpu registro_direccion_info = _get_direccion_registro(registro_direccion);

	//convierto la direccion a int para poder usarla mas facilmente
	int direccion_logica = leer_valor_registro_como_int(registro_direccion_info.direccion, 	registro_direccion_info.tamano	);

	//solicito a memoria leer esa direccion, y espero la respuesta
	char* dato_leido  = leer_memoria(direccion_logica, registro_datos_info.tamano);
	if(!strcmp(dato_leido,"-1")){
		log_info(logger_cpu,"PID: <%d> - Page Fault - Ejecutando: <MOV_IN> - <%s> - <%s>",contexto_cpu->pid, registro_datos, registro_direccion);
		return -1;	
	}

	//escribo el valor que me devolvio memoria en el registro correspondiente
	memcpy(registro_datos_info.direccion, dato_leido, registro_datos_info.tamano);	
	free(dato_leido);
	
	//genero un log para ver el valor leido y escrito en el registro
	int dato_leido_int = leer_valor_registro_como_int(registro_datos_info.direccion, registro_datos_info.tamano);
	//log obligario
	//Lectura/Escritura Memoria: “PID: <PID> - Acción: <LEER / ESCRIBIR> - Dirección Física: <DIRECCION_FISICA> - Valor: <VALOR LEIDO / ESCRITO>”.
	// log_info(logger_cpu,"PID: <%d> - Acción: <LEER> - Dirección Física: <%d> - Valor: <%d> \n", contexto_cpu->pid, direccion_logica, dato_leido_int);
	return 0;
}

static int _mov_out(t_instruccion* instruccion){
	log_info(logger_cpu, "--------------------------------------------MOV OUT--------------------------------------------");
// MOV_OUT (Registro Dirección, Registro Datos): 
// Lee el valor del Registro Datos y lo escribe en la dirección física de memoria 
// obtenida a partir de la Dirección Lógica almacenada en el Registro Dirección.	
	char* registro_direccion	= list_get(instruccion->parametros, 0);
    char* registro_datos		= list_get(instruccion->parametros, 1);
	//log obligatorio “PID: <PID> - Ejecutando: <INSTRUCCION> - <PARAMETROS>”.
	log_info(logger_cpu,"PID: <%d> - Ejecutando: <MOV_OUT> - <%s> - <%s>",contexto_cpu->pid, registro_direccion, registro_datos);

    info_registro_cpu registro_direccion_info = _get_direccion_registro(registro_direccion);
    info_registro_cpu registro_datos_info = _get_direccion_registro(registro_datos);
	int direccion_logica = leer_valor_registro_como_int(registro_direccion_info.direccion, registro_direccion_info.tamano);
	int dato_a_escribir = leer_valor_registro_como_int(registro_datos_info.direccion, registro_datos_info.tamano);

	int se_pudo_escribir = escribir_valor_en_memoria(direccion_logica,registro_datos_info.tamano, registro_datos_info.direccion);
	return se_pudo_escribir;
}

//Patricio- modifico agregando mmu
static t_solicitud_io* _io_std(t_instruccion* instruccion) {
    char *registro_direccion = list_get(instruccion->parametros, 1);
    char *registro_tamano = list_get(instruccion->parametros, 2);
    info_registro_cpu registro_dir_info = _get_direccion_registro(registro_direccion);
    info_registro_cpu registro_tam_info = _get_direccion_registro(registro_tamano);
    int direccion_logica_actual = *(uint8_t*)registro_dir_info.direccion;
    int valor_tamano_a_escribir = *(uint8_t*)registro_tam_info.direccion;

    log_info(logger_cpu,"[_io_std]: -- PID <%d> - PC<%d> - DIRECCION LOGICA <%d> - Tamaño <%d>", contexto_cpu->pid, contexto_cpu->registros_cpu.PC, direccion_logica_actual, valor_tamano_a_escribir);

    t_solicitud_io* pedido_io = crear_pedido_memoria(contexto_cpu->pid, valor_tamano_a_escribir);
    int total_escrito = 0;

    while (total_escrito < valor_tamano_a_escribir) {
        int desplazamiento_inicial = direccion_logica_actual % tamanio_pagina;
        int espacio_disponible_en_pagina = tamanio_pagina - desplazamiento_inicial;
        int bytes_a_escribir = min(espacio_disponible_en_pagina, valor_tamano_a_escribir - total_escrito);
        int direccion_fisica_actual = mmu(direccion_logica_actual);

        if (direccion_fisica_actual == -1) {
            eliminar_pedido_memoria(pedido_io);
            return NULL;
        }

        log_info(logger_cpu, "[_io_std]: -- PID <%d> - PC<%d> - DIRECCION FISICA <%d> - Bytes a escribir <%d>", contexto_cpu->pid, contexto_cpu->registros_cpu.PC, direccion_fisica_actual, bytes_a_escribir);
        agregar_a_pedido_memoria(pedido_io, " ", bytes_a_escribir, direccion_fisica_actual);

        direccion_logica_actual += bytes_a_escribir; // Mover la dirección lógica a la siguiente posición a escribir
        total_escrito += bytes_a_escribir; // Incrementar el contador de bytes escritos
    }

    return pedido_io;
}

static int _resize(t_instruccion* instruccion){
	// RESIZE 10
	//RESIZE (Tamaño): Solicitará a la Memoria ajustar el tamaño del proceso al tamaño pasado por parámetro. 
	//En caso de que la respuesta de la memoria sea Out of Memory, se deberá devolver el contexto de ejecución al 
	//Kernel informando de esta situación.
	int valor_new_size = atoi(list_get(instruccion->parametros, 0));
	//log obligatorio “PID: <PID> - Ejecutando: <INSTRUCCION> - <PARAMETROS>”.
	log_info(logger_cpu,"PID: <%d> - Ejecutando: <RESIZE> - <%d>",contexto_cpu->pid, valor_new_size);
	t_paquete* paquete = crear_paquete(RESIZE);
	agregar_datos_sin_tamaño_a_paquete(paquete, &contexto_cpu->pid, sizeof(int));
	agregar_datos_sin_tamaño_a_paquete(paquete, &valor_new_size, sizeof(int));
	enviar_paquete(paquete, socket_memoria);
	eliminar_paquete(paquete);
	
	sem_wait(&s_resize);
	log_info(logger_cpu,"[RESIZE]: -- PID <%d> - PC<%d> - RESPUESTA_MEMORIA <%d>",contexto_cpu->pid,contexto_cpu->registros_cpu.PC,respuesta_memoria);
	if(respuesta_memoria == -1){
		// En caso de que la respuesta de la memoria sea Out of Memory, 
		// se deberá devolver el contexto de ejecución al Kernel informando de esta situación.
		log_info(logger_cpu,"[_RESIZE]: -- PID <%d> - PC<%d> - RESIZE NO OK",contexto_cpu->pid,contexto_cpu->registros_cpu.PC);
		return -1;
	}
	return 0;
}

static void _set(t_instruccion* instruccion){
    char* nombre_registro = list_get(instruccion->parametros, 0);
    char* valor_string = list_get(instruccion->parametros, 1);
    uint32_t nuevo_valor = (uint32_t)atoi(valor_string); // Convierte el valor string a uint32_t una sola vez

	//log obligatorio
	log_info(logger_cpu,"PID: <%d> - Ejecutando: <SET> - <%s> - <%s>",contexto_cpu->pid, nombre_registro, valor_string);

    info_registro_cpu reg_info = _get_direccion_registro(nombre_registro);

    // Mostrar el valor actual del registro antes de la modificación
    if (reg_info.tamano == sizeof(uint8_t)) {
        *(uint8_t*)reg_info.direccion = (uint8_t)nuevo_valor; // Asigna el nuevo valor
    } else if (reg_info.tamano == sizeof(uint32_t)) {
        *(uint32_t*)reg_info.direccion = nuevo_valor; // Asigna el nuevo valor
    }
	log_info(logger_cpu,"PID: <%d> - Ejecutando: <SET> - Program Counter: <%d>",contexto_cpu->pid, contexto_cpu->registros_cpu.PC);
	// Instrucción Ejecutada: “PID: <PID> - Ejecutando: <INSTRUCCION> - <PARAMETROS>”.

}


static void _sum(t_instruccion* instruccion){
    char *registro_destino = list_get(instruccion->parametros, 0);
    char *registro_origen = list_get(instruccion->parametros, 1);
    info_registro_cpu registro_des_info = _get_direccion_registro(registro_destino);
    info_registro_cpu registro_ori_info = _get_direccion_registro(registro_origen);
	//log obligatorio “PID: <PID> - Ejecutando: <INSTRUCCION> - <PARAMETROS>”.
	log_info(logger_cpu,"PID: <%d> - Ejecutando: <SUM> - <%s> - <%s>",contexto_cpu->pid, registro_destino, registro_origen);

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

	//log obligatorio “PID: <PID> - Ejecutando: <INSTRUCCION> - <PARAMETROS>”.
	log_info(logger_cpu,"PID: <%d> - Ejecutando: <SUB> - <%s> - <%s>",contexto_cpu->pid, registro_destino, registro_origen);

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
	//log obligatorio “PID: <PID> - Ejecutando: <INSTRUCCION> - <PARAMETROS>”.
	log_info(logger_cpu,"PID: <%d> - Ejecutando: <JNZ> - <%s>",contexto_cpu->pid, registro_nombre);
    info_registro_cpu dir_registro_info = _get_direccion_registro(registro_nombre);
    uint32_t registro_valor = *(uint32_t*)dir_registro_info.direccion;
    if (registro_valor != 0) {
        uint32_t nueva_pc = atoi(list_get(instruccion->parametros, 1));
        // contexto_cpu->program_counter = nueva_pc;
        contexto_cpu->registros_cpu.PC = nueva_pc;
    }
}


// static void _mov_out(t_instruccion* instruccion){
// 	// (Registro Dirección, Registro Datos): 
// 	// Lee el valor del Registro Datos y lo escribe 
// 	// en la dirección física de memoria obtenida a partir de la Dirección Lógica almacenada en el Registro Dirección.
// 	char* registro_datos = list_get(instruccion->parametros, 1);
// 	info_registro_cpu registro_datos_info = _get_direccion_registro(registro_datos);
// 	uint8_t valor_del_registro_datos = *(uint8_t*)registro_datos_info.direccion;

// 	char* registro_direccion = list_get(instruccion->parametros, 0);
// 	info_registro_cpu registro_direccion_info = _get_direccion_registro(registro_direccion);
// 	uint8_t valor_direccion_logica = *(uint8_t*)registro_direccion_info.direccion;
	
// 	//guardar valor_del_registro_datos en la direccion fisica correspondiente a valor_direccion_logica
// }

// static void _copy_string(t_instruccion* instruccion){
// 	// (Tamaño): Toma del string apuntado por el registro SI y copia la cantidad 
// 	// de bytes indicadas en el parámetro tamaño a la posición de memoria apuntada por el registro DI. 
// 	log_warning(logger_cpu,"falta implementar la parte de memoria");

// 	uint32_t direccion_logica_destino = contexto_cpu->registros_cpu.DI;
// 	log_warning(logger_cpu,"traducir la direccion a fisica");

//     int tamano_a_copiar = atoi(list_get(instruccion->parametros, 0));

// 	char* string_a_guardar = malloc(tamano_a_copiar + 1);
// 	memcpy(string_a_guardar, (char*)contexto_cpu->registros_cpu.SI, tamano_a_copiar);
// 	string_a_guardar[tamano_a_copiar] = '\0';

// 	log_warning(logger_cpu,"pasarle a memoria el pedido");
// 	free(string_a_guardar);
// }

static void _f_exit(t_instruccion *inst){ 
	//log obligatorio “PID: <PID> - Ejecutando: <INSTRUCCION> - <PARAMETROS>”.
	log_info(logger_cpu,"PID: <%d> - Ejecutando: <EXIT>",contexto_cpu->pid);
    flag_ejecucion = false;
	devolver_contexto_a_dispatch(EXIT, inst);
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





//


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
    } else if (!strcmp(string_registro, "PC")) {
        info.direccion = &(contexto_cpu->registros_cpu.PC);
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

static int calcular_bytes_segun_registro(char* registro){
	log_info(logger_cpu,"calcular_bytes_segun_registro - REGISTRO: <%s>",registro);
	int bytes = -1;
	if(strcmp(registro,"AX")==0 || strcmp(registro,"BX")==0 || strcmp(registro,"CX")==0 || strcmp(registro,"DX")==0){
		bytes=1;
	}
	if(strcmp(registro,"EAX")==0 || strcmp(registro,"EBX")==0 || strcmp(registro,"ECX")==0 || strcmp(registro,"EDX")==0 || strcmp(registro,"SI")==0 || strcmp(registro,"DI")==0){
		bytes=4;
	}
	return bytes;
}