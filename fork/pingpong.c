#include "pingpong.h"
#include "error_utils.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#define READ 0
#define WRITE 1


void pingpong(void) {
	int pipefd_1[2];
	int pipefd_2[2];

	if (pipe(pipefd_1) == -1) {
		return error();
	}
	if (pipe(pipefd_2) == -1) {
		close_fd(pipefd_1);
		return error();
	}

	pid_t pid;
	if((pid = fork()) == -1) {
		return pipe_error(pipefd_1, pipefd_2);
	}
	
	pid_t my_pid = getpid();
	pid_t parent_pid = getppid();

	if (pid > 0) {
		close(pipefd_2[READ]);
		printf("Hola, soy PID %i:\n"
			"- pipe me devuelve: [%i, %i]\n"
			"- pipe me devuelve: [%i, %i]\n\n", 
			my_pid, pipefd_1[0], pipefd_1[1], pipefd_2[0], pipefd_2[1]);
		fflush(stdout);

		srandom(time(NULL));
		int num = random();
		if(write(pipefd_2[WRITE], &num, sizeof(int)) < 0) {
			return pipe_error(pipefd_1, pipefd_2);
		}
		close(pipefd_2[WRITE]);

		printf("Donde fork me devuelve %i:\n" 
			"- getpid me devuelve: %i\n"
  			"- getppid me devuelve: %i\n"
  			"- random me devuelve: %i\n"
  			"- envío valor %i a través de fd=%i\n\n",
  			pid, my_pid, parent_pid, num, num, pipefd_2[WRITE]);
		fflush(stdout);

		int to_recv;
		if(read(pipefd_1[READ], &to_recv, sizeof(int)) < 0) {
			return pipe_error(pipefd_1, pipefd_2);
		}
		close(pipefd_1[READ]);

		printf("Hola, de nuevo PID %i:\n"
  			"- recibí valor %i vía fd=%i\n\n",
  			pid, to_recv, pipefd_1[READ]);
		fflush(stdout);
		
		close(pipefd_1[WRITE]);
	} else {
		close(pipefd_1[READ]);
		int to_send;
		if(read(pipefd_2[READ], &to_send, sizeof(int)) < 0) {
			return pipe_error(pipefd_1, pipefd_2);
		}
		close(pipefd_2[READ]);
		close(pipefd_2[WRITE]);

		printf("Donde fork me devuelve %i:\n"
  			"- getpid me devuelve: %i\n"
  			"- getppid me devuelve: %i\n"
  			"- recibo valor %i vía fd=%i\n"
  			"- reenvío valor en fd=%i y termino\n\n",
  			pid, my_pid, parent_pid, to_send, pipefd_2[READ], pipefd_1[WRITE]);
		fflush(stdout);
		
		if(write(pipefd_1[WRITE], &to_send, sizeof(int)) < 0) {
			return pipe_error(pipefd_1, pipefd_2);
		}
		close(pipefd_1[WRITE]);
	}
}

int main() {
	pingpong();
	return 0;
}