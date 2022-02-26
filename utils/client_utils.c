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

int caller_open(const char* pathname){
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
                    err = openFile(path, O_CREATE | O_LOCK);
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
        CHECK_OPERATION(check == -1, fprintf(stderr, "Errore nella closedir.\n"); return -1);
    } else if(is_regular_file(pathname)){
        /* Gestisce la richiesta */
        err = writeFile(pathname, dirname);
        CHECK_OPERATION(err == -1, fprintf(stderr, "Errore nella chiamata a writeFile.\n"); return -1;);
    }
    return 0;
}