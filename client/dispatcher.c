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

int caller(const char *pathname);

int dispatcher(int argc, char *argv[]){
    int opt, flagf = 0, flagp = 0, time = 0, write_ops = 0, read_ops = 0, err_conn = 0, err_caller = 0, err_lock = 0, err_unlock = 0, err_close = 0;
    char *socketname = NULL, *dirnameD = NULL, *dirnamed = NULL, *path = NULL, *rest = NULL;
    
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
                path = NULL;
                rest = NULL;
                rest = realpath(optarg, path);
                CHECK_OPERATION(rest == NULL,
                    fprintf(stderr, " errore nella restituzione del path assoluto del file passato come parametro.\n");
                        return -1);

                sleep(time);
                err_caller = caller(rest);
                CHECK_OPERATION(err_caller == -1,
                    fprintf(stderr, " errore nella visione della directory.\n");
                        return -1);
                //writeFile
                err_unlock = unlockFile(rest);
                CHECK_OPERATION(err_unlock == -1,
                    fprintf(stderr, " error nella unlockFile.\n");
                        return -1);
                err_close = closeFile(rest);
                CHECK_OPERATION(err_close == -1,
                    fprintf(stderr, " error nella closeFile.\n");
                        return -1);

                free(path);

                break;
            
            /* Effettua la richiesta di scrittura dei file di una directory al server */
            case 'W': 
                write_ops = 1;
                path = NULL;
                rest = NULL;
                rest = realpath(optarg, path);
                CHECK_OPERATION(rest == NULL,
                    fprintf(stderr, " errore nella restituzione del path assoluto del file passato come parametro.\n");
                        return -1);
                sleep(time);
                err_caller = caller(rest);
                CHECK_OPERATION(err_caller == -1,
                    fprintf(stderr, " errore nella visione della directory.\n");
                        return -1);
                //writeFile
                err_unlock = unlockFile(rest);
                CHECK_OPERATION(err_unlock == -1,
                    fprintf(stderr, " error nella unlockFile.\n");
                        return -1);
                err_close = closeFile(rest);
                CHECK_OPERATION(err_close == -1,
                    fprintf(stderr, " error nella closeFile.\n");
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
                path = NULL;
                rest = NULL;
                rest = realpath(optarg, path);
                CHECK_OPERATION(rest == NULL,
                    fprintf(stderr, " errore nella restituzione del path assoluto del file passato come parametro.\n");
                        return -1);
                sleep(time);
                err_caller = caller(rest);
                CHECK_OPERATION(err_caller == -1,
                    fprintf(stderr, " errore nella visione della directory.\n");
                        return -1);
                //readFile
                err_unlock = unlockFile(rest);
                CHECK_OPERATION(err_unlock == -1,
                    fprintf(stderr, " error nella unlockFile.\n");
                        return -1);
                err_close = closeFile(rest);
                CHECK_OPERATION(err_close == -1,
                    fprintf(stderr, " error nella closeFile.\n");
                        return -1);
                free(rest);

                break;

            /* Effettua la richiesta di scrittura di 'R' file al server */
            case 'R':
                //int R = strtol(optarg, NULL, 10);
                read_ops = 1;
                sleep(time);
                err_caller = caller(rest);
                CHECK_OPERATION(err_caller == -1,
                    fprintf(stderr, " errore nella visione della directory.\n");
                        return -1);
                //readNFiles
                err_unlock = unlockFile(rest);
                CHECK_OPERATION(err_unlock == -1,
                    fprintf(stderr, " error nella unlockFile.\n");
                        return -1);
                err_close = closeFile(rest);
                CHECK_OPERATION(err_close == -1,
                    fprintf(stderr, " error nella closeFile.\n");
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
                path = NULL;
                rest = NULL;
                rest = realpath(optarg, path);
                CHECK_OPERATION(rest == NULL,
                    fprintf(stderr, " errore nella restituzione del path assoluto del file passato come parametro.\n");
                        return -1);
                sleep(time);
                err_lock = lockFile(rest);
                CHECK_OPERATION(err_lock == -1,
                    fprintf(stderr, " error nella lockFile.\n");
                        return -1);
                free(rest);

                break;
            
            /* Effettua la richiesta di rilasciare la lock su un file al server */
            case 'u':
                path = NULL;
                rest = NULL;
                rest = realpath(optarg, path);
                CHECK_OPERATION(rest == NULL,
                    fprintf(stderr, " errore nella restituzione del path assoluto del file passato come parametro.\n");
                        return -1);
                sleep(time);
                int err_unlock = unlockFile(rest);
                CHECK_OPERATION(err_unlock == -1,
                    fprintf(stderr, " error nella unlockFile.\n");
                        return -1);
                free(rest);
                
                break;
            
            /* Effettua la richiesta di cancellare un file al server */
            case 'c':
                path = NULL;
                rest = NULL;
                rest = realpath(optarg, path);
                CHECK_OPERATION(rest == NULL,
                    fprintf(stderr, " errore nella restituzione del path assoluto del file passato come parametro.\n");
                        return -1);
                sleep(time);
                int err_lock = lockFile(rest);
                CHECK_OPERATION(err_lock == -1,
                    fprintf(stderr, " error nella lockFile.\n");
                        return -1);
                int err_rem = removeFile(rest);
                CHECK_OPERATION(err_rem == -1,
                    fprintf(stderr, " error nella removeFile.\n");
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

int caller(const char *pathname){
    if(is_regular_file(pathname)){
        int err_open = openFile(pathname, O_CREATE | O_LOCK);
        CHECK_OPERATION(err_open == -1, 
            fprintf(stderr, " errore nella openFile.\n");
                return -1);
    } else if(is_directory(pathname)){
        DIR *dir = opendir(pathname);
        CHECK_OPERATION(dir == NULL, 
            fprintf(stderr, " errore sulla opendir.\n"); 
                return -1;);
        
        struct dirent *file;
        while((errno=0, file = readdir(dir))!=NULL && pathname != NULL){
            const char *reg_pat = nPath(pathname, file->d_name);
            CHECK_OPERATION(reg_pat==NULL,
                int check = closedir(dir); 
                    CHECK_OPERATION(check==-1, 
                        fprintf(stderr, "Errore nella chiusura della directory.\n"); 
                            return -1;) 
                        return -1); 
            if((strcmp(file->d_name, "..")!=0 && strcmp(file->d_name, ".")!=0) && is_directory(reg_pat)){
                caller(reg_pat);                
            }
        }
        int check = closedir(dir);
        CHECK_OPERATION((check==-1), fprintf(stderr, " errore sulla closedir.\n"); return -1;);
    }
    return 0;
}