include ../config.mak

.SUFFIXES:      .c .o .E
WALL=-Wall
CFLAGS= $(WALL) $(OPTFLAGS) -I.. -I../IMdkit/include $(GTKINC) -DDEBUG="0$(GCIN_DEBUG)" -DGCIN_TABLE_DIR=\"$(GCIN_TABLE_DIR)\"
OBJS = gcin-im-client.o ../im-srv/im-addr.o gcin-conf.o util.o

.c.o:
	$(CC) -c -fpic $(CFLAGS) -o $@ $<

gcin-im-client.so:	$(OBJS)
	$(CC) -shared $(OBJS) -o $@


gcin-conf.o: ../gcin-conf.c
	$(CC) -c -fpic $(CFLAGS) -o $@ $<

util.o: ../util.c
	$(CC) -c -fpic $(CFLAGS) -o $@ $<

clean:
	rm -f *.o *.so *~ *.E *.db config.mak tags core.* .depend

.depend:
	$(CC) $(CFLAGS) -MM *.c > $@

include .depend
