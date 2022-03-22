/**
 * @file dispatcher.c
 * @author Sara Grecu (s.grecu1@studenti.unipi.it)
 * @brief Dispatcher delle richieste
 * @version 0.1
 * @date 2022-03-09
 * 
 */
#include <stdio.h>
#include <dispatcher.h>
#include <worker.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <check_errors.h>
#include <limits.h>
#include <errno.h>

#include <utils.h>
#include <dirent.h>
#include <sys/stat.h>

#include <client_utils.h>

int dispatcher(int argc, char *argv[]){
    int opt, flagf = 0, flagp = 0, time = 0, write_ops = 0, read_ops = 0, err_conn = 0, err_caller = 0, err_lock = 0, err_unlock = 0, err_close = 0, R = -1;
    char *socketname = NULL, *dirnameD = NULL, *dirnamed = NULL, *rest = NULL;
    
    
    /* Inizializza la variabile globale che abilita le stampe delle API */
    printer = 0;
    while ((opt = getopt(argc, argv, "hf:w:W:D:r:R:d:t:l:u:c:p")) != -1) {
        int res = msleep(time);
        CHECK_OPERATION(res == -1, fprintf(stderr, "Errore nella nanosleep.\n"); break);
        switch(opt) {
            
            /* Stampa le opzioni accettate dal client */
            case 'h': 
                fflush(stdout);
                fprintf(stdout, ("Le opzioni accettate sono:\n-h per stampare tutte le opzioni accettate;\n-f filename per scoprire a quale socket connettersi\n-w dirname per inviare una richiesta di scrittura al server per i file contenuti in dirname;\n-W file1 per inviare una richiesta di scrittura di file1 al server;\n-D dirname per specificare la directory del server in cui scrivere i file specificati nelle opzioni -w e -W;\n-r file1 per inviare una richiesta di lettura del file1 al server;\n-R per inviare una richiesta di lettura di tutti i file contenuti nel server;\n-d dirname per specificare la directory del client dove memorizzare i file specificati nelle opzioni -r e -R;\n-t time per specificare il tempo in millisecondi che intercorre tra l' invio di due richieste successive;\n-l file1 per specificare la lista dei nomi dei file su cui acquisire la mutua esclusione;\n-u file1 per specificare la lista dei nomi dei file su cui rilasciare la mutua esclusione;\n-c file1 per specificare la lista dei file da rimuovere sul server se presenti;\n-p per abilitare le stampe sullo standard output per ogni operazione.\n"));
                break;

            /* Si connette al server */
            case 'f':
                if(flagf == 0){
                    socketname = malloc(sizeof(char)*(strlen(optarg)+1)); 
                    CHECK_OPERATION(socketname == NULL, 
                        perror("Allocazione non andata a buon fine.\n");
                        return -1);
                    strcpy(socketname, optarg);
                    CHECK_OPERATION(socketname == NULL, 
                        perror("Il nome della socket non puo' essere NULL.\n"); 
                        return -1);

                    /* Apre una connessione con il server e ne gestisce l'errore */
                    const struct timespec abs = {0, 80900};
                    err_conn = openConnection((const char*)socketname, 100, abs);
                    CHECK_OPERATION(err_conn == -1, 
                        free(socketname);
                        return -1);
                }
                break;

            /* Effettua la richiesta di scrittura di un file al server */
            case 'w':
                write_ops = 1;
                rest = realpath(optarg, NULL);
                CHECK_OPERATION(rest == NULL, fprintf(stderr, "File non trovato.\n"); break);
                int err_ = open_write_append(rest, dirnameD);
                CHECK_OPERATION(err_ == -1, fprintf(stderr, "L'operazione di write/append e' fallita.\n"); free((char*) rest); break);
                
                free((char*)rest);
                
                break;
            
            /* Effettua la richiesta di scrittura dei file di una directory al server */
            case 'W': 
                write_ops = 1;
                rest = realpath(optarg, NULL);
                printf("\n\nSONO NEL -W\n");
                CHECK_OPERATION(rest == NULL, fprintf(stderr, "File non trovato.\n"); break);

                int err_mixed = caller_two(open_write_append, rest, dirnameD);
                CHECK_OPERATION(err_mixed == -1, fprintf(stderr, "L'operazione di write/append e' fallita.\n"); free((char*)rest); break);

                free((char*)rest);

                break;

            case 'D':
                if(dirnameD != NULL) free(dirnameD);
                write_ops = 0;

                dirnameD = realpath(optarg, NULL);
                CHECK_OPERATION(dirnameD == NULL, fprintf(stderr, "Directory non trovata.\n"); break);
            
                break; 

            /* Effettua la richiesta di lettura di un file al server */
            case 'r': 
                read_ops = 1;
                rest = realpath(optarg, NULL);
                CHECK_OPERATION(rest == NULL, fprintf(stderr, "File non trovato.\n"); break);
            
                /* Richiede l'apertura e la lock sulla directory o sul file identificato da rest */
                err_caller = openFile(rest, O_LOCK);
                CHECK_OPERATION(err_caller == -1, free((char*)rest); break);

                void *buf;
                size_t size;

                /* Invia la richiesta di lettura del file identificato da rest */
                int err_r = readFile(rest, &buf, &size);
                CHECK_OPERATION(err_r == -1, free((char*)rest); break);
                if(!err_r && buf){
                    /* Se riceve un buffer non vuoto lo salva su disco */
                    int err_save = save_on_disk(dirnamed, optarg, buf, size);
                    CHECK_OPERATION(err_save == -1, free((char*)rest); break);
                    free(buf);
                }

                /* Richiede il rilascio della lock sul file iile identificato da rest */
                err_unlock = unlockFile(rest);
                CHECK_OPERATION(err_unlock == -1, free((char*)rest); break);

                /* Richiede la chiusura del file identificato da rest */  
                err_close = closeFile(rest);
                CHECK_OPERATION(err_close == -1, free((char*)rest); break);
                free((char*)rest);

                break;

            /* Effettua la richiesta di scrittura di 'R' file al server */
            case 'R':
                R = strtol(optarg, NULL, 10);
                read_ops = 1;
                
                /* Invia la richiesta di lettura di R file */
                int num_file = readNFiles(R, dirnamed);
                CHECK_OPERATION(num_file == 0, break);
            
                break;
            
            case 'd':
                if(dirnamed != NULL) free(dirnamed);
                read_ops = 0;
                dirnamed = realpath(optarg, NULL);
                CHECK_OPERATION(dirnamed == NULL, fprintf(stderr, "Directory non trovata.\n"); break);
                
                break;

            case 't':
                time = strtol(optarg, NULL, 10);
        
                break;

            /* Effettua la richiesta di acquisire la lock su un file al server */
            case 'l':
                rest = realpath(optarg, NULL);
                CHECK_OPERATION(rest == NULL, fprintf(stderr, "File non trovato.\n"); break);

                /* Richiede l'apertura e la lock sulla directory o sul file identificato da rest */
                err_caller = openFile(rest, 0);
                CHECK_OPERATION(err_caller == -1, free((char*)rest); break);

                err_lock = lockFile(rest);
                CHECK_OPERATION(err_lock == -1, free((char*)rest); break);
                free((char*)rest);

                break;
            
            /* Effettua la richiesta di rilasciare la lock su un file al server */
            case 'u':
                rest = realpath(optarg, NULL);
                CHECK_OPERATION(rest == NULL, fprintf(stderr, "File non trovato.\n"); break);

                /* Invia la richiesta di rilascio della lock sul file identificato da rest */
                err_unlock = unlockFile(rest);
                CHECK_OPERATION(err_unlock == -1, free((char*)rest); break);
                free((char*)rest);
                
                break;
            
            /* Richiede di cancellare un file al server */
            case 'c':
                rest = realpath(optarg, NULL);
                CHECK_OPERATION(rest == NULL, fprintf(stderr, "File non trovato.\n"); break);

                /* Richiede l'apertura e la lock sulla directory o sul file identificato da rest */
                err_caller = openFile(rest, 0);
                CHECK_OPERATION(err_caller == -1, free((char*)rest); break);

                /* Invia la richiesta di acquisizione della lock sul file */
                int err_lock = lockFile(rest);
                CHECK_OPERATION(err_lock == -1, free((char*)rest); break);

                /* Invia la richiesta di rimozione del file identificato da rest */
                int err_rem = removeFile(rest);
                CHECK_OPERATION(err_rem == -1, free((char*)rest); break);
                
                free((char*)rest);
                
                break;
            
            /* Abilita le stampe sulle operazioni */
            case 'p':
                if(flagp == 0)
                    printer = 1;

                break;
        }
    }
    
    /* Chiude la connessione con il server */
    err_conn = closeConnection(socketname);
    CHECK_OPERATION(err_conn == -1, fprintf(stderr, "Errore nella chiusura della connessione");
        if(dirnameD != NULL) free(dirnameD); 
        if(dirnamed != NULL) free(dirnamed);
        free(socketname);
        return -1);

    /* Se -d o -D sono state usante non congiuntamente ad operazioni di lettura e scrittura, viene generato un errore */
    CHECK_OPERATION((dirnameD != NULL && write_ops == 0) || (dirnamed != NULL && read_ops == 0), 
        fprintf(stderr, "Non puoi usare -d e -D isolate, le devi usare congiuntamente a -r/-R e -w/-W rispettivamente.\n"); 
            if(dirnameD) free(dirnameD); 
            if(dirnamed) free(dirnamed);
            free(socketname);
            return -1);

    /* Libera la memoria rimasta */
    free(socketname);
    if(dirnameD)
        free(dirnameD);
    if(dirnamed)
        free(dirnamed);
    return 0;
}