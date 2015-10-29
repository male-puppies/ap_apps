include $(ROOTDIR)/BaseVar.mk

PROG = websrv
LDFLAGS += -lm -L$(ROOTDIR)/src/lib/mosquitto-1.4.2/lib
CFLAGS += -D_BSD_SOURCE -W -Wall -pthread -O0 $(CFLAGS_EXTRA) -I$(ROOTDIR)/src/lib/mosquitto-1.4.2/lib

WEBSRV = httpd
WEBSRV_OBJS = websrv.o mongoose.o cJSON.o

AUTHD = authd
AUTHD_OBJS = auth_main.o auth_user.o auth_misc.o auth_local_server.o auth_mqtt.o auth_user_request_queue.o auth_center.o auth_client.o auth_utils.o cJSON.o

all: $(WEBSRV) $(AUTHD)

$(WEBSRV): $(WEBSRV_OBJS)
	$(CC) $(WEBSRV_OBJS) -o $(WEBSRV) $(LDFLAGS)
	$(STRIP) $(WEBSRV)

$(AUTHD): $(AUTHD_OBJS)
	$(CC) $(AUTHD_OBJS) -o $(AUTHD) $(LDFLAGS) -lmosquitto
	$(STRIP) $(AUTHD)

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(WEBSRV) $(AUTHD) *~ *.o *.core
