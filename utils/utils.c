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

int caller_open(const char *pathname){
    if(is_regular_file(pathname)){
        int err_open = openFile(pathname, O_CREATE | O_LOCK);
        CHECK_OPERATION(err_open == -1, 
            fprintf(stderr, " errore nella openFile.\n");
                return -1);
    } else if(is_directory(pathname)){
        DIR *dir = opendir(pathname);
        CHECK_OPERATION(dir == NULL, 
            fprintf(stderr, " errore sulla opendir.\n"); 
                return -1;);
        
        struct dirent *file;
        while((errno=0, file = readdir(dir))!=NULL && pathname != NULL){
            const char *reg_pat = nPath(pathname, file->d_name);
            CHECK_OPERATION(reg_pat==NULL,
                int check = closedir(dir); 
                    CHECK_OPERATION(check==-1, 
                        fprintf(stderr, "Errore nella chiusura della directory.\n"); 
                            return -1;) 
                        return -1); 
            if((strcmp(file->d_name, "..")!=0 && strcmp(file->d_name, ".")!=0) && is_directory(reg_pat)){
                caller(reg_pat);                
            }
        }
        int check = closedir(dir);
        CHECK_OPERATION((check==-1), fprintf(stderr, " errore sulla closedir.\n"); return -1;);
    }
    return 0;
}

int caller_write(const char *pathname, char* directory){
    if(is_regular_file(pathname)){
        int err_open = writeFile(pathname, directory);
        CHECK_OPERATION(err_open == -1, 
            fprintf(stderr, " errore nella openFile.\n");
                return -1);
    } else if(is_directory(pathname)){
        DIR *dir = opendir(pathname);
        CHECK_OPERATION(dir == NULL, 
            fprintf(stderr, " errore sulla opendir.\n"); 
                return -1;);
        
        struct dirent *file;
        while((errno=0, file = readdir(dir))!=NULL && pathname != NULL){
            const char *reg_pat = nPath(pathname, file->d_name);
            CHECK_OPERATION(reg_pat==NULL,
                int check = closedir(dir); 
                    CHECK_OPERATION(check==-1, 
                        fprintf(stderr, "Errore nella chiusura della directory.\n"); 
                            return -1;) 
                        return -1); 
            if((strcmp(file->d_name, "..")!=0 && strcmp(file->d_name, ".")!=0) && is_directory(reg_pat)){
                caller(reg_pat);                
            }
        }
        int check = closedir(dir);
        CHECK_OPERATION((check==-1), fprintf(stderr, " errore sulla closedir.\n"); return -1;);
    }
    return 0;
}