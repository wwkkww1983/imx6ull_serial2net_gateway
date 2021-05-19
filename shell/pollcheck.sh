#!/bin/sh

cd `dirname $0`

destIP=`sqlite3 ../SQLiteDB/olcheck.db << tag
select addr from destaddr;
.quit
tag`

while true
	do
	./checkol.sh $destIP >/dev/null 2>&1 #desti_addr
	sleep 30
	done
