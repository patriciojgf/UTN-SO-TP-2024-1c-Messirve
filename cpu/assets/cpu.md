# CPU

## Inicialización de estructuras

Al momento de inicializar su estructura, se inicializa el contexto de la cpu que tiene el siguiente formato:

``` c
typedef struct{
    int pid; // Identificador del proceso.
    int program_counter; // Número de la próxima instrucción a ejecutar.
    t_registros_cpu registros_cpu; // Registros de la CPU.
} t_contexto;
```

Además se inicializa la TLB, que va arrancar con valores negativos. La TBL va a tener `CANTIDAD_ENTRADAS_TLB` que viene definido en el archivo de configuración.
La TLB tiene la siguiente estructura: `pid | pagina | marco`

## Conexiones

Una vez que finaliza la creación de sus estructuras se van a realizar las conexiones a los demás módulos correspondientes.

``` c
gestionar_conexion_memoria();   //cliente   detach
gestionar_conexion_interrupt(); //servidor  detach
gestionar_conexion_dispatch();  //servidor  join
```

Por cada código de operación que reciba va a creaer un hilo para poder atender al módulo correspondiente.
