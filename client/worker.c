#include <stdio.h>
#include <stdlib.h>
#include <worker.h>

#include <sys/socket.h>
#include <check_errors.h>
#include <errno.h>

#include <sys/un.h>
#include <string.h>
#include <sys/types.h>

#include <unistd.h>
#include <socketIO.h>
#include <utils.h>


//TODO:il += nei byteletti non va bene se dopo c'e' un controllo == -1

int openConnection(const char* sockname, int msec, const struct timespec abstime){
    /* Crea il socket */
    fd_skt = socket(AF_UNIX, SOCK_STREAM, 0);
    CHECK_OPERATION(fd_skt == -1, errno = EINVAL; return -1);

    /* Salva sockname nella variabile globale socketname */
    socketname = malloc(sizeof(char) * strlen(sockname)+1);
    CHECK_OPERATION(socketname == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); errno = EINVAL; return -1);
    strcpy((char*)socketname, sockname);
    struct sockaddr_un sa;
    struct timespec abstime_temp = abstime;
    strcpy(sa.sun_path, sockname);
    sa.sun_family = AF_UNIX;

    /* Cerca di connettersi con il server tramite la socket creata */
    int success_connection = connect(fd_skt, (struct sockaddr *)&sa, sizeof(sa));
    while(success_connection == -1 && abstime_temp.tv_nsec>0){
        struct timespec t={0, (msec*90)};
        int success_nanosleep = nanosleep(&t, &abstime_temp); 
        CHECK_OPERATION((success_nanosleep==-1)&& printer == 1, fprintf(stderr, "Errore sulla nanosleep.\n"); return -1);
        abstime_temp.tv_nsec -= t.tv_nsec;
        success_connection = connect(fd_skt, (struct sockaddr *)&sa, sizeof(sa));
    }
    CHECK_OPERATION(success_connection==-1 && printer == 1, fprintf(stderr, "E' stata eseguita l'operazione 'openConnection' e non e' andata a buon fine.\n"); return -1); 
    CHECK_OPERATION(printer == 1, fprintf(stdout, "E' stata eseguita l'operazione 'openConnection' con successo.\n"); return 0);
    return 0;
}

int closeConnection(const char* sockname){
    CHECK_OPERATION((strcmp(sockname, socketname)!=0), 
        fprintf(stderr, "Il nome assegnato al socket e' scorretto.\n"); 
            errno = EINVAL;   
                return -1);

    close(fd_skt);
    free((char*)socketname);
    CHECK_OPERATION(printer == 1, fprintf(stdout, "E' stata eseguita la closeConnection con successo.\n"); return 0);

    return 0;
}

int openFile(const char *pathname, int flags){
    CHECK_OPERATION(pathname == NULL && (flags != 6 || flags != 2 || flags != 4), 
        fprintf(stderr, "Parametro non valido:");
            return -1); 
    
    /* Determina il tipo di richiesta da effettuare in base al valore di flags */
    char *request = NULL;
    if(flags == 6){
        request = "create;lock;";
    } else if(flags == 2){
        request = "create;";
    } else if(flags == 4){
        request = "lock;";
    }

    /* Crea la richiesta da inviare */
    int len = strlen(pathname)+strlen(request)+1;
    char* actual_request = malloc(sizeof(char)*len);
    CHECK_OPERATION(actual_request == NULL, 
        perror("Allocazione non andata a buon fine.\n");
            return -1);
    actual_request = strcpy(actual_request, request);
    actual_request = strcat(actual_request, pathname);
    
    /* Invia la richiesta */
    int byte_scritti = write_msg(fd_skt, actual_request, len); 
    CHECK_OPERATION(byte_scritti == -1,
        fprintf(stderr, "Non e' stato possibile inviare la richiesta al server.\n"); 
            free(actual_request);
                return -1);
    free(actual_request);

    /* Legge la risposta e in base al suo valore stampa una stringa se printer e' uguale ad 1 */
    int codice;
    int byte_letti = read_msg(fd_skt, &codice, sizeof(int)); 
    CHECK_OPERATION((byte_letti == -1 || codice == 202 || codice == 505 || codice == 606) && printer != 1, return -1);
    CHECK_OPERATION(codice == 202 && printer == 1, fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stato possibile eseguire la openFile sul file %s perche' il file e' stato bloccato da un altro client.\n", byte_scritti, byte_letti, pathname); return -1);
    CHECK_OPERATION(codice == 505 && printer == 1, fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stato possibile eseguire la openFile sul file %s  perche' il file esiste gia'.\n", byte_scritti, byte_letti, pathname);  return -1);
    CHECK_OPERATION(codice == 606 && printer == 1, fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stato possibile eseguire la openFile sul file %s il file non esiste e non e' stato specificato O_CREATE.\n", byte_scritti, byte_letti, pathname); return -1);
    CHECK_OPERATION(codice == 0 && printer == 1, fprintf(stdout, "Byte scritti: %d e byte letti:%d\nE' stata eseguita la openFile sul file %s con successo.\n", byte_scritti, byte_letti, pathname); return 0); 

    return 0;
}

int lockFile(const char* pathname){
    CHECK_OPERATION(pathname == NULL, 
        fprintf(stderr, "Parametro non valido.\n");
            return -1); 

    /* Crea la richiesta da inviare */
    int len = strlen(pathname)+strlen("lock;")+1;
    char* actual_request = malloc(sizeof(char)*len);
    CHECK_OPERATION(actual_request == NULL, 
        perror("Allocazione non andata a buon fine.\n");
            return -1);
    actual_request = strcpy(actual_request, "lock;");
    actual_request = strcat(actual_request, pathname);
    
    /* Invia la richiesta */
    int byte_scritti = write_msg(fd_skt, actual_request, len); 
    CHECK_OPERATION(byte_scritti == -1,
        fprintf(stderr, "Non e' stato possibile inviare la richiesta al server.\n"); 
            free(actual_request);
                return -1);
    free(actual_request);

    /* Legge la risposta e in base al suo valore stampa una stringa se printer e' uguale ad 1 */
    int codice;
    int byte_letti = read_msg(fd_skt, &codice, sizeof(int)); 
    CHECK_OPERATION((byte_letti == -1 || codice == 303 || codice == 707 || codice == 808) && printer != 1, return -1);

    while(codice == 202){
        fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stato possibile eseguire la lockFile sul file %s perche' il file e' stato bloccato da un altro client.\n", byte_scritti, byte_letti, pathname);
        byte_letti = read_msg(fd_skt, &codice, sizeof(int)); 
    }
    CHECK_OPERATION(codice == 303 && printer == 1, fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stato possibile eseguire la lockFile sul file %s  perche' non puoi richiederla dopo la closeFile.\n", byte_scritti, byte_letti, pathname);  return -1);
    CHECK_OPERATION(codice == 707 && printer == 1, fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stato possibile eseguire la lockFile sul file %s  perche' il file non esiste.\n", byte_scritti, byte_letti, pathname);  return -1);
    CHECK_OPERATION(codice == 808 && printer == 1, fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stato possibile eseguire la lockFile sul file %s  perche' devi chiamare prima la openFile.\n", byte_scritti, byte_letti, pathname);  return -1);
    CHECK_OPERATION((codice == 0 || codice == 909) && printer == 1, fprintf(stdout, "Byte scritti: %d e byte letti:%d\nE' stata eseguita la lockFile sul file %s con successo.\n", byte_scritti, byte_letti, pathname); return 0); 

    return 0;
}

int unlockFile(const char* pathname){
    CHECK_OPERATION(pathname == NULL, 
        fprintf(stderr, "Parametro non valido.\n");
            return -1); 

    /* Crea la richiesta da inviare */
    int len = strlen(pathname)+strlen("unlock;")+1;
    char* actual_request = malloc(sizeof(char)*len);
    CHECK_OPERATION(actual_request == NULL, 
        perror("Allocazione non andata a buon fine.\n");
            return -1);
    actual_request = strcpy(actual_request, "unlock;");
    actual_request = strcat(actual_request, pathname);

    /* Invia la richiesta */
    int byte_scritti = write_msg(fd_skt, actual_request, len); 
    CHECK_OPERATION(byte_scritti == -1,
        fprintf(stderr, "Non e' stato possibile inviare la richiesta al server.\n"); 
            free(actual_request);
                return -1);
    free(actual_request);

    /* Legge la risposta e in base al suo valore stampa una stringa se printer e' uguale ad 1 */
    int codice;
    int byte_letti = read_msg(fd_skt, &codice, sizeof(int)); 
    CHECK_OPERATION((byte_letti == -1 || codice == 202 || codice == 101 || codice == 303 || codice == 707 || codice == 808) && printer != 1, return -1);
    CHECK_OPERATION(codice == 101 && printer == 1, fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stato possibile eseguire la unlockFile sul file %s perche' il file non e' in stato di lock.\n", byte_scritti, byte_letti, pathname); return -1);
    CHECK_OPERATION(codice == 202 && printer == 1, fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stato possibile eseguire la unlockFile sul file %s perche' il file e' stato bloccato da un altro client.\n", byte_scritti, byte_letti, pathname); return -1);
    CHECK_OPERATION(codice == 303 && printer == 1, fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stato possibile eseguire la unlockFile sul file %s perche' non puoi richiederla dopo la closeFile.\n", byte_scritti, byte_letti, pathname);  return -1);
    CHECK_OPERATION(codice == 707 && printer == 1, fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stato possibile eseguire la unlockFile sul file %s perche' il file non esiste.\n", byte_scritti, byte_letti, pathname);  return -1);
    CHECK_OPERATION(codice == 808 && printer == 1, fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stato possibile eseguire la unlockFile sul file %s perche' devi chiamare prima la openFile.\n", byte_scritti, byte_letti, pathname);  return -1);
    CHECK_OPERATION(codice == 0 && printer == 1, fprintf(stdout, "Byte scritti: %d e byte letti:%d\nE' stata eseguita la unlockFile sul file %s con successo.\n", byte_scritti, byte_letti, pathname); return 0); 

    return 0;
}

int removeFile(const char* pathname){
    CHECK_OPERATION(pathname == NULL, 
        fprintf(stderr, "Parametro non valido.\n");
            return -1); 

    /* Crea la richiesta da inviare */
    int len = strlen(pathname)+strlen("remove;")+1;
    char* actual_request = malloc(sizeof(char)*len);
    CHECK_OPERATION(actual_request == NULL, 
        perror("Allocazione non andata a buon fine:");
            return -1);
    actual_request = strcpy(actual_request, "remove;");
    actual_request = strcat(actual_request, pathname);

    /* Invia la richiesta */
    int byte_scritti = write_msg(fd_skt, actual_request, len); 
    CHECK_OPERATION(byte_scritti == -1,
            free(actual_request);
                return -1);
    free(actual_request);

    /* Legge la risposta e in base al suo valore stampa una stringa se printer e' uguale ad 1 */
    int codice;
    int byte_letti = read_msg(fd_skt, &codice, sizeof(int)); 
    CHECK_OPERATION((byte_letti == -1 || codice == 101 || codice == 202 || codice == 303 || codice == 707) && printer != 1, return -1);
    CHECK_OPERATION(codice == 101 && printer == 1, fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stato possibile eseguire la removeFile  sul file %s perche' il file non e' in stato di lock.\n", byte_scritti, byte_letti, pathname); return -1);
    CHECK_OPERATION(codice == 202 && printer == 1, fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stato possibile eseguire la removeFile sul file %s perche' il file e' stato bloccato da un altro client.\n", byte_scritti, byte_letti, pathname); return -1);
    CHECK_OPERATION(codice == 303 && printer == 1, fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stato possibile eseguire la removeFile sul file %s perche' non puoi richiederla dopo la closeFile.\n", byte_scritti, byte_letti, pathname);  return -1);
    CHECK_OPERATION(codice == 707 && printer == 1, fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stato possibile eseguire la removeFile sul file %s perche' il file non esiste.\n", byte_scritti, byte_letti, pathname);  return -1);
    CHECK_OPERATION(codice == 0 && printer == 1, fprintf(stdout, "Byte scritti: %d e byte letti:%d\nE' stata eseguita la removeFile sul file %s con successo.\n", byte_scritti, byte_letti, pathname); return 0); 

    return 0;
}

int closeFile(const char* pathname){
    CHECK_OPERATION(pathname == NULL, 
        fprintf(stderr, "Parametro non valido:");
            return -1); 

    /* Crea la richiesta da inviare */
    int len = strlen(pathname)+strlen("close;")+1;
    char* actual_request = malloc(sizeof(char)*len);
    CHECK_OPERATION(actual_request == NULL, 
        perror("Allocazione non andata a buon fine.\n");
            return -1);
    actual_request = strcpy(actual_request, "close;");
    actual_request = strcat(actual_request, pathname);
    
    /* Invia la richiesta */
    int byte_scritti = write_msg(fd_skt, actual_request, len); 
    CHECK_OPERATION(byte_scritti == -1,
        fprintf(stderr, "Non e' stato possibile inviare la richiesta al server.\n"); 
            free(actual_request);
                return -1);
    free(actual_request);
    
    /* Legge la risposta e in base al suo valore stampa una stringa se printer e' uguale ad 1 */
    int codice;
    int byte_letti = read_msg(fd_skt, &codice, sizeof(int)); 
    CHECK_OPERATION((byte_letti == -1 || codice == 707 || codice == 808) && printer != 1, return -1);
    CHECK_OPERATION(codice == 707 && printer == 1, fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stato possibile eseguire la closeFile sul file %s perche' il file non esiste.\n", byte_scritti, byte_letti, pathname);  return -1);
    CHECK_OPERATION(codice == 808 && printer == 1, fprintf(stdout, "Byte scritti: %d e byte letti:%d\nNon e' stato possibile eseguire la closeFile sul file %s perche' devi fare prima la openFile.\n", byte_scritti, byte_letti, pathname); return -1); 
    CHECK_OPERATION(codice == 0 && printer == 1, fprintf(stdout, "Byte scritti: %d e byte letti:%d\nE' stata eseguita la closeFile sul file %s con successo.\n", byte_scritti, byte_letti, pathname); return 0); 

    return 0;
}

int readFile(const char* pathname, void** buf, size_t *size){ 
    CHECK_OPERATION(pathname == NULL, 
        fprintf(stderr, "Parametro non valido.\n");
            return -1); 

    /* Crea la richiesta da inviare */
    int byte_letti = 0;
    int len = strlen(pathname)+strlen("read;")+1;
    char* actual_request = malloc(sizeof(char)*len);
    CHECK_OPERATION(actual_request == NULL, 
        perror("Allocazione non andata a buon fine.\n");
            return -1);
    actual_request = strcpy(actual_request, "read;");
    actual_request = strcat(actual_request, pathname);
    
    /* Invia la richiesta */
    int byte_scritti = write_msg(fd_skt, actual_request, len); 
    CHECK_OPERATION(byte_scritti == -1,
        fprintf(stderr, "Non e' stato possibile inviare la richiesta al server.\n"); 
            free(actual_request);
                return -1);
    free(actual_request);

    /* Legge la risposta dal server */
    int codice;
    byte_letti = read_msg(fd_skt, &codice, sizeof(int)); 
    CHECK_OPERATION(byte_letti == -1 && printer != 1, return -1);
    CHECK_OPERATION(byte_letti == -1 && printer == 1,
        fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stata eseguita la readFile sul file %s con successo.\n", byte_scritti, byte_letti, pathname); 
            return -1;);

    /* Se l'operazione e' andata a buon fine */
    if(codice == 0){
        byte_letti += read_size(fd_skt, size); 
        CHECK_OPERATION(byte_letti == -1 && printer != 1, return -1);
        CHECK_OPERATION(byte_letti == -1 && printer == 1, 
            fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stata eseguita la readFile sul file %s con successo.\n", byte_scritti, byte_letti, pathname); 
                free(actual_request);
                    return -1);
        
        *buf = malloc(sizeof(char)*(*size));
        CHECK_OPERATION(*buf == NULL, 
            perror("Allocazione non andata a buon fine:");
                return -1);
        byte_letti += read_msg(fd_skt, *buf, sizeof(*size)); 
        CHECK_OPERATION(byte_letti == -1 && printer != 1, 
            return -1);
        CHECK_OPERATION(byte_letti == -1 && printer == 1, 
            fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stata eseguita la readFile sul file %s con successo.\n", byte_scritti, byte_letti, pathname); 
                free(actual_request);
                    return -1);

        CHECK_OPERATION(printer == 1, fprintf(stdout, "Byte scritti: %d e byte letti:%d\nE' stata eseguita la readFile sul file %s con successo.\n", byte_scritti, byte_letti, pathname));
    } 

    /* Se l'operazione non e' andata a buon fine */
    CHECK_OPERATION(codice == 707 && printer == 1, fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stata eseguita la readFile sul file %s con successo perche' il file non esiste.\n", byte_scritti, byte_letti, pathname); 
        return -1);
    CHECK_OPERATION(codice == 101 && printer == 1, fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stata eseguita la readFile sul file %s con successo perche' il file non e' instato di lock.\n", byte_scritti, byte_letti, pathname); 
        return -1);
    CHECK_OPERATION(codice == 303 && printer == 1, fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stata eseguita la readFile sul file %s con successo perche' non e' possibile eseguire operazioni dopo la closeFile.\n", byte_scritti, byte_letti, pathname); 
        return -1);
    CHECK_OPERATION(codice == 808 && printer == 1, fprintf(stderr, "Byte scritti: %d e byte letti:%d\nNon e' stata eseguita la readFile sul file %s con successo perche' devi richiedere prima la openFile.\n", byte_scritti, byte_letti, pathname); 
        return -1);

    CHECK_OPERATION((codice == 707 || codice == 101 || codice == 303 || codice == 808) && printer != 1, return -1);
    
    return 0;
}

int writeFile(const char* pathname, const char* dirname){ //TODO: DA RISCRIVERE TOTALMENTE
    CHECK_OPERATION(pathname == NULL, 
        fprintf(stderr, "Parametro non valido:");
            return -1); 

    /* Se la directory in cui memorizzare eventuali file eliminati dal server non e' NULL, viene inviata insieme alla richiesta di write */
    int len = strlen(pathname)+strlen("write;")+1;
    char* actual_request = malloc(sizeof(char)*len);
    CHECK_OPERATION(actual_request == NULL, 
        perror("Allocazione non andata a buon fine:");
            return -1);
    actual_request = strcpy(actual_request, "write;");
    actual_request = strcat(actual_request, pathname);
    
    /* Invia la richiesta */
    int byte_scritti = write_msg(fd_skt, actual_request, len); 
    CHECK_OPERATION(byte_scritti == -1,
        free(actual_request);
            return -1);
    free(actual_request);
    

    /* Legge la risposta e in base al suo valore stampa una stringa se printer e' uguale ad 1 */
    int codice;
    int byte_letti = read_msg(fd_skt, &codice, sizeof(int)); 
    CHECK_OPERATION((byte_letti == -1 || byte_letti == 808) && printer != 1, return -1);
    CHECK_OPERATION(byte_letti == 808 && printer == 1, 
        fprintf(stdout, "Byte scritti: %d e byte letti:%d\nNon e' stata eseguita la writeFile sul file %s con successo perche' devi fare prima la openFile.\n", byte_scritti, byte_letti, pathname);
            return -1);
    CHECK_OPERATION(codice == 0 && printer == 1, 
        fprintf(stdout, "Byte scritti: %d e byte letti:%d\nE' stata eseguita la writeFile sul file %s con successo.\n", byte_scritti, byte_letti, pathname); 
            return 0); 

    if(codice == 010){
        fprintf(stdout, "Byte scritti: %d e byte letti:%d\nE' stata eseguita la writeFile sul file %s con successo, liberando dello spazio.\n", byte_scritti, byte_letti, pathname); 
        size_t size_old;
        char* old_file;
        byte_letti = read_size(fd_skt, &size_old);
        CHECK_OPERATION(byte_letti == -1, return -1);
        old_file = malloc(sizeof(char)*size_old);
        CHECK_OPERATION(old_file == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return -1);
        byte_letti = read_msg(fd_skt, old_file, size_old);
        //TODO: scrivi su disco
        free(old_file);
    }

    return 0;
}

int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname){
    CHECK_OPERATION(pathname == NULL, 
        fprintf(stderr, "Parametro non valido:");
            return -1); 

    char *actual_request = NULL;
    int byte_scritti = 0, len = -1;

    if(dirname!=NULL){
        len = strlen(pathname)+strlen("append;")+strlen(dirname)+strlen(";")+1;
        actual_request = malloc(sizeof(char)*len);
        CHECK_OPERATION(actual_request == NULL, 
            perror("Allocazione non andata a buon fine:");
                return -1);
        actual_request = strcpy(actual_request, "append;");
        actual_request = strcat(actual_request, pathname);
        actual_request = strcat(actual_request, ";");
        actual_request = strcat(actual_request, dirname);
        
        /* Invia la richiesta */
        int byte_scritti = write_msg(fd_skt, actual_request, len); 
        CHECK_OPERATION(byte_scritti == -1,
            fprintf(stderr, "Non e' stato possibile inviare la richiesta al server:"); 
                free(actual_request);
                    return -1);
        free(actual_request);
    } else {
        len = strlen(pathname)+strlen("append;")+1;
        actual_request = malloc(sizeof(char)*len);
        CHECK_OPERATION(actual_request == NULL, 
            perror("Allocazione non andata a buon fine:");
                return -1);
        actual_request = strcat(actual_request, "append;");
        actual_request = strcat(actual_request, pathname);
        
        /* Invia la richiesta */
        int byte_scritti = write_msg(fd_skt, actual_request, len); 
        CHECK_OPERATION(byte_scritti == -1,
            fprintf(stderr, "Non e' stato possibile inviare la richiesta al server:"); 
                free(actual_request);
                    return -1);
        free(actual_request);
    }

    /* Legge la risposta e in base al suo valore stampa una stringa se printer e' uguale ad 1 */
    int codice;
    int byte_letti = read_msg(fd_skt, &codice, sizeof(int)); 
    CHECK_OPERATION(byte_letti == -1 && printer != 1, 
        return -1);
    CHECK_OPERATION(byte_letti == 808 && printer != 1, 
        return -1);
    CHECK_OPERATION(byte_letti == 808 && printer == 1, 
        fprintf(stdout, "Byte scritti: %d e byte letti:%d\nNon e' stata eseguita la writeFile sul file %s con successo perche' devi fare prima la openFile: ", byte_scritti, byte_letti, pathname);
            return -1);
    CHECK_OPERATION(codice == 0 && printer == 1, 
        fprintf(stdout, "Byte scritti: %d e byte letti:%d\nE' stata eseguita la writeFile sul file %s con successo: ", byte_scritti, byte_letti, pathname); 
            return 0); 
    CHECK_OPERATION(codice == 010 && printer == 1, 
        fprintf(stdout, "Byte scritti: %d e byte letti:%d\nE' stata eseguita la writeFile sul file %s con successo, liberando dello spazio: ", byte_scritti, byte_letti, pathname); 
            return 0); 
    
    return 0;
}
