#ifndef DEFS_H
#define DEFS_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

// color scape strings
#define COLOR_BLUE "\x1b[34m"
#define COLOR_RED "\x1b[31m"
#define COLOR_RESET "\x1b[0m"

#define END_STRING '\0'
#define END_LINE '\n'
#define SPACE ' '
#define EMPTY ""
#define CD "cd"
#define PWD "pwd"
#define EXIT "exit"
#define BACK_MSG "==> terminado: PID="

#define BUFLEN 1024
#define PRMTLEN 1024
#define MAXARGS 20
#define ARGSIZE 1024
#define FNAMESIZE 1024
#define PIDMAXLEN 10

// Command representation after parsed
#define EXEC 1
#define BACK 2
#define REDIR 3
#define PIPE 4

// Fd for pipes
#define READ 0
#define WRITE 1
#define STERR 2

#define EXIT_SHELL 1


#endif //DEFS_H
