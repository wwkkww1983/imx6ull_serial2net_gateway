#!/bin/sh

pingret="$(ping $1 -c 4)"
pingret_1="$(echo $pingret | grep "4 packets transmitted, 4 packets received, 0% packet loss")"
if test "$pingret_1" ; then
	
	echo 1 > /sys/class/leds/sys-led/brightness
	
else
	
	echo 0 > /sys/class/leds/sys-led/brightness
	
fi
