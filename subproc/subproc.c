#include <stdio.h>
#include <stdlib.h>
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
			*std_in = NULL;

			if (std_out) *std_out = NULL;

			return pid;
		}

		posix_spawn_file_actions_addclose(&actions, pipe_p2c[write]);
		posix_spawn_file_actions_adddup2(&actions, pipe_p2c[read], STDIN_FILENO);
		posix_spawn_file_actions_addclose(&actions, pipe_p2c[read]);
	}

	if (std_out) {
		if (pipe(pipe_c2p)) {
			fprintf(stderr, "Error: Fail to create pipe from child proc.\n");
			*std_out = NULL;

			if (std_in) {
				for (size_t i = 0; i < 2; i++)
					close(pipe_p2c[i]);

				*std_in = NULL;
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

		if (std_in) {
			for (size_t i = 0; i < 2; i++)
				close(pipe_p2c[i]);
			*std_in  = NULL;
		}

		if (std_out) {
			for (size_t i = 0; i < 2; i++)
				close(pipe_c2p[i]);
			*std_out = NULL;
		}

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
	const char    *_env_path  = getenv("PATH");
	      char    *env_path   = NULL;
	      size_t   num_paths  = 1, max_length_tmp;
	      size_t   max_length = 0;
	      char   **paths      = NULL;

	      char   *abs_path  = NULL;
	      char   *work_buf  = NULL;
	      size_t  buf_size;

	if ((_env_path == NULL) || (_env_path[0] == '\0')) {
		fprintf(stderr, "Error: The environment variable of \"PATH\" is not set.\n");
		return NULL;
	}

	env_path = strdup(_env_path);

	// Get max length of path string
	for (size_t i = 0; env_path[i] != '\0'; i++) {
		if (env_path[i] == ':')
			num_paths++;
	}
	paths = (char**)calloc(num_paths, sizeof (char*));

	paths[0] = strtok(env_path, ":");
	max_length = strlen(paths[0]);

	for (size_t i = 1; i < num_paths; i++) {
		paths[i] = strtok(NULL, ":");
		max_length_tmp = strlen(paths[i]);

		if (max_length_tmp > max_length)
			max_length = max_length_tmp;
	}

	// Check executable
	buf_size = max_length + strlen(exec_name) + 2; // "+2" means '/' and '\0'
	work_buf = (char*)calloc(buf_size, sizeof (char));

	for (size_t i = 0; i < num_paths; i++) {
		if ((abs_path = ngkh_check_exist_executable(paths[i], exec_name, work_buf, buf_size)) != NULL) {
			free(paths);
			free(work_buf);
			free(env_path);
			return abs_path;
		}
	}

	free(paths);
	free(work_buf);
	free(env_path);
	fprintf(stderr, "Error: The executable of \"%s\" cannot be found.\n", exec_name);

	return NULL; // This means the executable is not found.
}

char *ngkh_check_exist_executable(const char *dir, const char *exec_name, char *work_buf, size_t buf_size)
{
	struct stat  st;
	char        *abs_path  = NULL;
	char        *_work_buf = work_buf;
	size_t       _buf_size = ((work_buf) ? buf_size : PATH_MAX + 1);

	if ((strlen(dir) + strlen(exec_name) + 1) > (_buf_size - 1)) {
		fprintf(stderr, "Error: The path of executable is too long.\n");
		return NULL;
	}

	if (!work_buf)
		_work_buf = (char*)calloc(_buf_size, sizeof (char));

	sprintf(_work_buf, "%s/%s", dir, exec_name);
	if ((abs_path = realpath(_work_buf, NULL)) == NULL) {
		if (!work_buf)
			free(_work_buf);

		return NULL;
	}

	if (!work_buf)
		free(_work_buf);

	if (stat(abs_path, &st) == -1) {
		fprintf(stderr, "Error: Something occurred.\n");
		free(abs_path);
		abs_path = NULL;
		return NULL;
	}

	if (!S_ISREG(st.st_mode)) {
		free(abs_path);
		abs_path = NULL;
		return NULL;
	}

	return abs_path;
}

