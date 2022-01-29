#ifndef UTILS_H_
#define UTILS_H_
int is_regular_file(const char *path);
int is_directory(const char *path);
const char *nPath(const char* path, char *file_name);
int caller(const char *pathname);
#endif