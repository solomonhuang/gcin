include ../config.mak

OBJS = imgcin.o gtkimcontextgcin.o
GTK2IM=gtk-2.0/immodules
IMMODULES=$(libdir)/$(GTK2IM)
IMMODULES_LOCAL=/usr/$(LIB)/$(GTK2IM)


ifeq ($(MAC_OS),1)
EXTRA_INC=-I/sw/include
endif

.SUFFIXES:	.c .o .E
CFLAGS = $(OPTFLAGS) $(EXTRA_INC) $(GTKINC) -I../im-client -I.. -I../IMdkit/include -DCLIENT_LIB=1

ifeq ($(NEW_GTK_IM),Y)
CFLAGS += -DNEW_GTK_IM=1
endif

.c.o:
	$(CC) -c -fpic $(CFLAGS) -o $@ $<

im-gcin.so:     $(OBJS) ../im-client/libgcin-im-client.so
	$(CC) $(SO_FLAGS) $(OBJS) -L../im-client -lgcin-im-client -o $@ $(LDFLAGS)
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
