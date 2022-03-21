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
    printf("PATH: %s\n", path);
    /* Crea il file nella directory */
    FILE *new_file = fopen(path, "wb");
    CHECK_OPERATION(new_file == NULL, fprintf(stderr, "Errore nella fopen.\n"); free(path); return -1);
    
    /* Scrive sul file */
    size_t err_fwrite = fwrite(buf, size, 1, new_file); 
    CHECK_OPERATION(err_fwrite == -1, fprintf(stderr, "Errore nella fwrite.\n"); free(path); return -1);
    
    /* Chiude il file */
    int check = fclose(new_file);
    CHECK_OPERATION(check == -1, fprintf(stderr, "Errore nella fclose.\n"); free(path); return -1);

    free(path);

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

int max(int a, int b){
    if(a>b)
        return a;
    else
        return b;
}

float to_Mbytes(int bytes){
    return (bytes/1000000.0);
}