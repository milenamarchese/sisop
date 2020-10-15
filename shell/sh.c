#include "defs.h"
#include "types.h"
#include "readline.h"
#include "runcmd.h"

#include <sys/wait.h>
#include <signal.h>

char promt[PRMTLEN] = {0};

static void pid_to_str(int pid, char* num) {
		int size = 0;
		int n = pid;
		while (n > 0) {
			n = n / 10;
			size++;
		}

		num[size] = '\n';
		while (pid > 0) {
			size--;
			int d = pid % 10;
			num[size] = '0' + d;
			pid = pid / 10;
		}
}

static void handler() {
	pid_t pid;
	char* buf = BACK_MSG;
	while ((pid = waitpid(0, NULL, WNOHANG)) > 0) {
		char num [PIDMAXLEN] = {0};
		pid_to_str(pid, num);
		if (write(WRITE, buf, strlen(buf)) == -1 || write(WRITE, num, strlen(num)) == -1) {
			perror("Error");
		}
	}
}

// runs a shell command
static void run_shell() {

	char* cmd;

	while ((cmd = read_line(promt)) != NULL) {
		if (run_cmd(cmd) == EXIT_SHELL)
			return;
	}
}

// initialize the shell
// with the "HOME" directory
static void init_shell() {

	char buf[BUFLEN] = {0};
	char* home = getenv("HOME");

	if (chdir(home) < 0) {
		snprintf(buf, sizeof buf, "cannot cd to %s ", home);
		perror(buf);
	} else {
		snprintf(promt, sizeof promt, "(%s)", home);
	}	
}

int main(void) {

	init_shell();

	stack_t ss;

	ss.ss_sp = malloc(SIGSTKSZ);
	if (ss.ss_sp == NULL) {
	    return 1;
	}

	ss.ss_size = SIGSTKSZ;
	ss.ss_flags = 0;
	if (sigaltstack(&ss, NULL) == -1) {
	    return 1;
	}

	struct  sigaction sa;
	sa.sa_flags = SA_ONSTACK | SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = handler;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
       return 1;
    }

	run_shell();

	free(ss.ss_sp);

	return 0;
}

