include ../config.mak

SOFILE=libgcin-im-client.so
SOFILEVER=libgcin-im-client.so.1.2.5

.SUFFIXES:      .c .o .E .cpp
WALL=-Wall
CFLAGS= -DUNIX=1 $(WALL) $(OPTFLAGS) -I. -I.. -I../IMdkit/include $(GTKINC) \
        -DCLIENT_LIB=1 -DGCIN_BIN_DIR=\"$(GCIN_BIN_DIR)\" -DUNIX=1 \
        -DDEBUG="0$(GCIN_DEBUG)" -DGCIN_TABLE_DIR=\"$(GCIN_TABLE_DIR)\" \
        -DFREEBSD=$(FREEBSD) -fPIC
OBJS = gcin-im-client.o im-addr.o gcin-conf.o util.o gcin-crypt-fpic.o

.cpp.E:
	$(CC) $(CFLAGS) -E -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) $<
.cpp.o:
	$(CC) -x c -c $(CFLAGS) $<

$(SOFILEVER):   $(OBJS)
	$(CC) $(SO_FLAGS) -Wl,-soname,libgcin-im-client.so.1 $(OBJS) -lX11 -o $@ -L/usr/X11R6/lib
	ln -sf $(SOFILEVER) $(SOFILE)
	ln -sf $(SOFILEVER) $(SOFILE).1
	$(MAKE) -C ../gtk-im

LOCALLIB=/usr/local/$(LIB)

install:
	install -d $(gcinlibdir)
	install -m 755 $(SOFILEVER) $(gcinlibdir)
	cd $(gcinlibdir); rm -f $(SOFILE); ln -s $(SOFILEVER) $(SOFILE); \
	ln -sf $(SOFILEVER) $(SOFILE).1

gcin-conf.o: ../gcin-conf.cpp
	$(CCX) -c $(CFLAGS) -o $@ $<

util.o: ../util.cpp
	$(CCX) -c $(CFLAGS) -o $@ $<

im-addr.o: ../im-addr.cpp
	$(CCX) -c $(CFLAGS) -o $@ $<

gcin-crypt-fpic.o: ../gcin-crypt.cpp
	$(CCX) -c $(CFLAGS) -o $@ $<

clean:
	rm -f *.o *.so *.so.* *~ *.E *.db config.mak tags core.* .depend

.depend:
	$(CCX) $(CFLAGS) -MM *.cpp > $@

include .depend
