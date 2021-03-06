/*
 * File: logger.h
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

#ifndef LOGGER_H
#define	LOGGER_H

enum priority
{
	debug, info, warn, error
};

void init_logger();

void destroy_logger();

// add file for log messages
void log_to_file(const char *filename);

// sets logger priority for output
void log_set_priority(enum priority p);

void log_message(enum priority p, const char *message, ...);


#define	DEBUG(...)		log_message(debug, __VA_ARGS__)
#define	INFO(...) 		log_message(info, __VA_ARGS__)
#define	WARN(...) 		log_message(warn, __VA_ARGS__)
#define	ERR(...)  		log_message(error, __VA_ARGS__)

#define	APP_DEBUG_FNAME		DEBUG("BEGIN_FUNCTION: %s()", __func__)

#endif /* !LOGGER_H */
