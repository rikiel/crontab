/*
 * File: logger.c
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


#include <stdio.h>
#include <stdlib.h>	// abort
#include <stdarg.h>	// variadic functions
#include <sys/time.h>	// gettimeofday
#include <unistd.h>	// getpid
#include <assert.h>	// assert
#include <pthread.h>	// mutex

#include "logger.h"
#include "conf.h"


pthread_mutex_t mutex;

struct logger logger;

void
lock_m()
{
	if (pthread_mutex_lock(&mutex) != 0)
		abort();
}
void
unlock_m()
{
	if (pthread_mutex_unlock(&mutex) != 0)
		abort();
}

void
init_logger()
{
	APP_DEBUG_FNAME;

	logger.p = info;
	logger.files = malloc(sizeof (struct list));
	logger.files->next = NULL;
	logger.files->item = stdout;

	pthread_mutex_init(&mutex, NULL);
}

void
destroy_logger()
{
	APP_DEBUG_FNAME;

	struct list *l;
	l = logger.files;

	while (l != NULL) {
		fclose(l->item);

		l = l->next;
	}

	logger.files = NULL;
	pthread_mutex_destroy(&mutex);
}

// sets logger priority for output
void
log_set_priority(enum priority p)
{
	logger.p = p;
}

// add file for log messages
void
log_to_file(const char *filename)
{
	APP_DEBUG_FNAME;

	FILE *f;
	struct list *l;
	f = fopen(filename, "w");
	if (f == NULL || ferror(f)) {
		ERR("fopen log file '%s' failed, continue", filename);
		return;
	}
	l = logger.files;
	while (l->next != NULL)
		l = l->next;
	l->next = malloc(sizeof (struct list));
	l = l->next;
	l->next = NULL;
	l->item = f;
}

const char *
priority_to_string(enum priority p)
{
	const char *out;

	switch (p) {
		case debug:
			out = "DEBUG";
			break;
		case info:
			out = "INFO";
			break;
		case warn:
			out = "WARN";
			break;
		case error:
			out = "ERROR";
			break;
		default:
			abort();
	}
	return (out);
}

int
can_log(enum priority p)
{
	static enum priority vec[4] = {debug, info, warn, error};
	int i, j;

	i = j = 0;

	for (; ; ++i)
		if (vec[i] == logger.p)
			break;
	for (; ; ++j)
		if (vec[j] == p)
			break;

	return (i <= j);
}

// prints time, priority, pids to streams
int
print_init_message(enum priority p)
{
	// display 2 (/5) numbers
#define	PRINT_PATTERN	"%02i:%02i:%02i:%05i\t[%s]\t<%i>\t"
	struct list *l;
	time_t t;
	struct tm date;
	struct timeval us;

	if (!can_log(p))
		return (0);

	time(&t);
	localtime_r(&t, &date);
	gettimeofday(&us, NULL);

	lock_m();

	l = logger.files;
	while (l != NULL) {
		fprintf(l->item,
			PRINT_PATTERN,
			date.tm_hour,
			date.tm_min,
			date.tm_sec,
			(int)us.tv_usec,
			priority_to_string(p),
			(int)getpid());

		l = l->next;
	}
	return (1);
}

// prints message to logger
void
log_message(enum priority p, const char *message, ...)
{
	va_list va;
	struct list *l;
	FILE *f;

	if (!print_init_message(p))
		return;

	l = logger.files;
	while (l != NULL) {
		va_start(va, message);

		f = l->item;
		vfprintf(f, message, va);
		fprintf(f, "\n");
		fflush(f);

		l = l->next;
		va_end(va);
	}

	unlock_m();
}

void
print_cfg(const struct list *variables, const struct list *commands)
{
	APP_DEBUG_FNAME;
	struct variable *v;
	struct command *c;
	struct list *l;
	FILE *f;
	const struct list *var;
	const struct list *cmd;

	if (!print_init_message(info))
		return;

	l = logger.files;
	while (l != NULL) {
		var = variables;
		cmd = commands;
		f = l->item;
		fprintf(f, "%s",
				"CONFIG:\n"
				"*******CRON VARIABLES:*******\n");
		while (var != NULL) {
			v = (struct variable *)var->item;

			fprintf(f, "%s = %s\n",
					v->name, v->substitution);

			var = var->next;
		}
		fprintf(f, "*******CRON COMMANDS:*******\n");
		while (cmd != NULL) {
			c = (struct command *)cmd->item;

			fprintf(f, "%s; cmd: %s\n",
					time_to_string(c->seconds),
					c->cmd);

			cmd = cmd->next;
		}
		fflush(f);
		l = l->next;
	}
	unlock_m();
}
