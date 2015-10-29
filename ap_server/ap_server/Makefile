#include $(ROOTDIR)/BaseVar.mk

PROG = websrv
LDFLAGS += -lm -L$(LUA_LIB)
CFLAGS += -D_BSD_SOURCE -W -Wall -pthread -O0 $(CFLAGS_EXTRA) -I../mosquitto-1.4.2/lib/

WEBSRV = httpd
WEBSRV_OBJS = websrv.o mongoose.o cJSON.o

AUTHD = authd
AUTHD_OBJS = auth_main.o auth_user.o auth_misc.o auth_local_server.o auth_mqtt.o auth_user_request_queue.o auth_center.o auth_client.o auth_utils.o cJSON.o

all: $(WEBSRV) $(AUTHD)

$(WEBSRV): $(WEBSRV_OBJS)
	$(CC) $(WEBSRV_OBJS) -o $(WEBSRV) $(LDFLAGS) -I../mosquitto-1.4.2/lib/
	$(STRIP) $(WEBSRV)

$(AUTHD): $(AUTHD_OBJS)
	$(CC) $(AUTHD_OBJS) -o $(AUTHD) $(LDFLAGS) -I../mosquitto-1.4.2/lib/ -L$(MQTT_LIB) -lmosquitto
	$(STRIP) $(AUTHD)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(WEBSRV) $(AUTHD) *~ *.o *.core
