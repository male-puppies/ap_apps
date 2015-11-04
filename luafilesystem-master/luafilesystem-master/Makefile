# $Id: Makefile,v 1.36 2009/09/21 17:02:44 mascarenhas Exp $

T= lfs

CONFIG= ./config

include $(CONFIG)
 
SRCS= src/$T.c

lib: src/lfs.so

src/lfs.so: 
	MACOSX_DEPLOYMENT_TARGET="10.3"; export MACOSX_DEPLOYMENT_TARGET; 
	$(CC) $(SRCS) -shared  -fPIC -I$(LUA_INC) -L$(LUA_SO_DIR) -llua53 -lm -ldl -o src/lfs.so 

test: lib
	LUA_CPATH=./src/?.so lua tests/test.lua

install:
	mkdir -p $(LUA_LIBDIR)
	cp src/lfs.so $(LUA_LIBDIR)

clean:
	rm -f src/lfs.so
