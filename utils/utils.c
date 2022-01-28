#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <fcntl.h>
#include <check_errors.h>
#include <stdlib.h>

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

const char *nPath(const char* path, char *file_name){
    if(!path || !file_name) {
        errno = EINVAL;
        return NULL;
    }
    const char *new_path = calloc((strlen(path)+strlen(file_name)+2), sizeof(char));
    CHECK_OPERATION((new_path==NULL), fprintf(stderr, "Allocazione della calloc non andata a buon fine.\n"); return NULL;);   

    new_path = strcat((char*)new_path, path);
    new_path = strcat((char*)new_path, "/");
    new_path = strcat((char*)new_path, file_name);

    return new_path;
}