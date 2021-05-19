#!/bin/sh

cd `dirname $0`

while true
	do
	if $([ -n "$(pidof tcp_pthrgh)" ] || [ -n "$(pidof udp_pthrgh)" ] || \
	     [ -n "$(pidof modbusTCPRTU)" ]) ; then
		sqlite3 ../SQLiteDB/nettransconfig.db << taga
		update configs set status = 1;
		.quit
taga
		echo "set dtustatus 1"

	else
		sqlite3 ../SQLiteDB/nettransconfig.db << tagb
		update configs set status = 0;
		.quit
tagb
		echo "set dtustatus 0"

	fi

	sleep 1

	done

