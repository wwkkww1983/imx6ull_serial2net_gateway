#!/bin/sh

kill_process()
{
        if [ -n "$(pidof $1)" ] ; then      
        echo "kill $(pidof $1)" 
        kill $(pidof $1)
        fi      
}

open_exe()
{
        if [ -z "$(pidof $1)" ]; then
	#cmd
        $2 &
        echo "open $(pidof $1)"
        fi
}

cd `dirname $0`

sleep 1

kill_process tcp_pthrgh

sleep 1

open_exe tcp_pthrgh ../bin/tcp_pthrgh


exit

