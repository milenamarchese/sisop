#ifndef FIND_H
#define FIND_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <stdbool.h> 
#include <string.h>
#include <fcntl.h>

/*Opens a subdirectory in new_dir.*/
int open_directory(DIR* cwd, DIR** new_dir, const char* file_name);

/*Recursive part of find.*/
void _find(DIR* cwd, const char* name, char* path, size_t len, char* (*str_search)(const char*, const char*));

/*Wrapper for _find, initializes variables and selects string search function.*/
void find(bool case_s, const char* name);

#endif