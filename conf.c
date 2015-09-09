/*
 * File: conf.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "conf.h"
#include "logger.h"
#include "utils.h"

// REGEXes for matching crontab config:

// commented/empty lines
#define	CONF_REGEX_IGNORE	"^[[:blank:]]*(#|$)"

// variable: name=substitution
#define	CONF_REGEX_VARIABLE	"^([[:alpha:][:digit:]_]+)[[:blank:]]*="

// command lines:
// 	-> digits or '*'
#define	CONF_REGEX_COMMAND_MIN	"([*0-9]|[1-5][0-9])[[:blank:]]+"
#define	CONF_REGEX_COMMAND_HOUR	"([*0-9]|1[0-9]|2[0-3])[[:blank:]]+"
#define	CONF_REGEX_COMMAND_DOM	"([*0-9]|[12][0-9]|3[01])[[:blank:]]+"
#define	CONF_REGEX_COMMAND_MON	"([*0-9]|1[0-2])[[:blank:]]+"
#define	CONF_REGEX_COMMAND_DOW	"([*0-7])[[:blank:]]+"
// and command.. >1 non-blank character
#define	CONF_REGEX_COMMAND_COM	".*[^[:blank:]].*$"

// and full command line:
#define	CONF_REGEX_COMMAND \
				"^[[:blank:]]*" \
				CONF_REGEX_COMMAND_MIN \
				CONF_REGEX_COMMAND_HOUR \
				CONF_REGEX_COMMAND_DOM \
				CONF_REGEX_COMMAND_MON \
				CONF_REGEX_COMMAND_DOW \
				CONF_REGEX_COMMAND_COM

// allocate string with '\0'
#define	alloc_string(string) \
				alloc_string_size(strlen(string))
#define	alloc_string_size(size) \
				(malloc((size + 1) * sizeof (char)))

#define	min(n1, n2)		((n1 < n2) ? n1 : n2)
#define	max(n1, n2)		((n1 > n2) ? n1 : n2)


size_t
max_var_name_len(struct list *vars)
{
	size_t n = 0;
	for (; vars != NULL; vars = vars->next)
		n = max(n, strlen(((struct variable *)vars->item)->name));
	return (n);
}

struct list *
read_config(const char *filename)
{
	APP_DEBUG_FNAME;

	struct list *beg_cmd;
	struct list *beg_var;
	struct list *l;
	FILE *in;
	char *line;
	size_t len;
	ssize_t read_len;

	beg_cmd = NULL;
	beg_var = NULL;

	len = CONF_LINE_MAXLENGTH;
	line = alloc_string_size(len);

	in = fopen(filename, "r");
	if (in == NULL) {
		ERR("fopen('%s'): %s", filename, strerr());
		myabort();
	}
	else
	{
		while ((read_len = getline(&line, &len, in)) != -1) {
			// remove \n from the end
			if (read_len > 0)
				line[read_len - 1] = '\0';

			switch (check_line(line)) {
				case LINE_VARIABLE:
					l = malloc(sizeof (struct list));
					l->next = beg_var;
					l->item = create_var(line, beg_var);
					beg_var = l;
					break;
				case LINE_COMMAND:
					l = malloc(sizeof (struct list));
					l->next = beg_cmd;
					l->item = create_cmd(line, beg_var);
					beg_cmd = l;
				case LINE_BAD:
					WARN("bad line structure '%s'", line);
					break;
				case LINE_IGNORE:
					DEBUG("ignoring line '%s'", line);
					break;
			}
			l = NULL;
		}
		if (ferror(in))
			WARN("ferror while reading '%s', try to continue",
					filename);

		fclose(in);

		print_cfg(beg_var, beg_cmd);
	}
	free(line);
	delete_list(&beg_var);

	return (beg_cmd);
}

void
substitute(const char *text, struct list *variables, char *substitued)
{
	APP_DEBUG_FNAME;

	regex_t reg;
	regmatch_t match;
	char *regstr;	// stored regexp '\$VAR'
	size_t n;
	size_t maxlength;
	struct variable *v;
	struct {
		char *buff;
		char *ptr;
	} read, write;

#define	subst_buff_check(size) \
	if ((size) > CONF_SUBSTITUTION_MAXLENGTH) { \
		ERR("substitution buffer overflow"); \
		myabort(); \
	}

	subst_buff_check(strlen(text));

	regstr = alloc_string_size(max_var_name_len(variables));
	strcpy(regstr, "\\$");

	read.buff = alloc_string_size(CONF_SUBSTITUTION_MAXLENGTH);
	write.buff = alloc_string_size(CONF_SUBSTITUTION_MAXLENGTH);

	strcpy(read.buff, text);

	while (variables) {
		v = variables->item;
		maxlength = CONF_SUBSTITUTION_MAXLENGTH;
		read.ptr = read.buff;
		write.ptr = write.buff;
		strcpy(regstr + 2, v->name);
		compile_regex(&reg, regstr);

		while (regexec(&reg, read.ptr, 1, &match, 0) == 0) {
			// copy string before $VAR
			n = min(match.rm_so, maxlength);
			strncpy(write.ptr, read.ptr, n);
			write.ptr[n] = '\0';
			write.ptr += n;
			read.ptr += match.rm_eo;	// go after $VAR
			maxlength -= n;

			// copy $VAR substitution
			n = min(strlen(v->substitution), maxlength);
			subst_buff_check(n);
			strncpy(write.ptr, v->substitution, n);
			write.ptr[n] = '\0';
			write.ptr += n;
			maxlength -= n;
		}

		n = min(strlen(read.ptr), maxlength);
		subst_buff_check(n);
		strncpy(write.ptr, read.ptr, n);
		write.ptr[n] = '\0';

		swap_ptr((void **)&write.buff, (void **)&read.buff);

		variables = variables->next;
	}

	DEBUG("substitued '%s' ~> '%s'", text, read.buff);
	strcpy(substitued, read.buff);

	free(read.buff);
	free(write.buff);
}

struct variable *
create_var(char *text, struct list *vars)
{
	APP_DEBUG_FNAME;

	if (check_line(text) != LINE_VARIABLE) {
		ERR("create_var()");
		myabort();
	}

	size_t n;
	struct variable *var;

	var = malloc(sizeof (struct variable));
	n = run_r(CONF_REGEX_VARIABLE, text);
	trim(text);
	var->name = alloc_string(text);
	strcpy(var->name, text);
	text += n;
	trim(text);
	var->substitution = alloc_string(text);
	strcpy(var->substitution, text);

	substitute(var->substitution, vars, var->substitution);

	DEBUG("variable '%s' = '%s' created", var->name, var->substitution);

	return (var);
}

struct command *
create_cmd(char *text, struct list *vars)
{
	APP_DEBUG_FNAME;

	if (check_line(text) != LINE_COMMAND) {
		ERR("create_cmd()");
		myabort();
	}

	struct command *cmd;
	struct command_config c;
	size_t n;

#define	asterisk (strcmp(text, "*") == 0) ? -1 : strtol(text, NULL, 10)

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

	if (strlen(text) <= 0) {
		ERR("command_length <= 0");
		myabort();
	}

	c.cmd = alloc_string(text);
	strcpy(c.cmd, text);

	cmd = transform(&c);
	substitute(cmd->cmd, vars, cmd->cmd);

	DEBUG("command '%s: %s' created",
			time_to_string(cmd->seconds), cmd->cmd);

	return (cmd);

}

void
compile_regex(regex_t *reg, const char *text)
{
	// APP_DEBUG_FNAME;

	if (regcomp(reg, text, REG_EXTENDED) != 0) {
		DEBUG("%i", regcomp(reg, text, REG_EXTENDED));
		ERR("regcomp(%s)", text);
		myabort();
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

struct command *
transform(struct command_config *c)
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

	cmd->seconds = mktime(datetime);
	cmd->cmd = c->cmd;
	c->cmd = NULL;

	return (cmd);
}

size_t
run_r(const char *regex_str, char *text)
{
	// APP_DEBUG_FNAME;

	if (!match(text, regex_str)) {
		ERR("!match(regex)");
		myabort();
	}

	regex_t reg;
	regmatch_t match;

	compile_regex(&reg, regex_str);
	regexec(&reg, text, 1, &match, 0);
	text[match.rm_eo - 1] = '\0';
	regfree(&reg);

	return (match.rm_eo);
}

int
check_line(const char *line)
{
	// APP_DEBUG_FNAME;

	int out;

	if (match(line, CONF_REGEX_IGNORE))
		out = LINE_IGNORE;
	else if (match(line, CONF_REGEX_VARIABLE))
		out = LINE_VARIABLE;
	else if (match(line, CONF_REGEX_COMMAND))
		out = LINE_COMMAND;
	else
		out = LINE_BAD;

	return (out);
}
