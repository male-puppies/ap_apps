LUA_INCLUDE=-I../lua-5.1.5/src
LUA_LIB=-L../lua-5.1.5/src -llua 
all: 
	$(CC) -O2 -fPIC -shared $(LUA_INCLUDE) *.c -o parser.so $(LUA_LIB)
