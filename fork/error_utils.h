#ifndef ERROR_UTILS_H
#define ERROR_UTILS_H

/*Prints error message.*/
void error (void);

/*Closes the read and write file descriptors of a pipe if they are valid.*/
void close_fd (int* fd);

/*Prints error and closes one or two pairs of file descriptors.*/
void pipe_error (int* fd1, int* fd2);

#endif