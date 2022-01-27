#ifndef SOCKETIO_H_
#define SOCKETIO_H_

/**
 * @brief Legge la size del messaggio che sta per arrivare
 * 
 * @param size Parametro in cui memorizzare la size inviata dal destinatario
 * @return int Numero di byte letti (>=0 in caso di successo, -1 in caso di errore)
 */
int read_size(int fd_skt, size_t* size);

/**
 * @brief Legge un messaggio dal server
 * 
 * @param msg Parametro in cui memorizzare il messaggio
 * @param size Size del messaggio da leggere
 * @return int Numero di byte letti (>=0 in caso di successo, -1 in caso di errore)
 */
int read_msg(int fd_skt, void *msg, size_t size);

/**
 * @brief Scrive un messaggio al server
 * 
 * @param msg Messaggio da inviare al server
 * @param size Size del messaggio da inviare
 * @return int Numero di byte scritti (>=0 in caso di successo, -1 in caso di errore)
 */
int write_msg(int fd_skt, void *msg, size_t size);

/**
 * @brief Legge n byte dal file descriptor associato
 * 
 * @param fd File descriptor su cui effettuare la lettura
 * @param ptr Indica dove salvare i byte letti
 * @param n Numero di byte da leggere
 * @return ssize_t Byte letti
 */
ssize_t readn(int fd, void *ptr, size_t n);

/**
 * @brief Scrive n byte sul file descriptor associato
 * 
 * @param fd File descriptor su cui effettuare la scrttura
 * @param ptr Indica dove salvare i byte letti
 * @param n Numero di byte da scrivere
 * @return ssize_t Il numero di byte scritti
 */
ssize_t  writen(int fd, void *ptr, size_t n);

#endif