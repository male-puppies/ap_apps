src = ap_ioctl.c
all:libapioctl apioctl
libapioctl: 
	$(CC) -fpic -shared $(src) -o libapioctl.so -O2 
	
apioctl:libapioctl
	$(CC) -fpic -shared luawrap.c -o apioctl.so -I$(LUA_INC) -L $(LUA_INC) -llua -I . -L.  -lapioctl -O2
	
main:
	$(CC) main.c -L ./ -lapioctl -o main

.PHONY : clean
clean:
	rm -rf *.so
