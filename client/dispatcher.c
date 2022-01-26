#include <stdio.h>
#include <dispatcher.h>
#include <worker.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <check_errors.h>

int dispatcher(int argc, char *argv[]){
    int opt, flagf = 0, flagp = 0, time = 0, write_ops = 0, read_ops = 0, err_conn = 0;
    char *socketname = NULL, *dirnameD = NULL, *dirnamed = NULL;
    
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
                    socketname = malloc(sizeof(char)*strlen(optarg));  
                    CHECK_OPERATIONS(socketname == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"), return -1);
                    strcpy(socketname, optarg);
                }
                CHECK_OPERATIONS(socketname == NULL, 
                    fprintf(stderr, " il nome della socket non puo' essere NULL.\n"), 
                        return -1);

                /* Apre una connessione con il server */
                const struct timespec abs = {0, 80900};
                err_conn = openConnection((const char*)socketname, 100, abs);
                CHECK_OPERATIONS(err_conn == -1, 
                    fprintf(stderr, " errore nell'apertura della connessione.\n"), 
                        return -1);
                
                break;

            /* Effettua la richiesta di scrittura di un file al server */
            case 'w':
                write_ops = 1;
                printf("OPTARG PRIMA DI REALPATH: %s\n", optarg);
                const char* abs_path = (const char*) realpath(optarg, abs_path);
                sleep(time);
                //API per richiesta
            
                break;
            
            /* Effettua la richiesta di scrittura dei file di una directory al server */
            case 'W': 
                write_ops = 1;
                const char* abs_path = realpath(optarg, abs_path);

                sleep(time);
                //API per richiesta           
            
                break;

            case 'D':
                if(dirnameD != NULL) free(dirnameD);
                write_ops = 0;
                dirnameD = malloc(sizeof(char)*strlen(optarg));
                CHECK_OPERATIONS(dirnameD == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); free(socketname); if(dirnamed != NULL) free(dirnamed), return -1);
                strcpy(dirnameD, optarg);
            
                break;

            /* Effettua la richiesta di lettura di un file al server */
            case 'r': 
                read_ops = 1;
                const char* abs_path = realpath(optarg, abs_path);
                sleep(time);
                //API per richiesta
        
                break;

            /* Effettua la richiesta di scrittura di 'R' file al server */
            case 'R':
                //int R = strtol(optarg, NULL, 10);
                read_ops = 1;
                sleep(time);

                //API per richiesta

                break;
            
            case 'd':
                if(dirnamed != NULL) free(dirnamed);
                read_ops = 0;
                dirnamed = malloc(sizeof(char)*strlen(optarg));
                CHECK_OPERATIONS(dirnamed == NULL, 
                    fprintf(stderr, "Allocazione non andata a buon fine.\n"); 
                        free(socketname); 
                            if(dirnameD != NULL) free(dirnameD), 
                                return -1);

                strcpy(dirnamed, optarg);
                
                break;

            case 't':
                time = strtol(optarg, NULL, 10);
        
                break;

            /* Effettua la richiesta di acquisire la lock su un file al server */
            case 'l':
                const char* abs_path = realpath(optarg, abs_path);
                sleep(time);
                //API per richiesta
                break;
            
            /* Effettua la richiesta di rilasciare la lock su un file al server */
            case 'u':
                const char* abs_path = realpath(optarg, abs_path);
                sleep(time);
                //API per richiesta

                break;
            
            /* Effettua la richiesta di cancellare un file al server */
            case 'c':
                const char* abs_path = realpath(optarg, abs_path);
                sleep(time);
                //API per richiesta

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
    CHECK_OPERATIONS(err_conn == -1, fprintf(stderr, " errore nella chiusura della connessione");
        if(dirnameD != NULL) free(dirnameD); 
                free(socketname); 
                    if(dirnamed != NULL) free(dirnamed),
                        return -1);

    /* Se -d o -D sono state usante non congiuntamente ad operazioni di lettura e scrittura, viene generato un errore */
    CHECK_OPERATIONS((dirnameD != NULL && write_ops == 0) || (dirnamed != NULL && read_ops == 0), 
        fprintf(stderr, "Non puoi usare -d e -D isolate, le devi usare congiuntamente a -r/-R e -w/-W rispettivamente.\n"); 
            free(dirnameD); 
                free(socketname); 
                    if(dirnamed != NULL) free(dirnamed), 
                        return -1);
    
    /* Libera la memoria rimasta */
    free(socketname);
    if(dirnameD != NULL)
        free(dirnameD);
    if(dirnamed != NULL)
        free(dirnamed);

    return 0;
}