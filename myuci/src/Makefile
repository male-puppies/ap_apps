all: 
	$(CC) *.c -fPIC -shared -o uci53.so -I../../lua53/lua-5.3.1/src/ -L$(LUA53_SO_DIR) -llua53 -I.. -L.. -luci
clean:
	$(RM) -f *.so