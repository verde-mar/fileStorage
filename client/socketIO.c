#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>

#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>

#include <check_errors.h>
#include <unistd.h>
#include <signal.h>

#include <sys/wait.h>
#include <errno.h>
#include <socketIO.h>

int read_size(int fd_skt, size_t* size){
    int byte_letti = readn(fd_skt, size, sizeof(size_t));
    CHECK_OPERATIONS(byte_letti==-1, 
        fprintf(stderr, " errore nella lettura della size del messaggio.\n"), 
            return -1); 

    return byte_letti;
}

int read_msg(int fd_skt, void *msg, size_t size){
    CHECK_OPERATIONS(size<0, 
        fprintf(stderr, " parametri non validi.\n"),
            return -1); 
    int byte_letti = readn(fd_skt, msg, size);
    CHECK_OPERATIONS(byte_letti==-1, 
        fprintf(stderr, " errore nella lettura del messaggio.\n"), 
            return -1); 

    return byte_letti;
}

int write_msg(int fd_skt, void *msg, size_t size){
    int byte_scritti = write_size(fd_skt, &size);
    CHECK_OPERATIONS(byte_scritti == -1,
        fprintf(stderr, " errore nell'invio della size del messaggio.\n"),
            return -1);
    byte_scritti = writen(fd_skt, msg, size);
    CHECK_OPERATIONS(byte_scritti == -1,
        fprintf(stderr, " errore nell'invio del messaggio.\n"),
            return -1);
    
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