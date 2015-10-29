all:
	#$(CC) *.c -I$(LUA_INC) -L$(LUA_INC) -llua -I$(MQTT_INC) -L$(MQTT_INC) -lmosquitto -lm -fPIC -shared -o mosquitto.so
	$(CC) *.c -I$(LUA_INC) -L$(LUA_INC) -llua -I./mosquitto-1.4.2/lib -L$(MQTT_INC) -lmosquitto -lm -fPIC -shared -o mosquitto.so