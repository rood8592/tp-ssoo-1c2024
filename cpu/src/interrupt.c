#include "../include/interrupt.h"

extern t_log* logger;
extern int HAY_INTERRUPCION;
extern int PID_ACTUAL;
extern pthread_mutex_t mutex_interrupcion;
extern pthread_mutex_t mutex_pid;
extern sem_t semaforo_pcb_recibido;

void realizar_handshake_interrupt(int fd_interrupt)
{
    if(recibir_operacion(fd_interrupt)==HANDSHAKE)
    {
        char* moduloConectado = recibir_mensaje(fd_interrupt,logger);
        enviar_handshake_ok(logger,fd_interrupt,moduloConectado);
        free(moduloConectado);
    }
    else{
        log_error(logger,"Error al realizar el handshake de cpu-interrupt");
        abort();
    }
}

void inicializar_hilo_interrupt(int cliente_fd_conexion_interrupt)
{
    pthread_t hiloInterrupt;
    info_fd_conexion* fd_interrupt = malloc(sizeof(info_fd_conexion));
    fd_interrupt->fd = cliente_fd_conexion_interrupt;
    pthread_create(&hiloInterrupt,NULL,(void*) manejarConexionInterrupt,(void*) fd_interrupt);
    pthread_detach(hiloInterrupt);
}

void manejarConexionInterrupt(void* fd_interrupt)
{
    info_fd_conexion* fd_recibido = fd_interrupt;
    int fd_kernel_interrupt = fd_recibido->fd;
    free(fd_recibido);

    int pid;
    int coincide_pid = 0;

    while(1)
    {
        recv(fd_kernel_interrupt,&pid,sizeof(int),MSG_WAITALL); /*muchos mas problemas al agregar el semaforo*/

        pthread_mutex_lock(&mutex_pid);
        coincide_pid = pid==PID_ACTUAL;
        pthread_mutex_unlock(&mutex_pid);


        if(coincide_pid)
        {
            pthread_mutex_lock(&mutex_interrupcion);
            HAY_INTERRUPCION = 1;
            pthread_mutex_unlock(&mutex_interrupcion);
            log_debug(logger,"Llego la interrupcion al pid %d",pid); //SI NO ENTRA ES PORQUE SE IGNORO LA INTERRUPCION
            coincide_pid = 0;
        }                                                   
        
    }
}