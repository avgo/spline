
CC = gcc -std=gnu99 -ggdb

export PKG_CONFIG_DEPS = gtk+-2.0

BINS = bezier

bezier_OBJECTS = bezier.o

INCLUDE = -I../include $(shell pkg-config --cflags $(PKG_CONFIG_DEPS))

LIBS = -I../include $(shell pkg-config --libs $(PKG_CONFIG_DEPS))




all: $(BINS)

bezier: $(bezier_OBJECTS)
	$(CC) -o bezier $(bezier_OBJECTS) $(LIBS)

$(bezier_OBJECTS): %.o : %.c
	$(CC) $(INCLUDE) -c -o $@ $*.c

clean:
	rm -vf $(BINS) $(bezier_OBJECTS)
