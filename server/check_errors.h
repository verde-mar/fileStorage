/**
 * @file check_errors.h
 * @author Sara Grecu (s.grecu1@studenti.unipi.it)
 * @brief Contiene le macro necessarie alla verifica dell'esito delle operazioni
 * @version 0.1
 * 
 */
#ifndef _CHECK_ERRORS_
#define _CHECK_ERRORS_
#define CHECK_OPERATION(condition, operation) \
    if(condition) { \
        operation; \
    }
#define CHECK_CODICE(printer, codice, operazione, byte_letti, byte_scritti) \
    if(printer == 1){\
        fprintf(stderr, "Byte scritti: %d e byte letti:%d\n", byte_scritti, byte_letti); \
        if(codice == 101) {\
            fprintf(stderr, "Non e' stato possibile eseguire la %s perche' il file esiste gia'.\n", operazione); \
        } else if(codice == 202){ \
            fprintf(stderr, "Non e' stato possibile eseguire la %s perche' la lock del file e' stata acquisita da un altro client.\n", operazione); \
        } else if(codice == 303){ \
            fprintf(stderr, "Non e' stato possibile eseguire la %s perche' non e' possibile effettuare una operazione diversa dalla openFile o dalla lockFile dopo la closeFile.\n", operazione); \
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
        } else if(codice == 0){\
            fprintf(stdout, "La %s e' terminata con successo.\n", operazione); \
        } else if(codice == 909){\
            fprintf(stdout, "La %s e' terminata con successo, ma e' stato liberato dello spazio.\n", operazione); \
        }\
    }\
    if(codice == EINVAL || codice == ENOMEM || codice == EFAULT){\
        fprintf(stderr, "Qualcosa e' andato storto, riprova al prossimo avvio.\n"); \
    }
#define CHECK_PTHREAD(condizione) \
    if(condizione){\
        \
    }

#endif