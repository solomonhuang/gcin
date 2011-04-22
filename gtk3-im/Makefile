include ../config.mak

OBJS = imgcin.o gtkimcontextgcin.o
GTK3IM=gtk-3.0/immodules
GTKINC=`pkg-config --cflags gtk+-3.0`
LDFLAGS=`pkg-config --libs gtk+-3.0`
IMMODULES=$(libdir)/$(GTK3IM)
IMMODULES_LOCAL=/usr/$(LIB)/$(GTK3IM)


ifeq ($(MAC_OS),1)
EXTRA_INC=-I/sw/include
endif

.SUFFIXES:	.c .o .E
CFLAGS = $(OPTFLAGS) $(EXTRA_INC) $(GTKINC) -I../im-client -I.. -DUNIX=1 \
-I../IMdkit/include -DCLIENT_LIB=1 -DMAC_OS=$(MAC_OS) -DFREEBSD=$(FREEBSD)

.c.o:
	$(CC) -c -fPIC $(CFLAGS) -o $@ $<

.c.E:
	$(CC) -E $(CFLAGS) -o $@ $<

all:	im-gcin.so im-gcin-cur.so

im-gcin.so:     $(OBJS) ../im-client/libgcin-im-client.so
	$(CC) $(gcc_ld_run_path) $(SO_FLAGS) $(OBJS) -L../im-client -lgcin-im-client -o $@ $(LDFLAGS)
	rm -f core.*

im-gcin-cur.so:     $(OBJS) ../im-client/libgcin-im-client.so
	$(CC) $(SO_FLAGS) $(OBJS) -L../im-client -lgcin-im-client -o $@ $(LDFLAGS)
	rm -f core.*

clean:
	rm -f *.o *.so *~ core.* .depend *.E

install:
	if [ $(prefix) = /usr/local ]; then \
		install -d $(IMMODULES_LOCAL); \
		install -m 755 im-gcin.so $(IMMODULES_LOCAL); \
		gtk-query-immodules-3.0 --update-cache; \
	else \
		install -d $(IMMODULES); \
		install -m 755 im-gcin.so $(IMMODULES); \
	fi

.depend:
	$(CC) $(CFLAGS) -MM *.c > $@

include .depend
