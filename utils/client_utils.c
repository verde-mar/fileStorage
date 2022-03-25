/**
 * @file client_utils.c
 * @author Sara Grecu (s.grecu1@studenti.unipi.it)
 * @brief Contiene tutte le funzioni utili al client
 * @version 0.1
 * @date 2022-03-09
 * 
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <fcntl.h>
#include <check_errors.h>
#include <stdlib.h>

#include <worker.h>
#include <client_utils.h>
#include <utils.h>

#include <socketIO.h>

//TODO: dovrei documentare questa funzione nella relazione

int open_write_append(const char* rest, const char* dirnameD){
    int err_caller = -1, err_unlock = -1, err_close = -1;
    /* Richiede l'apertura e la lock sul file identificato da rest */
    err_caller = openFile(rest, O_CREATE | O_LOCK);
    CHECK_OPERATION(err_caller == -1, return -1);
    int err_w = 0, was_open = -1; 
    
    if(err_caller == 101){
        was_open = err_caller;
        err_caller = openFile(rest, O_LOCK);
        CHECK_OPERATION(err_caller == -1, return -1);
    }

    if(err_caller==0){
        /* Richiede la scrittura sul file identificato da rest */
        err_w = writeFile(rest, dirnameD);
        CHECK_OPERATION(err_w == 444 || err_w == -1,  
            err_unlock = unlockFile(rest);
            CHECK_OPERATION(err_unlock == -1, return -1);
            err_close = closeFile(rest);
            CHECK_OPERATION(err_close == -1, return -1);
            return -1;);
    }

    if(was_open == 101 && err_w == 606){
        size_t size;
        void *buf;

        int err_rbuf = read_from_file((char*)rest, &buf, &size);
        CHECK_OPERATION(err_rbuf == -1,
            err_close = closeFile(rest);
            CHECK_OPERATION(err_close == -1, return -1);
            err_unlock = unlockFile(rest);
            CHECK_OPERATION(err_unlock == -1, return -1);
            return -1;);
        
        int err_append = appendToFile(rest, buf, size, dirnameD);
        CHECK_OPERATION(err_append == -1, 
            err_close = closeFile(rest);
            CHECK_OPERATION(err_close == -1, return -1);
            err_unlock = unlockFile(rest);
            CHECK_OPERATION(err_unlock == -1, return -1);
            return -1;);
    }

    /* Richiede il rilascio della lock sul file iile identificato da rest */
    err_unlock = unlockFile(rest);
    CHECK_OPERATION(err_unlock == -1, return -1);

    /* Richiede la chiusura del file identificato da rest */  
    err_close = closeFile(rest);
    CHECK_OPERATION(err_close == -1, 
        err_unlock = unlockFile(rest);
        CHECK_OPERATION(err_unlock == -1, return -1);
        return -1;);

    return 0;
}

int caller_two(int (*fun) (const char*, const char*), const char* pathname, const char* dirnameD){ 
    int err;
    if(is_directory(pathname)){
        DIR *dir = opendir(pathname);
        CHECK_OPERATION(dir == NULL, fprintf(stderr, "Errore sulla opendir.\n"); return -1;);
        
        struct dirent *file;
        while((errno=0, file = readdir(dir))!=NULL && pathname != NULL){
            int len = strlen(pathname) + strlen(file->d_name) + strlen("/") + 1;
            const char *path = malloc(sizeof(char)*len);
            path = strcpy((char*)path, pathname);
            path = strcat((char*)path, "/");
            path = strcat((char*)path, file->d_name);

            if(strcmp(file->d_name, "..")!=0 && strcmp(file->d_name, ".")!=0){
                if(is_regular_file(path)){
                    err = fun(path, dirnameD);
                    CHECK_OPERATION(err == -1, fprintf(stderr, "Errore nella chiamata a fun(pathname).\n"); 
                            int check = closedir(dir);
                            CHECK_OPERATION(check == -1, fprintf(stderr, "Errore nella closedir.\n"); return -1);
                                return -1;);
                } else if(is_directory(path)){
                    int result = caller_two(fun, path, dirnameD);
                    CHECK_OPERATION(result == -1, 
                            int check = closedir(dir);
                            CHECK_OPERATION(check == -1, fprintf(stderr, "Errore nella closedir.\n"); return -1);
                                return -1;);
                }
            }
            free((char*)path);
        }
        int check = closedir(dir);
        CHECK_OPERATION(check == -1, fprintf(stderr, "Errore nella closedir.\n"); return -1);
    } else if(is_regular_file(pathname)){
        /* Gestisce la richiesta */
        err = fun(pathname, dirnameD);
        CHECK_OPERATION(err == -1, fprintf(stderr, "Errore nella chiamata a fun(pathname).\n"); return -1);

    }

    return 0;
}

int receiver(int *byte_letti, int *byte_scritti, size_t size_path, char** path, void** old_file, size_t *size_old){
    errno = 0;
    *byte_letti += read_size(fd_skt, &size_path); 
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la size del path che sta per arrivare e che sta per essere memorizzato.\n"); 
        return -1);
                
    *path = malloc(size_path);
    CHECK_OPERATION(*path == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
        return -1);

    errno = 0;
    *byte_letti += read_msg(fd_skt, *path, size_path);
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere il buffer che sta per arrivare e che sta per essere memorizzato.\n"); 
        return -1);
                
    errno = 0;
    *byte_letti += read_size(fd_skt, size_old); 
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la size del file che stavo per memorizzare.\n"); 
                return -1);
    
    *old_file = malloc(*size_old);
    CHECK_OPERATION(*old_file == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
            return -1);

    errno = 0;
    *byte_letti += read_msg(fd_skt, *old_file, *size_old);
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere il file.\n"); 
                return -1);
                
    return 0;
}