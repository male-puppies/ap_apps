all:
	$(CC) *.c -I../../lua53/lua-5.3.1/src/ -L$(LUA_SO_DIR) -llua53 -I../../mosquitto14/mosquitto-1.4.2/lib/ -L $(MQTT_SO_DIR) -lmosquitto -lm -fPIC -shared -o mosquitto.so