#include <hash.h>
#include <stdio.h>

#include <stdlib.h>

void *myfun(void *arg){
    add_hashtable((char*)arg, 1, 2);
    return NULL;
}

void *myfundelete(void *arg){
    
    node* deleted;
    del_hashtable((char*)arg, &deleted, 1);
    printf("deleted: %s\n", deleted->path);
    return NULL;
}

int main(int argc, char const *argv[])
{
    pthread_t tid;
    int err, status;
    create_hashtable(100);

    if((err=pthread_create(&tid, NULL, &myfun, "micio"))!=0){
        printf("errore\n");
    } else {
        pthread_join(tid, (void*)&status);
    }

    pthread_t tid1;
    int err1;
    if((err1=pthread_create(&tid1, NULL, &myfun, "cane"))!=0){
        printf("errore\n");
    } else {
        pthread_join(tid1, (void*)&status);
    }

    pthread_t tid2;
    int err2;
    if((err2=pthread_create(&tid2, NULL, &myfundelete, "cane"))!=0){
        printf("errore\n");
    } else {
        pthread_join(tid2, (void*)&status);
    }

    node_c* current = fifo_queue->head;
    while(current!=NULL){
        printf("current->path: %s\n", current->path);
        current = current->next;
    }
    destroy_hashtable();

    return 0;
}
