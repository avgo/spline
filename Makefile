
CC = gcc -std=gnu99 -ggdb

export PKG_CONFIG_DEPS = gtk+-2.0

BINS = bezier01

bezier01_OBJECTS = bezier01.o bezier.o

INCLUDE = -I../include $(shell pkg-config --cflags $(PKG_CONFIG_DEPS))

LIBS = -I../include $(shell pkg-config --libs $(PKG_CONFIG_DEPS))




all: $(BINS)

bezier01: $(bezier01_OBJECTS)
	$(CC) -o bezier01 $(bezier01_OBJECTS) $(LIBS)

$(bezier01_OBJECTS): %.o : %.c
	$(CC) $(INCLUDE) -c -o $@ $*.c

clean:
	rm -vf $(BINS) $(bezier01_OBJECTS)
