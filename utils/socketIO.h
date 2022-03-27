#ifndef SOCKETIO_H_
#define SOCKETIO_H_

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>

/**
 * @brief Legge la size del messaggio che sta per arrivare
 * 
 * @param fd_skt File descriptor della socket da cui ricevere il messaggio
 * @param size Parametro in cui memorizzare la size inviata dal destinatario
 * @return int Numero di byte letti (>=0 in caso di successo, -1 in caso di errore)
 */
int read_size(int fd_skt, size_t* size);

/**
 * @brief Legge un messaggio dal server
 * 
 * @param fd_skt File descriptor della socket da cui ricevere il messaggio
 * @param msg Parametro in cui memorizzare il messaggio
 * @param size Size del messaggio da leggere
 * @return int Numero di byte letti (>=0 in caso di successo, -1 in caso di errore)
 */
int read_msg(int fd_skt, void *msg, size_t size);

/**
 * @brief Scrive un messaggio al server
 * 
 * @param fd_skt File descriptor della socket a cui inviare il messaggio
 * @param msg Messaggio da inviare al server
 * @param size Size del messaggio da inviare
 * @return int Numero di byte scritti (>=0 in caso di successo, -1 in caso di errore)
 */
int write_msg(int fd_skt, void *msg, size_t size);

/**
 * @brief Scrive la size del messaggio da inviare
 * 
 * @param fd_skt File descriptor della socket a cui inviare la size del messaggio
 * @param size Size del messaggio
 * @return int Numero di byte scritti (>=0 in caso di successo, -1 in caso di errore)
 */
int write_size(int fd_skt, size_t* size);

/**
 * @brief Legge n byte dal file descriptor associato
 * 
 * @param fd File descriptor su cui effettuare la lettura
 * @param ptr Indica dove salvare i byte letti
 * @param n Numero di byte da leggere
 * @return ssize_t Numero di byte letti (>=0 in caso di successo, -1 in caso di errore)
 */
ssize_t readn(int fd, void *ptr, size_t n);

/**
 * @brief Scrive n byte sul file descriptor associato
 * 
 * @param fd File descriptor su cui effettuare la scrttura
 * @param ptr Indica dove salvare i byte letti
 * @param n Numero di byte da scrivere
 * @return ssize_t Numero di byte scritti (>=0 in caso di successo, -1 in caso di errore)
 */
ssize_t  writen(int fd, void *ptr, size_t n);

/**
 * @brief Setta la maschera dei segnali
 * 
 * @param mask Maschera
 * @return int 0 in caso di successo, -1 altrimenti
 */
int set_mask(sigset_t *mask);

/**
 * @brief Restituisce l'indice massimo tra i descrittori attivi
 * 
 * @param set Set dei descrittori di file
 * @param fdmax Indice massimo corrente
 * @return int i in caso di successo, -1 in caso di fallimento
 */
int aggiorna(fd_set set, int fdmax);

/**
 * @brief Effettua la bind e la listen: crea la socket, e ci si mette in ascolto
 * 
 * @param fd_skt fd della socket
 * @param set Set dei fd da ascoltare 
 * @param socket_name Nome della socket
 * @return int 0 se ha successo, -1 altrimenti
 */
int bind_listen(int *fd_skt, fd_set *set, char* socket_name);

#endif