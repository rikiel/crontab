/*
 * File: conf.h
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

#ifndef CONF_H
#define CONF_H

#include "logger.hpp"
#include <err.h>

#define VAR_NAME_MAXLENGTH 10
#define VAR_SUBSTITUTION_MAXLENGTH 32
#define CONF_LINE_MAXLENGTH 64
#define CONF_COMMAND_MAXLENGHT 64
#define CONF_SUBSTITUTION_OUT_MAXLENGTH 96

struct variable
{
    char name[VAR_NAME_MAXLENGTH + 1];
    char subst[VAR_SUBSTITUTION_MAXLENGTH + 1];
    
    struct variable* next;
};

struct command
{
    char min;
    char hour;
    char dom;
    char month;
    char dow;
    char command[CONF_COMMAND_MAXLENGHT + 1];

    struct command* next;
};

#define LINE_IGNORE			1
#define LINE_VARIABLE		2
#define LINE_COMMAND		3
#define LINE_BAD			4

command* read_file(const char* filename);

int check_line(const char* line);

void substitute_line(const char* line, struct variable* vars);



#endif /* !CONF_H */

