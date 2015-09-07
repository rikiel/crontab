/*
 * File: crontab.c
 *
 * Copyright (C) 2015 Richard Eliáš <richard.elias@matfyz.cz>
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


#include <unistd.h>
#include <stdlib.h>

#include "crontab.h"
#include "conf.h"
#include "logger.h"
#include "utils.h"


#define	CRON_SLEEP_TIME		60


void
run_cron(const char *filename)
{
	APP_DEBUG_FNAME;

	size_t iter = 0;
	struct list *cfg = NULL;
	size_t tosleep;

	INFO("START cron_daemon");

	while (1) {
		++iter;
		DEBUG("cron iteration #%lu", iter);

		cfg = read_config(filename);
		run_commands(cfg);
		delete_list(&cfg);
		wait_children();

		DEBUG("SLEEP cron_daemon");
		tosleep = CRON_SLEEP_TIME;
		while ((tosleep = sleep(tosleep)) != 0)
			;
	}
}

void
run_commands(const struct list *cmd)
{
	APP_DEBUG_FNAME;

	time_t now;
	struct command *c;

	time(&now);
	now = (now / 60) * 60;

	while (cmd) {
		c = cmd->item;

		if (abs(now - c->seconds) < CRON_SLEEP_TIME / 2)
			run_command(c->cmd);

		cmd = cmd->next;
	}
}

void
run_command(const char *command)
{
	APP_DEBUG_FNAME;

	pid_t p;

	p = fork();
	switch (p) {
		case -1:
			ERR("fork: '%s'", strerr());
			myabort();
			break;
		case 0:
			INFO("fork() OK, run() /bin/bash -c '%s'", command);

			execl("/bin/bash", "bash", "-c", command, (char *)NULL);

			ERR("exec(%s) failed: %s; aborting", command, strerr());
			myabort();
		default:
			INFO("pid %i created to run '%s'", (int)p, command);
			register_process(p);
			break;
	}
}
