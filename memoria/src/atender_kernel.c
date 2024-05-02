#include "atender_kernel.h"

/*IN*/
void iniciar_estructura_proceso(t_buffer* buffer){
    int pid=0;
    int size_path=0;
    char* path;
    int desplazamiento = 0;

    memcpy(&pid, buffer +desplazamiento, sizeof(int));
    desplazamiento+=sizeof(int);
    memcpy(&size_path,buffer +desplazamiento, sizeof(int));
    desplazamiento+=sizeof(int);
    path=malloc(size_path);
    memcpy(path, buffer +desplazamiento, size_path);

    t_proceso* proceso = crear_proceso(pid,path);
    list_add(lista_procesos_en_memoria, proceso);
    confirmar_proceso_creado();
}




/*OUT*/
void confirmar_proceso_creado(){
    log_warning(logger_memoria, "VER SI ES NECESARIO UN RETARDO");
    t_paquete* paquete = crear_paquete(INICIAR_PROCESO_MEMORIA_OK);
    
    enviar_paquete(socket_kernel, paquete);
    eliminar_paquete(paquete);
}