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


#include "conf.h"
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>


#include <iostream>
#include <sstream>

using namespace std;


#define __TOSTRING__(X) #X
#define __TOSTRING__2(X) __TOSTRING__(X)

#define min(x, y) (x < y ? x : y)


#define CONF_VAR_REGEX		"[[:alnum:]]{1," \
                            __TOSTRING__2(VAR_NAME_MAXLENGTH) "}[[:blank:]]*="

#define CONF_COMMAND_REGEX_MIN  "([*[:digit:]]|[1-5][0-9])[[:blank:]]"
#define CONF_COMMAND_REGEX_HOUR "([*[:digit:]]|1[0-9]|2[0-3])[[:blank:]]"
#define CONF_COMMAND_REGEX_DOM  "([*[:digit:]]|[12][0-9]|3[01])[[:blank:]]"
#define CONF_COMMAND_REGEX_MON  "([*[:digit:]]|1[0-2])[[:blank:]]"
#define CONF_COMMAND_REGEX_DOW  "([*[0-7])[[:blank:]]"
#define CONF_COMMAND_REGEX_COM  ".+$"

#define CONF_COMMAND_REGEX      CONF_COMMAND_REGEX_MIN \
                                CONF_COMMAND_REGEX_HOUR \
                                CONF_COMMAND_REGEX_DOM \
                                CONF_COMMAND_REGEX_MON \
                                CONF_COMMAND_REGEX_DOW \
                                CONF_COMMAND_REGEX_COM

extern char **environ;

struct vars
{
    char name[VAR_NAME_MAXLENGTH + 1];
    char substitution[VAR_SUBSTITUTION_MAXLENGTH  + 1];

    struct vars* next;
};




void compile_regex(regex_t * reg, const char* text)
{
    if (regcomp(reg, text, REG_EXTENDED) != 0)
    {
        ERROR("regcomp");
        exit(1);
    }
}

int check_line(const char* line)
{
    // line ma byt dlzky CONF_LINE_MAXLENGTH
    //
    regex_t reg;
    const char* regexs_ignore[2] = {
        "^[[:blank:]]*#",   // comment
        "^[[:blank:]]*$",   // blank line
    };
    const char* regex_variable = CONF_VAR_REGEX;
    const char* regex_command_line = CONF_COMMAND_REGEX;
    
    // ignore line:
    for (int i = 0; i < 2; ++i)
    {
        compile_regex(&reg, regexs_ignore[i]);
        if (regexec(&reg, line, 0, NULL, 0) == 0)
        {
            //DEBUG("MATCH ignore_line: '%s'", line);
            regfree(&reg);
            return LINE_IGNORE;
        }
        else
            regfree(&reg);
    }

    // variable line:
    compile_regex(&reg, regex_variable);
    if (regexec(&reg, line, 0, NULL, 0) == 0)
    {
        //DEBUG("MATCH variable_line: '%s'", line);
        regfree(&reg);
        return LINE_VARIABLE;
    }
    else
        regfree(&reg);

    // command line:
    compile_regex(&reg, regex_command_line);
    if (regexec(&reg, line, 0, NULL, 0) == 0)
    {
        //DEBUG("MATCH command_line: '%s'", line);
        regfree(&reg);
        return LINE_COMMAND;
    }
    else
        regfree(&reg);
    
    // not matched on anything, bad line..
    ERROR("bad line '%s'", line);
    return LINE_BAD;
}

void fill_variable(char name[VAR_NAME_MAXLENGTH], char substitution[VAR_SUBSTITUTION_MAXLENGTH], char* text)
{
    DEBUG("TEXT: %s\n", text);
    regex_t reg;
    regmatch_t match;
    char* name_begin;
    char* name_end;

    // rozdeli text na nazov a obsah premennej..
    compile_regex(&reg, CONF_VAR_REGEX);
    regexec(&reg, text, 1, &match, 0);
    name_begin = text + match.rm_so;
    name_end = text + match.rm_eo;
    name_end[-1] = '\0';
    //*(name_end - 1) = '\0';
    regfree(&reg);

    // zmaze biele znaky nakonci nazvu...
    compile_regex(&reg, "[[:blank:]]*$");
    regexec(&reg, name_begin, 1, &match, 0);
    name_begin[match.rm_so] = '\0';
    // aj nakonci obsahu
    regexec(&reg, name_end, 1, &match, 0);
    name_end[match.rm_so] = '\0';
    regfree(&reg);
    // a aj na zaciatku obsahu...
    compile_regex(&reg, "^[[:blank:]]*");
    regexec(&reg, name_end, 1, &match, 0);
    name_end = name_end + match.rm_eo;

    INFO("creating new variable '%s'='%s'", name_begin, name_end);
    strncpy(name, name_begin, VAR_NAME_MAXLENGTH);
    strncpy(substitution, name_end, VAR_SUBSTITUTION_MAXLENGTH);

    name[VAR_NAME_MAXLENGTH] = '\0';
    substitution[VAR_SUBSTITUTION_MAXLENGTH] = '\0';


    // TODO: PO odkomentovani hadze bud malloc() memory corruption, alebo segfault... 
    //
    //
    //if (strlen(name) != strlen(name_begin))
        //ERROR("variable '%s' is longer than maximum, truncating to '%s'!", name_begin, name);
    //DEBUG("%s :\t %s :\t %s", name, name_end, substitution);
    //if (strlen(substitution) != strlen(name_end))
        //ERROR("substitution of variable '%s' as '%s' is longer than maximum, truncating to '%s'", name, name_end, substitution);
}

void substitute_text(struct vars* v, char* text_in, char out[CONF_SUBSTITUTION_OUT_MAXLENGTH + 1])
{
    regex_t reg;
    regmatch_t match;
    char regstr[VAR_NAME_MAXLENGTH + 3] = {'\\', '$'};
    size_t n;
    size_t maxlength;
    char *readbegin = (char*)malloc((CONF_SUBSTITUTION_OUT_MAXLENGTH + 1) * sizeof(char));
    char *writebegin = (char*)malloc((CONF_SUBSTITUTION_OUT_MAXLENGTH + 1) * sizeof(char));
    char *read_buf;
    char *write_buf;

    strcpy(readbegin, text_in);
    //INFO("INPUT_TEXT: %s", readbegin);
    while(v)
    {
        read_buf = readbegin;
        write_buf = writebegin;
        maxlength = CONF_SUBSTITUTION_OUT_MAXLENGTH;

        strcpy(regstr + 2, v->name);    // vytvori regexp "\$PREMENNA"
        compile_regex(&reg, regstr);
        //DEBUG("REGEXP: %s", regstr);

        // v cykle prechadza vsetky vyskyty $PREMENNA v texte a nahradzuje ich za vysledok...
        // prechadza iba jednorazovo.. 
        while(regexec(&reg, read_buf, 1, &match, 0) == 0)
        {
            //DEBUG("readbuf1: %s", read_buf);
            // skopiruje prvych n znakov do vyskytu $PREMENNA
            n = min((size_t)match.rm_so, maxlength);
            strncpy(write_buf, read_buf, n);
            write_buf[n] = '\0';
            write_buf += n;
            read_buf += match.rm_eo;
            maxlength -= n;
            //DEBUG("out1: '%s'", writebegin);

            //DEBUG("readbuf2: %s", read_buf);
            // skopiruje substituciu za $PREMENNA
            n = min(strlen(v->substitution), maxlength);
            strncpy(write_buf, v->substitution, n);
            write_buf[n] = '\0';
            write_buf += n;
            maxlength -= n;
            //DEBUG("out2: '%s'", writebegin);
        }

        // skopiruje zvysok, teda string po vyskyte $PREMENNA + '\0'
        n = min(strlen(read_buf), maxlength);
        strncpy(write_buf, read_buf, n);
        write_buf[n] = '\0';
        //DEBUG("out3: '%s'", writebegin);

        // swap read and write buffer
        write_buf = readbegin;
        readbegin = writebegin;
        writebegin = write_buf;
        write_buf = NULL;

        regfree(&reg);
        v = v->next;
    }
    strcpy(out, readbegin);
    //cout << "RBEGIN: " << readbegin << endl;
    //cout << "WBEGIN: " << writebegin << endl;
    //INFO("OUT: %s", out);
}


struct command_config
{
    char min;
    char hour;
    char dom;
    char month;
    char dow;
    char command[CONF_COMMAND_MAXLENGHT + 1];

    struct command_config* next;
};

void fill_command(struct command_config* c, char* text)
{
    //DEBUG("TEXT: %s", text);
    regex_t reg;
    regmatch_t match;

    //DEBUG(text);
    compile_regex(&reg, CONF_COMMAND_REGEX_MIN);
    regexec(&reg, text, 1, &match, 0);
    regfree(&reg);
    text[match.rm_eo - 1] = '\0';
    c->min = (text[match.rm_so] == '*' ? -1 : atoi(text));
    //cout << (int)c->min << " ";
    text += match.rm_eo;

    //DEBUG(text);
    compile_regex(&reg, CONF_COMMAND_REGEX_HOUR);
    regexec(&reg, text, 1, &match, 0);
    regfree(&reg);
    text[match.rm_eo - 1] = '\0';
    c->hour = (text[match.rm_so] == '*' ? -1 : atoi(text));
    //cout << (int)c->hour << " ";
    text += match.rm_eo;

    //DEBUG(text);
    compile_regex(&reg, CONF_COMMAND_REGEX_DOM);
    regexec(&reg, text, 1, &match, 0);
    regfree(&reg);
    text[match.rm_eo - 1] = '\0';
    c->dom = (text[match.rm_so] == '*' ? -1 : atoi(text));
    //cout << (int)c->dom << " ";
    text += match.rm_eo;

    //DEBUG(text);
    compile_regex(&reg, CONF_COMMAND_REGEX_MON);
    regexec(&reg, text, 1, &match, 0);
    regfree(&reg);
    text[match.rm_eo - 1] = '\0';
    c->month = (text[match.rm_so] == '*' ? -1 : atoi(text));
    //cout << (int)c->month << " ";
    text += match.rm_eo;

    //DEBUG(text);
    compile_regex(&reg, CONF_COMMAND_REGEX_DOW);
    regexec(&reg, text, 1, &match, 0);
    regfree(&reg);
    text[match.rm_eo - 1] = '\0';
    c->dow = (text[match.rm_so] == '*' ? -1 : atoi(text) % 7);  // 7 -> 0
    //cout << (int)c->dow << " ";
    text += match.rm_eo;

    //DEBUG(text);
    compile_regex(&reg, CONF_COMMAND_REGEX_COM);
    regexec(&reg, text, 1, &match, 0);
    regfree(&reg);
    strncpy(c->command, text + match.rm_so, CONF_COMMAND_MAXLENGHT);
    //cout << c->command << endl;
}

vars* init_environment_variables()
{
    vars begin;
    vars* var_end = &begin;
    begin.next = NULL;

    while (*environ != NULL)
    {
        //printf("%s\n", *environ);
        var_end->next = (struct vars*)malloc(sizeof(vars));
        var_end = var_end->next;
        var_end->next = NULL;
        fill_variable(var_end->name, var_end->substitution, *environ);
         //obidva stringy maju rozne dlzky...
        var_end->substitution[VAR_SUBSTITUTION_MAXLENGTH] = '\0';

        ++environ;
    }
    exit(0);

    return begin.next;
}

struct com_pair read_file(const char* filename)
{
    FILE* input;
    char* line = NULL;
    size_t len = 0;
    ssize_t read_length;
    char substitued[CONF_SUBSTITUTION_OUT_MAXLENGTH + 1];
    vars var_begin;
    vars *var_end = &var_begin;
    var_begin.next = NULL;
    command_config com_begin;
    command_config* com_end = &com_begin;
    com_begin.next = NULL;
    
    input = fopen(filename, "r");
    if (input == NULL)
    {
        ERROR("fopen err");
        exit(1);
    }

    init_environment_variables();
    while ((read_length = getline(&line, &len, input)) != -1)
    {
        // odstrani \n z konca
        if (read_length > 0)
            line[read_length - 1] = '\0';
        switch(check_line(line))
        {
            case LINE_VARIABLE:
                var_end->next = (struct vars*)malloc(sizeof(vars));
                var_end = var_end->next;
                var_end->next = NULL;
                fill_variable(var_end->name, var_end->substitution, line);
                substitute_text(var_begin.next, var_end->substitution, substitued);
                strncpy(var_end->substitution, substitued, VAR_SUBSTITUTION_MAXLENGTH);
                // obidva stringy maju rozne dlzky...
                var_end->substitution[VAR_SUBSTITUTION_MAXLENGTH] = '\0';
                INFO("VAR_AFTER_SUBSTITUTION: %s = %s", var_end->name, var_end->substitution);
                break;
            case LINE_COMMAND:
                com_end->next = (struct command_config*)malloc(sizeof(command_config));
                com_end = com_end->next;
                com_end->next = NULL;
                substitute_text(var_begin.next, line, substitued);
                INFO("COMMAND_SUBSTITUTION: '%s'", substitued);
                fill_command(com_end, substitued);
                break;
            case LINE_BAD:
                LOG("bad line structure '%s', ignoring", line);
            case LINE_IGNORE:
                break;
                //INFO("ignoring line '%s'", line);
        }
    }
    free(line);
    fclose(input);

    var_end = var_begin.next;
    while(var_end)
    {
        var_begin.next = var_end->next;
        free(var_end);
        var_end = var_begin.next;
    }

    return transform_commands(com_begin.next);
}


int command_sort_comp_function(const void* c1, const void* c2)
{
    return ((struct command*)c1)->seconds - ((struct command*)c2)->seconds;
}

time_t tm_to_time(struct tm* datetime, struct command_config* com)
{
    datetime->tm_sec = 0;
    //mins:
    if (com->min != -1)
        datetime->tm_min = com->min;
    //hours:
    if (com->hour != -1)
        datetime->tm_hour = com->hour - 1;
    //day_of_month
    if (com->dom != -1)
    {
        // mal by som zmenit den na com->dom
        // ale este ked dnesny_dow == com->dow, mam spusit command dnes..
        // resp. naplanuj to na taky den, ktory je najblizsie dnesku..
        int distance;
        int comdow = com->dow % 7;
        distance = (comdow >= datetime->tm_wday) ?
            comdow - datetime->tm_wday : 7 - datetime->tm_wday + comdow;

        if (distance > com->dom - datetime->tm_mday)
            datetime->tm_mday = com->dom;
        else
            datetime->tm_mday += distance;
    }
    //month
    if (com->month != -1)
        datetime->tm_mon = com->month - 1;

    /*
    //date_of_week
    if (com->dow != -1)
        datetime->tm_wday = com->dow;
    */

    return mktime(datetime);
}
    
struct com_pair transform_commands(struct command_config* conf)
{
    if (conf == NULL)
        return {0, NULL};

    time_t actual_secs;
    struct tm* datetime;
    struct command* c;
    struct command_config* it = NULL;
    int com_count = 0;
    int i = 0;
    const char* time_format = "%H:%M:%S\t%d-%m-%y";
    char buf[32];
    struct com_pair out;

    it = conf;
    while (it)
    {
        ++com_count;
        it = it->next;
    }

    time(&actual_secs);
    datetime = localtime(&actual_secs);

    strftime(buf, 32, time_format, datetime);
    printf("DNES: %s\n", buf);

    c = (struct command*)malloc(sizeof(struct command) * com_count);
    bzero(c, com_count);

    i = 0;
    it = conf;
    while(it)
    {
        datetime = localtime(&actual_secs);
        c[i].seconds = tm_to_time(datetime, it);
        strcpy(c[i].command_exec, conf->command);

        strftime(buf, 32, time_format, datetime);
        printf("date: %s\n", buf);

        ++i;
        it = it->next;
    }

    qsort(c, com_count, sizeof(struct command), command_sort_comp_function);

    for (i = 0; i < com_count; ++i)
    {
        cout << c[i].seconds << "\t" << asctime(localtime(&c[i].seconds));
    }
    out.commands = c;
    out.count = com_count;

    return out;
}


