/*
 * File: utils.h
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

#ifndef UTILS_H
#define	UTILS_H

#include <time.h>

#include "logger.h"


struct list
{
	void *item;
	struct list *next;
};

void delete_list(struct list **lptr);

void swap_ptr(void **p1, void **p2);

const char *strerr();

const char *time_to_string(time_t t);

/*
 * removes all blank-characters from beginning and end
 */
void trim(char *str);

/*
 * prints usage message
 *
 * declaration in main.cpp
 */
void usage();

/*
 * handle arguments from usage();
 */
int handle_args(int argc, char **argv);

/*
 * handle all processess after crontab exit
 */
void register_process(pid_t pid);
void kill_processess();
void signal_handler(int);

/*
 * calls waitpid() on all fork-ed child processes
 */
void wait_children();

void myexit(int ret);
void myabort();

void set_signal_handler();

#ifdef __sun
#include <stdio.h>

int getline(char **line_ptr, size_t *line_size, FILE *stream);

#endif

#endif /* !UTILS_H */
