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
            int len = strlen(pathname) + 1;
            char *new_path = malloc(sizeof(char)*len);
            new_path = strcat(new_path, pathname);
            new_path = strcat(new_path, "/");
            new_path = strcat(new_path, file->d_name);
            new_path[len] = '\0';
            if((strcmp(file->d_name, "..")!=0 && strcmp(file->d_name, ".")!=0) && is_directory(new_path)){
                caller_open(new_path);                
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
            int len = strlen(pathname) + strlen(directory) + 1;
            char *new_path = malloc(sizeof(char)*len);
            new_path = strcat(new_path, pathname);
            new_path = strcat(new_path, "/");
            new_path = strcat(new_path, file->d_name);
            new_path[len] = '\0';
            if((strcmp(file->d_name, "..")!=0 && strcmp(file->d_name, ".")!=0) && is_directory(new_path)){
                caller_write(new_path, directory);                
            }
        }
        int check = closedir(dir);
        CHECK_OPERATION((check==-1), fprintf(stderr, " errore sulla closedir.\n"); return -1;);
    }
    return 0;
}

int save_on_disk(char *dirname, const char* path, char* buf, size_t size){ 
    char *str = NULL;
    char *pars = (char*) path;
    char* token = strtok_r(pars, "/", &str); //TODO:si puo' fare in maniera piu' elegante?
    char* element;

    while(token) {
        element = token;
        token = strtok_r(NULL, "/", &str);
	}
    
    int len = strlen(dirname) + strlen(element) + 1;
    char *new_path = malloc(sizeof(char)*len);
    new_path = strcat(new_path, dirname);
    new_path = strcat(new_path, "/");
    new_path = strcat(new_path, element);
    new_path[len] = '\0';

    DIR *dir = opendir(dirname);
    CHECK_OPERATION(dir == NULL, 
        fprintf(stderr, " errore sulla opendir.\n"); 
            return -1;);
            
    FILE *new_file = fopen(new_path, "w");
    CHECK_OPERATION(new_file == NULL, 
        fprintf(stderr, " errore sulla fopen.\n"); 
            return -1;);

    int err_fwrite = fwrite(buf, size, 1, new_file);
    CHECK_OPERATION(err_fwrite == -1, 
        fprintf(stderr, " errore sulla fwrite.\n"); 
            return -1;);

    int close_f = fclose(new_file);
    CHECK_OPERATION(close_f == -1, 
        fprintf(stderr, " errore sulla fclose.\n"); 
            return -1;);

    int check = closedir(dir);
    CHECK_OPERATION((check==-1), fprintf(stderr, " errore sulla closedir.\n"); return -1;);

    return 0;
}