/*
 * File: main.c
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
#include "conf.h"
#include "crontab.h"
#include "logger.hpp"

int main()
{
    printf("run\n");

    //check_line("* * * * *  ");
    //check_line("1 2 3 4 5  ahoj");
    //read_file("crontab");
    //sort_commands(read_file("crontab"));
    //
    read_file("crontab");
    return 0;
}




