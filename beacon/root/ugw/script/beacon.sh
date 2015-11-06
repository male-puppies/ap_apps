#!/bin/sh

ps | grep beacon/main.lua | grep -v grep >/dev/null 
test $? -eq 0 && exit 0

errorfile=/tmp/ugw/log/apmgr.error 

test -d /tmp/ugw/log/ || mkdir -p /tmp/ugw/log/ 
cd /ugw/apps/beacon/

while :; do 
	echo "`date` `uptime` start beacon" >>$errorfile
	lua /ugw/apps/beacon/main.lua >/dev/null 2>>$errorfile
	sleep 2
done
