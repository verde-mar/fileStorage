#ifndef SOCKETIO_H_
#define SOCKETIO_H_



/**
 * @brief Legge n byte dal file descriptor associato
 * 
 * @param fd, file descriptor su cui effettuare la lettura
 * @param ptr, indica dove salvare i byte letti
 * @param n, numero di byte da leggere
 * @return ssize_t, byte letti
 */
ssize_t readn(int fd, void *ptr, size_t n);

/**
 * @brief Scrive n byte sul file descriptor associato
 * 
 * @param fd, file descriptor su cui effettuare la scrttura
 * @param ptr, indica dove salvare i byte letti
 * @param n, numero di byte da scrivere
 * @return ssize_t, il numero di byte scritti
 */
ssize_t  writen(int fd, void *ptr, size_t n);

#endif