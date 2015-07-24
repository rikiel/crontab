/*
 * File: utils.c
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


#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <getopt.h>
#include "utils.h"
#include "conf.h"


void trim(char* str)
{
    char* beg;
    char* end;

    if (str == NULL)
        return;

    beg = str;
    end = str + strlen(str);
    while(beg != end && isspace(*beg))
        ++beg;
    while(beg != end && (*end == '\0' || isspace(*end)))
        --end;
    if (beg != end || *end != '\0')
        end[1] = '\0';
    for (; (*str = *beg); ++str, ++beg);
    // ^^ own strcpy, default do not work.. (ex: " /bin/bash")
}

void swap_ptr(void** p1, void** p2)
{
    void * p = *p2;
    *p2 = *p1;
    *p1 = p;
}

void delete_list(struct list* l)
{
    APP_DEBUG_FNAME;

    struct list* next;
    while(l)
    {
        next = l->next;
        free(l);
        l = next;
    }
}

const char* time_to_string(time_t t)
{
#define TIME_FORMAT     "DD.MM.YYYY HH:MM"
#define STR_LENGTH      (sizeof(TIME_FORMAT))
    static char buff[STR_LENGTH];
    struct tm date;

    localtime_r(&t, &date);
    strftime(buff, STR_LENGTH, "%d.%m.%Y %H:%M", &date);

    return buff;
}

int handle_args(int argc, char** argv)
{
    APP_DEBUG_FNAME;

    int ch;
    struct option long_opts[] = {
        {"help", no_argument, NULL, 'h'},
        {"debug", no_argument, NULL, 'd'},
        {"log-to", required_argument, NULL, 'l'},
        {NULL, 0, NULL, 0}
    };

    while ((ch = getopt_long(argc, argv, "hdl:", long_opts, NULL)) != -1)
    {
        DEBUG("WHILE");
        switch(ch)
        {
            case 'h':
                DEBUG("HELP");
                usage();
                exit(0);
                break;
            case 'd':
                DEBUG("DEBUG");
                SET_LOGGER_PRIORITY("DEBUG");
                break;
            case 'l':
                DEBUG("LOG_TO '%s", optarg);
                log_to_file(optarg);
                break;
            default:
                ERR("wrong parameter '%s'", optarg);
                usage();
                exit(1);
                break;
        }
    }

    return optind;
}


