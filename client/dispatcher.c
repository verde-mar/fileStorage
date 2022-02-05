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

int dispatcher(int argc, char *argv[]){
    int opt, flagf = 0, flagp = 0, time = 0, write_ops = 0, read_ops = 0, err_conn = 0, err_caller = 0, err_lock = 0, err_unlock = 0, err_close = 0, R = -1;
    char *socketname = NULL, *dirnameD = NULL, *dirnamed = NULL, *rest = NULL;
    
    /* Inizializza la variabile globale che abilita le stampe delle API */
    printer = 0;
    while ((opt = getopt(argc, argv, "hf:w:W:D:r:R:d:t:l:u:c:p")) != -1) {
        switch(opt) {

            /* Stampa le opzioni accettate dal client */
            case 'h': 
                fflush(stdout);
                fprintf(stdout, ("Le opzioni accettate sono:\n-h per stampare tutte le opzioni accettate;\n-f filename per scoprire a quale socket connettersi\n-w dirname per inviare una richiesta di scrittura al server per i file contenuti in dirname;\n-W file1 per inviare una richiesta di scrittura di file1 al server;\n-D dirname per specificare la directory del server in cui scrivere i file specificati nelle opzioni -w e -W;\n-r file1 per inviare una richiesta di lettura del file1 al server;\n-R per inviare una richiesta di lettura di tutti i file contenuti nel server;\n-d dirname per specificare la directory del client dove memorizzare i file specificati nelle opzioni -r e -R;\n-t time per specificare il tempo in millisecondi che intercorre tra l' invio di due richieste successive;\n-l file1 per specificare la lista dei nomi dei file su cui acquisire la mutua esclusione;\n-u file1 per specificare la lista dei nomi dei file su cui rilasciare la mutua esclusione;\n-c file1 per specificare la lista dei file da rimuovere sul server se presenti;\n-p per abilitare le stampe sullo standard output per ogni operazione.\n"));
                   
                return 0;

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
                    CHECK_OPERATION(err_conn == -1 && printer == 1, 
                        fprintf(stderr, " errore nell'apertura della connessione.\n");
                            return -1);
                }
                break;

            /* Effettua la richiesta di scrittura di un file al server */
            case 'w':
                write_ops = 1;
                rest = NULL;
                rest = realpath(optarg, NULL);
                CHECK_OPERATION(rest == NULL,
                    fprintf(stderr, " errore nella restituzione del path assoluto del file passato come parametro.\n");
                        return -1);

                sleep(time);
                err_caller = caller_open(rest);
                CHECK_OPERATION(err_caller == -1,
                    fprintf(stderr, " errore nella visione della directory.\n");
                        return -1);

                int err_w = writeFile(rest, dirnameD);
                CHECK_OPERATION(err_w == -1,
                    fprintf(stderr, " errore nella writeFile.\n");
                        return -1);
                err_unlock = unlockFile(rest);
                CHECK_OPERATION(err_unlock == -1,
                    fprintf(stderr, " errore nella unlockFile.\n");
                        return -1);
                err_close = closeFile(rest);
                CHECK_OPERATION(err_close == -1,
                    fprintf(stderr, " errore nella closeFile.\n");
                        return -1);


                break;
            
            /* Effettua la richiesta di scrittura dei file di una directory al server */
            case 'W': 
                write_ops = 1;
                rest = NULL;
                rest = realpath(optarg, NULL);
                CHECK_OPERATION(rest == NULL,
                    fprintf(stderr, " errore nella restituzione del path assoluto del file passato come parametro.\n");
                        return -1);
                sleep(time);
                err_caller = caller_open(rest); //TODO: una miglioria possibile puo' essere fare una caller generica con parametri variabili
                CHECK_OPERATION(err_caller == -1,
                    fprintf(stderr, " errore nella visione della directory.\n");
                        return -1);
                int err_W = caller_write(rest, dirnameD);
                CHECK_OPERATION(err_W == -1,
                    fprintf(stderr, " errore nella unlockFile.\n");
                        return -1);
                err_unlock = unlockFile(rest);
                CHECK_OPERATION(err_unlock == -1,
                    fprintf(stderr, " errore nella unlockFile.\n");
                        return -1);

                err_close = closeFile(rest);
                CHECK_OPERATION(err_close == -1,
                    fprintf(stderr, " errore nella closeFile.\n");
                        return -1);

                free(rest);

                break;

            case 'D':
                if(dirnameD != NULL) free(dirnameD);
                write_ops = 0;
                dirnameD = malloc(sizeof(char)*strlen(optarg));
                CHECK_OPERATION(dirnameD == NULL, 
                    fprintf(stderr, "Allocazione non andata a buon fine.\n"); 
                        free(socketname); 
                            if(dirnamed != NULL) free(dirnamed);
                                return -1);
                strcpy(dirnameD, optarg);
            
                break;

            /* Effettua la richiesta di lettura di un file al server */
            case 'r': 
                read_ops = 1;
                rest = NULL;
                rest = realpath(optarg, NULL); //siamo sicuri che ci vada NULL e non PATH?
                CHECK_OPERATION(rest == NULL,
                    fprintf(stderr, " errore nella restituzione del path assoluto del file passato come parametro.\n");
                        return -1);
                sleep(time);
                err_caller = caller_open(rest);
                CHECK_OPERATION(err_caller == -1,
                    fprintf(stderr, " errore nella visione della directory.\n");
                        return -1);
                char *buf;
                size_t size;
                int err_r = readFile(rest, &buf, &size);
                CHECK_OPERATION(err_r == -1,
                    fprintf(stderr, " errore nella readFile.\n");
                        return -1;);
                
                char* find_abs_dir = realpath(dirnamed, NULL);
                CHECK_OPERATION(find_abs_dir == NULL,
                    fprintf(stderr, " errore nella restituzione del path assoluto della directory passata come parametro.\n");
                        return -1);
                int err_save = save_on_disk(find_abs_dir, rest, buf, size);
                CHECK_OPERATION(err_save == -1,
                    fprintf(stderr, " errore nella save_on_disk.\n");
                        return -1);
                if(*buf!=NULL) free(*buf); //TODO:da annotare nella relazione, il fatto che venga liberato in questo punto
                err_unlock = unlockFile(rest);
                CHECK_OPERATION(err_unlock == -1,
                    fprintf(stderr, " errore nella unlockFile.\n");
                        return -1);
                err_close = closeFile(rest);
                CHECK_OPERATION(err_close == -1,
                    fprintf(stderr, " errore nella closeFile.\n");
                        return -1);
                free(rest);

                break;

            /* Effettua la richiesta di scrittura di 'R' file al server */
            case 'R':
                R = strtol(optarg, NULL, 10);
                read_ops = 1;
                sleep(time);
                err_caller = caller_open(rest);
                CHECK_OPERATION(err_caller == -1,
                    fprintf(stderr, " errore nella visione della directory.\n");
                        return -1);
                int num_file = readNFiles(R, dirnamed);
                CHECK_OPERATION(num_file == -1,
                    fprintf(stderr, " errore nella readNFiles.\n");
                        return -1);
                CHECK_OPERATION(printer == 1, fprintf(stdout, "Sono stati letti %d file dal server.\n", num_file));
                err_unlock = unlockFile(rest);
                CHECK_OPERATION(err_unlock == -1,
                    fprintf(stderr, " errore nella unlockFile.\n");
                        return -1);
                err_close = closeFile(rest);
                CHECK_OPERATION(err_close == -1,
                    fprintf(stderr, " errore nella closeFile.\n");
                        return -1);
                free(rest);

                break;
            
            case 'd':
                if(dirnamed != NULL) free(dirnamed);
                read_ops = 0;
                dirnamed = malloc(sizeof(char)*strlen(optarg));
                CHECK_OPERATION(dirnamed == NULL, 
                    fprintf(stderr, "Allocazione non andata a buon fine.\n"); 
                        free(socketname); 
                            if(dirnameD != NULL) free(dirnameD);
                                return -1);

                strcpy(dirnamed, optarg);
                
                break;

            case 't':
                time = strtol(optarg, NULL, 10);
        
                break;

            /* Effettua la richiesta di acquisire la lock su un file al server */
            case 'l':
                rest = NULL;
                rest = realpath(optarg, NULL);
                CHECK_OPERATION(rest == NULL,
                    fprintf(stderr, " errore nella restituzione del path assoluto del file passato come parametro.\n");
                        return -1);
                sleep(time);
                err_lock = lockFile(rest);
                CHECK_OPERATION(err_lock == -1,
                    fprintf(stderr, " errore nella lockFile.\n");
                        return -1);
                free(rest);

                break;
            
            /* Effettua la richiesta di rilasciare la lock su un file al server */
            case 'u':
                rest = NULL;
                rest = realpath(optarg, NULL);
                CHECK_OPERATION(rest == NULL,
                    fprintf(stderr, " errore nella restituzione del path assoluto del file passato come parametro.\n");
                        return -1);
                sleep(time);
                int err_unlock = unlockFile(rest);
                CHECK_OPERATION(err_unlock == -1,
                    fprintf(stderr, " errore nella unlockFile.\n");
                        return -1);
                free(rest);
                
                break;
            
            /* Effettua la richiesta di cancellare un file al server */
            case 'c':
                rest = NULL;
                rest = realpath(optarg, NULL);
                CHECK_OPERATION(rest == NULL,
                    fprintf(stderr, " errore nella restituzione del path assoluto del file passato come parametro.\n");
                        return -1);
                sleep(time);
                int err_lock = lockFile(rest);
                CHECK_OPERATION(err_lock == -1,
                    fprintf(stderr, " errore nella lockFile.\n");
                        return -1);
                int err_rem = removeFile(rest);
                CHECK_OPERATION(err_rem == -1,
                    fprintf(stderr, " errore nella removeFile.\n");
                        return -1);
                
                free(rest);
                
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
    CHECK_OPERATION(err_conn == -1, fprintf(stderr, " errore nella chiusura della connessione");
        if(dirnameD != NULL) free(dirnameD); 
                free(socketname); 
                    if(dirnamed != NULL) free(dirnamed);
                        return -1);

    /* Se -d o -D sono state usante non congiuntamente ad operazioni di lettura e scrittura, viene generato un errore */
    CHECK_OPERATION((dirnameD != NULL && write_ops == 0) || (dirnamed != NULL && read_ops == 0), 
        fprintf(stderr, "Non puoi usare -d e -D isolate, le devi usare congiuntamente a -r/-R e -w/-W rispettivamente.\n"); 
            free(dirnameD); 
                free(socketname); 
                    if(dirnamed != NULL) free(dirnamed);
                        return -1);
    
    /* Libera la memoria rimasta */
    free(socketname);
    if(dirnameD != NULL)
        free(dirnameD);
    if(dirnamed != NULL)
        free(dirnamed);

    return 0;
}