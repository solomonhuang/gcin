include ../config.mak

.SUFFIXES:      .c .o .E
WALL=-Wall
CFLAGS= $(WALL) $(OPTFLAGS) -I.. -I../im-client  -I../IMdkit/include $(GTKINC) -DDEBUG="0$(GCIN_DEBUG)"
OBJS = im-srv.o im-addr.o im-dispatch.o

im-srv.a:	$(OBJS)
	ar cr $@ $(OBJS)
	ranlib $@

clean:
	rm -f *.a *.o *~ *.E *.db config.mak tags core.* .depend

.depend:
	$(CC) $(CFLAGS) -MM *.c > $@

include .depend
