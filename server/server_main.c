#include <stdio.h>
#include <signal.h>
#include <check_errors.h>

#include <threadpool.h>
#include <gestore_segnali.h>
#include <sys/select.h>

#include <string.h>
#include <unistd.h>
#include <utils.h>

#include <socketIO.h>
#include <stdlib.h>
#include <errno.h>

/**
 * @brief Routine di chiusura di tutto il file storage
 * 
 * @param pool Threadpool
 */
void routine_chiusura(threadpool_t **pool, pthread_t tid_signal){
    int err_d1 = destroy_threadpool(pool);
    CHECK_OPERATION(err_d1 == -1, 
        fprintf(stderr, "Errore nella distruzione del threadpool.\n"); 
        int err_d2 = destroy_hashtable();
        CHECK_OPERATION(err_d2 == -1, fprintf(stderr, "Errore nella distruzione della tabella hash.\n"); exit(-1));
        int err_join = pthread_join(tid_signal, NULL);
        CHECK_OPERATION(err_join != 0, fprintf(stderr, "Errore nell'attesa del gestore dei segnali.\n"); exit(-1));
        exit(-1));

    int err_d2 = destroy_hashtable();
    CHECK_OPERATION(err_d2 == -1, 
        fprintf(stderr, "Errore nella distruzione della tabella hash.\n"); 
        int err_join = pthread_join(tid_signal, NULL);
        CHECK_OPERATION(err_join != 0, fprintf(stderr, "Errore nell'attesa del gestore dei segnali.\n"); exit(-1));
        exit(-1));

    int err_join = pthread_join(tid_signal, NULL);
    CHECK_OPERATION(err_join != 0, fprintf(stderr, "Errore nell'attesa del gestore dei segnali.\n"); exit(-1));

}

int read_input(const char* file_config, char **socketname, size_t *size, int *num_workers, int *num_file){
    FILE *config = fopen(file_config, "r");
    CHECK_OPERATION(config == NULL, fprintf(stderr, "Errore sulla fopen.\n"); return -1);

    int scan1 = fscanf(config, "socketname: %s\n", *socketname);
    CHECK_OPERATION(scan1 == -1, fprintf(stderr, "Errore nella fscanf leggendo la socketname.\n"); return -1);

    scan1 = fscanf(config, "size: %ld\n", size);
    CHECK_OPERATION(scan1 == -1, fprintf(stderr, "Errore nella fscanf leggendo la size totale.\n"); return -1);

    scan1 = fscanf(config, "numero di workers: %d\n", num_workers);
    CHECK_OPERATION(scan1 == -1, fprintf(stderr, "Errore nella fscanf leggendo il numero di workers.\n"); return -1);

    scan1 = fscanf(config, "numero massimo di file accettabili: %d\n", num_file);
    CHECK_OPERATION(scan1 == -1, fprintf(stderr, "Errore nella fscanf leggendo il numero di file accettabili.\n"); return -1);

    int closing = fclose(config);
    CHECK_OPERATION(closing == -1, fprintf(stderr, "Errore sulla fclose.\n"); return -1);

    return 0;
}

int main(int argc, char const *argv[]) {
    int fd_skt, num_file, num_workers;
    size_t size;
    char *socketname = malloc(sizeof(char)*255);
    CHECK_OPERATION(socketname == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); exit(-1));

    /* Inizializza le variabili necessarie al funzionamento del server */
    int readers = read_input(argv[1], &socketname, &size, &num_workers, &num_file);
    CHECK_OPERATION(readers == -1, 
        fprintf(stderr, "Errore nella lettura del file di configurazione.\n"); 
        free(socketname);
        exit(-1));
    
    /* Crea la pipe da utilizzare per inviare l'avvenuta ricezione di un segnale al main */
    int signal_pipe[2];
    int err_pipe_signal = pipe(signal_pipe);
    CHECK_OPERATION(err_pipe_signal==-1, 
        fprintf(stderr, "Errore nella creazione della signal_pipe.\n"); 
        free(socketname);
        exit(-1));

    /* Crea la pipe da utlizzare dagli worker per inviare le risposte al master */
    int response_pipe[2];
    int err_pipe_response = pipe(response_pipe);
    CHECK_OPERATION(err_pipe_response==-1, 
        fprintf(stderr, "Errore nella creazione della response_pipe .\n"); 
        free(socketname);
        exit(-1));

    /* Definisce la maschera dei segnali */
    sigset_t mask;
    int err_set_mask = set_mask(&mask);
    CHECK_OPERATION(err_set_mask == -1, 
        fprintf(stderr, "Errore nel setting della maschera.\n"); 
        free(socketname);
        exit(-1));

    /* Ignora SIGPIPE per evitare di essere terminato da una scrittura su un socket */
    struct sigaction s;
    memset(&s,0,sizeof(s));    
    s.sa_handler=SIG_IGN;
    CHECK_OPERATION(sigaction(SIGPIPE,&s,NULL) == -1, 
        fprintf(stderr, "Errore nella sigaction.\n"); 
        free(socketname);
        exit(-1));

    /* Crea il threadpool */
    threadpool_t* pool;
    int err_create_pool = create_threadpool(&pool, num_workers, response_pipe[1]);
    CHECK_OPERATION(err_create_pool == -1, 
        fprintf(stderr, "Errore nella creazione del theradpool.\n"); 
        free(socketname);
        exit(-1));

    /* Crea la tabella hash */
    int err_hash = create_hashtable(size, num_file, (char*)argv[2]);
    CHECK_OPERATION(err_hash == -1, 
        fprintf(stderr, "Errore nella creazione della tabella hash.\n"); 
        free(socketname);
        exit(-1));

    /* Avvia il thread gestore dei segnali */
    sigHandler_t handlerArgs = { &mask, signal_pipe[1] };
    pthread_t tid_signal;
    int err_signal = pthread_create(&tid_signal, NULL, &gestore_segnali, &handlerArgs);
    CHECK_OPERATION((err_signal==-1), 
        fprintf(stderr, "Errore nella creazione del gestore dei segnali.\n"); 
        int err_d1 = destroy_threadpool(&pool);
        CHECK_OPERATION(err_d1 == -1, fprintf(stderr, "Errore nella distruzione del threadpool.\n"); 
            int err_d2 = destroy_hashtable();
            CHECK_OPERATION(err_d2 == -1, fprintf(stderr, "Errore nella distruzione della tabella hash.\n"); exit(-1));
            exit(-1));
        int err_d2 = destroy_hashtable();
        CHECK_OPERATION(err_d2 == -1, fprintf(stderr, "Errore nella distruzione della tabella hash.\n"); exit(-1));
        free(socketname);
        exit(-1)); 

    /* Effettua le operazioni di bind e listen */
    fd_set set, rdset;
    int err = bind_listen(&fd_skt, &set, socketname);
    CHECK_OPERATION(err==-1, routine_chiusura(&pool, tid_signal); free(socketname); exit(-1));
    
    /* Crea e inizializza i set di file descriptor */
    FD_ZERO(&rdset); 
    FD_ZERO(&set);
    
    /* Aggiunge i fd delle pipe in lettura e del fd della socket al set */
    FD_SET(fd_skt, &set);
    FD_SET(signal_pipe[0], &set);
    FD_SET(response_pipe[0], &set);

    /* Calcola il massimo dei file descriptor, tra quelli delle pipe e quella rest */
    int fd_max = max(fd_skt, signal_pipe[0]);
    fd_max = max(fd_max, response_pipe[0]);

    int err_select;
    int ended = 1;

    while(ended){
        rdset = set;
        err_select = select(fd_max+1, &rdset, NULL, NULL, NULL);
        CHECK_OPERATION(err_select==-1, fprintf(stderr, "Errore nella select."); routine_chiusura(&pool, tid_signal); free(socketname); exit(-1));

        for (int fd = 0; fd<=fd_max;fd++) {
            int fd_c;
            if (FD_ISSET(fd, &rdset)) {
                /* E' arrivata una nuova richiesta */
                if (fd == fd_skt) { 
                    /* Viene accettato un nuovo client */
                    fd_c = accept(fd_skt,NULL,0);
                    CHECK_OPERATION(fd_c == -1, fprintf(stderr, "Errore nell'accetatzione di un client.\n"); routine_chiusura(&pool, tid_signal);  free(socketname); exit(-1));
                    
                    /* Aggiorna il file di log */
                    fprintf(table->file_log, "Accept\n");
                    fprintf(table->file_log, "E' stato accettato il client: %d\n", fd_c);

                    /* Inserisce il fd del client tra quelli in ascolto */
                    FD_SET(fd_c, &set);
                    fd_max = max(fd_max, fd_c); 
                } 
                /* E' arrivato un segnale di chiusura */
                else if(fd == signal_pipe[0]){ 
                    int sig;
                    /* Legge il tipo di segnale dalla pipe */
                    int err_readn = readn(signal_pipe[0], &sig, sizeof(int));
                    CHECK_OPERATION(err_readn == -1 , fprintf(stderr, "Errore sulla readn nella lettura del segnale arrivato."); continue);
                    fprintf(table->file_log, "Tipo di segnale ricevuto per la chiusura: %d.\n", sig);

                    /* Chiude la welcoming socket per non accettare piu' nuove connessioni */
                    int socket_chiusa = close(fd_skt);
                    CHECK_OPERATION(socket_chiusa == -1, fprintf(stderr, "C'e' stato un errore nella chiusura della welcoming socket.\n"); routine_chiusura(&pool, tid_signal); exit(-1));
                    FD_CLR(fd_skt, &set);
                    fd_max = aggiorna(set, fd_max);

                    /* Se arriva un SIGINT o un SIGQUIT */
                    if(sig == 2 || sig == 3){
                        ended = 0;
                        int resp_pipe = close(response_pipe[0]);
                        CHECK_OPERATION(resp_pipe == -1, fprintf(stderr, "Errore nella chiusura della pipe delle risposte in lettura.\n"); routine_chiusura(&pool, tid_signal); exit(-1));
                        FD_CLR(response_pipe[0], &set); 
                        fd_max = aggiorna(set, fd_max);
                    } 
                    /* Se arriva un SIGHUP */
                    else { 
                        for(int i = 0; i < pool->num_thread; i++) {
                            int err_push = push_queue(NULL, -1, NULL, 0, &(pool->pending_requests));
                            CHECK_OPERATION(err_push == -1, fprintf(stderr, "Errore nell'invio di richieste NULL per la terminazione.\n"); routine_chiusura(&pool, tid_signal); exit(-1));
                        }
                    } 
                    /* Chiude la pipe dei segnali in lettura e la elimina dal set */
                    int closed_pipe = close(signal_pipe[0]);
                    CHECK_OPERATION(closed_pipe == -1, fprintf(stderr, "Errore nella chiusura della pipe dei segnali in lettura.\n"); routine_chiusura(&pool, tid_signal); exit(-1));
                    FD_CLR(signal_pipe[0], &set); 
                    fd_max = aggiorna(set, fd_max);

                    /* Chiude la pipe dei segnali in scrittura e la elimina dal set */
                    closed_pipe = close(signal_pipe[1]);
                    CHECK_OPERATION(closed_pipe == -1, fprintf(stderr, "Errore nella chiusura della pipe dei segnali in lettura.\n"); routine_chiusura(&pool, tid_signal); exit(-1));
                    FD_CLR(signal_pipe[1], &set); 
                    fd_max = aggiorna(set, fd_max);
                } 

                /* Uno worker ha elaborato la risposta */
                else if(fd == response_pipe[0]){ 
                    response *risp = NULL;
                    int err_resp = readn(response_pipe[0], &risp, sizeof(response*));
                    CHECK_OPERATION(err_resp <= 0,
                        if (risp) 
                            free(risp);
                        ended = 0;
                        break;
                    );
                    
                    errno = 0;
                    int err_write = write_size(risp->fd_richiesta, &(risp->errore));
                    CHECK_OPERATION(err_write == -1, FD_CLR(fd, &set); fd_max = aggiorna(set, fd_max); ); 
                    
                    if(risp->path){
                        int err_path = write_msg(risp->fd_richiesta, risp->path, (strlen(risp->path)+1)*sizeof(char));
                        CHECK_OPERATION(err_path == -1, 
                            fprintf(stderr, "Errore nell'invio del path a %d.\n", risp->fd_richiesta); 
                            int err_close = close(fd); 
                            if(err_close == -1) 
                                perror("Errore nella chiusura di un file descriptor");
                            FD_CLR(fd, &set); 
                            fd_max = aggiorna(set, fd_max);
                        );
                        free(risp->path);
                    }
                    
                    if(risp->buffer_file){
                        int err_buff = write_msg(risp->fd_richiesta, risp->buffer_file, (risp->size_buffer));
                        CHECK_OPERATION(err_buff == -1, 
                            int err_close = close(fd); 
                            if(err_close == -1) 
                                perror("Errore nella chiusura di un file descriptor");
                            FD_CLR(fd, &set); 
                            fd_max = aggiorna(set, fd_max);
                        );
                        free(risp->buffer_file);
                    } 
                    
                    if(risp->deleted){
                         int err_path = write_msg(risp->fd_richiesta, (char*)(risp->deleted)->path, (strlen((char*)(risp->deleted)->path) + 1)*sizeof(char));
                        CHECK_OPERATION(err_path == -1, 
                            fprintf(stderr, "Errore nell'invio del path del file rimosso a %d.\n", risp->fd_richiesta); 
                            int err_close = close(fd); 
                            if(err_close == -1) 
                                perror("Errore nella chiusura di un file descriptor");
                            FD_CLR(fd, &set); 
                            fd_max = aggiorna(set, fd_max);
                        );
                        
                        if((risp->deleted)->buffer == NULL){
                            risp->deleted->size_buffer = 0;
                        }
                        int err_buff = write_msg(risp->fd_richiesta, (risp->deleted)->buffer, risp->deleted->size_buffer);
                        CHECK_OPERATION(err_buff == -1, 
                            fprintf(stderr, "Errore nell'invio del file rimosso a %d.\n", risp->fd_richiesta); 
                            int err_close = close(fd); 
                            if(err_close == -1) 
                                perror("Errore nella chiusura di un file descriptor");
                            FD_CLR(fd, &set); 
                            fd_max = aggiorna(set, fd_max);
                        );
                       
                       int del = definitely_deleted(&(risp->deleted));
                        CHECK_OPERATION(del == -1, 
                            fprintf(stderr, "Errore nell'eliminazione definitiva del file.\n"); 
                            int err_close = close(fd); 
                            if(err_close == -1) 
                                perror("Errore nella chiusura di un file descriptor");
                            FD_CLR(fd, &set); 
                            fd_max = aggiorna(set, fd_max);
                        );
                    
                    }

                    free(risp);
                } else { /* E' arrivata una richiesta da un client registrato */
                    size_t size;
                    int err_read = read_size(fd, &size);
                    if(err_read == 0 || err_read == -1){
                        fprintf(table->file_log, "Il client %d ha chiuso la connessione.\n", fd);
                        FD_CLR(fd, &set); 
                        fd_max = aggiorna(set, fd_max);
                    } else {  
                        char* request = malloc(size); 
                        CHECK_OPERATION(request == NULL, 
                            fprintf(stderr, "Allocazione non andata a buon fine.\n"); 
                            int err_close = close(fd); 
                            if(err_close == -1) 
                                perror("Errore nella chiusura di un file descriptor");
                            FD_CLR(fd, &set); 
                            fd_max = aggiorna(set, fd_max);
                        );
                        
                        err_read = read_msg(fd, request, size);
                        CHECK_OPERATION(err_read == -1, 
                            fprintf(stderr, "Errore nella lettura della richiesta.\n"); 
                            int err_close = close(fd); 
                            if(err_close == -1) 
                                perror("Errore nella chiusura di un file descriptor");
                            FD_CLR(fd, &set); 
                            fd_max = aggiorna(set, fd_max);
                        );

                        size_t size_buffer = 0;
                        err_read = read_size(fd, &size_buffer);
                        CHECK_OPERATION(err_read==-1, 
                            fprintf(stderr, "Errore nella lettura della size.\n");
                            int err_close = close(fd); 
                            if(err_close == -1) 
                                perror("Errore nella chiusura di un file descriptor");
                            FD_CLR(fd, &set); 
                            fd_max = aggiorna(set, fd_max);
                        );

                        void* buffer = NULL;
                        if(size_buffer > 0){
                            buffer = malloc(size_buffer);
                            CHECK_OPERATION(buffer == NULL, 
                                fprintf(stderr, "Allocazione non andata a buon fine.\n"); 
                                int err_close = close(fd); 
                                if(err_close == -1) 
                                    perror("Errore nella chiusura di un file descriptor");
                                FD_CLR(fd, &set); 
                                fd_max = aggiorna(set, fd_max);
                            );

                            err_read = read_msg(fd, buffer, size_buffer);
                            CHECK_OPERATION(err_read == -1, 
                                fprintf(stderr, "Errore nella lettura della richiesta.\n"); 
                                int err_close = close(fd); 
                                if(err_close == -1) 
                                    perror("Errore nella chiusura di un file descriptor");
                                FD_CLR(fd, &set); 
                                fd_max = aggiorna(set, fd_max);
                            );
                        } 
                        int push_req = push_queue(request, fd, buffer, size_buffer, &(pool)->pending_requests);
                        /* Se c'e' un errore sulla push_queue la richiesta viene persa */
                        CHECK_OPERATION(push_req == -1, fprintf(stderr, "Errore nella push della coda.\n");  free(request); if(buffer){free(buffer);} continue);
                    }
                }
            }
        }
    }
    fprintf(stdout, "\nNumero di volte in cui e' stato chiamato l'algoritmo di sostituzione: %d\nMassima size ammessa: %d\nMassimo numero di file ammessi: %d\nSize del file storage raggiunta appena prima di chiudere il server: %d\nMassima size raggiunta: %d\nMassimo numero di file raggiunti: %d\n", fifo_queue->how_many_cache, table->max_size, table->max_file, table->curr_size, table->max_size_reached, table->max_file_reached);
    fprintf(table->file_log, "Max_size_reached: %d\nMax_files_reached: %d\nReplaced: %d\n", table->max_size_reached, table->max_file_reached, fifo_queue->how_many_cache);
    print_elements();
    routine_chiusura(&pool, tid_signal);
    
    free(socketname);

    return 0;
}