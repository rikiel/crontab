/*
 * File: utils.c
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


#include <string.h>	// strlen
#include <strings.h>	// bzero
#include <ctype.h>	// isspace
#include <stdlib.h>	// exit
#include <stdio.h>	// fclose
#include <getopt.h>	// long_opts
#include <errno.h>	// errno
#include <unistd.h>	// fork
#include <signal.h>	// kill
#include <assert.h>	// assert

#include "utils.h"

pid_t pgid;

void
trim(char *str)
{
	char *beg;
	char *end;

	if (str == NULL)
		return;

	beg = str;
	end = str + strlen(str);
	while (beg != end && isspace(*beg))
		++beg;
	while (beg != end && (*end == '\0' || isspace(*end)))
		--end;
	if (beg != end || *end != '\0')
		end[1] = '\0';
	for (; (*str = *beg); ++str, ++beg);
	// ^^ own strcpy, default do not work.. (ex: " /bin/bash")
}

void
swap_ptr(void **p1, void **p2)
{
	void *p = *p2;
	*p2 = *p1;
	*p1 = p;
}

void
delete_list(struct list *l)
{
	// APP_DEBUG_FNAME;

	struct list *next;
	while (l) {
		next = l->next;
		free(l);
		l = next;
	}
}

const char *
time_to_string(time_t t)
{
#define	TIME_FORMAT	"DD.MM.YYYY HH:MM:SS"
	static char buff[STR_LENGTH(TIME_FORMAT)];
	struct tm date;

	localtime_r(&t, &date);
	strftime(buff, STR_LENGTH(TIME_FORMAT), "%d.%m.%Y %H:%M:%S", &date);

	return (buff);
}

int
handle_args(int argc, char **argv)
{
	APP_DEBUG_FNAME;

	int ch;
	struct option long_opts[] = {
		{"help", no_argument, NULL, 'h'},
		{"debug", no_argument, NULL, 'd'},
		{"log-to", required_argument, NULL, 'l'},
		{NULL, 0, NULL, 0}
	};

	while ((ch = getopt_long(argc, argv, "hdl:", long_opts, NULL)) != -1) {
		switch (ch) {
			case 'h':
				usage();
				exit(0);
				break;
			case 'd':
				log_set_priority(debug);
				break;
			case 'l':
				log_to_file(optarg);
				break;
			default:
				ERR("wrong parameter '%s'", optarg);
				usage();
				exit(1);
				break;
		}
	}
	fclose(stdin); // we do not want input in cron/children

	return (optind);
}

void
set_pgid()
{
	APP_DEBUG_FNAME;

	pgid = getpgid(0);
}

void
add_process_to_pgid()
{
	APP_DEBUG_FNAME;

	setpgid(getpid(), pgid);
}

void
exit_handler()
{
	APP_DEBUG_FNAME;

	assert(pgid > 0);

	WARN("killing all subprocessess for pgid '%i'", (int)pgid);
	destroy_logger();
	kill(-pgid, SIGTERM);	// kills me too..
}

void
set_exit_handler()
{
	APP_DEBUG_FNAME;

	struct sigaction act;
	bzero(&act, sizeof (struct sigaction));

	sigemptyset(&act.sa_mask);
	act.sa_handler = exit_handler;
	act.sa_flags = 0;
	if (sigaction(SIGSEGV, &act, NULL) != 0	||
			sigaction(SIGINT, &act, NULL) != 0) {
		WARN("sigactions was not set, err=%s", strerror(errno));
		errno = 0;
	}

	atexit(exit_handler);
}

#ifdef __sun

#include <assert.h>

int
getline(char **line_ptr, size_t *line_size, FILE *stream)
{
	APP_DEBUG_FNAME;

	int ch;
	char *ptr = *line_ptr;
	int n = 0;

	while ((ch = getc(stream)) != EOF && ch != '\n') {
		if (n == *line_size)
			break;
		*ptr++ = ch;
		++n;
	}
	assert(*line_size != n + 1);
	ptr[1] = '\0';

	if (ch == EOF && n == 0)
		return (-1);
	else
		return (n + 1);
}

#endif
