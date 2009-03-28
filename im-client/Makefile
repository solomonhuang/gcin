include ../config.mak

.SUFFIXES:      .c .o .E
WALL=-Wall
CFLAGS= $(WALL) $(OPTFLAGS) -I. -I.. -I../IMdkit/include $(GTKINC) \
        -DCLIENT_LIB=1 -DGCIN_BIN_DIR=\"$(GCIN_BIN_DIR)\" \
        -DDEBUG="0$(GCIN_DEBUG)" -DGCIN_TABLE_DIR=\"$(GCIN_TABLE_DIR)\" \
        -DFREEBSD=$(FREEBSD)
OBJS = gcin-im-client.o im-addr.o gcin-conf.o util.o gcin-crypt.o

.c.E:
	$(CC) $(CFLAGS) -E -o $@ $<

.c.o:
	$(CC) -c -fpic $(CFLAGS) -o $@ $<

libgcin-im-client.so:   $(OBJS)
	$(CC) $(SO_FLAGS) $(OBJS) -o $@
	$(MAKE) -C ../gtk-im

LOCALLIB=/usr/local/$(LIB)

install:
	install -d $(libdir)
	if [ $(prefix) = /usr/local ]; then \
	  ! [ -d $(LOCALLIB) ] && mkdir $(LOCALLIB) ; \
	  install -m 755 libgcin-im-client.so $(LOCALLIB); \
	  grep $(LOCALLIB) /etc/ld.so.conf > /dev/null 2>&1 || \
	  echo $(LOCALLIB) >> /etc/ld.so.conf; \
	  PATH=/sbin:/usr/sbin:$(PATH); ldconfig; \
	else \
	  install -m 755 libgcin-im-client.so $(libdir); \
	fi; \
	install -d $(includedir); \
	install -m 444 gcin-im-client.h $(includedir)

gcin-conf.o: ../gcin-conf.c
	$(CC) -c -fpic $(CFLAGS) -o $@ $<

util.o: ../util.c
	$(CC) -c -fpic $(CFLAGS) -o $@ $<

im-addr.o: ../im-srv/im-addr.c
	$(CC) -c -fpic $(CFLAGS) -o $@ $<

clean:
	rm -f *.o *.so *~ *.E *.db config.mak tags core.* .depend

.depend:
	$(CC) $(CFLAGS) -MM *.c > $@

include .depend
