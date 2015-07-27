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
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include "logger.h"

struct logger logger;	// definition

void init_logger()
{
	APP_DEBUG_FNAME;

	logger.p = debug;
	logger.files = malloc(sizeof (struct list));
	logger.files->next = NULL;
	logger.files->item = stdout;
}

// sets logger priority for output
void log_set_priority(enum priority p)
{
	logger.p = p;
}

// add file for log messages
void log_to_file(const char *filename)
{
	APP_DEBUG_FNAME;

	FILE* f;
	struct list* l;
	f = fopen(filename, "w");
	if (f == NULL)
	{
		ERR("fopen log file '%s' failed, continue", filename);
		return;
	}
	l = logger.files;
	while (l->next != NULL)
		l = l->next;
	l->next = malloc(sizeof (struct list));
	l->next->next = NULL;
	l->next->item = f;
}

const char* priority_to_string()
{
	switch (logger.p) {
		case debug:
			return ("DEBUG");
		case info:
			return ("INFO");
		case warn:
			return ("WARN");
		case error:
			return ("ERROR");
		default:
			abort();
	}
}

long get_actual_ms()
{
	struct timeval t;

	gettimeofday(&t, NULL);
	return t.tv_usec / 1000;
}

int log_priority(enum priority p)
{
	return logger.p == debug ||
		(logger.p == info && p != debug) ||
		(logger.p == warn && (p == warn || p == error)) || 
		(logger.p == error && p == error);
}

// prints message to logger
void log_message(enum priority p, const char *message, ...)
{
#define LOG_PATTERN		"%i:%i:%i:%li\t<%s>\t[%i]\t%s"

// LOG_TIME_PATTERN	HH:MM:SS:mmm
// PATTERN:		TIME\t<PRIORITY>\t[PID] MESSAGE


#define TIME_LENGTH		12
#define PRIORITY_LENGTH		5
#define PID_LENGTH		20

	va_list va;
	time_t t;
	struct tm date;
	char* buff;
	size_t n;
	struct list* l;

	if (!log_priority(p))
		return;

	va_start(va, message);
	time(&t);
	localtime_r(&t, &date);

	n = TIME_LENGTH + PRIORITY_LENGTH + PID_LENGTH + strlen(message) + 1;
	buff = malloc(n);

	sprintf(buff, LOG_PATTERN,
			date.tm_hour,
			date.tm_min,
			date.tm_sec,
			get_actual_ms(),
			priority_to_string(),
			(int)getpid(),
			message);

	l = logger.files;

	while(l != NULL)
	{
		vfprintf(l->item, buff, va);

		l = l->next;
	}

	va_end(va);
}






