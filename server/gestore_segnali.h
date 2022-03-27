#ifndef GESTORE_SEGNALI_H_
#define GESTORE_SEGNALI_H_

#include <signal.h>

/**
 * @brief Oggetto ausiliario usata per la gestione dei segnali
 * 
 */
typedef struct {
    sigset_t     *set;           
    int           signal_pipe;   
} sigHandler_t;

/**
 * @brief La funzione gestore segnali
 * 
 * @param arg, struct contenente maschera dei segnali e descrittore della pipe usata per inviare i segnali da gestore_segnali al main
 * @return void*
 */
void *gestore_segnali(void *arg);

#endif