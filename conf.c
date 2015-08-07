/*
 * File: conf.c
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
#include <regex.h>
#include <string.h>
#include <assert.h>

#include "conf.h"
#include "utils.h"

#define	min(n1, n2)		((n1 < n2) ? n1 : n2)

#define	__TOSTRING__(X)		#X
#define	__TOSTRING__2(X)	__TOSTRING__(X)

// REGEXes for matching crontab config file:

// commented/empty lines
#define	CONF_REGEX_IGNORE	"^[[:blank:]]*(#|$)"
// variable: name=..
//  where `name` is 1..n characters long
#define	CONF_REGEX_VARIABLE \
				"([[:alnum:]]{1," \
				__TOSTRING__2(CONF_VAR_NAME_MAXLENGTH) \
				"})[[:blank:]]*="

// digits or '*' and blank characters
#define	CONF_REGEX_COMMAND_MIN	"([*[:digit:]]|[1-5][0-9])[[:blank:]]+"
#define	CONF_REGEX_COMMAND_HOUR	"([*[:digit:]]|1[0-9]|2[0-3])[[:blank:]]+"
#define	CONF_REGEX_COMMAND_DOM	"([*[:digit:]]|[12][0-9]|3[01])[[:blank:]]+"
#define	CONF_REGEX_COMMAND_MON	"([*[:digit:]]|1[0-2])[[:blank:]]+"
#define	CONF_REGEX_COMMAND_DOW	"([*[0-7])[[:blank:]]+"
// more than one non-blank character
#define	CONF_REGEX_COMMAND_COM	".*[^[:blank:]].*$"

#define	CONF_REGEX_COMMAND \
				"[[:blank:]]*" \
				CONF_REGEX_COMMAND_MIN \
				CONF_REGEX_COMMAND_HOUR \
				CONF_REGEX_COMMAND_DOM \
				CONF_REGEX_COMMAND_MON \
				CONF_REGEX_COMMAND_DOW \
				CONF_REGEX_COMMAND_COM

void
compile_regex(regex_t *reg, const char *text)
{
	// APP_DEBUG_FNAME;

	if (regcomp(reg, text, REG_EXTENDED) != 0) {
		ERR("regcomp(%s)", text);
		abort();
	}
}

int
match(const char *text, const char *regex_str)
{
	// APP_DEBUG_FNAME;

	int out;
	regex_t reg;

	compile_regex(&reg, regex_str);
	out = (regexec(&reg, text, 0, NULL, 0) == 0);
	regfree(&reg);

	return (out);
}

int
check_line(const char *line)
{
	// APP_DEBUG_FNAME;

	if (match(line, CONF_REGEX_IGNORE))
		return (LINE_IGNORE);
	if (match(line, CONF_REGEX_VARIABLE))
		return (LINE_VARIABLE);
	if (match(line, CONF_REGEX_COMMAND))
		return (LINE_COMMAND);

	return (LINE_BAD);
}

struct command *
transform(const struct command_config *c)
{
	APP_DEBUG_FNAME;

	struct command *cmd;
	time_t actual_secs;
	struct tm *datetime;

	cmd = malloc(sizeof (struct command));
	time(&actual_secs);
	datetime = localtime(&actual_secs);

	datetime->tm_sec = 0;
	if (c->min != -1)
		datetime->tm_min = c->min;
	if (c->hour != -1)
		datetime->tm_hour = c->hour;
	if (c->month != -1)
		datetime->tm_mon = c->month - 1;
	if (c->dom != -1) {
		// if day_of_week != actual_day_of_week,
		// change it to day_of_month
		if (c->dow != datetime->tm_wday)
			datetime->tm_mday = c->dom;
	}

	assert(strlen(c->command) < CONF_COMMAND_MAXLENGTH);
	strcpy(cmd->cmd, c->command);
	cmd->seconds = mktime(datetime);

	return (cmd);
}

size_t
run_r(const char *regex_str, char *text)
{
	// APP_DEBUG_FNAME;
	assert(match(text, regex_str));

	regex_t reg;
	regmatch_t match;

	compile_regex(&reg, regex_str);
	// args: regexec(REGEXP, TEXT, MATCH_ARRAY_SIZE, MATCH_ARRAY, FLAGS)
	regexec(&reg, text, 1, &match, 0);
	text[match.rm_eo - 1] = '\0';
	regfree(&reg);

	return (match.rm_eo);
}

struct command *
create_cmd(char *text, struct list *vars)
{
	APP_DEBUG_FNAME;

	assert(check_line(text) == LINE_COMMAND);

	struct command *cmd;
	struct command_config c;
	size_t n;

#define	asterisk (strcmp(text, "*") == 0) ? -1 : strtol(text, NULL, 10)
	// ^^ matched by regexp, so no errors should occur

	n = run_r(CONF_REGEX_COMMAND_MIN, text);
	c.min = asterisk;
	text += n;
	n = run_r(CONF_REGEX_COMMAND_HOUR, text);
	c.hour = asterisk;
	text += n;
	n = run_r(CONF_REGEX_COMMAND_DOM, text);
	c.dom = asterisk;
	text += n;
	n = run_r(CONF_REGEX_COMMAND_MON, text);
	c.month = asterisk;
	text += n;
	n = run_r(CONF_REGEX_COMMAND_DOW, text);
	c.dow = asterisk;
	text += n;

	assert(strlen(text) < CONF_COMMAND_MAXLENGTH);
	strcpy(c.command, text);

	cmd = transform(&c);
	substitute(cmd->cmd, vars, cmd->cmd);

	DEBUG("command '%s: %s' created",
			time_to_string(cmd->seconds), cmd->cmd);

	return (cmd);
}

struct variable *
create_var(char *text, struct list *vars)
{
	APP_DEBUG_FNAME;

	assert(check_line(text) == LINE_VARIABLE);

	size_t n;
	struct variable *var;

	var = malloc(sizeof (struct variable));
	n = run_r(CONF_REGEX_VARIABLE, text);
	trim(text);
	assert(strlen(text) < CONF_VAR_NAME_MAXLENGTH);
	strcpy(var->name, text);
	text += n;
	trim(text);
	assert(strlen(text) < CONF_SUBSTITUTION_MAXLENGTH);
	strcpy(var->substitution, text);

	substitute(var->substitution, vars, var->substitution);

	DEBUG("variable '%s = %s' created", var->name, var->substitution);

	return (var);
}

void
substitute(const char *text, struct list *vars, char *substitued)
{
	APP_DEBUG_FNAME;
	assert(strlen(text) < CONF_SUBSTITUTION_MAXLENGTH);

	regex_t reg;
	regmatch_t match;
	char regstr[CONF_VAR_NAME_MAXLENGTH + 3] = {'\\', '$'};
	// ^^ there will be regexp \$VAR
	struct variable *v;
	size_t n;
	size_t maxlength;
	struct {
		char *buff;
		char *ptr;
	} read, write;

	read.buff = malloc((CONF_SUBSTITUTION_MAXLENGTH + 1) * sizeof (char));
	write.buff = malloc((CONF_SUBSTITUTION_MAXLENGTH + 1) * sizeof (char));

	assert(strlen(text) < CONF_SUBSTITUTION_MAXLENGTH);
	strcpy(read.buff, text);

	while (vars) {
		v = (struct variable *)vars->item;
		maxlength = CONF_SUBSTITUTION_MAXLENGTH;
		read.ptr = read.buff;
		write.ptr = write.buff;
		strcpy(regstr + 2, v->name);
		compile_regex(&reg, regstr);

		while (regexec(&reg, read.ptr, 1, &match, 0) == 0) {
			// copy string before "$VAR"
			n = min(match.rm_so, maxlength);
			strncpy(write.ptr, read.ptr, n);
			write.ptr[n] = '\0';
			write.ptr += n;
			read.ptr += match.rm_eo;	// go after $VAR
			maxlength -= n;

			// copy $VAR substitution
			n = min(strlen(v->substitution), maxlength);
			assert(n < maxlength);
			strncpy(write.ptr, v->substitution, n);
			write.ptr[n] = '\0';
			write.ptr += n;
			maxlength -= n;
		}

		n = min(strlen(read.ptr), maxlength);
		assert(n < maxlength);
		strncpy(write.ptr, read.ptr, n);
		write.ptr[n] = '\0';

		swap_ptr((void**)&write.buff, (void**)&read.buff);

		vars = vars->next;
	}
	DEBUG("substitute('%s') = '%s'", text, read.buff);
	strcpy(substitued, read.buff);

	free(read.buff);
	free(write.buff);
}

int
read_config(const char *filename, struct list **commands)
{
	APP_DEBUG_FNAME;

	struct list *beg_cmd;
	struct list *beg_var;
	struct list *l;
	FILE *input;
	char *line;
	size_t len;
	ssize_t read_length;

	beg_cmd = NULL;
	beg_var = NULL;
	*commands = NULL;

	len = 128;
	line = malloc(len);

	input = fopen(filename, "r");
	if (input == NULL) {
		ERR("fopen(%s)", filename);
		free(line);
		return (-1);
	}

	while ((read_length = getline(&line, &len, input)) != -1) {
	// delete \n from the end
		if (read_length > 0)
			line[read_length - 1] = '\0';
		switch (check_line(line)) {
			case LINE_VARIABLE:
				l = (struct list *)
					malloc(sizeof (struct list));
				l->next = beg_var;
				l->item = create_var(line, beg_var);
				beg_var = l;
				break;
			case LINE_COMMAND:
				l = (struct list *)
					malloc(sizeof (struct list));
				l->next = beg_cmd;
				l->item = create_cmd(line, beg_var);
				beg_cmd = l;
				break;
			case LINE_BAD:
				WARN("bad line structure");
			case LINE_IGNORE:
				DEBUG("ignoring line '%s'", line);
				break;
		}
		l = NULL;
	}
	if (ferror(input))
		WARN("ERROR while reading '%s', trying to continue", filename);

	print_cfg(beg_var, beg_cmd);

	fclose(input);
	free(line);
	delete_list(beg_var);

	*commands = beg_cmd;
	return (0);
}
