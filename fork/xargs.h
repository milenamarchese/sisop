#ifndef FIND_H
#define FIND_H


/*Frees dinamically allocated memory in args.*/
void free_args(char** args, int count);

/*If the amount of lines isn't divisible by NARGS completes args with NULL.*/
void complete_args(int count, char** args);

void execute(char** command);

void xargs(char* command);

#endif