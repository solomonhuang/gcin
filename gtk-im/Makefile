include ../config.mak

OBJS = imgcin.o gtkimcontextgcin.o
GTK2IM=gtk-2.0/immodules
IMMODULES=$(libdir)/$(GTK2IM)
IMMODULES_LOCAL=/usr/lib/$(GTK2IM)

.SUFFIXES:	.c .o .E
CFLAGS = $(OPTFLAGS) $(GTKINC) -I../im-client -I.. -I../IMdkit/include -DCLIENT_LIB=1

.c.o:
	$(CC) -c -fpic $(CFLAGS) -o $@ $<

im-gcin.so:     $(OBJS) ../im-client/libgcin-im-client.so
	$(CC) -shared $(OBJS) -L../im-client -lgcin-im-client -o $@ $(LDFLAGS)
	rm -f core.*

clean:
	rm -f *.o *.so *~ core.* .depend

install:
	if [ $(prefix) = /usr/local ]; then \
		install -d $(IMMODULES_LOCAL); \
		install -m 755 im-gcin.so $(IMMODULES_LOCAL); \
	else \
		install -d $(IMMODULES); \
		install -m 755 im-gcin.so $(IMMODULES); \
	fi

.depend:
	$(CC) $(CFLAGS) -MM *.c > $@

include .depend
