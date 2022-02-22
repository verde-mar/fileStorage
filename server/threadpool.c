#include <threadpool.h>
#include <check_errors.h>

void* working(){

}

int create_threadpool(int num_thread, int pipe_lettura){
    CHECK_OPERATION(num_thread <= 0, fprintf(stderr, "Parametri non validi.\n"); return -1);
    threadpool = malloc(sizeof(threadpool_t));

    threadpool->response_pipe = pipe_lettura;
    threadpool->pending_requests = malloc(sizeof(list_c));

    /* Crea l'array di thread */
    threadpool->threads = (pthread_t *)malloc(sizeof(pthread_t) * num_thread);
    ASSERT_OPERATION(threadpool->threads==NULL, fprintf(stderr, "Errore nella creazione della coda di thread -- malloc -- (createThreadPool).\n"); return NULL);
    for(int i = 0; i < num_thread; i++) {
        if(pthread_create(&(threadpool->threads[i]), NULL, working, (void*)threadpool) != 0) {
	        /* errore fatale, libero tutto forzando l'uscita dei threads */
            int err_destroy = destroy_threadpool(threadpool);
            ASSERT_OPERATION(err_destroy==-1, fprintf(stderr, "Errore nella distruzione del ThreadPool\n"); int err_free = freePoolResources(pool); ASSERT_OPERATION(err_free==-1, fprintf(stderr, "Errore nella liberazione delle risorse (createThreadPool).\n"); return NULL); errno = EFAULT; return NULL;);
        }
    }
}