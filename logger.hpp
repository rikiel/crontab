/*
 * File: logger.hpp
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

#ifndef LOGGER_HPP
#define	LOGGER_HPP

#ifdef NODEF


#ifdef __cplusplus

#include <log4cpp/Category.hh>
extern log4cpp::Category& logger;

extern "C"
{
#endif

// C wrappers:

// add file for log messages
void log_to_file(const char *filename);

// prints message to logger
void log_debug(const char *message, ...);
void log_info(const char *message, ...);
void log_warn(const char *message, ...);
void log_error(const char *message, ...);

// sets logger priority for output
void log_priority(const char *priority);

#ifdef __cplusplus
}
#endif


#define	DEBUG(...)	log_debug(__VA_ARGS__)
#define	INFO(...)	log_info(__VA_ARGS__)
#define	WARN(...)	log_warn(__VA_ARGS__)
#define	ERR(...)	log_error(__VA_ARGS__)

#define	SET_LOGGER_PRIORITY(PRIORITY) \
			log_priority(#PRIORITY);

#define	APP_DEBUG_FNAME \
			DEBUG("BEGIN_FUNCTION: %s()", __PRETTY_FUNCTION__)
#endif
#endif /* !LOGGER_HPP */
