#!/bin/sh

ps | grep 'mosquitto -c' | grep -v grep >/dev/null 
test $? -eq 0 && exit 0

errorfile=/tmp/ugw/log/apmgr.error 

test -d /tmp/ugw/log/ || mkdir -p /tmp/ugw/log/  

while :; do 
	echo "`date` `uptime` start mosquitto" >>$errorfile
	mosquitto -c /etc/config/mosquitto.conf
	sleep 2
done

