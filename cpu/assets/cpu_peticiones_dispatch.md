# Peticiones DISPATCH

## atender_peticiones_dispatch

Las operaciones que va a realizar son las siguientes:

- PCB: se va a encargar de ejecutar el proceso solicitado por kernel. Le va a pedir a memoria las instrucciones que tiene que ejecutar, se va a quedar esperando mediante un semáforo hasta que pueda avanzar (`sem_wait(&s_instruccion_actual)`). Una vez que recibe la instrucción, dependiendo del tipo que sea va a ejecutar lo que le corresponda.
- SIGNAL: va a recibir la instrucción y le va a enviar un signal para que se ejecute la instrucción`sem_post(&s_signal_kernel)`. Por el momento sem_post se encuentra comentado, igualmente se envia a Kernel para que realice lo que corresponda.
