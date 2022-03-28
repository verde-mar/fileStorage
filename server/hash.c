/**
 * @file hash.c
 * @author Sara Grecu (s.grecu1@studenti.unipi.it)
 * @brief Implementazione della tabella hash
 * @version 0.1
 * @date 2022-03-09
 * 
 */
#include <stdio.h>
#include <hash.h>
#include <check_errors.h>

#include <stdlib.h>
#include <string.h>
#include <utils.h>

unsigned long hash_function(char *str){
    unsigned long hash = 5381;
    int c = *str++;

    while (c){
        hash += + c;
        c = *str++;
    }
    return (hash%16);
}

int create_hashtable(size_t size, int num_file, char* log_file){
    /* Inizializza la struttura dati della tabella */
    table = (hashtable*) malloc(sizeof(hashtable));
    CHECK_OPERATION(table == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
        return -1);

    /* Inizializza l'array delle liste di trabocco */
    table->queue = malloc(sizeof(list_t*)*16);
    CHECK_OPERATION(table->queue == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
        return -1);

    /* Inizializza le liste di trabocco*/
    for (int i = 0; i < 16; i++) {
        int err = create_list(&(table->queue[i])); 
        CHECK_OPERATION(err == -1, return -1);
    }

    /* Inizia la coda FIFO */
    int create = create_fifo();
    CHECK_OPERATION(create == -1,
        fprintf(stderr, "Errore nella creazione della coda FIFO.\n");
        return -1);

    /* Inizializza la size corrente */
    table->curr_size = 0;
    /* Inizializza la size massima della tabella hash */
    table->max_size = size;
    /* Inizializza il numero massimo di file da avere nel file storage */
    table->max_file = num_file;
    /* Massimo numero di file raggiunti durante l'esecuzione */
    table->max_file_reached = 0;
    /* Massima size raggiunta durante l'esecuzione */
    table->max_size_reached = 0;   

    table->file_log = fopen(log_file, "w");
    CHECK_OPERATION(table->file_log == NULL, fprintf(stderr, "Errore nell'apertura/creazione del file di log.\n"); return -1);
    fprintf(table->file_log, "E' stata creata la tabella hash.\n");

    return 0;
}

int destroy_hashtable (){
    /* Elimina tutte le liste di trabocco */
    int destroy;
    for (int i = 0; i < 16; i++)
        if (table->queue[i]) {
            destroy = destroy_list(&(table->queue[i]));
            CHECK_OPERATION(destroy == -1, 
                fprintf(stderr, "La lista di trabocco che si voleva eliminare e' vuota.\n"); continue);
        }
    free(table->queue);

    /* Elimina la coda FIFO */
    int del = delete_fifo(&fifo_queue);
    CHECK_OPERATION(del == -1, fprintf(stderr, "Errore nella eliminazione della coda cache.\n"); return -1);

    fprintf(table->file_log, "E' stata distrutta la tabella hash.\n");

    int err_close = fclose(table->file_log);
    CHECK_OPERATION(err_close == -1, fprintf(stderr, "Errore nella chiusura del file di log.\n"); return -1);

    /* Elimina la tabella hash */
    free(table);

    return 0;
}

int creates_locks_hashtable(char *path, int fd, node** just_deleted){
    CHECK_OPERATION(path==NULL || fd<0, 
        fprintf(stderr, "Parametri non validi.\n");
        return -1);
    
    /* Calcola l'hash dell'elemento da inserire */
    int hash = hash_function(path); 
    int success = -1;

    if((fifo_queue->elements + 1) <= table->max_file){
        success = creates_locks(&(table->queue[hash]), path, fd, &(table->max_file_reached), table->file_log);
        CHECK_OPERATION(success == -1, fprintf(stderr, "Errore nella creazione del nodo.\n"); return -1);
    } else {
    /* Trova l'elemento in testa alla coda cache */
        PTHREAD_LOCK(fifo_queue->mutex);
        fifo_queue->how_many_cache++;
        char* to_delete = head_name(fifo_queue);
        PTHREAD_UNLOCK(fifo_queue->mutex);
        /* Preleva il  nodo dalla tabella hash */
        if(to_delete){
            int hash_del = hash_function(to_delete);
            int del = deletes(&(table->queue[hash_del]), to_delete, just_deleted, fd, &(table->curr_size), table->file_log);
            CHECK_OPERATION(del == -1, fprintf(stderr, "Errore nella deletes di %s\n", to_delete); return -1); 
            
            success = 909;   
            printf("\n\nsto per eliminare %s\n", to_delete);
        }
    }
    return success;
}

int creates_hashtable(char *path, int fd, node** just_deleted){
    CHECK_OPERATION(path==NULL || fd<0, 
        fprintf(stderr, "Parametri non validi.\n");
        return -1);
    
    /* Calcola l'hash dell'elemento da inserire */
    int hash = hash_function(path); 
    int success = -1;

    if((fifo_queue->elements + 1) <= table->max_file){
        success = creates(&(table->queue[hash]), path, fd, &(table->max_file_reached), table->file_log);
        CHECK_OPERATION(success == -1, fprintf(stderr, "Errore nella creazione del nodo.\n"); return -1);
    } else {
        /* Trova l'elemento in testa alla coda cache */
        PTHREAD_LOCK(fifo_queue->mutex);
        fifo_queue->how_many_cache++;
        char* to_delete = head_name(fifo_queue);
        
        PTHREAD_UNLOCK(fifo_queue->mutex);
        /* Preleva il  nodo dalla tabella hash */
        if(to_delete){
            int hash_del = hash_function(to_delete);
            int del = deletes(&(table->queue[hash_del]), to_delete, just_deleted, fd, &(table->curr_size), table->file_log); 
            CHECK_OPERATION(del == -1, fprintf(stderr, "Errore nella deletes di %s\n", to_delete); return -1); 
            success = 909;  

        }
    }
    return success;
}

int opens_locks_hashtable(char *path, int fd){
    CHECK_OPERATION(path==NULL || fd<0, 
        fprintf(stderr, "Parametri non validi.\n");
        return -1);
    
    /* Calcola l'hash dell'elemento da inserire */
    int hash = hash_function(path); 
    int success = -1;

    success = opens_locks(&(table->queue[hash]), path, fd, table->file_log);
    CHECK_OPERATION(success == -1, fprintf(stderr, "Errore nella acquisizione della lock del nodo.\n"); return -1);

    return success;
}

int opens_hashtable(char *path, int fd){
    CHECK_OPERATION(path==NULL, 
        fprintf(stderr, "Parametro non valido.\n");
        return -1);
    
    /* Calcola l'hash dell'elemento da inserire */
    int hash = hash_function(path); 
    int success = -1;

    success = opens(&(table->queue[hash]), path, fd, table->file_log);
    CHECK_OPERATION(success == -1, fprintf(stderr, "Errore nella acquisizione della lock del nodo.\n"); return -1);  

    return success;
}

void print_elements(){
    for (int i = 0; i < 16; i++)
        if (table->queue[i]) {
            print_hash(table->queue[i]);
        }
}

int del_hashtable(char *path, node **just_deleted, int fd){
    CHECK_OPERATION(path==NULL || fd<0,
        fprintf(stderr, "Parametri non validi.\n");
        return -1;);

    int hash = hash_function(path);
   
    /* Elimina un nodo */
    int success = deletes(&(table->queue[hash]), path, just_deleted, fd, &(table->curr_size), table->file_log);
    CHECK_OPERATION(success == -1, 
        fprintf(stderr, "Errore nell'eliminazione di un elemento nella tabella hash.\n"); 
        return -1);

    return success;
}

int close_hashtable(char *path, int fd){
    CHECK_OPERATION(path==NULL || fd<0,
        fprintf(stderr, "Parametri non validi.\n");
        return -1;);

    
    int hash = hash_function(path); 
   
    /* Chiude un nodo */
    int success = closes(&(table->queue[hash]), path, fd, table->file_log);
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nella chiusura di un elemento nella tabella hash.\n"); 
        return -1);
    
    return success;
}

int unlock_hashtable(char *path, int fd, int* fd_next){
    CHECK_OPERATION(path==NULL || fd<0,
        fprintf(stderr, "Parametri non validi %d\n", fd);
        return -1;);

    
    int hash = hash_function(path); 

    /* Rilascia la lock di un nodo */
    int success = unlock(&(table->queue[hash]), path, fd, fd_next, table->file_log);
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nel reset della lock di un elemento nella tabella hash.\n"); 
        return -1);

    return success;
}

int lock_hashtable(char *path, int fd){
    CHECK_OPERATION(path==NULL || fd<0,
        fprintf(stderr, "Parametri non validi.\n");
        return -1;);

    int hash = hash_function(path); 

    /* Acquisisce la lock di un nodo */
    int success = lock(&(table->queue[hash]), path, fd, table->file_log);
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nel set della lock di un elemento nella tabella hash.\n"); 
        return -1);

    return success;
}

int read_hashtable(char *path, void** buf, size_t *size_buf, int fd){
    CHECK_OPERATION(path==NULL || fd<0,
        fprintf(stderr, "Parametri non validi.\n");
        return -1;);
    
    int hash = hash_function(path); 

    /* Legge i dati di un nodo */
    int success = reads(&(table->queue[hash]), path, buf, size_buf, fd, table->file_log);
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nella lettura di un elemento nella tabella hash.\n"); 
        return -1);

    return success;
}

int append_hashtable(char* path, void* buf, size_t* size_buf, node** deleted, int fd){
    CHECK_OPERATION((path==NULL || fd<0) || buf==NULL ,
        fprintf(stderr, "Parametri non validi.\n");
        if(buf) free(buf);
        return -1;);
    
    int been_deleted = 0;
    if(*size_buf<=table->max_size){
        printf("STO PER CONTROLLARE SE LA SIZE DELL'ELEMENTO CHE VOGLIO INSERIRE VA BENE %s\n", path);
        if((table->curr_size + *size_buf) > table->max_size){
            /* Trova l'elemento in testa alla coda cache */
            PTHREAD_LOCK(fifo_queue->mutex);
            printf("HO APPENA PRESO LA LOCK DELLA CODA FIFO\n");
            fifo_queue->how_many_cache++;
            char* to_delete = head_name(fifo_queue);
            PTHREAD_UNLOCK(fifo_queue->mutex);
            printf("HO RILASCIATO LA LOCK DELLA CODA FIFO, E SO CHI VOGLIO ELIMINARE: %s\n", to_delete);
            /* Preleva il  nodo dalla tabella hash */
            if(to_delete){
                int hash_del = hash_function(to_delete);
                int del = deletes(&(table->queue[hash_del]), to_delete, deleted, fd, &(table->curr_size), table->file_log); 
                CHECK_OPERATION(del == -1, fprintf(stderr, "Errore nella deletes di %s\n", to_delete); if(buf) free(buf); return -1); 
                CHECK_OPERATION(del == 333, fprintf(stderr, "Non ci sono piu' elementi da eliminare.\n"); *deleted = NULL; if(buf) free(buf); return 333);
                if(buf) free(buf);
            } else {
                if(buf) free(buf);
                return 333;
            }
        }
    } else {
        if(buf) free(buf);
        return 444;
    }   
    if(been_deleted) return 909;

    int hash = hash_function(path);
    
    /* Se prima non e' stata fatta la writeFile, viene restituito un errore */
    CHECK_OPERATION(buf == NULL, return 707);

    /* Effettua la append su un nodo */
    int success = append_buffer(&(table->queue[hash]), path, buf, *size_buf, &(table->curr_size), &(table->max_size_reached), fd, table->file_log);
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nella append su un elemento nella tabella hash.\n"); 
        return -1);
        
    return success;
}

int write_hashtable(char* path, void* buf, size_t* size_buf, node** deleted, int fd){
    CHECK_OPERATION((path==NULL || fd<0) || buf==NULL ,
        fprintf(stderr, "Parametri non validi.\n");
        if(buf) free(buf);
        return -1;);
        
    /* Vengono fatti controlli sulla size dell'elemento da inserire */
    int been_deleted = 0;
    if(*size_buf<=table->max_size){
       //printf("STO PER CONTROLLARE SE LA SIZE DELL'ELEMENTO CHE VOGLIO INSERIRE VA BENE %s\n", path);
        if((table->curr_size + *size_buf) > table->max_size){
            /* Trova l'elemento in testa alla coda cache */
            PTHREAD_LOCK(fifo_queue->mutex);
            //printf("HO APPENA PRESO LA LOCK DELLA CODA FIFO\n");
            fifo_queue->how_many_cache++;
            char* to_delete = head_name(fifo_queue);
            PTHREAD_UNLOCK(fifo_queue->mutex);
            //printf("HO RILASCIATO LA LOCK DELLA CODA FIFO, E SO CHI VOGLIO ELIMINARE: %s\n", to_delete);
            /* Preleva il  nodo dalla tabella hash */
            if(to_delete){
                printf("[SERVER] devo eliminare il nodo con nome: %s\n", to_delete);
                int hash_del = hash_function(to_delete);
                int del = deletes(&(table->queue[hash_del]), to_delete, deleted, fd, &(table->curr_size), table->file_log); 
                CHECK_OPERATION(del == -1, fprintf(stderr, "Errore nella deletes di %s\n", to_delete); if(buf) free(buf); return -1); 
                CHECK_OPERATION(del == 333, fprintf(stderr, "Il thread e' stato deschedulato prima di poter eliminare %s, e ha trovato la lista a NULL.\n", to_delete); *deleted = NULL; if(buf) {free(buf);}  return 333);
                printf("[SERVER]Ho eliminato il nodo con path: %s\n", (*deleted)->path);
                if(buf) free(buf);
                if(deleted) been_deleted = 1;
            } else {
                if(buf) free(buf);
                return 333;
            }
        }
    } 
    /* L'elemento da inserire e' troppo grande */
    else {
        if(buf) free(buf);
        return 444;
    }   
    /* Se e' stato eliminato un elemento restituisce al client il codice d'errore */
    if(been_deleted) return 909;
    int hash = hash_function(path);

    /* Effettua la write sull'elemento */
    int success = writes(&(table->queue[hash]), path, buf, *size_buf, &(table->max_size), &(table->curr_size), &(table->max_size_reached), deleted, fd, table->file_log);
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nella scrittura di un elemento nella tabella hash.\n"); 
        return -1);
        
    return success;
}

int definitely_deleted(node** just_deleted){
    CHECK_OPERATION((*just_deleted)==NULL, fprintf(stderr, "Parametri non validi.\n"); return -1);

    /* Elimina la mutex e ne libera la memoria */
    PTHREAD_DESTROY_LOCK((*just_deleted)->mutex);
    free((*just_deleted)->mutex);
    if((*just_deleted)->buffer)   
        free((*just_deleted)->buffer);
    /* Libera la memoria associata al path del nodo */
    free((char*)(*just_deleted)->path);
    (*just_deleted)->path = NULL;

    /* Elimina la lista di attesa */
    int err_del_wl = delete_list_wait(&(*just_deleted)->waiting_list);
    CHECK_OPERATION(err_del_wl == -1, fprintf(stderr, "Errore nella eliminazione della lista di attesa.\n"); return -1);

    free((*just_deleted));
    *just_deleted = NULL;

    return 0;
}

int readN_hashtable(int N, void** buf, size_t *size_buf, int fd, char** path){
    CHECK_OPERATION(fd<0,
        fprintf(stderr, "Parametri non validi.\n");
        return -1;);
    
    int success = -1;
    
    /* Se il numero di elementi presenti e' maggiore o uguale dell' indice dell'elemento da leggere */
    if(fifo_queue->elements >= N){
        node_c *curr = fifo_queue->head;
        if(curr == NULL){
            *path = NULL;
            *buf = NULL;
            *size_buf = 0;
            success = 111;
            return success;
        }
        /* Scorre la lista finche' non trova l'elemento di indice N */
        while(curr->next && N >= 1){
            curr = curr->next;
            N--;
        }
        /* Acquisisce la lock sull'elemento */
        int hash = hash_function((char*)curr->path); 
        *path = (char*)curr->path;
        
        node *nodo = NULL;
        PTHREAD_LOCK(table->queue[hash]->mutex);

        for (nodo=table->queue[hash]->head; nodo != NULL; nodo=nodo->next)
            if (strcmp(nodo->path, *path) == 0){
                PTHREAD_UNLOCK(table->queue[hash]->mutex);
                PTHREAD_LOCK(nodo->mutex);
                *path = malloc(sizeof(char)*(strlen(nodo->path)+1));
                strcpy(*path, nodo->path);

                /* Legge il buffer */
                *size_buf = nodo->size_buffer;
                *buf = malloc(*size_buf);
                CHECK_OPERATION(*buf == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); free(path); path = NULL; free(buf); buf = NULL; *size_buf=0; PTHREAD_UNLOCK(nodo->mutex); return -1);
                void* check = memcpy(*buf, nodo->buffer, *size_buf);  
                CHECK_OPERATION(check == NULL, fprintf(stderr, "La memcpy della readN e' fallita.\n");free(path); path = NULL; free(buf); buf = NULL; *size_buf=0; PTHREAD_UNLOCK(nodo->mutex); return -1);
                
                fprintf(table->file_log, "Read %ld\n", nodo->size_buffer);

                PTHREAD_UNLOCK(nodo->mutex);

                success = 0;
                break;
            }

        if(nodo == NULL){
            PTHREAD_UNLOCK(table->queue[hash]->mutex);
            *path = NULL;
            *buf = NULL;
            *size_buf = 0;
            success = 111;
        }

        CHECK_OPERATION(success != 0, *path = NULL; *buf = NULL; *size_buf = 0;);
    } 
    /* Se N e' maggiore degli elementi restituiti viene generato un errore */
    else {
        *path = NULL;
        *buf = NULL;
        *size_buf = 0;
        success = 111;
    }
    
    return success;
}
