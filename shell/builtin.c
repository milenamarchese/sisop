#include "builtin.h"
#include "utils.h"

// On error prints de error code,
// set the status to -1 and
// returns true
static int error() {
	perror("Error:");
	status = -1;
	return 1;
}

int exit_shell(char* cmd) {
	return strcmp(cmd, EXIT) == 0;
}

// As the prompt is between brackets
// modifies it updating the current directory
static int modify_prompt(char* promt) {
	if (!getcwd(&promt[1], PRMTLEN-1)) {
		return -1;
	}
	strcat(promt, ")");
	return 0;
}

// In case cd is invoked either 
// chdir fails or not returns true
// to avoid unwanted calls to exec_cmd
int cd(char* cmd) {

	char cmd_copy[BUFLEN] = {0};
	strcpy(cmd_copy, cmd);

	char* dir= split_line(cmd_copy, SPACE);

	if (strcmp(cmd_copy, CD) != 0) {
		return 0;
	} if (strlen(dir) == 0) {
		dir = "/home";
	} if (chdir(dir) == -1 || modify_prompt(promt) == -1) {
		return error();
	}

	status = 0;
	return 1;
}

// In case pwd is invoked either 
// getcwd fails or not returns true
// to avoid unwanted calls to exec_cmd
int pwd(char* cmd) {

	if (strcmp(cmd, PWD) != 0) {
		return 0;
	}

	char pwd[BUFLEN];
	if (!getcwd(pwd, BUFLEN)) {
		return error();
	}

	fprintf(stdout, "%s\n", pwd);
	status = 0;
	return 1;
}

