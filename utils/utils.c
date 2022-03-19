/**
 * @file utils.c
 * @author Sara Grecu (s.grecu1@studenti.unipi.it)
 * @brief Contiene alcune funzioni utilizzate sia dal client che dal server
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

#include <utils.h>

int is_regular_file(const char *path) {
    struct stat path_stat;
    CHECK_OPERATION(stat(path, &path_stat)==-1, fprintf(stderr, " errore nella verifica sul file.\n"); return -1);

    return S_ISREG(path_stat.st_mode);
}

int is_directory(const char *path) {
    struct stat path_stat;
    CHECK_OPERATION(stat(path, &path_stat)==-1, fprintf(stderr, " errore nella verifica sulla directory.\n"); return -1);
    
    return S_ISDIR(path_stat.st_mode);
}


int save_on_disk(char *dirname, char* filename, void* buf, size_t size){ 
    CHECK_OPERATION(filename == NULL || size <= 0 || buf == NULL,
        fprintf(stderr, "Non posso memorizzare file nulli.\n"); 
            return -1);
    CHECK_OPERATION(dirname == NULL, return 0;);
    char* new_path;
    char *str1=NULL;
    char *pars1 = filename;
    char* token1 = strtok_r(pars1,  "/", &str1);


    while (token1) {
        new_path = token1;
        token1 = strtok_r(NULL, "/", &str1);
    }

    int len = strlen(dirname) + strlen(new_path) + strlen("/") + 1;
    char *path = malloc(sizeof(char)*len);
    CHECK_OPERATION(path == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return -1);
    path = strcpy(path, dirname);
    path = strcat(path, "/");
    path = strcat(path, new_path);

    /* Crea il file nella directory */
    FILE *new_file = fopen(path, "wb");
    CHECK_OPERATION(new_file == NULL, fprintf(stderr, "Errore nella fopen.\n"); return -1);
    
    /* Scrive sul file */
    size_t err_fwrite = fwrite(buf, size, 1, new_file); 
    CHECK_OPERATION(err_fwrite == -1, fprintf(stderr, "Errore nella fwrite.\n"); return -1);
    
    /* Chiude il file */
    int check = fclose(new_file);
    CHECK_OPERATION(check == -1, fprintf(stderr, "Errore nella fclose.\n"); return -1);

    free(path);

    return 0;
}

int caller(int (*fun) (const char*), const char* pathname){ 
    int err;
    if(is_directory(pathname)){
        DIR *dir = opendir(pathname);
        CHECK_OPERATION(dir == NULL, fprintf(stderr, " errore sulla opendir.\n"); return -1;);
        
        struct dirent *file;
        while((errno=0, file = readdir(dir))!=NULL && pathname != NULL){
            int len = strlen(pathname) + strlen(file->d_name) + strlen("/") + 1;
            const char *path = malloc(sizeof(char)*len);
            path = strcpy((char*)path, pathname);
            path = strcat((char*)path, "/");
            path = strcat((char*)path, file->d_name);

            if(strcmp(file->d_name, "..")!=0 && strcmp(file->d_name, ".")!=0){
                if(is_regular_file(path)){
                    err = fun(path);
                    CHECK_OPERATION(err == -1, fprintf(stderr, " errore nella chiamata a fun(pathname).\n"); 
                        free((char*)path);
                            int check = closedir(dir);
                                CHECK_OPERATION(check == -1, fprintf(stderr, " errore nella closedir.\n"); return -1);
                                    return -1;);
                } else if(is_directory(path)){
                    int result = caller(fun, path);
                    CHECK_OPERATION(result == -1, fprintf(stderr, " errore nella caller.\n"); 
                        free((char*)path);
                            int check = closedir(dir);
                                CHECK_OPERATION(check == -1, fprintf(stderr, " errore nella closedir.\n"); return -1);
                                    return -1;);
                }
            }
            free((char*)path);
        }
        int check = closedir(dir);
        CHECK_OPERATION(check == -1, fprintf(stderr, " errore nella closedir.\n"); return -1);
    } else if(is_regular_file(pathname)){
        /* Gestisce la richiesta */
        err = fun(pathname);
        CHECK_OPERATION(err == -1, fprintf(stderr, " errore nella chiamata a fun(pathname).\n"); return -1);

    }

    return 0;
}

int read_from_file(char *pathname, void** buf, size_t *size){
    CHECK_OPERATION(!pathname, fprintf(stderr, "Parametro non valido.\n"); return -1);

    /* Apre il file */
    FILE* file_toread = fopen(pathname, "rb");
    CHECK_OPERATION(file_toread == NULL, fprintf(stderr, "Non e' stato possibile aprire il file.\n"); return -1);
    struct stat st;
    stat(pathname, &st);
    *size = (st.st_size);
    *buf = malloc(*size);
    CHECK_OPERATION(*buf == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return -1);
   
    /* Legge il file */
    size_t err_fread = fread(*buf, *size, 1, file_toread);
    CHECK_OPERATION(err_fread == 0, fprintf(stderr, "Byte letti: %ld\nErrore nella lettura del file.\n", err_fread); return -1);

    /* Chiude il file */
    int err_close = fclose(file_toread);
    CHECK_OPERATION(err_close == -1, fprintf(stderr, "Errore nella chiusura del file.\n"); return -1);

    return 0;
}