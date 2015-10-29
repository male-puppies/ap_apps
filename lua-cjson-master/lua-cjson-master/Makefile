JSON_SOURCE=lua_cjson.c strbuf.c
JSON_SOURCE+=fpconv.c
	
all: 
	$(CC) $(JSON_SOURCE) -fPIC -shared -o cjson.so -I../../lua51/lua-5.1.5/src/ -L$(LUA53_SO_DIR) -llua51 -D BY_LUA_51=1
	$(CC) $(JSON_SOURCE) -fPIC -shared -o cjson.so -I$(LUA_INC) -L$(LUA_INC) -llua 