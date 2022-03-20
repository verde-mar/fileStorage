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
                fprintf(stderr, "La lista di trabocco che si voleva eliminare e' vuota.\n"););
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

int add_hashtable(char *name_file, int fd, int flags){
    CHECK_OPERATION(name_file==NULL || fd<0, 
        fprintf(stderr, "Parametro non valido.\n");
        return -1);

    /* Aggiunge l' elemento nella tabella hash */
    int hash = hash_function(name_file); 
    int success = -1;
    /* Se devono essere fatte operazioni di creazione */
    if((flags == 2 || flags == 6) && fifo_queue->elements <= table->max_file){
        success = add(&(table->queue[hash]), name_file, fd, flags, &(table->max_file_reached), table->file_log);   
        CHECK_OPERATION(success==-1, 
            fprintf(stderr, "Errore nell'inserimento di un elemento nella tabella hash.\n"); 
            return -1);
    } 
    /* Se non devono essere fatte operazioni di creazione */
    else if(flags != 2 && flags != 6){
        success = add(&(table->queue[hash]), name_file, fd, flags, &(table->max_file_reached), table->file_log);   
        CHECK_OPERATION(success==-1, 
            fprintf(stderr, "Errore nell'inserimento di un elemento nella tabella hash.\n"); 
            return -1);
    } 
    /* Se dovevano essere fatte operazioni di creazione ma non c'e' abbastanza spazio */
    else {
        success = 777;
    }
    return success;    
}

void print_elements(){
    for (int i = 0; i < 16; i++)
        if (table->queue[i]) {
            print_hash(table->queue[i]);
        }
}

int del_hashtable(char *name_file, node **just_deleted, int fd){
    CHECK_OPERATION(name_file==NULL || fd<0,
        fprintf(stderr, "Parametri non validi.\n");
        return -1;);

    int hash = hash_function(name_file);
   
    /* Elimina un nodo */
    int success = deletes(&(table->queue[hash]), name_file, just_deleted, fd, &(table->curr_size), table->file_log);
    CHECK_OPERATION(success == -1, 
        fprintf(stderr, "Errore nell'eliminazione di un elemento nella tabella hash.\n"); 
        return -1);

    return success;
}

int close_hashtable(char *name_file, int fd){
    CHECK_OPERATION(name_file==NULL || fd<0,
        fprintf(stderr, "Parametri non validi.\n");
        return -1;);

    
    int hash = hash_function(name_file); 
   
    /* Chiude un nodo */
    int success = closes(&(table->queue[hash]), name_file, fd, table->file_log);
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nella chiusura di un elemento nella tabella hash.\n"); 
        return -1);
    
    return success;
}

int unlock_hashtable(char *name_file, int fd){
    CHECK_OPERATION(name_file==NULL || fd<0,
        fprintf(stderr, "Parametri non validi.\n");
        return -1;);

    
    int hash = hash_function(name_file); 

    /* Rilascia la lock di un nodo */
    int success = unlock(&(table->queue[hash]), name_file, fd, table->file_log);
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nel reset della lock di un elemento nella tabella hash.\n"); 
        return -1);

    return success;
}

int lock_hashtable(char *name_file, int fd){
    CHECK_OPERATION(name_file==NULL || fd<0,
        fprintf(stderr, "Parametri non validi.\n");
        return -1;);

    int hash = hash_function(name_file); 

    /* Acquisisce la lock di un nodo */
    int success = lock(&(table->queue[hash]), name_file, fd, table->file_log);
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nel set della lock di un elemento nella tabella hash.\n"); 
        return -1);

    return success;
}

int read_hashtable(char *name_file, void** buf, size_t *size_buf, int fd){
    CHECK_OPERATION(name_file==NULL || fd<0,
        fprintf(stderr, "Parametri non validi.\n");
        return -1;);
    
    int hash = hash_function(name_file); 

    /* Legge i dati di un nodo */
    int success = reads(&(table->queue[hash]), name_file, buf, size_buf, fd, table->file_log);
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nella lettura di un elemento nella tabella hash.\n"); 
        return -1);

    return success;
}

int append_hashtable(char* name_file, void* buf, size_t* size_buf, node** deleted, int fd){
    CHECK_OPERATION((name_file==NULL || fd<0) || buf==NULL ,
        fprintf(stderr, "Parametri non validi.\n");
        return -1;);

    
    int been_deleted = 0;
    if(*size_buf<=table->max_size){
        table->curr_size += *size_buf;
        if(table->curr_size > table->max_size){
            /* Trova l'elemento in testa alla coda cache */
            PTHREAD_LOCK(fifo_queue->mutex);
            fifo_queue->how_many_cache++;
            been_deleted = 1;
            char* to_delete = head_name(fifo_queue);
            PTHREAD_UNLOCK(fifo_queue->mutex);
            /* Preleva il  nodo dalla tabella hash */
            if(to_delete){
                int hash_del = hash_function(to_delete);
                int open = add(&(table->queue[hash_del]), to_delete, fd, 5, &(table->max_size_reached), table->file_log);
                CHECK_OPERATION(open ==-1, return -1);
                int del = deletes(&(table->queue[hash_del]), to_delete, deleted, fd, &(table->curr_size), table->file_log); CHECK_OPERATION(del == -1, return -1;);
                CHECK_OPERATION(del == -1, return -1);
                if(buf) free(buf); 
            }
        }
    } else {
        free(buf);
        return 444;
    }   
    if(been_deleted) return 909;

    int hash = hash_function(name_file);
     
    /* Se prima non e' stata fatta la writeFile, viene restituito un errore */
    CHECK_OPERATION(buf == NULL, return 707);

    /* Effettua la append su un nodo */
    int success = append_buffer(&(table->queue[hash]), name_file, buf, *size_buf, &(table->max_size), &(table->curr_size), &(table->max_size_reached), fd, table->file_log);
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nella append su un elemento nella tabella hash.\n"); 
        if(buf) free(buf);
        return -1);
    return success;
}

int write_hashtable(char* name_file, void* buf, size_t* size_buf, node** deleted, int fd){
    CHECK_OPERATION((name_file==NULL || fd<0) || buf==NULL ,
        fprintf(stderr, "Parametri non validi.\n");
        if(buf) free(buf);
        return -1;);
    
    /* Vengono fatti controlli sulla size dell'elemento da inserire */
    int been_deleted = 0;
    if(*size_buf<=table->max_size){
        table->curr_size += *size_buf;
        if(table->curr_size > table->max_size){
            /* Trova l'elemento in testa alla lista cache */

            PTHREAD_LOCK(fifo_queue->mutex);
            fifo_queue->how_many_cache++;
            been_deleted = 1;
            char* to_delete = head_name(fifo_queue);
            PTHREAD_UNLOCK(fifo_queue->mutex);

            if(to_delete){
                /* Elimina l'elemento dalla tabella hash */
                int hash_del = hash_function(to_delete);
                int del = deletes(&(table->queue[hash_del]), to_delete, deleted, fd, &(table->curr_size), table->file_log); CHECK_OPERATION(del == -1, return -1;);
                CHECK_OPERATION(del == -1, return -1);
                if(buf) free(buf); 
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
    int hash = hash_function(name_file);

    /* Effettua la write sull'elemento */
    int success = writes(&(table->queue[hash]), name_file, buf, *size_buf, &(table->max_size), &(table->curr_size), &(table->max_size_reached), deleted, fd, table->file_log);
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nella scrittura di un elemento nella tabella hash.\n"); 
        if(buf) free(buf);
        return -1);

    return success;
}

int definitely_deleted(node** just_deleted){
    CHECK_OPERATION((*just_deleted)==NULL, fprintf(stderr, "Parametri non validi.\n"); return -1);

    PTHREAD_DESTROY_LOCK((*just_deleted)->mutex);
    PTHREAD_DESTROY_COND((*just_deleted)->locked); 
    free((*just_deleted)->locked);
    free((*just_deleted)->mutex);

    if((*just_deleted)->buffer)   
        free((*just_deleted)->buffer);
    free((char*)(*just_deleted)->path);
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
        if(curr == NULL)
            success = 111;
            
        /* Scorre la lista finche' non trova l'elemento di indice N */
        while(curr->next && N >= 1){
            curr = curr->next;
            N--;
        }
        
        /* Acquisisce la lock sull'elemento */
        int hash = hash_function((char*)curr->path); 
        *path = (char*)curr->path;
        success = add(&(table->queue[hash]), (char*)curr->path, fd, 5, &(table->max_size_reached), table->file_log);
        CHECK_OPERATION(success==-1, 
            fprintf(stderr, "Errore nella apertura di un elemento nella tabella hash.\n"); 
            return -1);

        /* Legge i dati dell'elemento */
        success = reads(&(table->queue[hash]), (char*)curr->path, buf, size_buf, fd, table->file_log);
        CHECK_OPERATION(success==-1, 
            fprintf(stderr, "Errore nella lettura di un elemento nella tabella hash.\n"); 
            return -1);

        /* Chiude l'elemento */
        success = closes(&(table->queue[hash]), (char*)curr->path, fd, table->file_log);
        CHECK_OPERATION(success==-1, 
            fprintf(stderr, "Errore nella chiusura di un elemento nella tabella hash.\n"); 
            return -1);
                  
    } 
    /* Se N e' maggiore degli elementi restituiti viene generato un errore */
    else {
        *path = NULL;
        *buf = NULL;
        success = 111;
    }
    
    return success;
}