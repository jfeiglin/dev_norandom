#!/bin/bash


# start dev random, then replace 
start_norandom () {
	mkdir -p /usr/lib/preload/
	cp preload_so.so /usr/lib/preload
	sync && insmod norandom.ko
}

stop_norandom(){
	sync && rmmod norandom
	rm /usr/lib/preload/preload_so.so
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
