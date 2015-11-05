LUA_INCLUDE=-I$(LUA_INC) 
LUA_LIB=-L$(LUA_SO_DIR) -llua53
all: 
	$(CC) -O2 -fPIC -shared -I../../lua53/lua-5.3.1/src/ *.c -o parser53.so $(LUA_LIB)
clean:
	$(RM) -f *.so