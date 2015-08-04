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
generate_crontab
run
