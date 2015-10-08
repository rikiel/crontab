/*
 * File: logger.c
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


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/file.h>

#include "logger.h"
#include "conf.h"
#include "utils.h"


#define	LOGGER_LOCK_FILE	".lock"

struct logger
{
	struct list *files;
	enum priority p;
	FILE *lock;
};

static struct logger logger = { NULL, info };

#define	log_to_all(...)	{ \
		struct list *l = logger.files; \
		while (l != NULL) { \
			fprintf(l->item, __VA_ARGS__); \
			l = l->next; \
		} \
	}
#define	log_flush_all() { \
		struct list *l = logger.files; \
		while (l != NULL) { \
			fflush(l->item); \
			l = l->next; \
		} \
	}


static void
lock()
{
	if (lockf(fileno(logger.lock), F_LOCK, 0) != 0) {
		ERR("lock: %s", strerr());
		myabort();
	}
}

static void
unlock()
{
	log_flush_all();

	if (lockf(fileno(logger.lock), F_ULOCK, 0) != 0) {
		ERR("unlock: %s", strerr());
		myabort();
	}
}


void
init_logger()
{
	logger.p = info;
	logger.files = malloc(sizeof (struct list));
	logger.files->next = NULL;
	logger.files->item = stdout;
	logger.lock = fopen(LOGGER_LOCK_FILE, "w+");

	DEBUG("logger inited");
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

	delete_list(&logger.files);
	fclose(logger.lock);
	remove(LOGGER_LOCK_FILE);
}

void
log_set_priority(enum priority p)
{
	logger.p = p;
}

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
	l = malloc(sizeof (struct list));
	l->next = logger.files;
	l->item = f;

	logger.files = l;
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

int
print_init_message(enum priority p)
{
	// display 2 (/5) numbers
#define	PRINT_PATTERN	"%02i:%02i:%02i:%05i\t[%s]\t<%i>\t"
	time_t t;
	struct tm date;
	struct timeval us;

	if (!can_log(p))
		return (0);

	time(&t);
	localtime_r(&t, &date);
	gettimeofday(&us, NULL);

	log_to_all(
			PRINT_PATTERN,
			date.tm_hour,
			date.tm_min,
			date.tm_sec,
			(int)us.tv_usec,
			priority_to_string(p),
			(int)getpid());

	return (1);
}

void
log_message(enum priority p, const char *message, ...)
{
	va_list va;
	struct list *l;
	FILE *f;

	lock();

	if (print_init_message(p)) {
		l = logger.files;
		while (l != NULL) {
			va_start(va, message);

			f = l->item;
			vfprintf(f, message, va);
			fprintf(f, "\n");

			l = l->next;
			va_end(va);
		}
	}

	unlock();
}

void
print_cfg(const struct list *variables, const struct list *commands)
{
	APP_DEBUG_FNAME;

	struct variable *v;
	struct command *c;

	lock();

	if (print_init_message(info)) {
		log_to_all("CONFIG:\n");
		log_to_all("*******CRON VARIABLES:*******\n");

		while (variables != NULL) {
			v = variables->item;
			log_to_all("%s = %s\n",
					v->name,
					v->substitution);
			variables = variables->next;
		}

		log_to_all("*******CRON COMMANDS:*******\n");

		while (commands != NULL) {
			c = commands->item;
			log_to_all("%s; cmd:%s\n",
					time_to_string(c->seconds),
					c->cmd);
			commands = commands->next;
		}
	}

	unlock();
}
