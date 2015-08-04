*********************************************************
*			CRONTAB				*
*********************************************************

Project implements subset of crontab functionality.

Compilation of project can be done by running `make build`. Project has no dependencies.

usage:
	-h | --help:	prints help message
	-d | --debug:	program will print debug messages
	-l <file> | --log-to=file:
			prints log messages to file, too. Can be used 0+ times.
	 <filename>:	last argument is crontab config file,
	 			where commands are stored.

syntax:
	all blank and commented lines are ignored
	variables look like VAR_NAME = VAR_CONTENT, where previous variables can be used in VAR_CONTENT like:
		VAR1 = a
		VAR2 = $VAR1
		=> VAR1 == a && VAR2 == a.
	command lines has 5 time/date fields followed by a command:
		field		allowed value
		-----		-------------
		minute		0-59
		hour		0-23
		day of month	0-30
		month		0-11 (Jan-)
		day of week	0-6 (Sun-)
			^^ allowed value for all fields is asterisk '*',
				which stands for any-value.
		command		command for shell

execution:
	commands are executed when the minute, hour and month fields match the current time AND when at least one of the two day fields (day of month/week) match.
	commands will be run in shell like: `/bin/bash -c 'command'`.
	cron examines config every minute.

example:
	run:
	$ ./mycrontab --log-to=mycrontab.log crontab.conf
	$ cat crontab.conf
		LOG = /tmp/crontab.log
		# use /bin/bash to run commands instead of the default /bin/sh
		# run every hour
		0 * * * *	$HOME/bin/daily.job	>> /tmp/jobs
		# run every week at sunday midnight
		0 0 * * 0	$HOME/bin/weekly.job	>> /tmp/jobs
		# run every sunday january and at 1.january at 0:00
		0 0 0 0 0	$HOME/bin/other.job	>> /tmp/jobs
