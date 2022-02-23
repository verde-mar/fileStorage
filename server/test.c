#include <hash.h>
#include <stdio.h>

#include <stdlib.h>
#include <check_errors.h>
#include <threadpool.h>

void *myfun(void *arg){
    add_hashtable((char*)arg, 1, 6);
    return NULL;
}


int main(int argc, char const *argv[])
{
    threadpool_t* t;
    create_threadpool(&t, 10, 2);
   
    /*pthread_t tid;
    int err, status;
    create_hashtable(100);

    if((err=pthread_create(&tid, NULL, &myfun, &t))!=0){
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


    destroy_hashtable();*/
    destroy_threadpool(&t);
    return 0;
}
