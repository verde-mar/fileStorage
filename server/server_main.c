#include <stdio.h>
#include <signal.h>
#include <check_errors.h>

#include <threadpool.h>
#include <gestore.h>
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
    CHECK_OPERATION(err_d1 == -1, fprintf(stderr, "Errore nella distruzione del threadpool.\n"); exit(-1));

    int err_d2 = destroy_hashtable();
    CHECK_OPERATION(err_d2 == -1, fprintf(stderr, "Errore nella distruzione della tabella hash.\n"); exit(-1));

    int err_join = pthread_join(tid_signal, NULL);
    CHECK_OPERATION(err_join != 0, fprintf(stderr, "Errore nell'attesa del gestore dei segnali.\n"); exit(-1));

}

/**
 * @brief Invia un errore predefinito al client quando c'e' un errore nella comunicazione
 * 
 * @param fd File descriptor del client a cui inviare l'errore
 */
void failed_communication(int fd){
    size_t risp = -1;
    int err_write = write_size(fd, &risp);
    CHECK_OPERATION(err_write == -1, failed_communication(fd));
}

int main(int argc, char const *argv[]) {
    int size = 100; //temporaneo
    int fd_skt;
    char *socket_name = "socket"; //temporaneo
    int num_file = 10;
    char *log_file = "./log_file.txt";

    /* Crea la pipe da utilizzare per inviare l'avvenuta ricezione di un segnale al main */
    int signal_pipe[2];
    int err_pipe_signal = pipe(signal_pipe);
    CHECK_OPERATION(err_pipe_signal==-1, fprintf(stderr, "Errore nella creazione della signal_pipe.\n"); exit(-1));

    /* Crea la pipe da utlizzare dagli worker per inviare le risposte al master */
    int response_pipe[2];
    int err_pipe_response = pipe(response_pipe);
    CHECK_OPERATION(err_pipe_response==-1, fprintf(stderr, "Errore nella creazione della response_pipe .\n"); exit(-1));

    /* Definisce la maschera dei segnali */
    sigset_t mask;
    int err_set_mask = set_mask(&mask);
    CHECK_OPERATION(err_set_mask == -1, fprintf(stderr, "Errore nel setting della maschera.\n"); exit(-1));

    /* Ignora SIGPIPE per evitare di essere terminato da una scrittura su un socket */
    struct sigaction s;
    memset(&s,0,sizeof(s));    
    s.sa_handler=SIG_IGN;
    CHECK_OPERATION((sigaction(SIGPIPE,&s,NULL) ) == -1, fprintf(stderr, "Errore nella sigaction.\n"); exit(-1));

    /* Crea il threadpool */
    threadpool_t* pool;
    int err_create_pool = create_threadpool(&pool, 2, response_pipe[1]);
    CHECK_OPERATION(err_create_pool == -1, fprintf(stderr, "Errore nella creazione del theradpool.\n"); return -1);

    /* Crea la tabella hash */
    int err_hash = create_hashtable(size, num_file, log_file);
    CHECK_OPERATION(err_hash == -1, fprintf(stderr, "Errore nella creazione della tabella hash.\n"); exit(-1));

    /* Avvia il thread gestore dei segnali */
    sigHandler_t handlerArgs = { &mask, signal_pipe[1] };
    pthread_t tid_signal;
    int err_signal = pthread_create(&tid_signal, NULL, &gestore_segnali, &handlerArgs);
    CHECK_OPERATION((err_signal==-1), fprintf(stderr, "Errore nella creazione del gestore dei segnali.\n"); exit(-1)); 

    /* Effettua le operazioni di bind e listen */
    fd_set set, rdset;
    int fd_num = bind_listen(&fd_skt, &set, socket_name);
    CHECK_OPERATION(err_signal==-1, exit(-1));
    
    /* Crea e inizializza i set di file descriptor */
    FD_ZERO(&rdset); 
    FD_ZERO(&set);
    
    /* Aggiunge i fd delle pipe in lettura e del fd della socket al set */
    FD_SET(fd_skt, &set);
    FD_SET(signal_pipe[0], &set);
    FD_SET(response_pipe[0], &set);

    /* Calcola il massimo dei file descriptor, tra quelli delle pipe e quella rest */
    int fd_max;
    if(fd_num < signal_pipe[0])
        fd_max = signal_pipe[0];
    else
        fd_max = fd_num;

    if(fd_max < response_pipe[0])
        fd_max = response_pipe[0]; 

    int err_select;
    int no_more = 1, end = 1;
    while(end){
        rdset = set;
        err_select = select(fd_max+1, &rdset, NULL, NULL, NULL);
        CHECK_OPERATION(err_select==-1, fprintf(stderr, "Errore nella select.\n"); exit(-1));
        for (int fd = 0; fd<=fd_max;fd++) {
            int fd_c;
            if (FD_ISSET(fd, &rdset)) {
                if (fd == fd_skt && no_more) { /* E' arrivata una nuova richiesta */
                    fd_c = accept(fd_skt,NULL,0);
                    CHECK_OPERATION(fd_c == -1, fprintf(stderr, "Errore nell'accetazione di un client.\n"); exit(-1));
                    fprintf(table->file_log, "E' stata accettata la connessione di %d.\n", fd_c);
                    FD_SET(fd_c, &set);
                    if (fd_c > fd_max) fd_max = fd_c; 
                } else if(fd == response_pipe[0]){ /* Uno worker ha elaborato la risposta */
                    response *risp;
                    int err_resp = readn(response_pipe[0], &risp, sizeof(response*));
                    CHECK_OPERATION(err_resp == -1 , fprintf(stderr, "Errore sulla readn nella lettura della risposta."); continue); 
                    fprintf(table->file_log, "Il worker ha elaborato la richiesta di %d ", fd_c);
                    
                    int err_write = write_size(risp->fd_richiesta, &risp->errore);
                    CHECK_OPERATION(err_write == -1, fprintf(stderr, "Errore nella scrittura della size del messaggio .\n"); failed_communication(fd);); 
                    fprintf(table->file_log, "con risultato: %ld.\n", risp->errore);
                    
                    if(risp->path){
                        int err_path = write_msg(risp->fd_richiesta, risp->path, (strlen(risp->path)+1)*sizeof(char));
                        CHECK_OPERATION(err_path == -1, fprintf(stderr, "Errore nell'invio del path.\n"); failed_communication(fd););
                        fprintf(table->file_log, "Per inviare la risposta della richiesta di %d e' stato necessario scrivere %ld byte per il path ", fd_c, (strlen(risp->path)+1)*sizeof(char));

                    }
                    
                    if(risp->buffer_file){
                        int err_buff = write_msg(risp->fd_richiesta, risp->buffer_file, (risp->size_buffer));
                        CHECK_OPERATION(err_buff == -1, fprintf(stderr, "Errore nell'invio del file.\n"); failed_communication(fd););
                        fprintf(table->file_log, "Per inviare la risposta della richiesta di %d e' stato necessario scrivere %ld byte per il buffer.\n", fd_c, risp->size_buffer);

                    }
                    
                    if(risp->deleted){
                        if((risp->deleted)->buffer){
                            int err_path = write_msg(risp->fd_richiesta, (char*)(risp->deleted)->path, (strlen((risp->deleted)->path) + 1)*sizeof(char));
                            CHECK_OPERATION(err_path == -1, fprintf(stderr, "Errore nell'invio del path del file.\n"); failed_communication(fd););
                            fprintf(table->file_log, "Per inviare la risposta della richiesta di %d e' stato necessario scrivere %ld byte per il path ", fd_c, (strlen(risp->path)+1)*sizeof(char));
                            
                            int err_buff = write_msg(risp->fd_richiesta, (risp->deleted)->buffer, risp->deleted->size_buffer);
                            CHECK_OPERATION(err_buff == -1, fprintf(stderr, "Errore nell'invio del file.\n"); failed_communication(fd););
                            fprintf(table->file_log, "Per inviare la risposta della richiesta di %d e' stato necessario scrivere %ld byte per il buffer.\n", fd_c, risp->size_buffer);
                            
                            int del = definitely_deleted(&(risp->deleted));
                            CHECK_OPERATION(del == -1, fprintf(stderr, "Errore nella eliminazione definitiva del nodo.\n"););
                        }
                    }

                    FD_SET(risp->fd_richiesta, &set);
                    if(risp->fd_richiesta > fd_max) fd_max = fd;

                    free(risp);
                } else if(fd == signal_pipe[0]){ /* E' arrivato un segnale di chiusura */
                    int sig;
                    /* Legge il tipo di segnale dalla pipe */
                    int err_readn = readn(signal_pipe[0], &sig, sizeof(int));
                    CHECK_OPERATION(err_readn == -1 , fprintf(stderr, "Errore sulla readn nella lettura del segnale arrivato."); continue);
                    fprintf(table->file_log, "E' stato ricevuto il segnale di %d ", sig);
                    
                    /* Assegna al flag 0 cosi' che non siano accettate nuove richieste di connessione */
                    no_more = 0; 
                
                    if(sig == 2||sig == 3){ /* Se arriva un SIGINT o un SIGQUIT */
                        end = 0;
                        close(response_pipe[0]);
                        fprintf(table->file_log, "quindi si procede per la chiusura immediata.\n");
                    } else { /* Se arriva un SIGHUP */
                        fprintf(table->file_log, "quindi si procede per la chiusura lenta.\n");
                        for(int i = 0; i < pool->num_thread; i++) {
                            int err_push = push_queue(NULL, -1, NULL, 0, &(pool->pending_requests));
                            CHECK_OPERATION(err_push == -1, fprintf(stderr, "Errore nell'invio di richieste NULL per la terminazione.\n"); exit(-1));
                        }
                    }
                    close(signal_pipe[0]);
                } else { /* E' arrivata una richiesta da un client registrato */
                    fprintf(table->file_log, "E' arrivata una richiesta dal client %d: ", fd);
                    size_t size;
                    int err_read = read_size(fd, &size);
                    if(err_read != -1 && err_read!=0){
                        char* request = malloc(size); 
                        CHECK_OPERATION(request == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); failed_communication(fd););
                        
                        err_read = read_msg(fd, request, size);
                        CHECK_OPERATION(err_read == -1, fprintf(stderr, "Errore nella lettura della richiesta.\n"); failed_communication(fd);); 

                        fprintf(table->file_log, "%s\n", request);

                        size_t size_buffer;
                        err_read = read_size(fd, &size_buffer);
                        CHECK_OPERATION(err_read==-1, fprintf(stderr, "Errore nella lettura della size.\n"); failed_communication(fd););

                        void* buffer = NULL;
                        if(size_buffer > 0){
                            buffer = malloc(size_buffer);
                            CHECK_OPERATION(buffer == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); failed_communication(fd););

                            err_read = read_msg(fd, buffer, size_buffer);
                            CHECK_OPERATION(err_read == -1, fprintf(stderr, "Errore nella lettura della richiesta.\n"); failed_communication(fd););
                        } 
                        
                        int push_req = push_queue(request, fd, buffer, size_buffer, &(pool)->pending_requests);
                        CHECK_OPERATION(push_req == -1, fprintf(stderr, "Errore nella push della coda.\n"); exit(-1));
                    } else if(err_read == 0){
                        fprintf(stdout, "Client disconnesso.\n");
                        FD_CLR(fd, &set); 
                        aggiorna(set, fd_max);
                    } 
                    /* Se fallisce la lettura del messaggio */
                    else if(err_read == -1){
                        fprintf(stderr, "Errore nella lettura della size del messaggio.\n");
                        failed_communication(fd);
                    }
                }
            }
        }
        if(pool->curr_threads == 0){
            end = 0;
            int close_pipe = close(response_pipe[0]);
            CHECK_OPERATION(close_pipe == -1, fprintf(stderr, "Errore nella chiusura della pipe.\n"); exit(-1));
        }
    }
    fprintf(stdout, "\nNumero di volte in cui e' stato chiamato l'algoritmo di sostituzione: %d\nMassima size ammessa: %d\nMassimo numero di file ammessi: %d\nSize del file storage raggiunta appena prima di chiudere il server: %d\nMassima size raggiunta: %f\nMassimo numero di file raggiunti: %d\n", fifo_queue->how_many_cache, table->max_size, table->max_file, table->curr_size, to_Mbytes(table->max_size_reached), table->max_file_reached);
    print_elements();
    routine_chiusura(&pool, tid_signal);

    return 0;
}