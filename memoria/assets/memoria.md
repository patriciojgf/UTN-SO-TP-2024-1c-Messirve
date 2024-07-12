# Memoria

## Inicialización de estructuras

Al momento de inicializar su estructura, se inician:

``` c
static void iniciar_estructuras(){
    lista_interfaz_socket = list_create();
    lista_procesos_en_memoria = list_create();
}
```

El espacio de usuario va ser igual al tamaño de memoria que viene definido en el archivo de congiguración

```  c
static void iniciar_espacio_de_usuario()
{
    memoria_espacio_usuario = malloc(TAM_MEMORIA);
    if(memoria_espacio_usuario == NULL)
    {
        error_show("Se produjo un error al iniciar espacio_usuario");
        exit(1);
    }
}W
```

## Conexiones

Una vez que finaliza la creación de sus estructuras se van a realizar las conexiones a los demás módulos correspondientes, va a iniciar cómo servidor y va a quedar a la espera que se conecten los demás módulos.

Cuando un módulo se conecta va a recibir un código para poder identificar el tipo de conexión. Los códigos aceptados son los siguientes:

- HANDSHAKE_KERNEL
- HANDSHAKE_CPU
- HANDSHAKE_IO_GEN
- HANDSHAKE_IO_STDIN
- HANDSHAKE_IO_STDOUT
- HANDSHAKE_IO_DIALFS

Por cada código recibido se va a crear un hilo, que a su vez cada hilo va a recibir una código de operación para poder para poder realizar lo que le corresponda.
