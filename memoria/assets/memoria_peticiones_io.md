# Peticiones IO

## Estructuras

``` c
typedef struct {
    int pid;
    uint32_t size_solicitud;
    uint32_t cantidad_accesos;    
    t_dato_memoria* datos_memoria;
} t_solicitud_io;
```

## atender_peticiones_stdin

Válida que la operación sea IO_STDIN_READ

## atender_peticiones_stdout

Válida que la operación sea IO_STDOUT_WRITE

## atender_peticiones_dialfs

Proximamente...
