/*
 * File: crontab.c
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


#include "crontab.h"
#include <iostream>
using namespace std;

void run_command(const char* command)
{
    int i = fork();

    switch(i)
    {
        case -1:
            LOG("fork error '%s'", strerror(errno));
            break;
        case 0: // child
            LOG("fork ok, running command: /bin/sh -c '%s'", command);
            execl("/bin/sh", "sh", "-c", command, NULL);
            ERROR("exec failed with error '%s'", strerror(errno));
            break;
        default:
            INFO("process with pid %i created", i);
            break;
    }
}

time_t tm_to_time(struct tm* datetime, struct command* com)
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
    
struct command_time_pair
{
    time_t secs;
    struct command* c;
};

int comp_function(const void* c1, const void* c2)
{
    return ((struct command_time_pair*)c1)->secs - ((struct command_time_pair*)c2)->secs;
}

void sort_commands(struct command* com)
{
    //if (com == NULL)
        //return;

    time_t actual_secs;
    struct tm* datetime;
    struct command_time_pair* min;
    int com_count = 0;
    int i = 0;
    struct command* it = com;

    while (it)
    {
        ++com_count;
        it = it->next;
    }
    
    time(&actual_secs);
    datetime = localtime(&actual_secs);
    printf("%i\n", (int)actual_secs);
    char buf[100];
    const char* format = "time: %H:%M:%S\t%d/%m/%y";
    strftime(buf, 100, format, datetime);
    printf("DNES: %s\n", buf);

    it = com;
    min = (struct command_time_pair*)malloc(sizeof(command_time_pair) * com_count);
    bzero(min, com_count);
    while (it)
    {
        datetime = localtime(&actual_secs);
        time_t other_secs = tm_to_time(datetime, it);
        strftime(buf, 100, format, datetime);
        printf("%s\n", buf);
        min[i].secs = other_secs;
        min[i].c = it;
        cout << min[i].secs << "\t" << asctime(localtime(&min[i].secs));
        ++i;
        it = it->next;
    }

    qsort(min, com_count, sizeof(struct command_time_pair), comp_function);

    for (int i = 0; i < com_count; ++i)
    {
        cout << min[i].secs << "\t" << asctime(localtime(&min[i].secs));
        min[i].c->next = (i + 1 < com_count ? min[i+1].c : NULL);
    }

    it = com;
    while (it)
    {
        cout << it->command << endl;
        it = it->next;
    }
}











