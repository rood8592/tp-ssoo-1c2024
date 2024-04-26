#include "../include/interrupt.h"
extern t_log* logger;

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

}