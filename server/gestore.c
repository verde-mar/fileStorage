#include <gestore.h>
#include <stdio.h>
#include <stdlib.h>

#include <check_errors.h>
#include <pthread.h>
#include <unistd.h>

void *gestore_segnali(void *arg) {
    sigset_t *set = ((sigHandler_t*)arg)->set;
    int fd_pipe   = ((sigHandler_t*)arg)->signal_pipe;
    int sig;

    while (1) {
        CHECK_OPERATION(sigwait(set, &sig), fprintf(stderr, "Errore nella sigwait.\n"); return (void*)NULL);
        if(sig == 2 || sig == 3){
            close(fd_pipe);
            printf("STO PER INTERROMPERE\n");
            break;
        } else if(sig == 1){
            close(fd_pipe);
            break;
        }
    }
    return (void*)NULL;	   
}