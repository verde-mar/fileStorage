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

int caller_open(const char* pathname){
    int err;
    if(is_directory(pathname)){
        DIR *dir = opendir(pathname);
        CHECK_OPERATION(dir == NULL, fprintf(stderr, "Errore sulla opendir.\n"); return -1;);
        
        struct dirent *file;
        while((errno=0, file = readdir(dir))!=NULL && pathname != NULL){
            int len = strlen(pathname) + strlen(file->d_name) + strlen("/") + 1;
            char *path = malloc(sizeof(char)*len);
            CHECK_OPERATION(path == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return -1);
            path = strcpy((char*)path, pathname);
            path = strcat((char*)path, "/");
            path = strcat((char*)path, file->d_name);
            path[len-1] = '\0';

            if(strcmp(file->d_name, "..")!=0 && strcmp(file->d_name, ".")!=0){
                if(is_regular_file(path)){
                    err = openFile((const char*)path, O_CREATE | O_LOCK);
                    CHECK_OPERATION(err == -1, fprintf(stderr, "Errore nella chiamata a openFile.\n"); 
                        free((char*)path);
                            int check = closedir(dir);
                                CHECK_OPERATION(check == -1, fprintf(stderr, "Errore nella closedir.\n"); return -1);
                                    return -1;);
                } else if(is_directory(path)){
                    int result = caller_open(path);
                    CHECK_OPERATION(result == -1, fprintf(stderr, "Errore nella caller.\n"); 
                        free((char*)path);
                            int check = closedir(dir);
                                CHECK_OPERATION(check == -1, fprintf(stderr, "Errore nella closedir.\n"); return -1);
                                    return -1;);
                }
            }
            free((char*)path);
        }
        int check = closedir(dir);
        CHECK_OPERATION(check == -1, fprintf(stderr, " errore nella closedir.\n"); return -1);
    } else if(is_regular_file(pathname)){
        /* Gestisce la richiesta */
        err = openFile(pathname, O_CREATE | O_LOCK);
        CHECK_OPERATION(err == -1, fprintf(stderr, " errore nella chiamata a openFile(pathname).\n"); return -1;);
    }
    return 0;
}

int caller_write(const char* pathname, const char *dirname){
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
                    err = writeFile(path, dirname);
                    CHECK_OPERATION(err == -1, fprintf(stderr, "Errore nella chiamata a writeFile.\n"); 
                        free((char*)path);
                        int check = closedir(dir);
                        CHECK_OPERATION(check == -1, fprintf(stderr, "Errore nella closedir.\n"); return -1);
                        return -1;);
                    CHECK_OPERATION(err == 808, 
                        size_t size;
                        void *buf;
                        int err_rbuf = read_from_file((char*)path, &buf, &size);
                        CHECK_OPERATION(err_rbuf == -1,
                            int err_close = closeFile(path);
                            CHECK_OPERATION(err_close == -1, free((char*)path); break);
                            int err_unlock = unlockFile(path);
                            CHECK_OPERATION(err_unlock == -1, free((char*)path); break);
                            free((char*)path);
                            break;);
                        
                        int err_append = appendToFile(path, buf, size, dirname);
                        CHECK_OPERATION(err_append == -1, 
                            int err_close = closeFile(path);
                            CHECK_OPERATION(err_close == -1, break);
                            int err_unlock = unlockFile(path);
                            CHECK_OPERATION(err_unlock == -1, break);
                            free((char*)path);
                            break;);
                    );
                } else if(is_directory(path)){
                    int result = caller_write(path, dirname);
                    CHECK_OPERATION(result == -1, fprintf(stderr, "Errore nella caller.\n"); 
                        free((char*)path);
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
        err = writeFile(pathname, dirname);
        CHECK_OPERATION(err == -1, fprintf(stderr, "Errore nella chiamata a writeFile.\n"); return -1;);
    }
    return 0;
}

int freed(int *byte_letti, int *byte_scritti, size_t size_path, char** path, void** old_file, size_t *size_old){
    
    errno = 0;
    *byte_letti += read_size(fd_skt, &size_path); 
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
        return -1);
                
    *path = malloc(size_path);
    CHECK_OPERATION(*path == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
        return -1);

    errno = 0;
    *byte_letti += read_msg(fd_skt, *path, size_path);
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
        return -1);
    printf("PATH DEL FILE: %s\n", *path);
                
    errno = 0;
    *byte_letti += read_size(fd_skt, size_old); 
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
                return -1);
    printf("SIZE DEL FILE LETTO: %ld\n", *size_old);
    
    *old_file = malloc(*size_old);
    CHECK_OPERATION(*old_file == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
            return -1);

    errno = 0;
    *byte_letti += read_msg(fd_skt, *old_file, *size_old);
    CHECK_OPERATION(errno == EFAULT,
        fprintf(stderr, "Non e' stato possibile leggere la risposta del server.\n"); 
                return -1);
    printf("DOPO AVER INSERITO IL BUFFER.\n");
                
    return 0;
}