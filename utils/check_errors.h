/**
 * @file check_errors.h
 * @author Sara Grecu (s.grecu1@studenti.unipi.it)
 * @brief Contiene le macro necessarie alla verifica dell'esito delle operazioni
 * @version 0.1
 * 
 */
#ifndef _CHECK_ERRORS_
#define _CHECK_ERRORS_
#define CHECK_OPERATIONS(condition, operation1, operation2) \
    if(condition){ \
        operation1; \
        operation2; \
    }
#define CHECK_OPERATION(condition, operation1) \
    if(condition) \
        operation1; 
    
#endif