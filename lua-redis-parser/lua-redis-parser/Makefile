LUA_INCLUDE=-I$(LUA_INC) 
LUA_LIB=-L$(LUA_INC) -llua
all: 
	$(CC) -O2 -fPIC -shared $(LUA_INCLUDE) *.c -o parser.so $(LUA_LIB)
