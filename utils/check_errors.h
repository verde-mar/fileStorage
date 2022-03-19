/**
 * @file check_errors.h
 * @author Sara Grecu (s.grecu1@studenti.unipi.it)
 * @brief Contiene le macro necessarie alla verifica dell'esito delle operazioni
 * @version 0.1
 * 
 */
#ifndef _CHECK_ERRORS_
#define _CHECK_ERRORS_

#include <pthread.h>

#define CHECK_OPERATION(condition, operation) \
    if(condition) { \
        operation; \
    }
#define CHECK_CODICE(printer, codice, operazione, byte_letti, byte_scritti) \
    if(printer == 1){\
        fprintf(stderr, "Byte scritti: %d e byte letti:%d\n", byte_scritti, byte_letti); \
        if(codice == 101) {\
            fprintf(stderr, "Non e' stato possibile eseguire la %s perche' il file esiste gia'. Prova a rieseguirla con il flag 5 per aprire il file e acquisire la lock, con il flag 0 solo per aprirlo o con il flag 2 solo per acquisire la lock.\n", operazione); \
        } else if(codice == 202){ \
            fprintf(stderr, "Non e' stato possibile eseguire la %s perche' la lock del file e' stata acquisita da un altro thread.\n", operazione); \
        } else if(codice == 303){ \
            fprintf(stderr, "Non e' stato possibile eseguire la %s perche' non e' possibile effettuare una operazione diversa dalla openFile dopo la closeFile.\n", operazione); \
        } else if(codice == 404){ \
            fprintf(stderr, "Non e' stato possibile eseguire la %s perche' il file non esiste e non e' stato specificato O_CREATE.\n", operazione); \
        } else if(codice == 505){\
            fprintf(stderr, "Non e' stato possibile eseguire la %s perche' il file non esiste.\n", operazione); \
        } else if(codice == 606){\
            fprintf(stderr, "Non e' stato possibile eseguire la %s perche' prima della writeFile devi fare la openFile.\n", operazione); \
        } else if(codice == 707){\
            fprintf(stderr, "Non e' stato possibile eseguire la %s perche' prima della appendFile devi fare la writeFile.\n", operazione); \
        } else if(codice == 808){\
            fprintf(stderr, "Non e' stato possibile eseguire la %s perche' e' gia' stata fatta. Puoi fare solo la appendToFile.\n", operazione); \
        } else if(codice == 111){\
            fprintf(stderr, "Non ci sono piu' file da leggere.\n");\
        } else if(codice == 333){\
            fprintf(stderr, "Il buffer era vuoto.\n"); \
        } else if(codice == 555){\
            fprintf(stderr, "La lock non e' stata acquisita.\n");\
        } else if(codice == 0){\
            fprintf(stdout, "La %s e' terminata con successo.\n", operazione); \
        } else if(codice == 909){\
            fprintf(stdout, "La %s e' terminata con successo, ma e' stato liberato dello spazio.\n", operazione); \
        } else if(codice == 444){\
            fprintf(stdout, "La %s non e' terminata con successo, perche' il file e' troppo grande e non c'erano altri elementi da eliminare.\n", operazione); \
        } else if(codice == 777){\
            fprintf(stdout, "La %s non e' terminata con successo, perche' non e' possibile aggiungere altri file al momento. Prova ad eliminarne qualcuno.\n", operazione); \
        }\
    }\
    if(codice == EINVAL || codice == ENOMEM || codice == EFAULT){\
        perror("Qualcosa e' andato storto, riprova al prossimo avvio.\n"); \
        exit(-1);\
    }
    
#define PTHREAD_LOCK(mtx) \
    if(pthread_mutex_lock(mtx) != 0){\
        fprintf(stderr, "Qualcosa e' andato storto in fase di gestione della sincronizzazione (pthread_lock).\nRiprova al prossimo avvio.\n");\
        exit(-1);\
    }

#define PTHREAD_UNLOCK(mtx) \
    if(pthread_mutex_unlock(mtx) != 0){\
        fprintf(stderr, "Qualcosa e' andato storto in fase di gestione della sincronizzazione (pthread_unlock).\nRiprova al prossimo avvio.\n");\
        exit(-1);\
    }
#define PTHREAD_INIT_LOCK(mtx) \
    if(pthread_mutex_init(mtx, NULL) != 0){\
        fprintf(stderr, "Qualcosa e' andato storto in fase di gestione della sincronizzazione (pthread_init_lock).\nRiprova al prossimo avvio.\n");\
        exit(-1);\
    }

#define PTHREAD_INIT_COND(cond) \
    if(pthread_cond_init(cond, NULL) != 0){\
        fprintf(stderr, "Qualcosa e' andato storto in fase di gestione della sincronizzazione (pthread_init_cond).\nRiprova al prossimo avvio.\n");\
        exit(-1);\
    }
#define PTHREAD_DESTROY_LOCK(mtx) \
    if(pthread_mutex_destroy(mtx) != 0){\
        fprintf(stderr, "Qualcosa e' andato storto in fase di gestione della sincronizzazione (pthread_destroy_lock).\nRiprova al prossimo avvio.\n");\
        exit(-1);\
    }

#define PTHREAD_DESTROY_COND(cond) \
    if(pthread_cond_destroy(cond) != 0){\
        fprintf(stderr, "Qualcosa e' andato storto in fase di gestione della sincronizzazione (pthread_destroy_cond).\nRiprova al prossimo avvio.\n");\
        exit(-1);\
    }

#define PTHREAD_COND_WAIT(cond, mtx) \
    if(pthread_cond_wait(cond, mtx) != 0){\
        fprintf(stderr, "Qualcosa e' andato storto in fase di gestione della sincronizzazione (pthread_cond_wait).\nRiprova al prossimo avvio.\n");\
        exit(-1);\
    }

#define PTHREAD_COND_SIGNAL(cond) \
    if(pthread_cond_signal(cond) != 0){\
        fprintf(stderr, "Qualcosa e' andato storto in fase di gestione della sincronizzazione (pthread_cond_signal).\nRiprova al prossimo avvio.\n");\
        exit(-1);\
    }

#endif