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

#include <stdlib.h>	// exit
#include <libgen.h>	// basename

#include "crontab.h"
#include "utils.h"


char *name = NULL;	// need in usage()

int
main(int argc, char **argv)
{
	int opts;

	init_logger();
	name = basename(argv[0]);
	opts = handle_args(argc, argv);

	argc -= opts;
	argv += opts;
	if (argc != 1) {
		ERR("too few/many parameters");
		usage();
		exit(1);
	}
	else
		run_cron(*argv);

	destroy_logger();

	return (0);
}

void
usage()
{
	INFO(
		"usage()\n"
		"\t%s [-h | --help]\n"
		"\t%s [-d | --debug] [-l |--log-to=filename] <filename>",
		name, name);
}
