#!/bin/sh

ps | grep mqttpub/main.lua | grep -v grep >/dev/null 
test $? -eq 0 && exit 0

errorfile=/tmp/ugw/log/apmgr.error 

test -d /tmp/ugw/log/ || mkdir -p /tmp/ugw/log/ 
cd /ugw/apps/mqttpub/
lua /ugw/apps/mqttpub/main.lua >/dev/null 2>>$errorfile

