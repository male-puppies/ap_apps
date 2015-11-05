all:
	$(CC) -fPIC -shared *.c -o lfs53.so -I../../../lua53/lua-5.3.1/src/ -L$(LUA_SO_DIR) -llua53 -lm -ldl
clean:
	$(RM) -f *.so 