#include "exec.h"
#include <sys/types.h>
#include <unistd.h>

// sets the "key" argument with the key part of
// the "arg" argument and null-terminates it
static void get_environ_key(char* arg, char* key) {

	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets the "value" argument with the value part of
// the "arg" argument and null-terminates it
static void get_environ_value(char* arg, char* value, int idx) {

	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// On error print error message and 
// exits
static void error(void) {
	perror("Error");
	_exit(-1);
}

static int set_environ_vars(char** eargv, int eargc) {

	for (int i = 0; i < eargc ; ++i) {
		char* current = eargv[i];
		char key[BUFLEN];
		char value[BUFLEN];
		get_environ_key(current, key);
		get_environ_value(current, value, block_contains(current, '='));
		if (setenv(key, value, 1) == -1) {
			return -1;
		}
	}

	return 0;

} 

static int open_redir_fd(char* file, int flags) {
	
	mode_t mode = S_IWUSR | S_IRUSR;
	int mode_flags;

	if (flags == READ) {
		mode_flags = O_CLOEXEC;
	} else {
		mode_flags = O_CREAT | O_WRONLY | O_TRUNC | O_CLOEXEC;
	}

	return open(file, mode_flags, mode);
}

// Opens a file in case the value of fd is invalid
// and redirects redir_fd to fd.
static void redirect_file(char* file, int redir_fd, int mode, int* fd) {

	if ((*fd) <= 0 && (*fd = open_redir_fd(file, mode)) == -1) {
			error();
	}

	if (dup2(*fd, redir_fd) == -1) {
		error();
	}

}

// Prints error message and closes all
// file descriptors in case they are open.
static void pipeline_error(int* fds, int fd) {
	perror("Error");
	if (fds[READ] >= 0) close(fds[READ]);
	if (fds[WRITE] >= 0) close(fds[WRITE]);	
	if (fd >= 0) close(fd);	
}

// Executes recursive pipeline redirection,
static void exec_pipeline(int fd, int i, struct cmd** commands) {
	if (commands[i] == NULL) {
		return;
	}

	int fds[2] = {-1};
	if (commands[i + 1] != NULL) {
		if (pipe(fds) == -1) {
			pipeline_error(fds, fd);
		}
	}

	pid_t pid;
	if ((pid = fork()) == -1) {
		pipeline_error(fds, fd);
	}

	if (pid == 0) {

		if (setpgid(0, getpid()) == -1) {
			pipeline_error(fds, fd);
			_exit(-1);
		}

		if (i != 0) { 					// the first command doesn't read
			if (dup2(fd, READ) == -1) {
				pipeline_error(fds, fd);
				exit(-1);
			}
			close(fd);
		}
		
		if (commands[i + 1] != NULL) { 	// every command except the last one dups when writing
			if (dup2(fds[WRITE], WRITE) == -1) {
				pipeline_error(fds, fd);
				exit(-1);
			}
			close(fds[WRITE]);						
			close(fds[READ]);
		}

		exec_cmd(commands[i]);
	}

	if (i != 0) {
		close(fd);
	}
	if (commands[i + 1] != NULL) {
		close(fds[WRITE]);
	}

	waitpid(pid, NULL, 0);
	exec_pipeline(fds[READ], i + 1, commands);
}


void exec_cmd(struct cmd* cmd) {

	struct execcmd* e;
	struct backcmd* b;
	struct execcmd* r;
	struct pipecmd* p;

	switch (cmd->type) {

		case EXEC: {
			e = (struct execcmd*) cmd;
			if (set_environ_vars(e->eargv, e->eargc) == -1) {
			 	error();
			}

			if (execvp(e->argv[0], e->argv) == -1) {
				error();
			}
			break;
		}
		
		case BACK: {
			b = (struct backcmd*) cmd;
			exec_cmd(b->c);
			break;
		}

		case REDIR: {

			r = (struct execcmd*) cmd;

			int fd_out = -1, fd_err = -1, fd_in = -1;

			if (strlen(r->out_file) !=0) {
				redirect_file(r->out_file,WRITE, WRITE, &fd_out);
			} if (strlen(r->in_file)) {
				redirect_file(r->in_file, READ, READ, &fd_in);
			} if (strlen(r->err_file) !=0) {
				if (block_contains(r->err_file, '&') == 0) {
					fd_err = fd_out;
				} 
				redirect_file(r->err_file, STERR, WRITE, &fd_err);
			}

			r->type = EXEC;
			exec_cmd((struct cmd*) r);
			break;
		}
		
		case PIPE: {

			p = (struct pipecmd*) cmd;
		
			exec_pipeline(-1, 0, p->commands);

			free_command(parsed_pipe);
			_exit(-1);
			break;
		}
	}
}


