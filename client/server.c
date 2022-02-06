#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h> /* ind AF_UNIX */
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h> 

#define N 100

int main (void) {
    int fd_skt, fd_c;
    struct sockaddr_un sa;

    strcpy(sa.sun_path, "socketname");
    sa.sun_family=AF_UNIX;

    fd_skt=socket(AF_UNIX,SOCK_STREAM,0);
    bind(fd_skt,(struct sockaddr *)&sa,sizeof(sa));
    listen(fd_skt,SOMAXCONN);
    fd_c=accept(fd_skt,NULL,0);  
    size_t size;
    read(fd_c, &size, sizeof(size_t));
    char *buffer = malloc(sizeof(char)*size);
    read(fd_c, buffer, size);
    printf("STRINGA RICEVUTA: %s\n", buffer);
    int codice_errore = 101;
    write(fd_c, &codice_errore, sizeof(int));
    read(fd_c, &size, sizeof(size_t));
    buffer = malloc(sizeof(char)*size);
    read(fd_c, buffer, size);
    printf("STRINGA RICEVUTA: %s\n", buffer);
    read(fd_c, &size, sizeof(size_t));
    buffer = malloc(sizeof(char)*size);
    read(fd_c, buffer, size);
    printf("STRINGA RICEVUTA: %s\n", buffer);
    read(fd_c, &size, sizeof(size_t));
    buffer = malloc(sizeof(char)*size);
    read(fd_c, buffer, size);
    printf("STRINGA RICEVUTA: %s\n", buffer);
    return 0;

}