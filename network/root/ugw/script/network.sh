#!/bin/sh

ps | grep network/main.lua | grep -v grep >/dev/null 
test $? -eq 0 && exit 0

errorfile=/tmp/ugw/log/apmgr.error 

test -d /tmp/ugw/log/ || mkdir -p /tmp/ugw/log/ 
cd /ugw/apps/network/
lua /ugw/apps/network/main.lua >/dev/null 2>>$errorfile

