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
    regex_t reg;
    regmatch_t match;
    char* name_begin;
    char* name_end;

    // rozdeli text na nazov a obsah premennej..
    compile_regex(&reg, CONF_VAR_REGEX);
    regexec(&reg, text, 1, &match, 0);
    name_begin = text + match.rm_so;
    name_end = text + match.rm_eo;
    *(name_end - 1) = '\0';
    regfree(&reg);

    // zmaze biele znaky nakonci nazvu...
    compile_regex(&reg, "[[:blank:]]*$");
    regexec(&reg, name_begin, 1, &match, 0);
    *(name_begin + match.rm_so) = '\0';
    // aj nakonci obsahu
    regexec(&reg, name_end, 1, &match, 0);
    *(name_end + match.rm_so) = '\0';
    regfree(&reg);
    // a aj na zaciatku obsahu...
    compile_regex(&reg, "^[[:blank:]]*");
    regexec(&reg, name_end, 1, &match, 0);
    name_end = name_end + match.rm_eo;

    //INFO("creating new variable '%s'='%s'", name_begin, name_end);
    strcpy(name, name_begin);
    strcpy(substitution, name_end);
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

void fill_command(struct command* c, char* text)
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

command* read_file(const char* filename)
{
    cout << CONF_COMMAND_REGEX << endl;
    FILE* input;
    char* line = NULL;
    size_t len = 0;
    ssize_t read_length;
    char substitued[CONF_SUBSTITUTION_OUT_MAXLENGTH + 1];
    vars var_begin;
    vars *var_end = &var_begin;
    var_begin.next = NULL;
    command com_begin;
    command* com_end = &com_begin;
    com_begin.next = NULL;
    
    input = fopen(filename, "r");
    if (input == NULL)
    {
        ERROR("fopen err");
        exit(1);
    }

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
                com_end->next = (struct command*)malloc(sizeof(command));
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
/*
    INFO("VARIABLES: ");
    var_end = var_begin.next;
    while (var_end)
    {
        INFO("vars: %s = %s", var_end->name, var_end->substitution);
        var_end = var_end->next;
    }
    INFO("COMMANDS");
    com_end = com_begin.next;
    while (com_end)
    {
        stringstream s;
        s << (int)com_end->min << " "
            << (int)com_end->hour << " "
            << (int)com_end->dom << " "
            << (int)com_end->month << " "
            << (int)com_end->dow << " "
            << com_end->command;
        INFO(s.str());
        com_end = com_end->next;
    }
*/
    return com_begin.next;
}









