JSON_SOURCE=lua_cjson.c strbuf.c
JSON_SOURCE+=fpconv.c
	
all: 
	$(CC) $(JSON_SOURCE) -fPIC -shared -o cjson53.so -I../../lua53/lua-5.3.1/src/ -L$(LUA53_SO_DIR) -llua53 
clean:
	$(RM) -f *.so