#!/bin/bash


# start dev random, then replace 
start_norandom () {
	mkdir -p /usr/lib/preload/
	cp preload_so.so /usr/lib/preload

	sync && insmod norandom.ko
	major=`dmesg | tail -1 | rev | awk '{print $1}' | rev | sed 's/[^0-9]*//g'`

	rm /dev/random
	rm /dev/urandom

	mknod /dev/random c $major 8
	mknod /dev/urandom c $major 9
}

stop_norandom(){
	rm /dev/random
	rm /dev/urandom

	sync && rmmod norandom

	rm /usr/lib/preload/preload_so.so

	mknod /dev/random c 1 8
	mknod /dev/urandom c 1 9
}

if [ "$1" == "start" ]; then
	start_norandom
else
	if [ "$1" == "stop" ]; then
		stop_norandom
	else
		echo "usage: $0 <start|stop>"
	fi
fi
