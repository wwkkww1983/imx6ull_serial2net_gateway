#!/bin/sh

cd `dirname $0`
eth0IP=`sqlite3 ../SQLiteDB/ifconfig.db << tagA
select eth0 from Ipaddr;
.quit
tagA`
eth1IP=`sqlite3 ../SQLiteDB/ifconfig.db << tagB
select eth1 from Ipaddr;
.quit
tagB`

ifconfig eth0 $eth0IP up

ifconfig eth1 $eth1IP up

###4G模块拨号###
rm /var/lock/LCK..ttyUSB2
cd /etc/ppp
./pppd call wcdma &
sleep 7 
################

cd `dirname $0`

./pollcheck.sh &

./dtustatus.sh  >/dev/null 2>&1 &

/etc/boa/boa
sleep 5

