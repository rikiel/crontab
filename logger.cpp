/*
 * File: logger.cpp
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

#ifdef NODEF

#ifdef __cplusplus

#include <sys/types.h>
#include <unistd.h>

#include <log4cpp/Appender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/Layout.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/PatternLayout.hh>

#include "logger.hpp"
extern "C"
{
#include "utils.h"
#include "conf.h"
}
/*
 * in log_* functions i prepend "[PID]" before message, so pattern will be
 * like: HH:MM:SS:mm <PRIORITY> [PID] MESSAGE
 * where mm (%l in pattern) are microseconds
 */
#define	LOG_PATTERN	"%d{%H:%M:%S:%l}\t<%p>\t%m%n"

log4cpp::Category &
init_logger()
{
	log4cpp::Appender *console_appender;
	log4cpp::PatternLayout *console_layout;

	console_layout = new log4cpp::PatternLayout();
	console_layout->setConversionPattern(LOG_PATTERN);

	console_appender = new log4cpp::OstreamAppender("console", &std::cout);
	console_appender->setLayout(console_layout);

	log4cpp::Category& log = log4cpp::Category::getRoot();
	log.setPriority(log4cpp::Priority::INFO);
	log.addAppender(console_appender);

	return (log);
}

log4cpp::Category &logger = init_logger();

inline std::string
pid_str(const char *s)
{
	std::stringstream str;
	str
		<< "["
		<< getpid()
		<< "]\t"
		<< s;
	return (str.str());
}

extern "C"
{
	void
	log_to_file(const char *filename)
	{
		// APP_DEBUG_FNAME;

		DEBUG("add log file '%s'", filename);

		log4cpp::Appender *appender;
		log4cpp::PatternLayout *layout;

		layout = new log4cpp::PatternLayout();
		layout->setConversionPattern(LOG_PATTERN);

		appender = new log4cpp::FileAppender("default", filename, true);
		appender->setLayout(layout);

		logger.addAppender(appender);
	}

#define	MY_LOG(PRIORITY) \
	{ \
		va_list va; \
		va_start(va, str); \
		logger.logva(log4cpp::Priority::PRIORITY, \
				pid_str(str).c_str(), va); \
		va_end(va); \
	}

	void
	log_debug  (const char *str, ...)
	{
		MY_LOG(DEBUG);
	}
	void
	log_info   (const char *str, ...)
	{
		MY_LOG(INFO);
	}
	void
	log_warn   (const char *str, ...)
	{
		MY_LOG(WARN);
	}
	void
	log_error  (const char *str, ...)
	{
		MY_LOG(ERROR);
	}

	void
	log_priority(const char *priority)
	{
#define	SET_PRIORITY(P)	logger.setPriority(log4cpp::Priority::P)
		std::string p = priority;

		if (p == "DEBUG")
			SET_PRIORITY(DEBUG);
		else if (p == "INFO")
			SET_PRIORITY(INFO);
		else if (p == "WARN")
			SET_PRIORITY(WARN);
		else
			WARN("not supported priority '%s'", priority);
	}

	void
	print_cfg(const struct list *variables, const struct list *commands)
	{
		APP_DEBUG_FNAME;
		struct variable *v;
		struct command *c;
		std::stringstream str;

		str
			<< "CONFIG:\n"
			<< "*******CRON VARIABLES:*******\n";

		while (variables) {
			v = (struct variable *)variables->item;

			str
				<< v->name
				<< " = "
				<< v->substitution
				<< std::endl;

			variables = variables->next;
		}
		while (commands) {
			c = (struct command *)commands->item;

			str
				<< time_to_string(c->seconds)
				<< "; cmd: "
				<< c->cmd
				<< std::endl;

			commands = commands->next;
		}

		INFO("%s", str.str().c_str());
	}
}

#endif
#endif
