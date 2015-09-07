/*
 * File: utils.c
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

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>

#include "utils.h"

struct list *pids = NULL;

void
delete_list(struct list **lptr)
{
	struct list *next;
	struct list *l;

	if (lptr == NULL)
		return;

	l = *lptr;
	while (l != NULL) {
		next = l->next;
		free(l);
		l = next;
	}
	*lptr = NULL;
}

void
swap_ptr(void **p1, void **p2)
{
	void *p;

	p = *p1;
	*p1 = *p2;
	*p2 = p;
}

const char *
strerr()
{
	const char *ptr = strerror(errno);
	errno = 0;
	return (ptr);
}

const char *
time_to_string(time_t t)
{
#define	TIME_FORMAT	"DD.MM.YYYY HH:MM:SS"
#define	TIME_STR_LEN	(sizeof (TIME_FORMAT))

	static char buff[TIME_STR_LEN];
	struct tm date;

	localtime_r(&t, &date);
	strftime(buff, TIME_STR_LEN, "%d.%m.%Y %H:%M:%S", &date);

	return (buff);
}

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
	while (*beg != '\0')
		*str++ = *beg++;
	*str = '\0';
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
				myexit(EXIT_SUCCESS);
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
				myabort();
				break;
		}
	}
	fclose(stdin); // we do not want input in cron/children

	return (optind);
}

void
register_process(pid_t pid)
{
	APP_DEBUG_FNAME;

	struct list *l;

	l = malloc(sizeof (struct list));
	l->item = malloc(sizeof (pid_t));
	memcpy(l->item, &pid, sizeof (pid_t));
	l->next = pids;

	pids = l;
}

void
kill_processess()
{
	APP_DEBUG_FNAME;
	WARN("killing all forked processess");
	pid_t *p;
	while (pids != NULL) {
		p = pids->item;
		INFO("killing pid %i", (int)*p);
		kill(*p, SIGTERM);
		pids = pids->next;
	}
	delete_list(&pids);
	wait_children();
}

void
signal_handler(int sig)
{
	APP_DEBUG_FNAME;
	DEBUG("catched signal %i", sig);

	if (sig == SIGUSR1)
		myexit(EXIT_SUCCESS);
	else {
		kill_processess();
		myabort();
	}
}

void
wait_children()
{
	APP_DEBUG_FNAME;

	int status;
	pid_t p;

	while ((p = waitpid(-1, &status, WNOHANG)) > 0)
		INFO("process %i exited with status %i",
				(int)p, (int)status);
}

void
myexit(int ret)
{
	APP_DEBUG_FNAME;

	DEBUG("myexit(%i)", ret);
	destroy_logger();
	exit(ret);
}

void
myabort()
{
	APP_DEBUG_FNAME;

	myexit(EXIT_FAILURE);
}

void
set_signal_handler()
{
	APP_DEBUG_FNAME;

	struct sigaction act;
	bzero(&act, sizeof (struct sigaction));

	sigemptyset(&act.sa_mask);
	act.sa_handler = signal_handler;
	act.sa_flags = 0;
	if (sigaction(SIGSEGV, &act, NULL) != 0	||
			sigaction(SIGINT, &act, NULL) != 0 ||
			sigaction(SIGTERM, &act, NULL) != 0 ||
			sigaction(SIGUSR1, &act, NULL) != 0)
		WARN("sigactions was not set, err=%s", strerr());
}


#ifdef __sun

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
	if (*line_size == n + 1) {
		ERR("getline");
		myabort();
	}
	ptr[1] = '\0';

	if (ch == EOF && n == 0)
		return (-1);
	else
		return (n + 1);
}

#endif
