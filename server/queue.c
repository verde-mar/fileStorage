#include <queue.h>
#include <stdlib.h>
#include <stdio.h>

#include <check_errors.h>
#include <string.h>
#include <errno.h>

#include <fifo.h>

int create_list(list_t **lista_trabocco){
    /* Inizializza la lista */
    *lista_trabocco = malloc(sizeof(list_t));
    CHECK_OPERATION(*lista_trabocco == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
            return -1);

    /* Inizializza il numero di elementi */
    (*lista_trabocco)->elements = 0;
    /* Inizializza la testa */
    (*lista_trabocco)->head = NULL;
    /* Inizializza la mutex */
    (*lista_trabocco)->mutex = malloc(sizeof(pthread_mutex_t));
    CHECK_OPERATION((*lista_trabocco)->mutex == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return -1);
    PTHREAD_INIT_LOCK((*lista_trabocco)->mutex);

    return 0;
}

int destroy_list(list_t **lista_trabocco){
    /* Se la lista di trabocco e' uguale a NULL non e' mai stata allocata */
    CHECK_OPERATION(*lista_trabocco == NULL,
        fprintf(stderr, "Parametro non valido.\n");
            return -1);

    node *tmp = NULL;
    /* Libera la memoria occupata da ciascun nodo */
    while ((*lista_trabocco)->head) {
        tmp = (*lista_trabocco)->head;
        (*lista_trabocco)->head = ((*lista_trabocco)->head)->next;

        /* Libera la memoria del nodo corrente */
        PTHREAD_DESTROY_LOCK(tmp->mutex, "destroy_list: tmp->mutex");
        PTHREAD_DESTROY_COND(tmp->locked); 
        free(tmp->mutex);
        free(tmp->locked);
        if(tmp->buffer)
            free(tmp->buffer);
        free((char*)tmp->path);

        free(tmp);
    }
    /* Distrugge la lock della lista di trabocco */
    PTHREAD_DESTROY_LOCK((*lista_trabocco)->mutex, "destroy_list: lista_trabocco->mutex");
    free((*lista_trabocco)->mutex);

    /* Libera la memoria occupata dalla lista di trabocco */
    free(*lista_trabocco);

    return 0;
}

/**
 * @brief Effettua la creazione effettiva di un nodo
 * 
 * @param queue Lista di trabocco in cui aggiungere il nodo
 * @param file_path Path del file da associare al nodo da creare
 * @param fd file descriptor di chi ha creato e acquisito la lock sul nodo
 * @return node* Nodo creato
 */
static node* node_create(list_t **queue, char* file_path, int fd, int flags){
    CHECK_OPERATION(!(*queue),
        fprintf(stderr, "Parametri non validi.\n");
        return NULL);

    /* Crea il nodo */
    node *curr = (node*)malloc(sizeof(node));
    CHECK_OPERATION(curr == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return NULL);

    curr->path = malloc(sizeof(char)*(strlen(file_path)+1));
    CHECK_OPERATION(curr->path == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return NULL);
    
    strcpy((char*)curr->path, file_path);
    curr->open = 1;
    curr->size_buffer = 0;
    curr->buffer = NULL;
    if(flags!=2)
        curr->fd_c = fd;
    else
        curr->fd_c = -1;
    curr->locked = malloc(sizeof(pthread_cond_t));
    CHECK_OPERATION(curr->locked == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return NULL);
    PTHREAD_INIT_COND(curr->locked);
    curr->mutex = malloc(sizeof(pthread_mutex_t));
    CHECK_OPERATION(curr->mutex == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return NULL);
    PTHREAD_INIT_LOCK(curr->mutex);

    return curr;
}

int add(list_t **lista_trabocco, char* file_path, int fd, int flags){
    CHECK_OPERATION(!(*lista_trabocco),
        fprintf(stderr, "Parametri non validi.\n");
            return -1);
    
    /* Aggiunge il nodo in testa alla lista di trabocco, se non esiste gia' */
    node* check_exists = look_for_node(lista_trabocco, file_path);
    
    /* Se e' stata specificata l'operazione di creazione */
    if((flags == 2 || flags == 6) && check_exists == NULL){
        node* curr = node_create(lista_trabocco, file_path, fd, flags);
        CHECK_OPERATION(curr == NULL, return -1);

        PTHREAD_LOCK(fifo_queue->mutex);
        PTHREAD_LOCK((*lista_trabocco)->mutex);
       
        int adder = add_fifo((char*)curr->path);
        CHECK_OPERATION(adder == -1, 
            PTHREAD_UNLOCK(fifo_queue->mutex);
            PTHREAD_UNLOCK((*lista_trabocco)->mutex);
            return -1);

        curr->next = (*lista_trabocco)->head; 
        (*lista_trabocco)->head = curr;
        
        PTHREAD_UNLOCK(fifo_queue->mutex);
        PTHREAD_UNLOCK((*lista_trabocco)->mutex);
        if(flags==2) return 0;

    } else if((flags == 5 || flags == 0 || flags == 4) && check_exists != NULL){
        PTHREAD_LOCK(check_exists->mutex);
            check_exists->open = 1;
        PTHREAD_UNLOCK(check_exists->mutex);
        if(!flags) return 0;
    }

    if(flags == 6 || flags == 4 || flags == 5){
        int check_lock = lock(lista_trabocco, file_path, fd);
        return check_lock;
    } 

    /* Se non e' stata specificata nessuna operazione valida, allora restituisce un errore */
    CHECK_OPERATION((flags!=2 && flags !=6) && (flags!=4 && flags != 0) && flags != 5, 
        fprintf(stderr, "Flag non validi.\n");
        return 404;);

    CHECK_OPERATION(check_exists!=NULL, fprintf(stderr, "Il nodo esiste gia'.\n"); return 101);
    
    return 505;
}

int deletes(list_t **lista_trabocco, char* file_path, node** just_deleted, int fd, int *curr_size){
    CHECK_OPERATION(*lista_trabocco==NULL,
        fprintf(stderr, "Parametri non validi.\n");
        return -1);
    
    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL,
        fprintf(stderr, "Il nodo non e' stato trovato.\n");
        return 505);
            
    PTHREAD_LOCK(fifo_queue->mutex);
    PTHREAD_LOCK((*lista_trabocco)->mutex);
    int remover = del(file_path);
    CHECK_OPERATION(remover == -1, 
        PTHREAD_UNLOCK(fifo_queue->mutex);
        PTHREAD_UNLOCK((*lista_trabocco)->mutex);
        return -1);
    
    node* curr; /* Puntatore al nodo corrente */
    node *prev; /* Puntatore al nodo precedente */
    if ((*lista_trabocco)->head == NULL){ /* Lista vuota */
        PTHREAD_UNLOCK(fifo_queue->mutex);
        PTHREAD_UNLOCK((*lista_trabocco)->mutex);

        return 0;
    }
    curr = (*lista_trabocco)->head;
    if (strcmp(curr->path, file_path) == 0) { /* Cancellazione del primo nodo */
        if(curr->fd_c == fd){
            *just_deleted = curr;
            (*lista_trabocco)->head = curr->next; /* Aggiorna il puntatore alla testa */
            *curr_size = *curr_size - curr->size_buffer;
            PTHREAD_UNLOCK(fifo_queue->mutex);
            PTHREAD_UNLOCK((*lista_trabocco)->mutex);
            return 0;
        } else {
            PTHREAD_UNLOCK(fifo_queue->mutex);
            PTHREAD_UNLOCK((*lista_trabocco)->mutex);

            return 202;
        }
        
    }

    /* Scansione della lista a partire dal secondo nodo */
    prev = curr;
    curr = curr->next;
    while (curr != NULL) {
        if (strcmp(curr->path, file_path) == 0) {
            if(curr->fd_c == fd){
                *just_deleted = curr;
                prev->next = curr->next; 
                *curr_size = *curr_size - curr->size_buffer;
                PTHREAD_UNLOCK(fifo_queue->mutex);
                PTHREAD_UNLOCK((*lista_trabocco)->mutex);
                return 0;
            } 
            /* Non e' stata acquisita la lock */
            else if(curr->fd_c != fd){
                PTHREAD_UNLOCK(fifo_queue->mutex);
                PTHREAD_UNLOCK((*lista_trabocco)->mutex);

                return 202;
            }
        }
        prev = curr;
        curr = curr->next;
    }

    PTHREAD_UNLOCK(fifo_queue->mutex);
    PTHREAD_UNLOCK((*lista_trabocco)->mutex);

    return -1;
}

node* look_for_node(list_t **lista_trabocco, char* file_path){
    node* curr;
    PTHREAD_LOCK((*lista_trabocco)->mutex);
    for (curr=(*lista_trabocco)->head; curr != NULL; curr=curr->next)
        if (strcmp(curr->path, file_path) == 0){
            PTHREAD_UNLOCK((*lista_trabocco)->mutex);
            return(curr);
        }

    PTHREAD_UNLOCK((*lista_trabocco)->mutex);
    return NULL;
}

int closes(list_t **lista_trabocco, char* file_path, int fd){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);
    
    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL,
        fprintf(stderr, "Il nodo non e' stato trovato.\n");
                    return 505);

    PTHREAD_LOCK(nodo->mutex);
    
    /* Se il nodo e' aperto */
    if(nodo->open == 1 && nodo->fd_c == fd){
        nodo->open = 0;
        PTHREAD_UNLOCK(nodo->mutex);
        
        return 0;
    } 
    /* Se non ha acquisito la lock */
    else if(nodo->fd_c != fd){
        PTHREAD_UNLOCK(nodo->mutex);
        
        return 202;
    }
    /* Il nodo non e' stato aperto */
    else{
        PTHREAD_UNLOCK(nodo->mutex);

        return 303;
    }
    
    PTHREAD_UNLOCK(nodo->mutex);

    return -1;
}

int unlock(list_t **lista_trabocco, char* file_path, int fd){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
        return -1);

    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL,
        fprintf(stderr, "Il nodo non e' stato trovato.\n");
        return 505);

    PTHREAD_LOCK(nodo->mutex);
    /* Se ha acquisito la lock */ //TODO: specifica nella relazione che hai tolto il controllo per capire se il file e' open, ma sostituito dalla lock
    if(nodo->fd_c == fd){
        nodo->fd_c = -1;
        PTHREAD_COND_SIGNAL(nodo->locked);
    } 
    /* Se la lock non e' stata acquisita da nessuno */
    else if(nodo->fd_c == -1){
        PTHREAD_UNLOCK(nodo->mutex);
        
        return 555;
    }
    /* Se non ha acquisito la lock */
    else if(nodo->fd_c != fd){
        PTHREAD_UNLOCK(nodo->mutex);
        
        return 202;
    } 
    
    PTHREAD_UNLOCK(nodo->mutex);

    return 0;
}

int lock(list_t **lista_trabocco, char* file_path, int fd){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);
    
    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL,
        fprintf(stderr, "Il nodo non e' stato trovato.\n");
                    return 505);
                    
    PTHREAD_LOCK(nodo->mutex);

    /* Se la lock era gia' stata acquisita dallo stesso processo o era libera e il nodo e' aperto */
    if(nodo->open && (nodo->fd_c == fd || nodo->fd_c == -1)){     
        nodo->fd_c = fd;
    } 
    /* Se il nodo non e' aperto */
    else if(!nodo->open){
        PTHREAD_UNLOCK(nodo->mutex);

        return 303;
    }
    /* Se la lock e' stata acquisita da un altro thread */
    else {
        PTHREAD_UNLOCK(nodo->mutex);

        return 202;
    }

    PTHREAD_UNLOCK(nodo->mutex);

    return 0;
}

int reads(list_t **lista_trabocco, char* file_path, void** buf, size_t *size_buf, int fd){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);

    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL,
        *buf = NULL;
        *size_buf = 0;
        fprintf(stderr, "Il nodo non e' stato trovato.\n"); return 505);

    PTHREAD_LOCK(nodo->mutex);
    /* Se il nodo e' aperto*/ //TODO: fai notare nella relazione
    if(nodo->open){
        *size_buf = nodo->size_buffer;
        *buf = nodo->buffer;
    } 
    /* Se il nodo e' chiuso */
    else {
        PTHREAD_UNLOCK(nodo->mutex);

        return 303;
    }
    PTHREAD_UNLOCK(nodo->mutex);
    
    return 0;
}

int append_buffer(list_t **lista_trabocco, char* file_path, void* buf, size_t size_buf, int* max_size, int* curr_size, int fd){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);

    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL,
        fprintf(stderr, "Il nodo non e' stato trovato.\n");
        return 505);

    PTHREAD_LOCK(nodo->mutex);

    /* Se ha acquisito la lock e il nodo e' aperto */
    if(nodo->fd_c == fd && nodo->open == 1){
        nodo->buffer = realloc(nodo->buffer, nodo->size_buffer + size_buf); 
        CHECK_OPERATION(nodo->buffer == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); 
            PTHREAD_UNLOCK(nodo->mutex);
            return -1);
            
        memcpy(nodo->buffer + nodo->size_buffer, buf, size_buf);  
        nodo->size_buffer += (size_buf); 
        free(buf);
        PTHREAD_UNLOCK(nodo->mutex);
        
        return 0;
    } 
    /* Se non ha acquisito la lock */
    else if(nodo->fd_c != fd){
        if(buf) free(buf);
        PTHREAD_UNLOCK(nodo->mutex);

        return 202;
    } 
    /* Il nodo non e' stato aperto */
    else if(nodo->open == 0){
        if(buf) free(buf);
        PTHREAD_UNLOCK(nodo->mutex);

        return 303;
    }
    if(buf) free(buf);
    PTHREAD_UNLOCK(nodo->mutex);

    return -1;
}

int writes(list_t **lista_trabocco, char* file_path, void* buf, size_t size_buf, int *max_size, int* curr_size, node** deleted, int fd){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
        return -1);
            
    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL,
        fprintf(stderr, "Il nodo non e' stato trovato.\n");
        size_buf = 0;
        free(buf);
        return 505);
    
    PTHREAD_LOCK(nodo->mutex);
    if(!nodo->buffer){ //TODO: da gestire la restituzione di write

        /* Se ha acquisito la lock e il flag open e' a 1 */
        if(nodo->fd_c == fd && nodo->open == 1 && buf){
            nodo->buffer = malloc(size_buf);
            CHECK_OPERATION(nodo->buffer == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return -1);
            memcpy(nodo->buffer, buf, size_buf);  
            nodo->size_buffer = size_buf;
            free(buf);
            PTHREAD_UNLOCK(nodo->mutex);
        
            return 0;
        } 
        else if(nodo->fd_c == fd && nodo->open == 1){
            PTHREAD_UNLOCK(nodo->mutex);
        
            return 0;
        }
        /* Se non ha acquisito la lock */
        else if(nodo->fd_c != fd){
            if(buf) free(buf);
            PTHREAD_UNLOCK(nodo->mutex);
            return 202;
        } 
        /* Se il file non e' stato aperto */
        else if(nodo->open == 0){
            if(buf) free(buf);
            PTHREAD_UNLOCK(nodo->mutex);
            return 303;
        }
        
    } 
    /* Se ci e' gia' stato scritto sul nodo */
    else {
        if(buf) free(buf);
        PTHREAD_UNLOCK(nodo->mutex);

        return 808;
    }
}