/*
 * File: logger.h
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

#ifndef LOGGER_H
#define LOGGER_H

#include "utils.h"

enum priority
{
	debug, info, warn, error
};

struct logger
{
	struct list* files;
	enum priority p;
};

extern struct logger logger;

void init_logger();

// add file for log messages
void log_to_file(const char *filename);

// sets logger priority for output
void log_set_priority(enum priority p);

void log_message(enum priority p, const char *message, ...);







// prints message to logger
void log_debug(const char *message, ...);
void log_info(const char *message, ...);
void log_warn(const char *message, ...);
void log_error(const char *message, ...);



#define	DEBUG(...)	log_debug(__VA_ARGS__)
#define	INFO(...)	log_info(__VA_ARGS__)
#define	WARN(...)	log_warn(__VA_ARGS__)
#define	ERR(...)	log_error(__VA_ARGS__)

#define	APP_DEBUG_FNAME \
			DEBUG("BEGIN_FUNCTION: %s()", __PRETTY_FUNCTION__)

#endif /* !LOGGER_H */

