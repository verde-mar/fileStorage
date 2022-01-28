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

    strcpy(sa.sun_path, "socketname");
    sa.sun_family=AF_UNIX;

    fd_skt=socket(AF_UNIX,SOCK_STREAM,0);
    bind(fd_skt,(struct sockaddr *)&sa,sizeof(sa));
    listen(fd_skt,SOMAXCONN);
    fd_c=accept(fd_skt,NULL,0);  

    return 0;

}