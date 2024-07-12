# Peticiones KERNEL

## atender_peticiones_kernel

Antes de ejecutar cada instrucción, se realiza un sleep.

Operaciones que recibe la memoria por parte del kernel:

- INICIAR_PROCESO_MEMORIA: cuando recibe esta operación, recibo el path donde se encuentra para poder leer el archivo correspondiente y agregarle las instrucciones que le corresponde al proceso, además se crea el proceso con sus respectivos atributos, se inicializa la TP, se el proceso a la lista_procesos_en_memoria y se envia la confirmación de que el proceso fue creado.
- LIBERAR_ESTRUCTURAS_MEMORIA: recibimos el pid del proceso, por el momento no se usa, liberamos el espacio de usuario y avisamos que ya se liberó el espacio.
- OBTENER_MARCO: recibimos el pid del proceso, obtenemos el marcos y se lo enviamos. 

### Iniciar TP

Se obtiene la cantidad de páginas `cant_paginas = TAM_MEMORIA/tam_pagina`, luego se inicializa el bitmap de marcos libres. Se agrega la TP al proceso, con sus atributos inicializados en ceros. 

``` c
static void iniciar_bitmap(int cantidad_marcos)
{
    void *bitmap_memoria_usuario = malloc(cantidad_marcos / 8);
    bitmap = bitarray_create_with_mode(bitmap_memoria_usuario, cantidad_marcos / 8, LSB_FIRST);
}
```

``` c
void iniciar_tabla_de_pagina(t_proceso* proceso)
{
    log_info(logger_memoria, "Inicializando tabla de pagina");
    // log_protegido_mem(string_from_format("PID: %d", proceso->id));
    // log_info(logger_memoria, "PID: %d", proceso->id);
    log_info(logger_memoria,"TAM_PAGINA: %d", TAM_PAGINA);
    int tam_pagina = TAM_PAGINA; 
    log_info(logger_memoria, "tam_pagina: %d", tam_pagina);
    
    // int cant_paginas = ceil((size_proceso/tam_pagina)); //TODO: ver que onda con cant_paginas
    int cant_paginas = TAM_MEMORIA/tam_pagina;
    log_info(logger_memoria, "cant_paginas: %d", cant_paginas);   

    //TAM_PAG = TAM_MARCO
    iniciar_bitmap(cant_paginas);

    // proceso->tabla_de_paginas = list_create(); => se inicializa en crear_proceso

    for(int i = 0; i < cant_paginas; i++)
    {
        t_tabla_pagina* tdp = malloc(sizeof(t_tabla_pagina));
        tdp->marco = 0; 
        tdp->modificado = 0;
        tdp->presencia = 0;

        list_add(proceso->tabla_de_paginas, tdp);
    }

    // proceso_nuevo->cant_paginas = cant_paginas; //TODO: debería agregar cant_pagina a t_proceso
    log_info(logger_memoria, "Creacion de Tabla de Paginas PID: <%d>", proceso->id);
}
```

### Obtener marco

1. Se obtiene el proceso.
2. Una vez encontrado, se obtiene la TP del proceso.
3. Se verificar que la presencia sea 1, en caso que este presente se devuelvo el marco correspondiente al proceso. Caso contrario se devuelve -1.

``` c
static int _get_marco(int pid)
{
    t_proceso* proceso = get_proceso_memoria(pid);
    t_tabla_pagina* tdp = list_get(proceso->tabla_de_paginas, pid);

    if(tdp->presencia)
    {
        return tdp->marco;
    }

    return -1;
}
```
