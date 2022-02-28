#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>

#include <string.h>
#include <unistd.h>

#include <check_errors.h>
#include <unistd.h>
#include <signal.h>

#include <sys/wait.h>
#include <errno.h>
#include <socketIO.h>

int read_size(int fd_skt, size_t* size){
    int byte_letti = readn(fd_skt, size, sizeof(size_t));
    CHECK_OPERATION(byte_letti==-1, return -1); 

    return byte_letti;
}

int read_msg(int fd_skt, void *msg, size_t size){
    CHECK_OPERATION(size<0, 
        fprintf(stderr, "Parametri non validi.\n");
            return -1); 

    return readn(fd_skt, msg, size);
}

int write_size(int fd_skt, size_t* size){

    return writen(fd_skt, size, sizeof(size_t));
}

int write_msg(int fd_skt, void *msg, size_t size){
    int byte_scritti = write_size(fd_skt, &size);
    CHECK_OPERATION(byte_scritti == -1,
        fprintf(stderr, "Errore nell'invio della size del messaggio.\n");
            return -1);
    printf("byte_scritti nella write msg: %d\n", byte_scritti);
    byte_scritti = writen(fd_skt, msg, size);
    CHECK_OPERATION(byte_scritti == -1,
        fprintf(stderr, "Errore nell'invio del messaggio.\n");
            return -1);
    printf("byte_scritti nella write msg, per il msg: %d\n", byte_scritti);
    
    
    return byte_scritti;
}

ssize_t readn(int fd, void *ptr, size_t n) {  
   size_t   nleft;
   ssize_t  nread;
 
   nleft = n;
   while (nleft > 0) {
     if((nread = read(fd, ptr, nleft)) < 0) {
        if (nleft == n){ errno = EFAULT; return -1;} /* error, return -1 */
        else break; /* error, return amount read so far */
     } else if (nread == 0) break; /* EOF */
     nleft -= nread;
     ptr   += nread;
   }
   return(n - nleft); /* return >= 0 */
}

ssize_t  writen(int fd, void *ptr, size_t n) {  
   size_t   nleft;
   ssize_t  nwritten;
 
   nleft = n;
   while (nleft > 0) {
     if((nwritten = write(fd, ptr, nleft)) < 0) {
        if (nleft == n) { errno = EFAULT; return -1;} /* error, return -1 */
        else break; /* error, return amount written so far */
     } else if (nwritten == 0) break; 
     nleft -= nwritten;
     ptr   += nwritten;
   }
   return(n - nleft); /* return >= 0 */
}

int aggiorna(fd_set set, int fdmax){
    for(int i=(fdmax-1);i>=0;--i)
	    if (FD_ISSET(i, &set)) return i;
    return -1;
}

int set_mask(sigset_t *mask){
    int err_sigempty = sigemptyset(mask);
    CHECK_OPERATION(err_sigempty == -1, fprintf(stderr, "Errore nella sigemptyset.\n"); return -1);
    int err_sigadd = sigaddset(mask, SIGINT);
    CHECK_OPERATION(err_sigadd == -1, fprintf(stderr, "Errore nella sigaddset.\n"); return -1); 
    err_sigadd = sigaddset(mask, SIGQUIT);
    CHECK_OPERATION(err_sigadd == -1, fprintf(stderr, "Errore nella sigaddset.\n"); return -1);
    err_sigadd = sigaddset(mask, SIGHUP);
    CHECK_OPERATION(err_sigadd == -1, fprintf(stderr, "Errore nella sigaddset.\n"); return -1);    
    int err_sigmask = pthread_sigmask(SIG_SETMASK, mask, NULL);
    CHECK_OPERATION(err_sigmask != 0, fprintf(stderr, "Errore nella pthread_sigmask .\n"); return -1);

    return 0;
}

int bind_listen(int *fd_skt, fd_set *set, char* socket_name){
    struct sockaddr_un sa;
    int fd_num = 0;

    /* Crea la socket su cui collegarsi */
    strcpy(sa.sun_path, socket_name);
    sa.sun_family = AF_UNIX;
    *fd_skt = socket(AF_UNIX, SOCK_STREAM, 0);
    CHECK_OPERATION((*fd_skt==-1), fprintf(stderr, "Errore nella creazione della socket.\n"); return -1;);

    /* Assegna un indirizzo ad un socket */
    int err_bind = bind(*fd_skt, (struct sockaddr*)&sa, sizeof(sa));
    CHECK_OPERATION((err_bind==-1), fprintf(stderr, "Errore nella bind.\n");return -1;);

    /* Si mette in ascolto su quel socket */
    int err_listen = listen(*fd_skt, 10);
    CHECK_OPERATION((err_listen==-1), fprintf(stderr, "Errore nella listen.\n"); return -1;);
    if(*fd_skt > fd_num) fd_num = *fd_skt;
    

    return fd_num;
}