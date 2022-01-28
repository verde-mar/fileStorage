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

int openConnection(const char* sockname, int msec, const struct timespec abstime){
    /* Crea il socket */
    fd_skt = socket(AF_UNIX, SOCK_STREAM, 0);
    CHECK_OPERATION(fd_skt == -1, errno = EINVAL; return -1);

    /* Salva sockname nella variabile globale socketname */
    socketname = malloc(sizeof(char) * strlen(sockname)+1);
    CHECK_OPERATION(socketname == NULL, fprintf(stderr, "La malloc non e' andata a buon fine:"); errno = EINVAL; return -1);
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
        CHECK_OPERATION((success_nanosleep==-1)&& printer == 1, fprintf(stderr, "Errore sulla nanosleep:"); return -1);
        abstime_temp.tv_nsec -= t.tv_nsec;
        success_connection = connect(fd_skt, (struct sockaddr *)&sa, sizeof(sa));
    }
    CHECK_OPERATION(success_connection==-1 && printer == 1, fprintf(stderr, "E' stata eseguita l'operazione 'openConnection' e non e' andata a buon fine:"); return -1); 
    CHECK_OPERATION(printer == 1, fprintf(stdout, "E' stata eseguita l'operazione 'openConnection' con successo: "); return 0);
    return 0;
}

int closeConnection(const char* sockname){
    CHECK_OPERATION((strcmp(sockname, socketname)!=0), 
        fprintf(stderr, "Il nome assegnato al socket e' scorretto: "); 
            errno = EINVAL;   
                return -1);

    close(fd_skt);
    if(socketname!=NULL) free((char*)socketname);

    return 0;
}

int openFile(const char *pathname, int flags){
    CHECK_OPERATION(pathname == NULL && (flags != 6 || flags != 2 || flags != 4), 
        fprintf(stderr, "Parametro non valido:");
            return -1); 
    
    char *request = NULL;
    if(flags == 6){
        request = "create;lock;";
    } else if(flags == 2){
        request = "create;";
    } else if(flags == 4){
        request = "lock;";
    }

    int len = strlen(pathname)+strlen(request)+1;
    char* actual_request = malloc(sizeof(char)*(strlen(pathname)+strlen(request)+1));
    CHECK_OPERATION(actual_request == NULL, 
        perror("Allocazione non andata a buon fine.\n");
            return -1);

    actual_request = strcat(actual_request, request);
    actual_request = strcat(actual_request, pathname);
    actual_request[len] = '\0';
    
    int byte_scritti = write_msg(fd_skt, actual_request, strlen(actual_request)); //TODO:testa se funziona
    CHECK_OPERATION(byte_scritti == -1, 
            return -1);

    size_t size;
    int byte_letti = read_size(fd_skt, &size); //TODO:testa se funziona
    CHECK_OPERATION(byte_letti == -1, return -1);

    char* response = malloc(sizeof(char)*size);
    CHECK_OPERATION(response == NULL, 
        perror("Allocazione non andata a buon fine.\n");
            return -1);

    int byte_letti = read_msg(fd_skt, response, size); //TODO:testa se funziona
    CHECK_OPERATION(byte_letti == -1, return -1);

    return 0;
}