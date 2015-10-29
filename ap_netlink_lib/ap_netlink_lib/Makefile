src = ap_netlink.c

all:libapnetlink test_netlink

libapnetlink: 
	$(CC) -fpic -shared $(src) -o libapnetlink.so
	
test_netlink:
	$(CC) msg_proc.c -L ./ -lapnetlink -o test_netlink

.PHONY : clean

clean:
	rm -rf *.so test_netlink
