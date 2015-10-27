src = ap_ioctl.c
all:libapioctl apioctl
libapioctl: 
	$(CC) -fpic -shared $(src) -o libapioctl.so -O2 
	
apioctl:libapioctl
	$(CC) -fpic -shared luawrap.c -o apioctl.so -I../lua-5.1.5/src/ -L ../lua-5.1.5/src/ -I . -L. -llua -lapioctl -O2
	
main:
	$(CC) main.c -L ./ -lapioctl -o main

.PHONY : clean
clean:
	rm -rf *.so
