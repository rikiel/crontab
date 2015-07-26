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
#define	CONF_H

#include <time.h>
#include <regex.h>

#define	CONF_COMMAND_MAXLENGTH		128
#define	CONF_VAR_NAME_MAXLENGTH		10
#define	CONF_SUBSTITUTION_MAXLENGTH	CONF_COMMAND_MAXLENGTH

struct list;

struct variable {
    char name[CONF_VAR_NAME_MAXLENGTH + 1];
    char substitution[CONF_SUBSTITUTION_MAXLENGTH + 1];
};

struct command {
    time_t seconds;
    char cmd[CONF_COMMAND_MAXLENGTH + 1];
};

/*
 * like commands in crontab:
 *  min    hour    dayOfMonth  month   dayOfWeek  COMMAND
 */
struct command_config {
    char min;
    char hour;
    char dom;
    char month;
    char dow;
    char command[CONF_COMMAND_MAXLENGTH + 1];
};

/*
 * nacita cron config `filename`,
 * vysledok je v `commands` ako zoznam command-ov
 *
 * vrati 0 ak je vsetko ok, inac -1
 */
int read_config(const char *filename, struct list **commands);

/*
 * make substitution of $VAR from variables in `text`
 * output is saved in `substitued`
 */
void substitute(const char *text, struct list *variables, char *substitued);

/*
 * create variable from `text` with all substitutions
 */
struct variable *create_var(char *text, struct list *vars);

/*
 * create command from `text` with all substitutions
 */
struct command *create_cmd(char *text, struct list *vars);

/*
 * compile regex, if errors occurs, aborts execution
 */
void compile_regex(regex_t *reg, const char *text);

/*
 * test if `text` is matched to regexp in `regex_str`
 */
int match(const char *text, const char *regex_str);

/*
 * transformation from command_config representation to command representation
 *  computing next execution time in seconds
 */
struct command *transform(const struct command_config *c);

/*
 * runs matching of `regex_str` on `text`
 *  `text` will be modified to string matched by regex
 *  that means, text[0..n-1] is matched, text[n]='\0', returns n
 */
size_t run_r(const char *regex_str, char *text);

#define	LINE_IGNORE		1
#define	LINE_VARIABLE		2
#define	LINE_COMMAND		3
#define	LINE_BAD		4

/*
 * returns line type: command/variable/??
 *      ignore means that line is commented or empty
 *      bad means that it does not match to previous cases
 */
int check_line(const char *line);

/*
 * prints all variables and commands to logger
 *
 * definition in logger.cpp
 */
void print_cfg(const struct list *variables, const struct list *commands);

#endif /* !CONF_H */
