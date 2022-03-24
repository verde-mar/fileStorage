/**
 * @file worker.c
 * @author Sara Grecu (s.grecu1@studenti.unipi.it)
 * @brief Contiene le api richieste
 * @version 0.1
 * @date 2022-03-09
 * 
 */
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
#include <client_utils.h>

#include <sys/stat.h>
#include <utils.h>

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
        struct timespec t={(msec*90), (msec*90)};
        int success_nanosleep = nanosleep(&t, &abstime_temp); 
        CHECK_OPERATION((success_nanosleep==-1)&& printer == 1, fprintf(stderr, "Errore sulla nanosleep.\n"); free((char*)socketname); return -1);
        abstime_temp.tv_nsec -= t.tv_nsec;
        success_connection = connect(fd_skt, (struct sockaddr *)&sa, sizeof(sa));
    }
    CHECK_OPERATION(success_connection==-1, fprintf(stderr, "E' stata eseguita l'operazione 'openConnection' e non e' andata a buon fine.\n"); free((char*)socketname); return -1); 
    CHECK_OPERATION(printer == 1, fprintf(stdout, "E' stata eseguita l'operazione 'openConnection' con successo.\n"); return 0);
    return 0;
}

int closeConnection(const char* sockname){
    CHECK_OPERATION((strcmp(sockname, socketname)!=0), 
        fprintf(stderr, "Il nome assegnato alla socket e' scorretto.\n"); 
        errno = EINVAL;   
        return -1);

    int closed = close(fd_skt);
    CHECK_OPERATION(closed == -1, fprintf(stderr, "Errore sulla chiusura della socket.\n"); return -1);
    //ma se mettessi una nanosleep per aspettare?
    free((char*)socketname);
    CHECK_OPERATION(printer == 1, fprintf(stdout, "E' stata eseguita la closeConnection con successo.\n"); return 0);

    return 0;
}

int openFile(const char *pathname, int flags){
    CHECK_OPERATION(pathname == NULL, 
        fprintf(stderr, "Parametro non valido.\n");
        return -1); 

    /* Determina il tipo di richiesta da effettuare in base al valore di flags */
    char *request = NULL;
    if(flags == (O_CREATE | O_LOCK)){
        request = "create_lock;";
    } else if(flags == O_CREATE){
        request = "create;";
    } else if(flags == 0){
        request = "open;";
    } else if(flags == O_LOCK){
        request = "open_lock;";
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
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile inviare la richiesta al server.\n"); 
            free(actual_request);
            return -1);
    size_t size = 0;
    errno = 0;
    byte_scritti += write_size(fd_skt, &size); 
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile inviare la richiesta al server.\n"); 
        free(actual_request);
        return -1);

    free(actual_request);

    /* Legge la risposta e in base al suo valore stampa una stringa se printer e' uguale ad 1 */
    size_t codice;
    int byte_letti = read_size(fd_skt, &codice); 
    CHECK_OPERATION(byte_scritti == -1,
        fprintf(stderr, "Non e' stato possibile inviare la richiesta al server.\n"); 
        free(actual_request);
        return -1);

    CHECK_CODICE(printer, codice, pathname, "openFile", byte_letti, byte_scritti);

    return codice;
}

int lockFile(const char* pathname){
    CHECK_OPERATION(pathname == NULL, fprintf(stderr, "Parametro non valido.\n"); return -1); 

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

    size_t size = 0;
    errno = 0;
    byte_scritti += write_size(fd_skt, &size); 
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile inviare la richiesta al server.\n"); 
        free(actual_request);
        return -1);

    /* Legge la risposta e in base al suo valore stampa una stringa se printer e' uguale ad 1 */
    size_t codice;
    errno = 0;
    int byte_letti = read_size(fd_skt, &codice); 
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
        free(actual_request);
        return -1);

    free(actual_request);
    
    CHECK_CODICE(printer, codice, pathname, "lockFile", byte_letti, byte_scritti);

    return 0;
}

int unlockFile(const char* pathname){
    CHECK_OPERATION(pathname == NULL, fprintf(stderr, "Parametro non valido.\n"); return -1); 

    /* Crea la richiesta da inviare */
    int len = strlen(pathname)+strlen("unlock;")+1;
    char* actual_request = malloc(sizeof(char)*len);
    CHECK_OPERATION(actual_request == NULL, 
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
        return -1);

    actual_request = strcpy(actual_request, "unlock;");
    actual_request = strcat(actual_request, pathname);

    /* Invia la richiesta */
    int byte_scritti = write_msg(fd_skt, actual_request, len); 
    CHECK_OPERATION(byte_scritti == -1,
        fprintf(stderr, "Non e' stato possibile inviare la richiesta al server.\n"); 
        free(actual_request);
        return -1);

    size_t size = 0;
    errno = 0;
    byte_scritti += write_size(fd_skt, &size); 
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile inviare la richiesta al server.\n"); 
        free(actual_request);
        return -1);

    free(actual_request);

    /* Legge la risposta e in base al suo valore stampa una stringa se printer e' uguale ad 1 */
    size_t codice;
    errno = 0;
    int byte_letti = read_size(fd_skt, &codice); 
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
        return -1);

    
    CHECK_CODICE(printer, codice, pathname, "unlockFile", byte_letti, byte_scritti);
    
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

    size_t size = 0;
    errno = 0;
    byte_scritti += write_size(fd_skt, &size); 
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile inviare la richiesta al server.\n"); 
        free(actual_request);
        return -1);
    free(actual_request);

    /* Legge la risposta e in base al suo valore stampa una stringa se printer e' uguale ad 1 */
    size_t codice;
    int byte_letti = read_size(fd_skt, &codice); 
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
        return -1);

    
    CHECK_CODICE(printer, codice, pathname, "removeFile", byte_letti, byte_scritti);

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
    size_t size = 0;
    errno = 0;
    byte_scritti += write_size(fd_skt, &size); 
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile inviare la richiesta al server.\n"); 
        free(actual_request);
        return -1);
    free(actual_request);
    
    /* Legge la risposta e in base al suo valore stampa una stringa se printer e' uguale ad 1 */
    size_t codice;
    int byte_letti = read_size(fd_skt, &codice); 
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
        return -1);

    
    CHECK_CODICE(printer, codice, pathname, "closeFile", byte_letti, byte_scritti);

    return 0;
}

int readFile(const char* pathname, void** buf, size_t *size){ 
    CHECK_OPERATION(pathname == NULL, fprintf(stderr, "Parametro non valido.\n"); return -1); 

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

    *size = 0;
    errno = 0;
    byte_scritti += write_size(fd_skt, size); 
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile inviare la richiesta al server.\n"); 
        free(actual_request);
        return -1);
    free(actual_request);

    /* Legge la risposta dal server */
    size_t codice;
    int byte_letti = read_size(fd_skt, &codice); 
    CHECK_OPERATION(byte_letti==-1, fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); return -1);
   
    if(codice == 0){
        errno = 0;
        byte_letti += read_size(fd_skt, size); 
        CHECK_OPERATION(errno == EFAULT, fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); return -1);
        
        if(*size > 0){
            *buf = malloc(*size);
            CHECK_OPERATION(*buf == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return -1);
            
            byte_letti += read_msg(fd_skt, *buf, (*size)); 
            CHECK_OPERATION(errno == EFAULT, fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); free(*buf); return -1);
        }
    } else {
        *buf = NULL;
    }

    
    CHECK_CODICE(printer, codice, pathname, "readFile", byte_letti, byte_scritti);
    
    return 0;
}

int writeFile(const char* pathname, const char* dirname){
    CHECK_OPERATION(pathname == NULL,  fprintf(stderr, "Parametro non valido:"); return -1); 
    
    /* Legge dal file e inserisce i dati in buf */
    void *buf;
    size_t size = 0;
    int err_rbuf = read_from_file((char*)pathname, &buf, &size);
    CHECK_OPERATION(err_rbuf == -1, fprintf(stderr, "Errore nella lettura dal file.\n"); return -1);

    /* Se la directory in cui memorizzare eventuali file eliminati dal server non e' NULL, viene inviata insieme alla richiesta di write */
    size_t len = (strlen(pathname)+strlen("write;")+1)*sizeof(char);

    char* actual_request = malloc(len);
    CHECK_OPERATION(actual_request == NULL, fprintf(stderr, "Allocazione non andata a buon fine:"); return -1);
    actual_request = strcpy(actual_request, "write;");
    actual_request = strcat(actual_request, pathname);

    /* Invia la richiesta */
    int byte_scritti = write_msg(fd_skt, actual_request, len); 
    CHECK_OPERATION(byte_scritti == -1, free(actual_request); free(buf); return -1);
    
    /* Invia il buffer */
    byte_scritti += write_msg(fd_skt, buf, size); 
    CHECK_OPERATION(byte_scritti == -1, free(actual_request); free(buf); return -1);

    /* Legge la risposta e in base al suo valore stampa una stringa se printer e' uguale ad 1 */
    size_t codice;
    int byte_letti = read_size(fd_skt, &codice); 
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
        free(actual_request);
        free(buf);     
        return -1);
        
    CHECK_OPERATION(codice == -1, fprintf(stderr, "Errore nella write. Il file non poteva essere scritto.\n");
        free(actual_request);
        free(buf);     
        return -1);

    if(dirname != NULL){
        while(codice == 909){
            size_t size_old = 0, size_path = 0;
            void *old_file;
            char *path;
            int err_receiver = receiver(&byte_letti, &byte_scritti, size_path, &path, &old_file, &size_old);
            CHECK_OPERATION(err_receiver == -1, 
                fprintf(stderr, "Errore nella ricezione degli elementi inviati dal server.\n");
                free(path);
                free(old_file);
                free(actual_request);
                free(buf);     
                return -1);

            int err_save = save_on_disk((char*)dirname, path, old_file, size_old);
            CHECK_OPERATION(err_save == -1, 
                fprintf(stderr, "Errore nel salvataggio su disco\n");
                    free(path);
                    free(old_file);
                    free(actual_request);
                    free(buf);     
                    return -1);

            free(path);
            free(old_file);
            
            /* Invia la richiesta */
            byte_scritti += write_msg(fd_skt, actual_request, len); 
            CHECK_OPERATION(byte_scritti == -1, free(actual_request); free(buf); return -1);
            
            /* Invia il buffer */
            byte_scritti += write_msg(fd_skt, buf, size); 
            CHECK_OPERATION(byte_scritti == -1, free(actual_request); free(buf); return -1);
        
            /* Legge la risposta e in base al suo valore stampa una stringa se printer e' uguale ad 1 */
            byte_letti += read_size(fd_skt, &codice); 
            CHECK_OPERATION(errno == EFAULT,
                fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
                free(actual_request);
                free(buf);     
                return -1);
                
            CHECK_OPERATION(codice == -1, fprintf(stderr, "Errore nella write. Il file non poteva essere scritto.\n");
                free(actual_request);
                free(buf);     
                return -1);
        }
    }
    free(actual_request);
    free(buf);

    
    CHECK_CODICE(printer, codice, pathname, "writeFile", byte_letti, byte_scritti);

    return codice;
}

int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname){
    CHECK_OPERATION(pathname == NULL, 
        fprintf(stderr, "Parametro non valido:");
            return -1);
            
    int len = strlen(pathname)+strlen("append;")+1;
    char* actual_request = malloc(sizeof(char)*len);
    CHECK_OPERATION(actual_request == NULL, fprintf(stderr, "Allocazione non andata a buon fine."); return -1);

    actual_request = strcpy(actual_request, "append;");
    actual_request = strcat(actual_request, pathname);
    
    /* Invia la richiesta */
    int byte_scritti = write_msg(fd_skt, actual_request, len); 
    CHECK_OPERATION(byte_scritti == -1,
        free(actual_request);
        return -1);
    
    byte_scritti += write_msg(fd_skt, buf, size); 
    CHECK_OPERATION(byte_scritti == -1,
        free(actual_request);
        return -1);

    /* Legge la risposta e in base al suo valore stampa una stringa se printer e' uguale ad 1 */
    size_t codice;
    int byte_letti = read_size(fd_skt, &codice); 
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
        free(actual_request);
        return -1);

    if(dirname != NULL){
        while(codice == 909){
            size_t size_old, size_path = 0;
            void *old_file;
            char *path;

            int err_receiver = receiver(&byte_letti, &byte_scritti, size_path, &path, &old_file, &size_old);
            CHECK_OPERATION(err_receiver == -1, 
                fprintf(stderr, "Errore nella ricezione degli elementi inviati dal server.\n");
                free(path);
                free(old_file);
                free(actual_request);
                return -1;);

            /* Salva il file su disco */
            int check_save = save_on_disk((char*)dirname, path, old_file, size_old);
            CHECK_OPERATION(check_save == -1,
                fprintf(stderr, "Non e' stato possibile salvare il file su disco.\n");
                free(path);
                free(old_file);
                free(actual_request);
                return -1);

            free(path);
            free(old_file);

            /* Invia i dati del file */
            errno = 0;
            byte_scritti += write_msg(fd_skt, actual_request, len); 
            CHECK_OPERATION(errno == EFAULT,
                free(buf);
                free(actual_request);
                return -1);

            byte_scritti += write_msg(fd_skt, buf, size); 
            CHECK_OPERATION(byte_scritti == -1,
                free(actual_request);
                return -1);

            /* Legge la risposta  */
            int byte_letti = read_size(fd_skt, &codice); 
            CHECK_OPERATION(byte_letti == -1,
                fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
                free(actual_request);
                free(buf);
                return -1);
        }
    }
    free(actual_request);
    free(buf);

    
    CHECK_CODICE(printer, codice, pathname, "appendToFile", byte_letti, byte_scritti);
    
    return 0;
}

int readNFiles(int N, const char* dirname){ 
    void* file;
    size_t codice = 0, size_path = 0, size_file = 0;
    int byte_scritti = -1, byte_letti = -1, count = 0;
    char* actual_request, *path;
    unsigned short i=0;
    
    if(dirname && N<=0){
        while(codice == 0){  
            actual_request = malloc((sizeof(char)*(strlen("readN;\n")+1)) + sizeof(long));
            sprintf(actual_request, "readN;%d\n", i);
          
            /* Invia la richiesta */
            byte_scritti = write_msg(fd_skt, actual_request, (strlen(actual_request)+1)); 
            CHECK_OPERATION(errno == EFAULT,
                fprintf(stderr, "Non e' stato possibile inviare la richiesta al server.\n"); 
                free(actual_request);
                return -1);
                    
            size_t size = 0;
            errno = 0;
            byte_scritti += write_size(fd_skt, &size); 
            CHECK_OPERATION(errno == EFAULT,
                fprintf(stderr, "Non e' stato possibile inviare la richiesta al server.\n"); 
                    free(actual_request);
                    return -1);
            /* Legge la risposta dal server */
            errno = 0;
            byte_letti = read_size(fd_skt, &codice); 
            CHECK_OPERATION(errno == EFAULT,
                fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
                    free(actual_request);
                    return -1);

            if(codice == 0) {            
                int err_receiver = receiver(&byte_letti, &byte_scritti, size_path, &path, &file, &size_file);
                CHECK_OPERATION(err_receiver == -1, 
                    fprintf(stderr, "Errore nella ricezione degli elementi inviati dal server.\n");
                    free(path);
                    free(file);
                    free(actual_request);
                    return -1);

                /* Salva il file su disco */
                int check_save = save_on_disk((char*)dirname, path, file, size_file);
                CHECK_OPERATION(check_save == -1,
                    fprintf(stderr, "Non e' stato possibile salvare il file su disco.\n");
                    free(path);
                    free(file);
                    free(actual_request);
                    return -1);
                    

                free(file);
                free(path);
                i++; 
            }
            free(actual_request);
        }
    } else if(dirname && N>0){
        
        for(i=0; i<N; i++){
            actual_request = malloc((sizeof(char)*(strlen("readN;\n")+1)) + sizeof(long)); 
            sprintf(actual_request, "readN;%d\n", i);

            /* Invia la richiesta */
            byte_scritti = write_msg(fd_skt, actual_request, (strlen(actual_request)+1)*sizeof(char)); 
            CHECK_OPERATION(errno == EFAULT,
                fprintf(stderr, "Non e' stato possibile inviare la richiesta al server.\n"); 
                free(actual_request);
                return -1);
           
            size_t size = 0;
            errno = 0;
            byte_scritti += write_size(fd_skt, &size); 
            CHECK_OPERATION(errno == EFAULT,
                fprintf(stderr, "Non e' stato possibile inviare la richiesta al server.\n"); 
                free(actual_request);
                return -1);
                
            /* Legge la risposta dal server */
            errno = 0;
            byte_letti = read_size(fd_skt, &codice); 
            CHECK_OPERATION(errno == EFAULT,
                fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
                free(actual_request);
                return -1;);

            if(codice == 0){
                int err_receiver = receiver(&byte_letti, &byte_scritti, size_path, &path, &file, &size_file);
                CHECK_OPERATION(err_receiver == -1, 
                    fprintf(stderr, "Errore nella ricezione degli elementi inviati dal server.\n");
                    free(path);
                    free(file);
                    free(actual_request);
                    return -1);
                /* Salva il file su disco */
                int check_save = save_on_disk((char*)dirname, path, file, size_file);
                CHECK_OPERATION(check_save == -1,
                    fprintf(stderr, "Non e' stato possibile salvare il file su disco.\n");
                    free(path);
                    free(file);
                    free(actual_request);
                    return -1);
                free(file);
                free(path);
            } else {
                free(actual_request);
                break;
            }
        }
    }
    
    CHECK_CODICE(printer, codice, "ND", "readNFiles", byte_letti, byte_scritti);  

    return count;
}