#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <ngkh/subproc.h>

#include "subproc_local.h"

//===== ngkh_subproc() =====
pid_t ngkh_subproc(const char *exec_path, char *const *argv, FILE **std_in, FILE **std_out)
{
	enum { read = 0, write = 1 };

	posix_spawn_file_actions_t actions;
	pid_t                      pid = -1;

	int pipe_p2c[2], pipe_c2p[2];

	posix_spawn_file_actions_init(&actions);

	if (std_in) {
		if (pipe(pipe_p2c)) {
			fprintf(stderr, "Error: Fail to create pipe to child proc.\n");
			return pid;
		}

		posix_spawn_file_actions_addclose(&actions, pipe_p2c[write]);
		posix_spawn_file_actions_adddup2(&actions, pipe_p2c[read], STDIN_FILENO);
		posix_spawn_file_actions_addclose(&actions, pipe_p2c[read]);
	}

	if (std_out) {
		if (pipe(pipe_c2p)) {
			fprintf(stderr, "Error: Fail to create pipe from child proc.\n");

			if (std_in) {
				for (size_t i = 0; i < 2; i++)
					close(pipe_p2c[i]);
			}

			return pid;
		}

		posix_spawn_file_actions_addclose(&actions, pipe_c2p[read]);
		posix_spawn_file_actions_adddup2(&actions, pipe_c2p[write], STDOUT_FILENO);
		posix_spawn_file_actions_addclose(&actions, pipe_c2p[write]);
	}

	if (posix_spawn(&pid, exec_path, &actions, NULL, argv, NULL) != 0) {
		fprintf(stderr, "Error: Fail to spawn process.\n");
		pid = -1;

		return pid;
	}

	// For parent

	if (std_in)
		close(pipe_p2c[read]);
	if (std_out)
		close(pipe_c2p[write]);

	*std_in  = fdopen(pipe_p2c[write], "w");
	*std_out = fdopen(pipe_c2p[read],  "r");

	return pid;
}

//===== ngkh_find_exec_path() =====
char *ngkh_subproc_find_exec_path(const char *exec_name)
{
	char *env_path = getenv("PATH");
	char *path_dir;
	char abs_path[PATH_MAX+1];

	if ((env_path == NULL) || (env_path[0] == '\0')) {
		fprintf(stderr, "Error: The environment variable of \"PATH\" is not set.\n");
		return NULL;
	}

	path_dir = strtok(env_path, ":");
	if (ngkh_check_exist_executable(path_dir, exec_name, abs_path))
		return realpath(abs_path, NULL);

	while ((path_dir = strtok(NULL, ":")) != NULL) {
		if (ngkh_check_exist_executable(path_dir, exec_name, abs_path))
			return realpath(abs_path, NULL);
	}

	fprintf(stderr, "Error: The executable of \"%s\" cannot be found.\n", exec_name);

	return NULL; // if not found
}

bool ngkh_check_exist_executable(const char *dir, const char *exec_name, char *abs_path)
{
	char        _abs_path[PATH_MAX+1];
	struct stat st;

	if ((strlen(dir) + strlen(exec_name)) > PATH_MAX) {
		fprintf(stderr, "Error: The path of executable is too long.\n");
		return false;
	}

	sprintf(_abs_path, "%s/%s", dir, exec_name);
	if (!realpath(_abs_path, abs_path))
		return false;

	if (stat(abs_path, &st) == -1) {
		fprintf(stderr, "Error: Something occurred.\n");
		return false;
	}

	if (!S_ISREG(st.st_mode))
		return false;

	return true;
}

