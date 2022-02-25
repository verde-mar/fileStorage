#include <threadpool.h>
#include <check_errors.h>
#include <utils.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int tokenizer(char *to_token, char** operation, char** path, char** file, char **fd){
    CHECK_OPERATION((to_token==NULL), fprintf(stderr, "Stringa da tokenizzare non valida.\n"); return -1);
    char *pars = to_token, *str = NULL;
    char* token = strtok_r(pars, ";", &str);
    *operation = token;
    token = strtok_r(NULL, ";", &str);
    *path = token;
    token = strtok_r(NULL, ";", &str);
    *file = token;
    token = strtok_r(NULL, ";", &str);
    *fd = token;

    return 0;
}

int invia_risposta(threadpool_t *pool, int err, int fd, char* buf, char* path, node *deleted){
    CHECK_OPERATION(pool==NULL, 
        fprintf(stderr, "Parametri non validi.\n"); 
            return -1);
    
    int err_pipe = 0;
    response *risp = malloc(sizeof(response));
    CHECK_OPERATION(risp==NULL, 
        fprintf(stderr, "Allocazione non valida.\n"); 
            return -1);
    risp->fd_richiesta = fd;
    risp->errore = err;
    risp->path = path;
    risp->buffer_file = buf;
    risp->deleted = deleted;
    fflush(stdout);
    /* Scrive il puntatore della response sulla pipe delle risposte */
    err_pipe = write(pool->response_pipe, &risp, sizeof(response*)); 
    printf("err_pipe: %d\n", err_pipe);
    CHECK_OPERATION(err_pipe==-1, 
        fprintf(stdout, "La pipe delle risposte e' chiusa, quindi il thread deve terminare.\n");
            return -1);
        
    return 0;
}

/**
 * @brief Funzione che ogni worker esegue per gestire le richieste
 * 
 * @param threadpool Puntatore al threadpool
 * @return void* NULL
 */
static void* working(void* pool){
    CHECK_OPERATION(!(pool), fprintf(stderr, "Parametri non validi.\n"); return (void*)NULL);
    threadpool_t **threadpool = (threadpool_t**) pool;

    while(1){
        /* Preleva una richiesta dalla coda  delle richieste */
        char* req = remove_fifo((*threadpool)->pending_requests);
        /* Se la richiesta e' NULL allora e' iniziata la routine di chiusura */
        CHECK_OPERATION(req == NULL, (*threadpool)->curr_threads--;  return (void*)NULL);
        
        /* Tokenizza la richiesta */
        char *operation, *path, *file, *fd;
        int err_token = tokenizer(req, &operation, &path, &file, &fd);
        CHECK_OPERATION(err_token == -1, fprintf(stderr, "Errore nella tokenizzazione della stringa di richiesta.\n"); return NULL);
        printf("operation: %s\n", operation);

        /* In base alla richiesta chiama il metodo corretto e invia la risposta al thread main */
        if(!strcmp(operation, "write")){
            node *deleted;
            int fd_c = strtol(fd, NULL, 10);
            int err_write = write_hashtable(path, file, &deleted, fd_c);
            int err_invio = invia_risposta((*threadpool), err_write, fd_c, NULL, NULL, deleted);
            CHECK_OPERATION(err_invio == -1, return (void*)NULL);

        } else if(!strcmp(operation, "read")){
            int fd_c = strtol(fd, NULL, 10);
            char *buf;
            int err_read = read_hashtable(path, &buf, fd_c);
            int err_invio = invia_risposta((*threadpool), err_read, fd_c, buf, NULL, NULL);
            CHECK_OPERATION(err_invio == -1, return (void*)NULL);
        } else if(!strcmp(operation, "append")){
            node *deleted;
            int fd_c = strtol(fd, NULL, 10);
            int err_append = append_hashtable(path, file, &deleted, fd_c);
            int err_invio = invia_risposta((*threadpool), err_append, fd_c, NULL, NULL, deleted);
            CHECK_OPERATION(err_invio == -1, return (void*)NULL);
        } else if(!strcmp(operation, "lock")){
            int fd_c = strtol(fd, NULL, 10);
            int err_lock = lock_hashtable(path, fd_c);
            int err_invio = invia_risposta((*threadpool), err_lock, fd_c, NULL, NULL, NULL);
            CHECK_OPERATION(err_invio == -1, return (void*)NULL);
        } else if(!strcmp(operation, "unlock")){
            int fd_c = strtol(fd, NULL, 10);
            int err_unlock = unlock_hashtable(path, fd_c);
            int err_invio = invia_risposta((*threadpool), err_unlock, fd_c, NULL, NULL, NULL);
            CHECK_OPERATION(err_invio == -1, return (void*)NULL);
        } else if(!strcmp(operation, "close")){
            int fd_c = strtol(fd, NULL, 10);
            int err_close = close_hashtable(path, fd_c);
            int err_invio = invia_risposta((*threadpool), err_close, fd_c, NULL, NULL, NULL);
            CHECK_OPERATION(err_invio == -1,  return (void*)NULL);
        } else if(!strcmp(operation, "remove")){
            int fd_c = strtol(fd, NULL, 10);
            int err_rem = del_hashtable(path, NULL, fd_c); 
            int err_invio = invia_risposta((*threadpool), err_rem, fd_c, NULL, NULL, NULL);
            CHECK_OPERATION(err_invio == -1,  return (void*)NULL);
        } else if(!strcmp(operation, "readN")){
            int fd_c = strtol(fd, NULL, 10);
            char *buf;
            int err_read = read_hashtable(path, &buf, fd_c);
            int err_invio = invia_risposta((*threadpool), err_read, fd_c, buf, path, NULL);
            CHECK_OPERATION(err_invio == -1, return (void*)NULL);
        } else if(!strcmp(operation, "create") || !strcmp(operation, "open")){
            int fd_c = strtol(fd, NULL, 10);
            int flags = -1;
            if(!strcmp(operation, "create"))
                flags = 6;
            else    
                flags = 2;
            int err_open_create = add_hashtable(path, fd_c, flags); 
            int err_invio = invia_risposta((*threadpool), err_open_create, fd_c, NULL, NULL, NULL);
            CHECK_OPERATION(err_invio == -1, return (void*)NULL);
        } 
    }
    return (void*)NULL;
}

int create_threadpool(threadpool_t** threadpool, int num_thread, int pipe_scrittura){
    CHECK_OPERATION(num_thread <= 0 || pipe_scrittura<0, fprintf(stderr, "Parametri non validi.\n"); return -1);
    *threadpool = malloc(sizeof(threadpool_t));
    CHECK_OPERATION(threadpool == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return -1);

    (*threadpool)->response_pipe = pipe_scrittura;
    (*threadpool)->num_thread = num_thread;

    /* Inizializza la coda condivisa tra il thread main e gli worker */
    int err = create_fifo(&(*threadpool)->pending_requests); 
    CHECK_OPERATION(err == -1, fprintf(stderr, "Errore nella creazione della coda condivisa tra thread main e gli worker.\n"); return -1);

    (*threadpool)->curr_threads = num_thread;

    /* Crea l'array di thread */
    (*threadpool)->threads = (pthread_t *)malloc(sizeof(pthread_t) * num_thread);
    CHECK_OPERATION((*threadpool)->threads == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return -1);
    for(int i = 0; i < num_thread; i++) {
        if(pthread_create(&((*threadpool)->threads[i]), NULL, &working, (void*)&(*threadpool)) != 0) {
	        /* Errore fatale, libera tutto forzando l'uscita dei thread */
            int err_destroy = destroy_threadpool(threadpool);
            CHECK_OPERATION(err_destroy == -1, fprintf(stderr, "Errore nella distruzione del threadpool.\n"); return -1);
        }
    }

    return 0;
}

int destroy_threadpool(threadpool_t **threadpool){
    CHECK_OPERATION(!(*threadpool), fprintf(stderr, "Parametri non validi.\n"); return -1);
    int del = 0;

    for(int i = 0; i < (*threadpool)->num_thread; i++) {
        push_queue(NULL, &((*threadpool)->pending_requests));
    }

    for(int i = 0; i < (*threadpool)->num_thread; i++) {
        if(pthread_join((*threadpool)->threads[i], NULL) != 0) {
            del = delete_fifo(&(*threadpool)->pending_requests);
            CHECK_OPERATION(del == -1, fprintf(stderr, "Errore nella liberazione della memoria durante la destroy_threadpool.\n"); return -1);
        }
    }
    free((*threadpool)->threads);

    del = delete_fifo(&(*threadpool)->pending_requests);
    CHECK_OPERATION(del == -1, fprintf(stderr, "Errore nella liberazione della memoria durante la destroy_threadpool.\n"); return -1);

    free(*threadpool);

    return 0;
}