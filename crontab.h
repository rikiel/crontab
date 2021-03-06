/*
 * File: crontab.h
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

#ifndef CRONTAB_H
#define	CRONTAB_H

struct list;

/*
 * runs cron daemon with config
 */
void run_cron(const char *config_file);

/*
 * run all commands that needs to be executed in this minute
 */
void run_commands(const struct list *cmd);

/*
 * run command: "bash -c $command"
 */
void run_command(const char *command);

#endif /* !CRONTAB_H */
