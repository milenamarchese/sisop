#include "find.h"
#include "error_utils.h"
#define FLAG "-i"


int open_directory(DIR* cwd, DIR** new_dir, const char* file_name) {
	int local_fd;
	int fd;
	if ((local_fd = dirfd(cwd)) == -1) {
		error();
		return -1;
	} if ((fd = openat(local_fd, file_name, O_DIRECTORY)) == -1) {
		close(local_fd);
		error();
		return -1;
	} if ((*new_dir = fdopendir(fd)) == NULL) {
		close(local_fd);
		close(fd);
		error();
		return -1;
	}
	return fd;
}

void _find(DIR* cwd, const char* name, char* path, size_t len, char* (*str_search)(const char *, const char *)) {
	struct dirent* dp;
	while ((dp = readdir(cwd)) != NULL) {
		const char* file_name = dp->d_name;
		if (strcmp(file_name, ".") == 0 || strcmp(file_name, "..") == 0) {
			continue;
		} if (str_search(file_name, name) != NULL) {
			printf("%.*s%s\n", (int)len, path, file_name);
		} if (dp->d_type == DT_DIR) {
			DIR* dir = NULL;
			int fd;
			if ((fd =open_directory(cwd, &dir, file_name)) == -1) return;
			strcpy(&path[len],file_name);
			size_t new_len = strlen(path);
			path[new_len] = '/';
			_find(dir, name, path, new_len + 1, (*str_search));
			closedir(dir);
			close(fd);
		}
	}
}

void find(bool case_s, const char* name) {
	DIR* cwd = opendir(".");
	if (cwd == NULL) {
		return error();
	}
	char path[PATH_MAX];
	char* (*str_search)(const char*, const char*) = case_s ? strcasestr : strstr;
	_find(cwd, name, path, 0, str_search);
	closedir(cwd);
}

int main(int argc, const char** argv) {
	bool case_sensitive;
	if (argc == 3) {
		const char* flag = argv[argc - 2];
		if (strcmp(flag, FLAG) != 0)
			return 1;
		case_sensitive = true;
	} else if (argc == 2) {
		case_sensitive = false;
	} else {
		return 1;
	}
	const char* name = argv[argc - 1];
	find(case_sensitive, name);
	return 0;
}
