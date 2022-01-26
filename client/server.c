#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h> /* ind AF_UNIX */
#include <string.h>

#define N 100

int main (void) {
    int fd_skt, fd_c;
    struct sockaddr_un sa;
    printf("prima di strcpy");
    strcpy(sa.sun_path, "socketname");
    sa.sun_family=AF_UNIX;

    printf("prima della creazione della socket\n");
    fd_skt=socket(AF_UNIX,SOCK_STREAM,0);
    bind(fd_skt,(struct sockaddr *)&sa,sizeof(sa));
    printf("Prima della listen e dopo la bind\n");
    listen(fd_skt,SOMAXCONN);
    printf("dopo la listen\n");
    fd_c=accept(fd_skt,NULL,0);
    printf("VALORE DI FD_C: %d\n", fd_c);
    
    return 0;

}