#include <threadpool.h>
#include <check_errors.h>

void* working(threadpool_t **threadpool){
    CHECK_OPERATION(!(*threadpool), fprintf(stderr, "Parametri non validi.\n"); return -1);

    int err_pop = pop_queue();
    CHECK_OPERATION(err_pop == -1, fprintf(stderr, "Errore nella pop di una richiesta.\n"); return -1);
    
    //1 - tokenize richiesta
    //2- chiama determinato metodo della tabella hash

    return NULL;
}

static int invia_risposta(threadpool_t *pool, int err, int fd, char* buf, char* path){
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
    
    /* Scrive il puntatore della response sulla pipe delle risposte */
    err_pipe = write(pool->response_pipe, *risp, sizeof(response)); 
    CHECK_OPERATION(err_pipe==-1, 
        return -1);

    return 0;
}

int create_threadpool(threadpool_t** threadpool, int num_thread, int pipe_lettura){
    CHECK_OPERATION(num_thread <= 0, fprintf(stderr, "Parametri non validi.\n"); return -1);
    threadpool = malloc(sizeof(threadpool_t));

    (*threadpool)->response_pipe = pipe_lettura;
    (*threadpool)->pending_requests = malloc(sizeof(list_c));

    /* Crea l'array di thread */
    (*threadpool)->threads = (pthread_t *)malloc(sizeof(pthread_t) * num_thread);
    for(int i = 0; i < num_thread; i++) {
        if(pthread_create(&((*threadpool)->threads[i]), NULL, working, (void*)threadpool) != 0) {
	        /* errore fatale, libero tutto forzando l'uscita dei threads */
            int err_destroy = destroy_threadpool(threadpool);
        }
    }
}