src = lib_commit.c commit_to_driver.c config_parse.c radio.c vap.c ap_ioctl.c apcommit.c dump.c custom_utility.c

all:libapcommit 

libapcommit: 
	$(CC) -g -fpic -shared  $(src)   -o apcommit.so -I../lua-5.1.5/src/ -L../lua-5.1.5/src/ 
	$(STRIP) apcommit.so
.PHONY : clean

clean:
	rm -rf *.so 
