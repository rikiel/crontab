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
#include <thread>


#define CRON_SLEEP_TIME 60
#define CRON_EMPTY_SLEEP_TIME 300
#define CRON_MAX_SLEEP_TIME 300



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

void run_cron(const char* configfile)
{
    int index;
    while(1)
    {
        index = 0;
        struct com_pair p = read_file(configfile);
        if (p.count == 0)
            sleep(CRON_EMPTY_SLEEP_TIME);
        else
        {
            while (index < p.count && 
                    CRON_SLEEP_TIME >=
                        (int)(p.commands[index].seconds - time(NULL)))
            {   // maju byt vykonane v tejto minute...
                schedule_cron(p.commands[index].seconds);
                run_command(p.commands[index].command_exec);
                ++index;
            }
            sleep(CRON_SLEEP_TIME);
        }
    }
}

void schedule_cron(time_t t)
{
    while(CRON_SLEEP_TIME < (int)(t - time(NULL)))
    {
        sleep(CRON_SLEEP_TIME / 2);
    }
}








