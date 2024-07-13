# Intrucciones

La CPU va ejecutar distintas instrucciones:

- EXIT: va a devolver el contexto al dispatch para el Kernel pueda finalizar al proceso.
- SET: le va a asignar un nuevo valor. pasado por parámetro al registro. Ej: `SET BX 1`
- SUM: suma al Registro Destino el Registro Origen y deja el resultado en el Registro Destino. Ej: `SUM AX BX`
- SUB: resta al Registro Destino el Registro Origen y deja el resultado en el Registro Destino. Ej: `SUB AX BX`
- JNZ: : si el valor del registro es distinto de cero, actualiza el program counter al número de instrucción pasada por parámetro. Ej: `JNZ AX 4`
- RESIZE: solicitará a la Memoria ajustar el tamaño del proceso al tamaño pasado por parámetro. En caso de que la respuesta de la memoria sea Out of Memory, se deberá devolver el contexto de ejecución al Kernel informando de esta situación. Ej: `RESIZE 128`//TODO: consultar implementación
- COPY_STRING: falta implementar la parte de memoria. Ej: `COPY_STRING 8`
- MOV_IN: Ej: `MOV_IN EDX ECX`
- MOV_OUT: `MOV_OUT EDX ECX`
- IO_GEN_SLEEP: `IO_GEN_SLEEP Int1 10`
- IO_STDOUT_WRITE: `IO_STDOUT_WRITE Int3 BX EAX`
- IO_STDIN_READ: `IO_STDIN_READ Int2 EAX AX`
- WAIT: `WAIT RECURSO_1`
- SIGNAL: esta instrucción solicita al Kernel que se libere una instancia del recurso indicado por parámetro. Ej: `SIGNAL RECURSO_1`
- FALTA IMPLEMENTAR Ej: `IO_FS_CREATE Int4 notas.txt`
- FALTA IMPLEMENTAR Ej: `IO_FS_DELETE Int4 notas.txt`
- FALTA IMPLEMENTAR Ej: `IO_FS_TRUNCATE Int4 notas.txt ECX`
- FALTA IMPLEMENTAR Ej: `IO_FS_WRITE Int4 notas.txt AX ECX EDX`
- FALTA IMPLEMENTAR Ej: `IO_FS_READ Int4 notas.txt BX ECX EDX`
