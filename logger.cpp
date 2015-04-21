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

#include "logger.hpp"

#include <log4cpp/Appender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/Layout.hh>
#include <log4cpp/BasicLayout.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/PatternLayout.hh>

using namespace std;

log4cpp::Category& init_logger()
{
    log4cpp::Appender* console_appender;
    log4cpp::Appender* file_appender;
    log4cpp::PatternLayout* console_layout;
    log4cpp::PatternLayout* file_layout;
    std::string logfile = "build/program.log";
    std::string pattern = "%d{%H:%M:%S:%l} %u:\t[%p] %m%n";

    console_appender = new log4cpp::OstreamAppender("console", &std::cout);
    file_appender = new log4cpp::FileAppender("default", logfile, false);   // append=false=>truncate

    file_layout = new log4cpp::PatternLayout();
    console_layout = new log4cpp::PatternLayout();
    console_layout->setConversionPattern(pattern);
    file_layout->setConversionPattern(pattern);

    file_appender->setLayout(file_layout);
    console_appender->setLayout(console_layout);

    log4cpp::Category& log = log4cpp::Category::getRoot();
    log.setPriority(log4cpp::Priority::DEBUG);

    log.addAppender(console_appender);
    log.addAppender(file_appender);

    return log;
}

log4cpp::Category& logger = init_logger();

