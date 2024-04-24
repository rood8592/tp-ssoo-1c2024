#include "../include/main.h"
 //el extern que va por delante es cuando queremos utilizar este logger en muchos archivos o lugares, pra no redeclarar lo mismo
t_log* logger;
t_config* config;



int main(int argc, char* argv[]) {
    /*FILE* archivoPseudocodigo = fopen("codigoPrueba.txt","r+");
    //char* instruccion = leerCadenaArchivos(f);
    char** instrucciones = pasarArchivoEstructura(archivoPseudocodigo);
    fclose(archivoPseudocodigo);

    string_array_destroy(instrucciones);
    */
    //ESTO ANDA NO BORRARLO
    int server_fd = 0;
    int fd_cpu = 0;
    int fd_kernel = 0;

    logger = iniciar_logger("memoria.log", "MODULO MEMORIA", 1, LOG_LEVEL_DEBUG);
    config = iniciar_config("memoria.config",logger);
    //
    
    //
    if(!iniciar_conexiones(logger,config,&server_fd,&fd_cpu,&fd_kernel))
    {
        log_error(logger,"Error al crear conexiones iniciales");
        exit(1);
    }
    inicializar_hilos(logger,fd_cpu,fd_kernel);

    //////
    /*
    uint32_t pc_recibido;
    recv(fd_cpu, &pc_recibido, sizeof(int32_t), MSG_WAITALL);
    enviar_mensaje(instrucciones[pc_recibido],fd_cpu,INSTRUCCION);
    */
    //DESTRUIR ESTRUCTURA CON SUS STRINGS DENTRO
    //string_array_destroy(instrucciones);
    //
    while(escucharConexionesIO(logger,server_fd));
    terminar_programa(logger,config,&fd_cpu,&fd_kernel);
    return 0; //puede ir exit_sucess
}












