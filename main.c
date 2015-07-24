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
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "conf.h"
#include "crontab.h"
#include "logger.hpp"
#include "utils.h"

#define	CRONTAB_CONFIG	"crontab.conf"

char *name = NULL;

int
main(int argc, char **argv)
{
	int opts;

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
