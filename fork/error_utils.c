#include "error_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define READ 0
#define WRITE 1

void error (void) {	
	perror("Error:");
}

void close_fd(int* fd) {
	if (fd[READ] >= 0) close(fd[READ]);
	if (fd[WRITE] >= 0) close(fd[WRITE]);
}

void pipe_error(int* fd1, int* fd2) {
	error();
	if(fd1 != NULL) close_fd(fd1);
	if(fd2 != NULL) close_fd(fd2);
}