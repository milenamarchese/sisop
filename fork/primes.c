#include "primes.h"
#include "error_utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#define READ 0
#define WRITE 1


void filter(int fd) {
	int prime;
	if (read(fd, &prime, sizeof(int)) < 1) {
		close(fd);
		return;
	}

	int fds[2];
	if (pipe(fds) == -1) {
		return error();
	} 

	printf("primo %i\n", prime);
	fflush(stdout);

	pid_t pid = fork();

	if (pid == 0) {
		close(fds[WRITE]);
		close(fd);
		filter(fds[READ]);
		filter(fds[READ]);
		return;
	}

	int next;
	while(read(fd, &next, sizeof(int)) > 0) {
		if(next % prime != 0) {
			if (write(fds[WRITE], &next, sizeof(int)) == -1) {
				return pipe_error(fds, NULL);
			}
		}
	}
	close(fds[READ]);
	close(fds[WRITE]);
	wait(NULL);
}

void primes(int n) {
	int fds[2];
	if (pipe(fds) == -1) return;

	pid_t pid;
	if((pid = fork()) == -1) {
		return pipe_error(fds, NULL);
	}

	if (pid == 0) {
		close(fds[WRITE]);
		filter(fds[READ]);
		close(fds[READ]);
		return;
	}

	close(fds[READ]);
	for (int i = 2; i < n; ++i) {
		if (write(fds[WRITE], &i, sizeof(int)) == -1) {
			return pipe_error(fds, NULL);
		} 
	}
	close(fds[WRITE]);
	wait(NULL);
}


int main(int argc, const char** argv) {
	if (argc != 2) return 1;

	int n = atoi(argv[argc - 1]);

	if (n < 2) return 1;

	primes(n);
	return 0;
}
