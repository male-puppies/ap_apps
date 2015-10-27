INCLUDE=-I../lua-5.1.5/src/ 
LIB=-L../lua-5.1.5/src/ -llua -ldl 
TARGET=netp.so
all:$(TARGET)

$(TARGET):netp.cpp
	$(GCC) -fPIC -shared -O2 -o $(TARGET) netp.cpp $(INCLUDE) $(LIB) 
	
clean:
	rm -f *.o *.so $(TARGET)

install:
	cp -f $(TARGET) $(BASE_ROOT)/lib/
	
