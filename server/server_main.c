//TODO: se una richiesta e' NULL devi inviare un errore generico al client
//TODO: routine di chiusura:
    //1 - SIGINT, SIGQUIT: chiude accept, chiude la pipe risposte in lettura;
    //2- SIGHUP: chiude accept, chiude la pipe risposte in lettura se il numero dei thread attivi e' 0.
//TODO: aggiungi alla richiesta, il file descriptor del client 
//TODO: rende la struct risposta, una o piu' stringhe da inviare al client
//TODO: fa terminare il gestore dei segnali dopo il selector
#include <stdio.h>
#include <signal.h>
#include <check_errors.h>

#include <threadpool.h>
#include <gestore.h>
#include <sys/select.h>

#include <string.h>
#include <unistd.h>

int main(int argc, char const *argv[]) {
    int size = 100;

    /* Crea la pipe da utilizzare per inviare l'avvenuta ricezione di un segnale al main */
    int signal_pipe[2];
    int err_pipe_signal = pipe(signal_pipe);
    CHECK_OPERATION(err_pipe_signal==-1, fprintf(stderr, "Errore nella creazione della signal_pipe.\n"); return -1);

    /* Crea la pipe da utlizzare dagli worker per inviare le risposte al master */
    int response_pipe[2];
    int err_pipe_response = pipe(response_pipe);
    CHECK_OPERATION(err_pipe_response==-1, fprintf(stderr, "Errore nella creazione della response_pipe .\n"); return -1);

    /* Definisce la maschera dei segnali */
    sigset_t mask;
    int err_sigempty = sigemptyset(&mask);
    CHECK_OPERATION(err_sigempty == -1, fprintf(stderr, "Errore nella sigemptyset.\n"); return -1);
    int err_sigadd = sigaddset(&mask, SIGINT);
    CHECK_OPERATION(err_sigadd == -1, fprintf(stderr, "Errore nella sigaddset.\n"); return -1); 
    err_sigadd = sigaddset(&mask, SIGQUIT);
    CHECK_OPERATION(err_sigadd == -1, fprintf(stderr, "Errore nella sigaddset.\n"); return -1);
    err_sigadd = sigaddset(&mask, SIGHUP);
    CHECK_OPERATION(err_sigadd == -1, fprintf(stderr, "Errore nella sigaddset.\n"); return -1);    
    int err_sigmask = pthread_sigmask(SIG_SETMASK, &mask,NULL);
    CHECK_OPERATION(err_sigmask != 0, fprintf(stderr, "Errore nella pthread_sigmask .\n"); return -1);

    /* Ignora SIGPIPE per evitare di essere terminato da una scrittura su un socket */
    struct sigaction s;
    memset(&s,0,sizeof(s));    
    s.sa_handler=SIG_IGN;
    CHECK_OPERATION((sigaction(SIGPIPE,&s,NULL) ) == -1, fprintf(stderr, "Errore nella sigaction.\n"); return -1);

    /* Crea il threadpool */
    threadpool_t* pool;
    int err_create_pool = create_threadpool(&pool, 2, response_pipe[1]);
    /* Crea la tabella hash */
    int err_hash = create_hashtable(size);
    CHECK_OPERATION(err_hash == -1, fprintf(stderr, "Errore nella creazione della tabella hash.\n"); return -1);

    /* Avvia il thread gestore dei segnali */
    sigHandler_t handlerArgs = { &mask, signal_pipe[1] };
    pthread_t tid_signal;
    int err_signal = pthread_create(&tid_signal, NULL, &gestore_segnali, &handlerArgs);
    CHECK_OPERATION((err_signal==-1), fprintf(stderr, "Errore nella creazione del gestore dei segnali.\n"); return -1;);

    /* Crea e inizializza i set di file descriptor */
    fd_set set, tmpset;
    FD_ZERO(&tmpset); 
    FD_ZERO(&set);
    
    /* Aggiunge i fd delle pipe in lettura e del fd della socket al set */
    //FD_SET(fd_skt, &set);
    FD_SET(signal_pipe[0], &set);
    FD_SET(response_pipe[0], &set);
    
    
    push_queue((char*)argv[1], &(pool->pending_requests));
    push_queue((char*)argv[2], &(pool->pending_requests));
    //destroy_threadpool(&pool);
    //destroy_hashtable();
    

    return 0;
}
