#include <stdio.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/types.h>

#include <ngkh/subproc.h>

pid_t ngkh_subproc(char *const *argv, FILE **std_in, FILE **std_out)
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

	if (posix_spawnp(&pid, argv[0], &actions, NULL, argv, NULL) != 0) {
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

