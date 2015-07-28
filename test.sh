#!/bin/bash

generate_crontab() {
	rm -f crontab
	(
	cat crontab.default
	min="`date +%M`"
	min1="`expr \( $min + 1 \) % 60`"
	echo "$min * * * * yes > /dev/null"
	echo "$min1 * * * * killall yes"
	) >> crontab
}
run() {
	make run
}
kill_all() {
	echo kill_all $$
	ps -e -o ppid= -o pid= | grep "$$ " 
	ps -e -o ppid= -o pid=
	ps -e -o ppid= -o pid= | grep "$$ " |
	while read pid pid
	do
		echo kill $pid
		kill $pid
	done
}
terminate() {
	TIME=2
	sleep $TIME && kill_all &
}

generate_crontab
run&
terminate
