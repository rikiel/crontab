/*
 * File: utils.h
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

#ifndef UTILS_H
#define	UTILS_H

#include "logger.hpp"
#include <time.h>

#define	STR_LENGTH(s)	(sizeof (s))

struct command_config;
struct command;
struct variable;

struct list
{
	void *item;
	struct list *next;
};

void delete_list(struct list *l);

void swap_ptr(void **p1, void **p2);

const char * time_to_string(time_t t);

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

#endif /* !UTILS_H */
