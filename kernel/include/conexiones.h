#ifndef KERNEL_CONEXIONES_H_
#define KERNEL_CONEXIONES_H_

#include <stdio.h>
#include <stdlib.h>
#include "../../utils/include/sockets.h"
#include "../../utils/include/protocolo.h"
#include "../../utils/include/logsConfigs.h"
#include <pthread.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>




//INICIAR CONEXIONES DEBE TENER UN MANEJO DE ERRORES
int iniciar_conexiones(t_config* config,t_log* logger,int* fd_memoria,int* fd_cpu_dispatch, int* fd_cpu_interrupt,int* fd_escucha_interfaces);
void realizar_handshakes_kernel(int fd_memoria,int fd_cpu_dispatch, int fd_cpu_interrupt);
int escucharConexionesIO(t_log* logger,int fd_escucha_interfaces);
void procesarConexionesIO(void* datosServerInterfaces);
void terminar_programa(t_log* logger,t_config* config,int* fd_memoria,int* fd_cpu_dispatch,int* fd_cpu_interrupt);
//void escucharConexionesIO(void* datosServerInterfaces);


#endif 