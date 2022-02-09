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

#include <sys/stat.h>

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
    CHECK_OPERATION(byte_scritti == -1,
        fprintf(stderr, "Non e' stato possibile inviare la richiesta al server.\n"); 
            free(actual_request);
                return -1);
    CHECK_CODICE(printer, codice, "openFile", byte_letti, byte_scritti);

    return codice;
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
    errno = 0;
    int byte_letti = read_msg(fd_skt, &codice, sizeof(int)); 
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
            free(actual_request);
                return -1);

    CHECK_CODICE(printer, codice, "lockFile", byte_letti, byte_scritti);

    while(codice == 202){
        errno = 0;
        byte_letti += read_msg(fd_skt, &codice, sizeof(int)); 
        CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
            free(actual_request);
                return -1);
    }

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
    errno = 0;
    int byte_letti = read_msg(fd_skt, &codice, sizeof(int)); 
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
            free(actual_request);
                return -1);

    CHECK_CODICE(printer, codice, "unlockFile", byte_letti, byte_scritti);
    
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
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
            free(actual_request);
                return -1);

    CHECK_CODICE(printer, codice, "removeFile", byte_letti, byte_scritti);

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
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
            free(actual_request);
                return -1);

    CHECK_CODICE(printer, codice, "closeFile", byte_letti, byte_scritti);

    return 0;
}

int readFile(const char* pathname, void** buf, size_t *size){ 
    CHECK_OPERATION(pathname == NULL, 
        fprintf(stderr, "Parametro non valido.\n");
            return -1); 

    /* Crea la richiesta da inviare */
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
    int byte_letti = read_msg(fd_skt, &codice, sizeof(int)); 
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
            free(actual_request);
                return -1);

    if(codice == 0){
        errno = 0;
        byte_letti += read_size(fd_skt, size); 
        CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
            free(actual_request);
                return -1);
        
        *buf = malloc(sizeof(char)*(*size));
        CHECK_OPERATION(*buf == NULL, 
            fprintf(stderr, "Allocazione non andata a buon fine:"); 
                return -1);

        byte_letti += read_msg(fd_skt, *buf, (*size)); 
        CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
            free(actual_request);
                return -1);

    } 

    CHECK_CODICE(printer, codice, "readFile", byte_letti, byte_scritti);
    
    return 0;
}

int writeFile(const char* pathname, const char* dirname){ 
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

    /* Apre il file */
    FILE *new_file = fopen(pathname, "r");
    CHECK_OPERATION(new_file == NULL, fprintf(stderr, "Errore nella fopen.\n"); return -1);

    /* Scrive sul file */
    struct stat st;
    stat(pathname, &st);
    int size = st.st_size;
    char* buf = malloc(sizeof(char)*size);
    int err_fwrite = fwrite(buf, size, 1, new_file); 
    CHECK_OPERATION(err_fwrite == -1, fprintf(stderr, "Errore nella fwrite.\n"); return -1);

    /* Chiude il file */
    int check = fclose(new_file);
    CHECK_OPERATION(check == -1, fprintf(stderr, "Errore nella fclose.\n"); return -1);

    /* Invia i dati del file */
    errno = 0;
    byte_scritti += write_msg(fd_skt, buf, size); 
    CHECK_OPERATION(errno == EFAULT,
        free(buf);
            return -1);
    free(buf);

    /* Legge la risposta e in base al suo valore stampa una stringa se printer e' uguale ad 1 */
    int codice;
    int byte_letti = read_msg(fd_skt, &codice, sizeof(int)); 
    CHECK_OPERATION(byte_letti == -1,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
            free(actual_request);
                return -1);

    if(codice == 909 && dirname != NULL){
        size_t size_old, size_path;
        char *old_file, *path;

        /* Legge il path del file appena eliminato */
        errno = 0;
        byte_letti += read_size(fd_skt, &size_path);
        CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
            free(actual_request);
                return -1);
        path = malloc(sizeof(char)*size_path);
        CHECK_OPERATION(path == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return -1);

        errno = 0;
        byte_letti = read_msg(fd_skt, path, size_path);
        CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
            free(actual_request);
                return -1);

        /* Legge il file e lo memorizza su disco */
        errno = 0;
        byte_letti = read_size(fd_skt, &size_old);
        CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
            free(actual_request);
                return -1);
        old_file = malloc(sizeof(char)*size_old);
        CHECK_OPERATION(old_file == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return -1);

        errno = 0;
        byte_letti = read_msg(fd_skt, old_file, size_old);
        CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
            free(actual_request);
                return -1);

        int err_save = save_on_disk((char*)dirname, optarg, old_file, size_old);
        CHECK_OPERATION(err_save == -1, 
            fprintf(stderr, "Errore nel salvataggio su disco");
                free(path);
                    free(old_file);
                        return -1;);

        free(path);
        free(old_file);
    }

    CHECK_CODICE(printer, codice, "writeFile", byte_letti, byte_scritti);

    return codice;
}

int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname){
    CHECK_OPERATION(pathname == NULL, 
        fprintf(stderr, "Parametro non valido:");
            return -1);
    
    /* Se la directory in cui memorizzare eventuali file eliminati dal server non e' NULL, viene inviata insieme alla richiesta di write */
    int len = strlen(pathname)+strlen("append;")+1;
    char* actual_request = malloc(sizeof(char)*len);
    CHECK_OPERATION(actual_request == NULL, 
        perror("Allocazione non andata a buon fine.");
            return -1);
    actual_request = strcpy(actual_request, "append;");
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
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
            free(actual_request);
                return -1);

    if(codice == 909 && dirname != NULL){
        size_t size_old, size_path;
        char *old_file, *path;

        /* Legge il path del file appena eliminato */
        byte_letti += read_size(fd_skt, &size_path);
        CHECK_OPERATION(byte_letti == -1, return -1);
        path = malloc(sizeof(char)*size_path);
        CHECK_OPERATION(path == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return -1);
        byte_letti += read_msg(fd_skt, path, size_path);
        CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
            free(path);
                return -1;);

        /* Legge il file e lo memorizza su disco */
        byte_letti += read_size(fd_skt, &size_old);
        CHECK_OPERATION(byte_letti == -1, 
            free(path);
                return -1);
        old_file = malloc(sizeof(char)*size_old);
        CHECK_OPERATION(old_file == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); 
            free(path);
                return -1;);
       
        byte_letti += read_msg(fd_skt, old_file, size_old);
        CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
            free(path);
                free(old_file);
                    return -1;);

        int err_save = save_on_disk((char*)dirname, optarg, old_file, size_old);
        CHECK_OPERATION(err_save == -1, 
            fprintf(stderr, "Errore nel salvataggio su disco");
                free(path);
                    free(old_file);
                        return -1;);

        free(path);
        free(old_file);
    }

    CHECK_CODICE(printer, codice, "appendToFile", byte_letti, byte_scritti);
    
    return 0;
}

int readNFiles(int N, const char* dirname){
    CHECK_OPERATION(dirname == NULL, 
        fprintf(stderr, "Parametro non valido.\n");
            return -1);
    char *path, *file;
    int codice = -1, byte_scritti = -1, byte_letti = -1;
    size_t size_path = -1, size_file = -1;

    while(N>0 || codice != 111){    
        errno = 0;   
        N--; 

        /* Invia la richiesta */
        byte_scritti = write_msg(fd_skt, "read;", (strlen("read;")+1)); 
        CHECK_OPERATION(errno == EFAULT,
            fprintf(stderr, "Non e' stato possibile inviare la richiesta al server.\n"); 
                return -1);

        /* Legge la risposta dal server */
        byte_letti = read_msg(fd_skt, &codice, sizeof(int)); 
        CHECK_OPERATION(errno == EFAULT,
            fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
                    return -1);

        /* Se la lettura e' andata a buon fine */
        if(codice == 0){
            /* Legge la size del path del file da leggere */
            errno = 0;
            byte_letti += read_size(fd_skt, &size_path); 
            CHECK_OPERATION(errno == EFAULT,
            fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
                    return -1);
            
            path = malloc(sizeof(char)*size_path);
            CHECK_OPERATION(path == NULL, 
                fprintf(stderr, "Allocazione non andata a buon fine:"); 
                    return -1);

            /* Legge il path */
            byte_letti += read_msg(fd_skt, path, size_path); 
            CHECK_OPERATION(errno == EFAULT,
            fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
                    return -1);

            /* Legge la size del file da leggere */
            errno = 0;
            byte_letti += read_size(fd_skt, &size_file); 
            CHECK_OPERATION(errno == EFAULT,
            fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
                    return -1);
            
            file = malloc(sizeof(char)*size_file);
            CHECK_OPERATION(file == NULL, 
                fprintf(stderr, "Allocazione non andata a buon fine:"); 
                    return -1);

            /* Legge il file */
            byte_letti += read_msg(fd_skt, file, size_file); 
            CHECK_OPERATION(errno == EFAULT,
            fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
                    return -1);   

            /* Salva il file su disco */
            int check_save = save_on_disk((char*)dirname, path, file, size_file);
            CHECK_OPERATION(check_save == -1,
                fprintf(stderr, "Non e' stato possibile salvare il file su disco.\n");
                    return -1);

            free(file);
            free(path); 
        } 
    }
    CHECK_CODICE(printer, codice, "readNFiles", byte_letti, byte_scritti);
    

    return N;
}