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

//TODO: come strutturare le stampe per -p abilitato

int openConnection(const char* sockname, int msec, const struct timespec abstime){
    /* Crea il socket */
    fd_skt = socket(AF_UNIX, SOCK_STREAM, 0);
    CHECK_OPERATIONS(fd_skt == -1, errno = EINVAL, return -1);

    /* Salva sockname nella variabile globale socketname */
    socketname = malloc(sizeof(char) * strlen(sockname)+1);
    CHECK_OPERATIONS(socketname == NULL, fprintf(stderr, "La malloc non e' andata a buon fine:"); errno = EINVAL, return -1);
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
        CHECK_OPERATION((success_nanosleep==-1), return -1);
        abstime_temp.tv_nsec -= t.tv_nsec;
        success_connection = connect(fd_skt, (struct sockaddr *)&sa, sizeof(sa));
    }
    CHECK_OPERATIONS(success_connection==-1, fprintf(stderr, "La connect non e' andata a buon fine:"), return -1); 

    return 0;
}

int closeConnection(const char* sockname){
    CHECK_OPERATIONS((strcmp(sockname, socketname)!=0), 
        fprintf(stderr, "Il nome assegnato al socket e' scorretto: "); 
            errno = EINVAL,     
                return -1);

    close(fd_skt);
    if(socketname!=NULL) free((char*)socketname);

    return 0;
}