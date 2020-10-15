#include "xargs.h"
#include "error_utils.h"
#ifndef NARGS
#define NARGS 4
#endif
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>


void free_args(char** args, int count) {
	for (int i = 1; i < count + 1; ++i) {
		free(args[i]);
	}
}

void complete_args(int count, char** args) {
	for (int i = count; i < (NARGS + 1); ++i) {
		args[i + 1] = NULL;
	}
}

void execute(char** command) {
	pid_t pid;
	if ((pid = fork()) == -1) {
		return error();
	}

	if (pid == 0 && (execvp(command[0], command) == -1)) {
		return error();
	}
	wait(NULL);
}

void xargs(char* command) {
	char *line = NULL;
	size_t len = 0;
	ssize_t nread;
	char* args[NARGS + 2];
	args[0] = command;

	int count = 0;
	while ((nread = getline(&line, &len, stdin))) {
		if (nread != EOF) {
			line[nread - 1] = '\0';
			args[count + 1] = line;
			count += 1;
			line = NULL;
		} if (count == NARGS || nread == EOF) {
			complete_args(count, args);
			execute(args);
			free_args(args, NARGS);
			count = 0;
			if(nread == EOF) break;
		}
	}
	free(line);
}

int main(int argc, const char** argv) {
	if (argc != 2) return 1;

	char* command = argv[1] == NULL ? "echo" : (char*)argv[1];

	xargs(command);
	return 0;
}
