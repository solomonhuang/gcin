include ../config.mak


SOFILE=libgcin-im-client.so
SOFILEVER=libgcin-im-client.so.1.1.0

.SUFFIXES:      .c .o .E
WALL=-Wall
CFLAGS= $(WALL) $(OPTFLAGS) -I. -I.. -I../IMdkit/include $(GTKINC) \
        -DCLIENT_LIB=1 -DGCIN_BIN_DIR=\"$(GCIN_BIN_DIR)\" \
        -DDEBUG="0$(GCIN_DEBUG)" -DGCIN_TABLE_DIR=\"$(GCIN_TABLE_DIR)\" \
        -DFREEBSD=$(FREEBSD) -fPIC
OBJS = gcin-im-client.o im-addr.o gcin-conf.o util.o gcin-crypt-fpic.o

.c.E:
	$(CC) $(CFLAGS) -E -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

$(SOFILEVER):   $(OBJS)
	$(CC) $(SO_FLAGS) -Wl,-soname,libgcin-im-client.so.1 $(OBJS) -o $@
	ln -sf $(SOFILEVER) $(SOFILE)
	ln -sf $(SOFILEVER) $(SOFILE).1
	$(MAKE) -C ../gtk-im

LOCALLIB=/usr/local/$(LIB)

install:
	install -d $(gcinlibdir)
	install -m 755 $(SOFILEVER) $(gcinlibdir)
	cd $(gcinlibdir); rm -f $(SOFILE); ln -s $(SOFILEVER) $(SOFILE); \
	ln -sf $(SOFILEVER) $(SOFILE).1
	install -d $(includedir)
	install -m 444 gcin-im-client.h $(includedir)

gcin-conf.o: ../gcin-conf.c
	$(CC) -c $(CFLAGS) -o $@ $<

util.o: ../util.c
	$(CC) -c $(CFLAGS) -o $@ $<

im-addr.o: ../im-srv/im-addr.c
	$(CC) -c $(CFLAGS) -o $@ $<

gcin-crypt-fpic.o: gcin-crypt.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -f *.o *.so *.so.* *~ *.E *.db config.mak tags core.* .depend

.depend:
	$(CC) $(CFLAGS) -MM *.c > $@

include .depend
