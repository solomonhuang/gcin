include ../config.mak

OBJS = imxim.o gtkimcontextxim.o

.SUFFIXES:	.c .o .E
OFLAGS = -g
CFLAGS = $(OFLAG) $(GTKINC) -I../im-client

.c.o:
	$(CC) -c -fpic $(CFLAGS) -o $@ $<
	rm -f core.*

gtk-im.so:	$(OBJS)
	$(CC) -shared $(OBJS) -o $@

clean:
	rm -f *.o *.so *~ core.*

.depend:
	$(CC) $(CFLAGS) -MM *.c > $@

include .depend
