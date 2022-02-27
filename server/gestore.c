#include <gestore.h>
#include <stdio.h>
#include <stdlib.h>

#include <check_errors.h>
#include <pthread.h>
#include <unistd.h>

#include <socketIO.h>

void *gestore_segnali(void *arg) {
    sigset_t *set = ((sigHandler_t*)arg)->set;
    int fd_pipe   = ((sigHandler_t*)arg)->signal_pipe;
    int sig;

    while (1) {
        CHECK_OPERATION(sigwait(set, &sig), fprintf(stderr, "Errore nella sigwait.\n"); return (void*)NULL);
        if(sig == 2 || sig == 3){
            int err_write = writen(fd_pipe, &sig, sizeof(int));
            CHECK_OPERATION(err_write == -1, fprintf(stderr, "Errore nell'invio del tipo di segnale.\n"); return (void*)NULL);
            return (void*)NULL;	 
        } else if(sig == 1){
            int err_write = writen(fd_pipe, &sig, sizeof(int));
            CHECK_OPERATION(err_write == -1, fprintf(stderr, "Errore nell'invio del tipo di segnale.\n"); return (void*)NULL);
            return (void*)NULL;	 
        }
    }
    return (void*)NULL;	   
}