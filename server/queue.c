#include <queue.h>
#include <stdlib.h>
#include <stdio.h>

#include <check_errors.h>
#include <string.h>
#include <errno.h>

#include <fifo.h>
#include <utils.h>

int create_list(list_t **lista_trabocco){
    /* Crea la lista */
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
        PTHREAD_DESTROY_LOCK(tmp->mutex);
        PTHREAD_DESTROY_COND(tmp->locked); 
        free(tmp->mutex);
        free(tmp->locked);
        if(tmp->buffer)
            free(tmp->buffer);
        free((char*)tmp->path);

        free(tmp);
    }
    /* Distrugge la lock della lista di trabocco */
    PTHREAD_DESTROY_LOCK((*lista_trabocco)->mutex);
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
static node* node_create(list_t **queue, char* file_path, int fd){
    CHECK_OPERATION(!(*queue),
        fprintf(stderr, "Parametri non validi.\n");
        return NULL);

    /* Crea il nodo */
    node *curr = (node*)malloc(sizeof(node));
    CHECK_OPERATION(curr == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return NULL);

    /* Inizializza la maschera */
    FD_ZERO(&(curr->open));
    /* Apre il file */
    FD_SET(fd, &(curr->open));

    /* Inizializza il path del file */
    curr->path = malloc(sizeof(char)*(strlen(file_path)+1));
    CHECK_OPERATION(curr->path == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return NULL);
    strcpy((char*)curr->path, file_path);

    /* Inizializza la size del buffer e il buffer */
    curr->size_buffer = 0;
    curr->buffer = NULL;
    /* Inizializza la lock logica */
    curr->fd_c = -1;
    /* Inizializza il flag che indica se l'ultima operazione e' stata la create_lock */
    curr->fd_create_open = -1;

    /* Inizializza la mutex e la variabile di condizione */
    curr->locked = malloc(sizeof(pthread_cond_t));
    CHECK_OPERATION(curr->locked == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return NULL);
    PTHREAD_INIT_COND(curr->locked);
    curr->mutex = malloc(sizeof(pthread_mutex_t));
    CHECK_OPERATION(curr->mutex == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return NULL);
    PTHREAD_INIT_LOCK(curr->mutex);

    return curr;
}

int creates(list_t **lista_trabocco, char* file_path, int fd, int *max_file_reached, FILE* file_log){
    CHECK_OPERATION(!(*lista_trabocco),
        fprintf(stderr, "Parametri non validi.\n");
        return -1);

    /* Cerca per verificare se il file esiste gia' nella tabella */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo != NULL, return 101);
    CHECK_OPERATION(nodo == NULL, 
        nodo = node_create(lista_trabocco, file_path, fd);
        if(nodo == NULL) return -1);
    
    PTHREAD_LOCK(fifo_queue->mutex);
    PTHREAD_LOCK((*lista_trabocco)->mutex);
    
    /* Inserisci il file nella coda cache */
    int adder = add_fifo((char*)nodo->path);
    CHECK_OPERATION(adder == -1, 
        PTHREAD_UNLOCK(fifo_queue->mutex);
        PTHREAD_UNLOCK((*lista_trabocco)->mutex);
        return -1);
    
    /* Aggiorna il numero di file presenti nella tabella hash */
    *max_file_reached = max(*max_file_reached, fifo_queue->elements);

    /* Aggiunge effettivamente il nodo alla tabella hash */
    nodo->next = (*lista_trabocco)->head; 
    (*lista_trabocco)->head = nodo;

    /* Aggiorna il file di log */
    fprintf(file_log, "Create\n");

    PTHREAD_UNLOCK(fifo_queue->mutex);
    PTHREAD_UNLOCK((*lista_trabocco)->mutex);

    return 0;
}

int creates_locks(list_t **lista_trabocco, char* file_path, int fd, int *max_file_reached, FILE* file_log){
    CHECK_OPERATION(!(*lista_trabocco),
        fprintf(stderr, "Parametri non validi.\n");
        return -1);

    /* Cerca per verificare se il file esiste gia' nella tabella */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo != NULL, return 101);
    CHECK_OPERATION(nodo == NULL, 
        nodo = node_create(lista_trabocco, file_path, fd);
        if(nodo == NULL) return -1);
    
    PTHREAD_LOCK(fifo_queue->mutex);
    PTHREAD_LOCK((*lista_trabocco)->mutex);
    
    /* Inserisci il file nella coda cache */
    int adder = add_fifo((char*)nodo->path);
    CHECK_OPERATION(adder == -1, 
        PTHREAD_UNLOCK(fifo_queue->mutex);
        PTHREAD_UNLOCK((*lista_trabocco)->mutex);
        return -1);
    
    /* Aggiorna il numero di file presenti nella tabella hash */
    *max_file_reached = max(*max_file_reached, fifo_queue->elements);

    /* Aggiunge effettivamente il nodo alla tabella hash */
    nodo->next = (*lista_trabocco)->head; 
    (*lista_trabocco)->head = nodo;

    /* Setta la lock */
    nodo->fd_c = fd;
    /* Setta il flag che indica se l'operazione immediatamente precedente e' stata la create_lock */
    nodo->fd_create_open = fd;

    /* Aggiorna il file di log */
    fprintf(file_log, "Create_Lock\n");

    PTHREAD_UNLOCK(fifo_queue->mutex);
    PTHREAD_UNLOCK((*lista_trabocco)->mutex);

    return 0;
}

int opens_locks(list_t **lista_trabocco, char* file_path, int fd, FILE* file_log){
    CHECK_OPERATION(!(*lista_trabocco),
        fprintf(stderr, "Parametri non validi.\n");
        return -1);

    /* Cerca per verificare se il file esiste gia' nella tabella */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL, return 404);
    
    PTHREAD_LOCK(nodo->mutex);

    /* Apre il file */
    FD_SET(fd, &(nodo->open));

    /* Se la lock e' stata acquisita da un altro thread */
    if(nodo->fd_c != -1 && nodo->fd_c!=fd){
        /* Richiude il file per provare al prossimo avvio e restituisce un errore */
        FD_CLR(fd, &(nodo->open));
        PTHREAD_UNLOCK(nodo->mutex);

        return 202; //TODO: ha senso che nella openFile del client, questo cicli finche' non ottiene la lock?
    }

    /* Setta la lock */
    nodo->fd_c = fd;

    /* Aggiorna il file di log */
    fprintf(file_log, "Open_Lock\n");

    PTHREAD_UNLOCK(nodo->mutex);

    return 0;
}

int opens(list_t **lista_trabocco, char* file_path, int fd, FILE* file_log){
    CHECK_OPERATION(!(*lista_trabocco),
        fprintf(stderr, "Parametri non validi.\n");
        return -1);

    /* Cerca per verificare se il file esiste gia' nella tabella */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL, return 404);
    
    PTHREAD_LOCK(nodo->mutex);

    /* Apre il file */
    FD_SET(fd, &(nodo->open));

    /* Aggiorna il file di log */
    fprintf(file_log, "Open\n");

    PTHREAD_UNLOCK(nodo->mutex);

    return 0;
}

int deletes(list_t **lista_trabocco, char* file_path, node** just_deleted, int fd, int *curr_size, FILE* file_log){
    CHECK_OPERATION(*lista_trabocco==NULL,
        fprintf(stderr, "Parametri non validi.\n");
        return -1);
    
    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL,
        return 505);
            
    PTHREAD_LOCK(fifo_queue->mutex);
    PTHREAD_LOCK((*lista_trabocco)->mutex);
    int remover = del(file_path);
    CHECK_OPERATION(remover == -1, 
        PTHREAD_UNLOCK(fifo_queue->mutex);
        PTHREAD_UNLOCK((*lista_trabocco)->mutex);
        return -1);
    node* curr; /* Puntatore al nodo corrente */
    if ((*lista_trabocco)->head == NULL){ /* Lista vuota */
        fprintf(file_log, "Delete\n"); //TODO: specifica nella relazione
        PTHREAD_UNLOCK(fifo_queue->mutex);
        PTHREAD_UNLOCK((*lista_trabocco)->mutex);

        return 0;
    }

    curr = (*lista_trabocco)->head;
    if (strcmp(curr->path, file_path) == 0) { /* Cancellazione del primo nodo */
    
        if(curr->fd_c == fd){
            *just_deleted = curr;
            (*lista_trabocco)->head = curr->next; /* Aggiorna il puntatore alla testa */
            *curr_size -= curr->size_buffer;

            FD_CLR(fd, &(curr->open));
            fprintf(file_log, "Delete\n");

            PTHREAD_UNLOCK(fifo_queue->mutex);
            PTHREAD_UNLOCK((*lista_trabocco)->mutex);

            return 0;
        } 
        /* Non e' stata acquisita la lock */
        else if(curr->fd_c!=fd){
            PTHREAD_UNLOCK(fifo_queue->mutex);
            PTHREAD_UNLOCK((*lista_trabocco)->mutex);

            return 202;
        }
        
    }
    
    /* Scansione della lista a partire dal secondo nodo */
    node* prev = curr;
    curr = curr->next;
    while (curr != NULL) {
        if(curr->fd_c == fd && FD_ISSET(fd, &(curr->open))){
            *just_deleted = curr;
            prev->next = curr->next; /* Aggiorna il puntatore alla testa */
            *curr_size -= curr->size_buffer;

            FD_CLR(fd, &(curr->open));

            fprintf(file_log, "Delete\n");

            PTHREAD_UNLOCK(fifo_queue->mutex);
            PTHREAD_UNLOCK((*lista_trabocco)->mutex);

            return 0;
        } 
        /* Il file non e' aperto */
        else if(!FD_ISSET(fd, &(curr->open))){
            PTHREAD_UNLOCK(fifo_queue->mutex);
            PTHREAD_UNLOCK((*lista_trabocco)->mutex);

            return 303;
        }
        /* Non e' stata acquisita la lock */
        else if(curr->fd_c!=fd){
            PTHREAD_UNLOCK(fifo_queue->mutex);
            PTHREAD_UNLOCK((*lista_trabocco)->mutex);

            return 202;
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
            return curr;
        }
    PTHREAD_UNLOCK((*lista_trabocco)->mutex);
    return NULL;
}

int closes(list_t **lista_trabocco, char* file_path, int fd, FILE* file_log){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);
    
    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL, return 505);

    PTHREAD_LOCK(nodo->mutex);
    
    /* Se il nodo e' aperto */
    if(FD_ISSET(fd, &(nodo->open))){
        FD_CLR(fd, &(nodo->open));
        
        fprintf(file_log, "Close\n");

        PTHREAD_UNLOCK(nodo->mutex);
        
        return 0;
    } 
    /* Il nodo non e' stato aperto */
    else {
        PTHREAD_UNLOCK(nodo->mutex);

        return 303;
    }
    
    PTHREAD_UNLOCK(nodo->mutex);

    return -1;
}

int unlock(list_t **lista_trabocco, char* file_path, int fd, FILE* file_log){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
        return -1);

    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL,
        return 505);

    PTHREAD_LOCK(nodo->mutex);
    /* Se ha acquisito la lock e il nodo e' aperto */
    if(nodo->fd_c == fd && FD_ISSET(fd, &(nodo->open))){
        nodo->fd_c = -1;
        
        fprintf(file_log, "Unlock\n");
    } 
    /* Se il nodo non e' aperto */
    else if(!FD_ISSET(fd, &(nodo->open))){
        PTHREAD_UNLOCK(nodo->mutex);
        
        return 303;
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

int lock(list_t **lista_trabocco, char* file_path, int fd, FILE* file_log){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);
    
    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL, return 505);
                    
    PTHREAD_LOCK(nodo->mutex);

    /* Se la lock era gia' stata acquisita dallo stesso processo o era libera e il nodo e' aperto */
    if(FD_ISSET(fd, &(nodo->open)) && (nodo->fd_c == fd || nodo->fd_c == -1)){     
        nodo->fd_c = fd;
        
        fprintf(file_log, "Lock\n");
    } 
    /* Se il nodo non e' aperto */
    else if(!FD_ISSET(fd, &(nodo->open))){
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

int reads(list_t **lista_trabocco, char* file_path, void** buf, size_t *size_buf, int fd, FILE* file_log){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
        return -1);

    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL,
        *buf = NULL;
        *size_buf = 0;
        return 505);

    PTHREAD_LOCK(nodo->mutex);
    /* Se il nodo e' aperto ed e' stata acquisita la lock */
    if(FD_ISSET(fd, &(nodo->open)) && nodo->fd_c == fd){
        *size_buf = nodo->size_buffer;
        *buf = nodo->buffer;
        
        fprintf(file_log, "Read %ld\n", nodo->size_buffer);
    } 
    /* Se non e' stata acquisita la lock */
    else if(nodo->fd_c==-1){
        *buf = NULL;
        PTHREAD_UNLOCK(nodo->mutex);

        return 555;
    }
    /* Se la lock e' stata acquisita da un altro thread */
    else if(nodo->fd_c!=fd){
        *buf = NULL;
        PTHREAD_UNLOCK(nodo->mutex);

        return 202; //TODO: anche qui non dovrei ciclare?
    }
    /* Se il nodo e' chiuso */
    else {
        *buf = NULL;
        PTHREAD_UNLOCK(nodo->mutex);

        return 303;
    }
    PTHREAD_UNLOCK(nodo->mutex);
    
    return 0;
}

int append_buffer(list_t **lista_trabocco, char* file_path, void* buf, size_t size_buf, int* curr_size, int* max_size_reached, int fd, FILE* file_log){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
        if(buf) free(buf);
        return -1);

    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL,
        if(buf) free(buf);
        return 505);

    PTHREAD_LOCK(nodo->mutex);

    /* Se ha acquisito la lock e il nodo e' aperto */
    if(nodo->fd_c == fd && FD_ISSET(fd, &(nodo->open))){
        /* Effettua la append del buffer passato come parametro e aggiorna la size del nodo */
        nodo->buffer = realloc(nodo->buffer, nodo->size_buffer + size_buf); 
        CHECK_OPERATION(nodo->buffer == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); 
            PTHREAD_UNLOCK(nodo->mutex);
            free(buf);
            return -1);

        memcpy(nodo->buffer + nodo->size_buffer, buf, size_buf);
        
        nodo->size_buffer += (size_buf); 

        /* Aggiorna la massima size raggiunta */
        *max_size_reached = max(*curr_size, *max_size_reached);
        fprintf(file_log, "Write %ld\n", size_buf);

        /* Aggiorna la size corrente della tabella hash */
        *curr_size = *curr_size + nodo->size_buffer;
    } 
    /* Se non ha acquisito la lock */
    else if(nodo->fd_c != fd){
        if(buf) free(buf);
        PTHREAD_UNLOCK(nodo->mutex);

        return 202; //TODO: anche qui, perche' non ciclo?
    } 
    /* Il nodo non e' stato aperto */
    else if(!FD_ISSET(fd, &(nodo->open))){
        if(buf) free(buf);
        PTHREAD_UNLOCK(nodo->mutex);

        return 303;
    }
    if(buf) free(buf);
    PTHREAD_UNLOCK(nodo->mutex);
        
    return 0;
}

int writes(list_t **lista_trabocco, char* file_path, void* buf, size_t size_buf, int *max_size, int* curr_size, int* max_size_reached, node** deleted, int fd, FILE* file_log){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
        if(buf) free(buf);
        return -1);
            
    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL,
        size_buf = 0;
        if(buf)free(buf);
        return 505);
    
    PTHREAD_LOCK(nodo->mutex);
    if(nodo->fd_create_open == fd){ //TODO: specificare nella relazione che solo chi crea puo' fare la write
        if(buf){
            nodo->buffer = malloc(size_buf);
            CHECK_OPERATION(nodo->buffer == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); free(buf); return -1);
            memcpy(nodo->buffer, buf, size_buf);  
            nodo->size_buffer = size_buf;
            *curr_size += size_buf;
            free(buf);
            *max_size_reached = max(*curr_size, *max_size_reached);
            fprintf(file_log, "Write %ld\n", size_buf);
            nodo->fd_create_open = -1;
            PTHREAD_UNLOCK(nodo->mutex);
        
            return 0;
        } 
        
    } 
    /* Se non e' stata fatta la openFile(O_CREATE | O_LOCK) */
    else {
        if(buf) free(buf);
        PTHREAD_UNLOCK(nodo->mutex);

        return 606;
    }

    if(buf) free(buf);
    PTHREAD_UNLOCK(nodo->mutex);

    return 0;
}

void print_hash(list_t *queue){
    node* curr = queue->head;
    while(curr){
        if(!curr->buffer)   
            fprintf(stdout, "Elemento con buffer vuoto nella tabella hash: %s\n", curr->path);
        else
            fprintf(stdout, "Elemento con buffer non vuoto nella tabella hash: %s\n", curr->path);
        curr = curr->next;
    }
}