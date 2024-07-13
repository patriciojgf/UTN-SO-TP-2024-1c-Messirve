# Peticiones MEMORIA

## atender_peticiones_memoria

Operaciones que recibe CPU por parte de la memoria:

- FETCH_INSTRUCCION_RESPUESTA: recibo el tamaño de la instrucción y hago `sem_post(&s_instruccion_actual)` para que continue la ejecución del proceso.
- TAMANIO_PAGINA: va recibir el tamaño de página del proceso y lo va a guarda en la variable `TAM_PAG`.
