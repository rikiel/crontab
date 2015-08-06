/*
 * File: crontab.c
 *
 * Copyright (C) 2015 Richard Eliáš <richard@ba30.eu>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#include <errno.h>	// errno
#include <unistd.h>	// fork
#include <string.h>	// strerror
#include <stdlib.h>	// abort
#include <assert.h>	// assert
#include <sys/wait.h>	// waitpid

#include "crontab.h"
#include "conf.h"
#include "utils.h"

#define	CRON_SLEEP_TIME		60
#define	MAX_ERRORS		10


void
run_cron(const char *config_file)
{
	APP_DEBUG_FNAME;

	size_t iterations = 0;
	struct list *cfg = NULL;

	INFO("cron_daemon: START");

	while (1) {
		++iterations;
		DEBUG("cron iteration #%lu", iterations);

		if (read_config(config_file, &cfg)) {
			ERR("read_config('%s') failed", config_file);
			abort();
		}

		run_commands(cfg);
		delete_list(cfg);

		wait_children();

		DEBUG("SLEEPING");
		sleep(CRON_SLEEP_TIME);
	}
}

void
run_command(const char *command)
{
	APP_DEBUG_FNAME;

	int i;

	i = fork();
	switch (i) {
		case -1:
			ERR("fork: '%s'", strerror(errno));
			abort();
			break;
		case 0: // child
			INFO("FORK ok, RUN: /bin/bash -c '%s'",
					command);
			// execl("/bin/bash", "bash", "-c", command, NULL);
			// ^^ warning: missing sentinel in function
			// 	call [-Wformat=] on Solaris
			add_process_to_pgid();
			execl("/bin/bash", "bash", "-c", command, (char *)0);
			ERR("exec command failed with error '%s', aborting",
				strerror(errno));
			abort();
			break;
		default:
			INFO("PID %i created, RUN '%s'",
					i, command);
			break;
	}
}

void
run_commands(const struct list *cmd)
{
	APP_DEBUG_FNAME;

	time_t now;
	struct command *c;

	time(&now);
	now = now / 60 * 60;

	while (cmd) {
		c = (struct command *)cmd->item;

		// vv should be run in this minute
		if (abs(now - c->seconds) < CRON_SLEEP_TIME / 2)
			run_command(c->cmd);
		cmd = cmd->next;
	}
}

void
wait_children()
{
	APP_DEBUG_FNAME;

	int status;
	pid_t pid;
	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		assert(WIFEXITED(status));
		INFO("process %li exited with status %i",
				(int)pid, status);
	}
}
