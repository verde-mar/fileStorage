#include <threadpool.h>
#include <check_errors.h>
#include <utils.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <socketIO.h>

static int tokenizer(char *to_token, char** operation, char** path, char** file){
    CHECK_OPERATION((to_token==NULL), fprintf(stderr, "Stringa da tokenizzare non valida.\n"); return -1);
    char *pars = to_token, *str = NULL;
    char* token = strtok_r(pars, ";", &str);
    *operation = token;
    token = strtok_r(NULL, ";", &str);
    *path = token;
    token = strtok_r(NULL, ";", &str);
    *file = token;

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
    err_pipe = writen(pool->response_pipe, &risp, sizeof(response*)); 
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
        request* req = pop_queue((*threadpool)->pending_requests);
        /* Se la richiesta e' NULL allora e' iniziata la routine di chiusura */
        CHECK_OPERATION(req->request == NULL, (*threadpool)->curr_threads--; free(req); return (void*)NULL);
        free(req->request);
        free(req);
        /* Tokenizza la richiesta */
        char *operation, *path, *file;
        int err_token = tokenizer(req->request, &operation, &path, &file);
        CHECK_OPERATION(err_token == -1, fprintf(stderr, "Errore nella tokenizzazione della stringa di richiesta.\n"); return (void*)NULL);
        
        printf("operation: %s\npath:%s\n", operation, path);
        /* In base alla richiesta chiama il metodo corretto e invia la risposta al thread main */
        if(!strcmp(operation, "write")){
            node *deleted;
            int err_write = write_hashtable(path, file, &deleted, req->fd);
            CHECK_OPERATION(err_write == -1, fprintf(stderr, "Errore sulla write_hashtable.\n"); return (void*)NULL);
            
            int err_invio = invia_risposta((*threadpool), err_write, req->fd, NULL, NULL, deleted);
            CHECK_OPERATION(err_invio == -1, fprintf(stderr, "Errore nell'invio della risposta.\n"); return (void*)NULL);

        } else if(!strcmp(operation, "read")){
            char *buf;
            int err_read = read_hashtable(path, &buf, req->fd);
            CHECK_OPERATION(err_read == -1, fprintf(stderr, "Errore sulla read_hashtable.\n"); return (void*)NULL);
            int err_invio = invia_risposta((*threadpool), err_read, req->fd, buf, NULL, NULL);
            CHECK_OPERATION(err_invio == -1, fprintf(stderr, "Errore nell'invio della risposta.\n"); return (void*)NULL);
        } else if(!strcmp(operation, "append")){
            node *deleted;
            int err_append = append_hashtable(path, file, &deleted, req->fd);
            CHECK_OPERATION(err_append == -1, fprintf(stderr, "Errore sulla append_hashtable.\n"); return (void*)NULL);
            int err_invio = invia_risposta((*threadpool), err_append, req->fd, NULL, NULL, deleted);
            CHECK_OPERATION(err_invio == -1, fprintf(stderr, "Errore nell'invio della risposta.\n"); return (void*)NULL);
        } else if(!strcmp(operation, "lock")){
            int err_lock = lock_hashtable(path, req->fd);
            CHECK_OPERATION(err_lock == -1, fprintf(stderr, "Errore sulla lock_hashtable.\n"); return (void*)NULL);
            int err_invio = invia_risposta((*threadpool), err_lock, req->fd, NULL, NULL, NULL);
            CHECK_OPERATION(err_invio == -1, fprintf(stderr, "Errore nell'invio della risposta.\n"); return (void*)NULL);
        } else if(!strcmp(operation, "unlock")){
            int err_unlock = unlock_hashtable(path, req->fd);
            CHECK_OPERATION(err_unlock == -1, fprintf(stderr, "Errore sulla unlock_hashtable.\n"); return (void*)NULL);
            int err_invio = invia_risposta((*threadpool), err_unlock, req->fd, NULL, NULL, NULL);
            CHECK_OPERATION(err_invio == -1, fprintf(stderr, "Errore nell'invio della risposta.\n"); return (void*)NULL);
        } else if(!strcmp(operation, "close")){
            int err_close = close_hashtable(path, req->fd);
            CHECK_OPERATION(err_close == -1, fprintf(stderr, "Errore sulla close_hashtable.\n"); return (void*)NULL);
            int err_invio = invia_risposta((*threadpool), err_close, req->fd, NULL, NULL, NULL);
            CHECK_OPERATION(err_invio == -1, fprintf(stderr, "Errore nell'invio della risposta.\n"); return (void*)NULL);
        } else if(!strcmp(operation, "remove")){
            int err_rem = del_hashtable(path, NULL, req->fd); 
            CHECK_OPERATION(err_rem == -1, fprintf(stderr, "Errore sulla del_hashtable.\n"); return (void*)NULL);
            int err_invio = invia_risposta((*threadpool), err_rem, req->fd, NULL, NULL, NULL);
            CHECK_OPERATION(err_invio == -1, fprintf(stderr, "Errore nell'invio della risposta.\n"); return (void*)NULL);
        } else if(!strcmp(operation, "readN")){
            char *buf;
            int err_read = read_hashtable(path, &buf, req->fd);
            CHECK_OPERATION(err_read == -1, fprintf(stderr, "Errore sulla readN_hashtable.\n"); return (void*)NULL);
            int err_invio = invia_risposta((*threadpool), err_read, req->fd, buf, path, NULL);
            CHECK_OPERATION(err_invio == -1, fprintf(stderr, "Errore nell'invio della risposta.\n"); return (void*)NULL);
        } else if(!strcmp(operation, "create") || !strcmp(operation, "open")){
            int flags = -1;
            if(!strcmp(operation, "create"))
                flags = 6;
            else    
                flags = 2;
            int err_open_create = add_hashtable(path, req->fd, flags); 
            CHECK_OPERATION(err_open_create == -1, fprintf(stderr, "Errore sulla add_hashtable.\n"); return (void*)NULL);
            int err_invio = invia_risposta((*threadpool), err_open_create, req->fd, NULL, NULL, NULL);
            CHECK_OPERATION(err_invio == -1, fprintf(stderr, "Errore nell'invio della risposta.\n"); return (void*)NULL);
            
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
    int err = create_req(&(*threadpool)->pending_requests); 
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

    if((*threadpool)->curr_threads>0)
        for(int i = 0; i < (*threadpool)->num_thread; i++) {
            int err_push = push_queue(NULL, &((*threadpool)->pending_requests));
            CHECK_OPERATION(err_push == -1, fprintf(stderr, "Errore nell'invio di richieste NULL per la terminazione.\n"); return -1);
        }

    for(int i = 0; i < (*threadpool)->num_thread; i++) {
        if(pthread_join((*threadpool)->threads[i], NULL) != 0) {
            del = del_req(&(*threadpool)->pending_requests);
            CHECK_OPERATION(del == -1, fprintf(stderr, "Errore nella liberazione della memoria durante la destroy_threadpool.\n"); return -1);
        }
    }
    free((*threadpool)->threads);

    del = del_req(&(*threadpool)->pending_requests);
    CHECK_OPERATION(del == -1, fprintf(stderr, "Errore nella liberazione della memoria durante la destroy_threadpool.\n"); return -1);

    free(*threadpool);

    return 0;
}