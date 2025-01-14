#include "../include/consola.h"

extern t_log *logger;
extern t_log *logger_obligatorio;
extern t_config *config;

extern t_squeue *lista_procesos_new;
extern t_squeue *lista_procesos_ready;
extern t_squeue *lista_procesos_ready_plus;
extern t_squeue *lista_procesos_exec;
extern t_squeue *lista_procesos_exit;
extern t_slist *lista_procesos_blocked;
extern t_slist *lista_recursos_blocked;

extern sem_t grado_de_multiprogramacion;
extern sem_t proceso_en_cola_new;
extern sem_t proceso_en_cola_ready;
extern sem_t pasar_a_ejecucion;

extern sem_t planificacion_new_iniciada;
extern sem_t planificacion_ready_iniciada;
extern sem_t planificacion_exec_iniciada;
extern sem_t planificacion_blocked_iniciada;
extern sem_t hay_una_peticion_de_proceso;

extern sem_t planificacion_detenida;

extern bool planificacion_iniciada;
extern pthread_mutex_t mutex_plani_iniciada;

t_squeue *squeue_path;

char *opciones[] = {
    "EJECUTAR_SCRIPT",
    "INICIAR_PROCESO /scripts_memoria/",
    "FINALIZAR_PROCESO",
    "DETENER_PLANIFICACION",
    "INICIAR_PLANIFICACION",
    "MULTIPROGRAMACION",
    "PROCESO_ESTADO",
    NULL};
// Por parte de la documentación compartida en UTNSO
char **custom_completion(const char *text, int start, int end)
{
    char **matches = NULL;

    if (start == 0)
    {
        matches = rl_completion_matches(text, &custom_completion_generator);
    }
    else
    {
        rl_attempted_completion_over = 1;
    }

    return matches;
}

char *custom_completion_generator(const char *text, int state)
{
    static int list_index, len;
    const char *name;

    if (!state)
    {
        list_index = 0;
        len = strlen(text);
    }

    while ((name = opciones[list_index]))
    {
        list_index++;

        if (strncasecmp(name, text, len) == 0)
        {
            return strdup(name);
        }
    }
    return NULL;
}

pthread_mutex_t hilo_pid_mutex;
int pid_contador = 0;

extern int multiprog;
extern char *algoritmo;

bool interrupcion_usuario = false;

int fd_dispatch;
int fd_interrupt;
int fd_mem;
int fd_IO;
// asdasd
void iniciar_consola(void *fd_info)
{
    squeue_path = squeue_create();
    pthread_t hilo_crear_proceso;
    pthread_create(&hilo_crear_proceso, NULL, (void *)crear_proceso, NULL);
    pthread_detach(hilo_crear_proceso);
    // t_log* logger;
    // logger = iniciar_logger("kernel.log","Kernel",1,LOG_LEVEL_INFO);
    info_fd *auxiliar = fd_info;
    // les asigno un valor a las variables globales
    fd_dispatch = auxiliar->fd_cpu_dispatch;
    fd_interrupt = auxiliar->fd_cpu_interrupt;
    fd_mem = auxiliar->fd_memoria;
    fd_IO = auxiliar->fd_escucha_interfaces;

    rl_attempted_completion_function = custom_completion;

    char *leido;
    printf("EJECUTAR_SCRIPT [PATH] \n");
    printf("INICIAR_PROCESO [PATH] \n");
    printf("FINALIZAR_PROCESO [PID] \n");
    printf("DETENER_PLANIFICACION \n");
    printf("INICIAR_PLANIFICACION \n");
    printf("MULTIPROGRAMACION [VALOR]\n");
    printf("PROCESO_ESTADO \n");

    leido = readline("> ");
    bool validarComando;
    while (!string_is_empty(leido))
    {
        validarComando = validar_instrucciones_leidas(leido);
        if (!validarComando)
        {
            log_error(logger, "Comando no reconocido");
            free(leido);
            leido = readline("> ");
            continue;
        }
        add_history(leido);
        instrucciones_consola(leido);
        free(leido);
        leido = readline("> ");
    }
    free(leido);
}

bool validar_instrucciones_leidas(char *leido)
{
    char **instruccion_leida = string_split(leido, " ");
    bool valido;
    // log_info(logger, "%s y %s y %s", instruccion_leida[0], instruccion_leida[1], instruccion_leida[2]);

    if (strcmp(instruccion_leida[0], "EJECUTAR_SCRIPT") == 0 && instruccion_leida[1] != NULL && strcmp(instruccion_leida[1], "") != 0)
        valido = true;
    else if (strcmp(instruccion_leida[0], "INICIAR_PROCESO") == 0 && instruccion_leida[1] != NULL && strcmp(instruccion_leida[1], "") != 0)
        valido = true;
    else if (strcmp(instruccion_leida[0], "FINALIZAR_PROCESO") == 0 && instruccion_leida[1] != NULL && strcmp(instruccion_leida[1], "") != 0)
        valido = true;
    else if (strcmp(instruccion_leida[0], "DETENER_PLANIFICACION") == 0)
        valido = true;
    else if (strcmp(instruccion_leida[0], "INICIAR_PLANIFICACION") == 0)
        valido = true;
    else if (strcmp(instruccion_leida[0], "MULTIPROGRAMACION") == 0 && instruccion_leida[1] != NULL && strcmp(instruccion_leida[1], "") != 0)
        valido = true;
    else if (strcmp(instruccion_leida[0], "PROCESO_ESTADO") == 0)
        valido = true;
    else
        valido = false;

    string_array_destroy(instruccion_leida);

    return valido;
}

void instrucciones_consola(char *leido)
{
    char **instruccion_leida = string_split(leido, " ");

    if (strcmp(instruccion_leida[0], "EJECUTAR_SCRIPT") == 0)
        ejecutar_script(instruccion_leida[1]);
    else if (strcmp(instruccion_leida[0], "INICIAR_PROCESO") == 0)
    {
        // iniciar_proceso(instruccion_leida[1]);
        char *aux = string_duplicate(instruccion_leida[1]);
        squeue_push(squeue_path, aux);
        sem_post(&hay_una_peticion_de_proceso);
    }
    else if (strcmp(instruccion_leida[0], "FINALIZAR_PROCESO") == 0)
        finalizar_proceso(atoi(instruccion_leida[1]));
    else if (strcmp(instruccion_leida[0], "DETENER_PLANIFICACION") == 0)
        detener_planificacion();
    else if (strcmp(instruccion_leida[0], "INICIAR_PLANIFICACION") == 0)
        iniciar_planificacion();
    else if (strcmp(instruccion_leida[0], "MULTIPROGRAMACION") == 0)
        multiprogramacion(atoi(instruccion_leida[1]));
    else if (strcmp(instruccion_leida[0], "PROCESO_ESTADO") == 0)
        proceso_estado();

    string_array_destroy(instruccion_leida);
}

void ejecutar_script(char *path)
{
    char *path_archivo_scripts = string_new();
    char *path_scripts = config_get_string_value(config, "PATH_SCRIPTS"); 
    //string_append_with_format(&pathv2, ".%s" ,path);
    string_append(&path_archivo_scripts, path_scripts);
    string_append(&path_archivo_scripts, path);
    FILE *arch_instrucciones;
    char *linea_almacenada = NULL;
    size_t tam_almacenamiento = 0;
    int linea_leida;

    arch_instrucciones = fopen(path_archivo_scripts, "rt");
    if (arch_instrucciones == NULL)
    {
        log_error(logger, "No se pudo abrir el archivo de instrucciones");
        return;
    }
    while ((linea_leida = getline(&linea_almacenada, &tam_almacenamiento, arch_instrucciones)) != -1)
    {
        strtok(linea_almacenada, "\n");
        if (!string_is_empty(linea_almacenada))
        {
            if (!validar_instrucciones_leidas(linea_almacenada))
            {
                log_error(logger, "Instruccion no reconocida");
                continue;
            }
            instrucciones_consola(linea_almacenada);
        }
    }
    free(path_archivo_scripts);
    free(linea_almacenada);
    free(path_scripts);
    fclose(arch_instrucciones);
}

void iniciar_proceso(char *path)
{
    // printf("iniciar_proceso \n");
    // log_debug(logger,"PATH A MANDAR: %s",path);
    t_pcb *nuevo_pcb = crear_pcb();
    enviar_nuevo_proceso(&nuevo_pcb->pid, path, fd_mem);

    // Le envio las instrucciones a memoria y espero respuesta
    int respuesta;
    recv(fd_mem, &respuesta, sizeof(int), MSG_WAITALL);
    if (respuesta == ARCHIVO_INVALIDO)
    {
        pthread_mutex_lock(&hilo_pid_mutex);
        pid_contador--;
        pthread_mutex_unlock(&hilo_pid_mutex);
        log_warning(logger, "PARA UN POCO, REVISA LA RUTA DEL ARCHIVO");
        free(nuevo_pcb);
        return;
    }

    squeue_push(lista_procesos_new, nuevo_pcb);
    log_info(logger_obligatorio, "Se crea el proceso %d en NEW", nuevo_pcb->pid);

    sem_post(&proceso_en_cola_new);
}

void crear_proceso()
{

    while (1)
    {
        sem_wait(&hay_una_peticion_de_proceso);
        char *path_aux = squeue_pop(squeue_path);
        iniciar_proceso(path_aux);
        free(path_aux);
    }
}
/***************

FINALIZAR PROCESO

-SE LE DELEGA A UN HILO LA RESPONSABILIDAD DE FINALIZAR EL PROCESO PARA NO TRABAR LA CONSOLA
-SI LA PLANIFICACION ESTABA DETENIDA AL MOMENTO DE DIGITAR EL COMANDO, SE BUSCA EL PCB POR PID Y SE LO SACA, NADA MAS
-SI LA PLANIFICACION ESTABA INICIADA, SE DEBE DETENERLA, BUSCAR EL PID, Y VOLVERLA A INICIAR
-PONER UN ELSE EN DONDE EL PID NO EXISTA

*****************/
void finalizar_proceso(int pid_eliminar)
{
    int *arg_pid = malloc(sizeof(int));
    *arg_pid = pid_eliminar;
    pthread_t fin_proceso;
    pthread_create(&fin_proceso, NULL, (void *)hilo_elimina_proceso, arg_pid);
}

// HILO ENCARGADO DE ELIMINAR EL PROCESO, RECIBE COMO ARGUMENTO EL PID DEL PCB A ELIMINAR
void hilo_elimina_proceso(int *arg)
{
    int pid = *arg;
    free(arg);
    // SI LA PLANIFICACION ESTA DETENIDA, SOLO LO BUSCO
    if (!get_plani_iniciada())
    {
        buscar_proceso_finalizar(pid);
    }
    // SINO DEBO DETENERLA PRIMERO, ASEGURARME DE QUE SE DETUVO CORRECTAMENTE, SACAR EL PCB DE DONDE ESTE E INICIAR LA PLANI
    else
    {
        set_plani_iniciada(false);
        pthread_t detener_new, detener_ready, detener_exec, detener_blocked;
        pthread_create(&detener_new, NULL, (void *)detener_cola_new_confirmacion, NULL);
        pthread_create(&detener_ready, NULL, (void *)detener_cola_ready_confirmacion, NULL);
        pthread_create(&detener_exec, NULL, (void *)detener_cola_exec_confirmacion, NULL);
        pthread_create(&detener_blocked, NULL, (void *)detener_cola_blocked_confirmacion, NULL);
        pthread_detach(detener_new);
        pthread_detach(detener_ready);
        pthread_detach(detener_exec);
        pthread_detach(detener_blocked);
        sem_wait(&planificacion_detenida);
        sem_wait(&planificacion_detenida);
        sem_wait(&planificacion_detenida);
        sem_wait(&planificacion_detenida);
        buscar_proceso_finalizar(pid);
        iniciar_planificacion();
    }
}

// GETTER DE PLANI INICIADA CON MUTEX
bool get_plani_iniciada()
{
    bool estado_planificacion;
    pthread_mutex_lock(&mutex_plani_iniciada);
    estado_planificacion = planificacion_iniciada;
    pthread_mutex_unlock(&mutex_plani_iniciada);

    return estado_planificacion;
}

// SETTER PLANI INICIADA CON MUTEX
void set_plani_iniciada(bool valor)
{
    pthread_mutex_lock(&mutex_plani_iniciada);
    planificacion_iniciada = valor;
    pthread_mutex_unlock(&mutex_plani_iniciada);
}

// FUNCION QUE BUSCA POR TODAS LAS COLAS HASTA ENCONTRAR EL PCB BUSCADO POR PID
void buscar_proceso_finalizar(int pid)
{
    t_pcb *pcb_auxiliar;
    bool _elemento_encontrado(t_pcb * pcb)
    {
        bool coincide = pcb->pid == pid;
        return coincide;
    }
    if (squeue_any_satisfy(lista_procesos_new, (void *)_elemento_encontrado))
    {
        pcb_auxiliar = squeue_remove_by_condition(lista_procesos_new, (void *)_elemento_encontrado);
        manejar_fin_con_motivo(INTERRUPTED_BY_USER_NEW, pcb_auxiliar);
    }
    else if (squeue_any_satisfy(lista_procesos_ready, (void *)_elemento_encontrado))
    {
        pcb_auxiliar = squeue_remove_by_condition(lista_procesos_ready, (void *)_elemento_encontrado);
        manejar_fin_con_motivo(INTERRUPTED_BY_USER_READY, pcb_auxiliar);
    }
    else if (squeue_any_satisfy(lista_procesos_ready_plus, (void *)_elemento_encontrado))
    {
        pcb_auxiliar = squeue_remove_by_condition(lista_procesos_ready_plus, (void *)_elemento_encontrado);
        manejar_fin_con_motivo(INTERRUPTED_BY_USER_READY_PLUS, pcb_auxiliar);
    }
    else if (squeue_any_satisfy(lista_procesos_exec, (void *)_elemento_encontrado))
    {
        pcb_auxiliar = squeue_peek(lista_procesos_exec);
        int pid_auxiliar = pcb_auxiliar->pid;
        if (strcmp(algoritmo, "RR") == 0 || strcmp(algoritmo, "VRR") == 0)
            interrupcion_usuario = true;

        enviar_interrupcion(USER_INTERRUPT, pid_auxiliar, fd_interrupt);
    }
    else if (squeue_any_satisfy(lista_procesos_exit, (void *)_elemento_encontrado))
    {
        log_error(logger, "QUE HACES, SI YA ESTA EN EXIT");
    }
    else if (slist_find_pcb_iterating_each_queue(lista_procesos_blocked, pid))
    {
    }
    else if (!lista_recursos_is_empty())
    {
        bool fueEncontrado = false;
        void _eliminar_proceso(t_recurso * recurso)
        {
            if (squeue_any_satisfy(recurso->cola_blocked, (void *)_elemento_encontrado))
            {
                pcb_auxiliar = squeue_remove_by_condition(recurso->cola_blocked, (void *)_elemento_encontrado);
                fueEncontrado = true;
            }
        }
        pthread_mutex_lock(lista_recursos_blocked->mutex);
        list_iterate(lista_recursos_blocked->lista, (void *)_eliminar_proceso);
        pthread_mutex_unlock(lista_recursos_blocked->mutex);
        if (fueEncontrado)
            manejar_fin_con_motivo(INTERRUPTED_BY_USER_BLOCKED, pcb_auxiliar);
    }
    else
    {
        log_warning(logger, "No se encontro el proceso %d", pid);
    }
    // iniciar_planificacion();
}
/******************
 DETENER PLANIFICACION

-SE CREA UN HILO POR CADA COLA, CADA UNO TIENE LA TAREA DE HACER UN WAIT SOBRE SU SEMAFORO
-AL HACER WAIT SOBRE TODOS LAS COLAS, LA PLANIFICACION QUEDA DETENIDA

**************************/

void detener_planificacion()
{
    // printf("detener_planificador \n");
    if (get_plani_iniciada())
    {
        set_plani_iniciada(false);
        pthread_t detener_new, detener_ready, detener_exec, detener_blocked;
        pthread_create(&detener_new, NULL, (void *)detener_cola_new, NULL);
        pthread_create(&detener_ready, NULL, (void *)detener_cola_ready, NULL);
        pthread_create(&detener_exec, NULL, (void *)detener_cola_exec, NULL);
        pthread_create(&detener_blocked, NULL, (void *)detener_cola_blocked, NULL);
        pthread_detach(detener_new);
        pthread_detach(detener_ready);
        pthread_detach(detener_exec);
        pthread_detach(detener_blocked);
        log_info(logger, "Se detuvo la planificacion");
    }
    else
    {
        log_info(logger, "la plani esta pausada ya");
    }
}

/******
FUNCIONES QUE SOLO HACEN WAIT AL SEMAFORO CORRESPONDIENTE
****/
void detener_cola_new(void *arg)
{
    sem_wait(&planificacion_new_iniciada);
}
void detener_cola_ready(void *arg)
{
    sem_wait(&planificacion_ready_iniciada);
}
void detener_cola_exec(void *arg)
{
    sem_wait(&planificacion_exec_iniciada);
}
void detener_cola_blocked(void *arg)
{
    sem_wait(&planificacion_blocked_iniciada);
}
/******
FUNCIONES QUE HACEN WAIT Y CONFIRMAN CON UN POST A UN SEMAFORO ADICIONAL, UTIL PARA FINALIZAR UN PROCESO
********/
void detener_cola_new_confirmacion(void *arg)
{
    sem_wait(&planificacion_new_iniciada);
    sem_post(&planificacion_detenida);
}
void detener_cola_ready_confirmacion(void *arg)
{
    sem_wait(&planificacion_ready_iniciada);
    sem_post(&planificacion_detenida);
}
void detener_cola_exec_confirmacion(void *arg)
{
    sem_wait(&planificacion_exec_iniciada);
    sem_post(&planificacion_detenida);
}
void detener_cola_blocked_confirmacion(void *arg)
{
    sem_wait(&planificacion_blocked_iniciada);
    sem_post(&planificacion_detenida);
}

/***************

INICIAR_PLANIFICACION

****************/
void iniciar_planificacion()
{
    if (!get_plani_iniciada())
    {
        // printf("iniciar_planificacion \n");
        set_plani_iniciada(true);
        int valor;
        sem_post(&planificacion_new_iniciada);
        sem_getvalue(&planificacion_new_iniciada, &valor);
        //printf("\nESTO EN CONSOLA, VALOR NEW INICIADA: %d \n", valor);
        sem_post(&planificacion_ready_iniciada);
        sem_post(&planificacion_exec_iniciada);
        sem_post(&planificacion_blocked_iniciada);
        log_info(logger, "Planificación %s iniciada", config_get_string_value(config, "ALGORITMO_PLANIFICACION"));
    }
}

void multiprogramacion(int valor)
{
    // printf("multiprogramacion \n");
    cambiar_grado_de_multiprogramacion(valor);
}

void proceso_estado()
{
    // printf("proceso_estado \n");
    if (!squeue_is_empty(lista_procesos_new))
    {
        char* lista_pids = string_new();
        char *pids_listar = listar_pids(lista_procesos_new);
        string_n_append(&lista_pids, pids_listar, string_length(pids_listar)-2);
        log_info(logger, "Procesos cola new: %s", lista_pids);
        free(pids_listar);
        free(lista_pids);
    }
    else
        log_info(logger, "La cola new esta vacia");

    if (!squeue_is_empty(lista_procesos_ready))
    {
        char* lista_pids = string_new();
        char *pids_listar = listar_pids(lista_procesos_ready);
        string_n_append(&lista_pids, pids_listar, string_length(pids_listar)-2);
        log_info(logger, "Procesos cola ready: %s", lista_pids);
        free(pids_listar);
        free(lista_pids);
    }
    else
        log_info(logger, "La cola ready esta vacia");

    if (strcmp(algoritmo, "VRR") == 0 && !squeue_is_empty(lista_procesos_ready_plus))
    {
        char* lista_pids = string_new();
        char *pids_listar = listar_pids(lista_procesos_ready_plus);
        string_n_append(&lista_pids, pids_listar, string_length(pids_listar)-2);
        log_info(logger, "Procesos cola ready plus: %s", lista_pids);
        free(pids_listar);
        free(lista_pids);
    }
    else if (strcmp(algoritmo, "VRR") == 0)
        log_info(logger, "La cola ready plus esta vacia");

    if (!squeue_is_empty(lista_procesos_exec))
    {
        char* lista_pids = string_new();
        char *pids_listar = listar_pids(lista_procesos_exec);
        string_n_append(&lista_pids, pids_listar, string_length(pids_listar)-2);
        log_info(logger, "Procesos cola exec: %s", lista_pids);
        free(pids_listar);
        free(lista_pids);
    }
    else
        log_info(logger, "La cola exec esta vacia");

    if (!squeue_is_empty(lista_procesos_exit))
    {
        char* lista_pids = string_new();
        char *pids_listar = listar_pids(lista_procesos_exit);
        string_n_append(&lista_pids, pids_listar, string_length(pids_listar)-2);
        log_info(logger, "Procesos cola exit: %s", lista_pids);
        free(pids_listar);
        free(lista_pids);
    }
    else
        log_info(logger, "La cola exit esta vacia");
    if (!lista_interfaces_is_empty())
    {

        char *pids = string_new();
        void listar_pids_blocked(t_list_io * interfaz)
        {
            if (!cola_io_is_empty(interfaz))
            {
                char *auxiliar_pids = pids_blocked(interfaz);
                string_append_with_format(&pids, "%s", auxiliar_pids);
                free(auxiliar_pids);
            }
        }

        pthread_mutex_lock(lista_procesos_blocked->mutex);
        list_iterate(lista_procesos_blocked->lista, (void *)listar_pids_blocked);
        pthread_mutex_unlock(lista_procesos_blocked->mutex);

        if (!string_is_empty(pids))
        {
            char* lista_pids = string_new();
            string_n_append(&lista_pids, pids, string_length(pids)-2);
            log_info(logger, "Procesos cola blocked: %s", lista_pids);
            free(lista_pids);    
        }
        else
            log_info(logger, "La cola blocked (interfaces) esta vacia");
        
        free(pids);
    }
    else
        log_info(logger, "No hay interfaz conectada");

    if (!lista_recursos_is_empty())
    {
        char *pids = string_new();

        void listar_pids_blocked(t_recurso * recurso)
        {
            if (!squeue_is_empty(recurso->cola_blocked))
            {
                char *auxiliar_pids = listar_pids(recurso->cola_blocked);
                string_append_with_format(&pids, "%s", auxiliar_pids);
                free(auxiliar_pids);
            }
        }

        pthread_mutex_lock(lista_recursos_blocked->mutex);
        list_iterate(lista_recursos_blocked->lista, (void *)listar_pids_blocked);
        pthread_mutex_unlock(lista_recursos_blocked->mutex);

        if (!string_is_empty(pids))
        {
            char* lista_pids = string_new();
            string_n_append(&lista_pids, pids, string_length(pids)-2);
            log_info(logger, "Procesos cola blocked (recursos): %s", lista_pids);
            free(lista_pids);    
        }
        else
            log_info(logger, "La cola blocked (recursos) esta vacia");
        
        free(pids);
    }
    else
        log_info(logger, "No hay recursos para usar");

}



char *pids_blocked(t_list_io *interfaz)
{
    char *pids = string_new();

        void obtener_pids_blocked_gen(t_elemento_iogenerica * elemento_iogenerico)
        {
            char *pid = string_itoa(elemento_iogenerico->pcb->pid);
            if (!(strcmp(pid, " ") == 0))
            {
                string_append_with_format(&pids, "%s, ", pid);
            }
            free(pid);
        }
        void obtener_pids_blocked_in_out(t_elemento_io_in_out * elemento_io)
        {
            char *pid = string_itoa(elemento_io->pcb->pid);
            if (!(strcmp(pid, " ") == 0))
            {
                string_append_with_format(&pids, "%s, ", pid);
            }
            free(pid);
        }

        void obtener_pids_blocked_dialfs(t_elemento_io_fs * elemento_fs){
            char *pid = string_itoa(elemento_fs->pcb->pid);
            if (!(strcmp(pid, " ") == 0))
            {
                string_append_with_format(&pids, "%s, ", pid);
            }
            free(pid);
        }
    
    if(interfaz->tipo_interfaz == IO_GEN_SLEEP){
        cola_io_iterate(interfaz, (void *) obtener_pids_blocked_gen);
    }
    else if(interfaz->tipo_interfaz == IO_STDIN_READ || interfaz->tipo_interfaz == IO_STDOUT_WRITE){
        cola_io_iterate(interfaz, (void *) obtener_pids_blocked_in_out);
    }
    else if(interfaz->tipo_interfaz == IO_FS){
        cola_io_iterate(interfaz, (void *) obtener_pids_blocked_dialfs);
    }
    return pids;
}

////////////

void cambiar_grado_de_multiprogramacion(int nuevo_grado_mp)
{
    if (nuevo_grado_mp > multiprog)
    {
        int diferencia_entre_grados = nuevo_grado_mp - multiprog;
        for (int i = 0; i < diferencia_entre_grados; i++)
        {
            sem_post(&grado_de_multiprogramacion);
        }
    }
    else if (nuevo_grado_mp < multiprog)
    {
        int diferencia_entre_grados = multiprog - nuevo_grado_mp;
        for (int i = 0; i < diferencia_entre_grados; i++)
        {
            // creo el hilo para que no se congele la consola
            pthread_t hilo_cambio_mp;
            pthread_create(&hilo_cambio_mp, NULL, (void *)bajar_grado, NULL);
            pthread_detach(hilo_cambio_mp);
        }
    }
    log_warning(logger, "Se cambio el grado de multiprogramacion: Anterior %d - Nuevo: %d", multiprog, nuevo_grado_mp);
    multiprog = nuevo_grado_mp;
}

void bajar_grado()
{
    sem_wait(&grado_de_multiprogramacion);
}

//////Funciones procesos
t_pcb *crear_pcb()
{
    t_pcb *pcb_creado = malloc(sizeof(t_pcb));

    pcb_creado->pid = asignar_pid();
    pcb_creado->pc = 0;
    pcb_creado->quantum = config_get_int_value(config, "QUANTUM");
    pcb_creado->estado = NEW;
    pcb_creado->registros_CPU = iniciar_registros_vacios();
    return pcb_creado;
}

t_registros_generales iniciar_registros_vacios()
{
    t_registros_generales registro_auxiliar;

    registro_auxiliar.AX = 0;
    registro_auxiliar.BX = 0;
    registro_auxiliar.CX = 0;
    registro_auxiliar.DX = 0;
    registro_auxiliar.EAX = 0;
    registro_auxiliar.EBX = 0;
    registro_auxiliar.ECX = 0;
    registro_auxiliar.EDX = 0;
    registro_auxiliar.SI = 0;
    registro_auxiliar.DI = 0;

    return registro_auxiliar;
}

int asignar_pid()
{
    int pid_a_asignar;
    pthread_mutex_lock(&hilo_pid_mutex);
    pid_a_asignar = pid_contador++;
    pthread_mutex_unlock(&hilo_pid_mutex);
    return pid_a_asignar;
}