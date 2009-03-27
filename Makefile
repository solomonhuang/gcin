OPTFLAGS=-g

include config.mak

.SUFFIXES:	.c .o .E

OBJS=gcin.o IC.o eve.o win0.o pho.o tsin.o win1.o util.o pho-util.o gcin-conf.o tsin-util.o \
     win-sym.o intcode.o pho-sym.o win-int.o win-pho.o gcin-settings.o table-update.o win-gtab.o \
     gtab.o gtab-util.o phrase.o win-inmd-switch.o pho-dbg.o locale.o win-pho-near.o
OBJS_TSLEARN=tslearn.o util.o gcin-conf.o pho-util.o tsin-util.o gcin-send.o pho-sym.o \
             table-update.o locale.o gcin-settings.o
OBJS_phod2a=phod2a.o pho-util.o gcin-conf.o pho-sym.o table-update.o pho-dbg.o locale.o \
             gcin-settings.o util.o
OBJS_tsa2d=tsa2d.o gcin-send.o util.o pho-sym.o gcin-conf.o locale.o
OBJS_phoa2d=phoa2d.o pho-sym.o gcin-send.o gcin-conf.o locale.o
OBJS_kbmcv=kbmcv.o pho-sym.o util.o locale.o
OBJS_tsd2a=tsd2a.o pho-sym.o pho-dbg.o locale.o util.o
OBJS_gcin2tab=gcin2tab.o gtab-util.o util.o locale.o
OBJS_gcin_steup=gcin-setup.o gcin-conf.o util.o gcin-send.o gcin-settings.o gcin-setup-list.o \
locale.o gcin-setup-pho.o about.o
WALL=-Wall
CFLAGS= $(WALL) $(OPTFLAGS) $(GTKINC) -I./IMdkit/include -DDEBUG="0$(GCIN_DEBUG)" \
        -DGCIN_TABLE_DIR=\"$(GCIN_TABLE_DIR)\"  -DDOC_DIR=\"$(DOC_DIR)\" \
        -DGCIN_ICON_DIR=\"$(GCIN_ICON_DIR)\" -DGCIN_VERSION=\"$(GCIN_VERSION)\" \
        -DGCIN_SCRIPT_DIR=\"$(GCIN_SCRIPT_DIR)\" -DGCIN_BIN_DIR=\"$(GCIN_BIN_DIR)\" \
        -DSYS_ICON_DIR=\"$(SYS_ICON_DIR)\"

IMdkitLIB = IMdkit/lib/libXimd.a
im-srv = im-srv/im-srv.a

.c.E:
	$(CC) $(CFLAGS) -E -o $@ $<

PROGS=gcin tsd2a tsa2d phoa2d phod2a tslearn gcin-setup gcin2tab
PROGS_CV=kbmcv

all:	$(PROGS) $(DATA) $(PROGS_CV) gcin.spec
	$(MAKE) -C data
	$(MAKE) -C im-client
	$(MAKE) -C gtk-im

gcin:   $(OBJS) $(IMdkitLIB) $(im-srv)
	$(CC) -o $@ $(OBJS) $(IMdkitLIB) $(im-srv) -lXtst $(LDFLAGS) -L/usr/X11R6/lib
	rm -f core.*
	ln -sf $@ $@.test

tslearn:        $(OBJS_TSLEARN)
	$(CC) -o $@ $(OBJS_TSLEARN) $(LDFLAGS)

gcin-setup:     $(OBJS_gcin_steup)
	$(CC) -o $@ $(OBJS_gcin_steup) $(LDFLAGS)

phoa2d: $(OBJS_phoa2d)
	$(CC) -o $@ $(OBJS_phoa2d) $(LDFLAGS)

phod2a: $(OBJS_phod2a)
	$(CC) -o $@ $(OBJS_phod2a) $(LDFLAGS)

tsa2d:  $(OBJS_tsa2d)
	$(CC) -o $@ $(OBJS_tsa2d) $(LDFLAGS)

tsd2a:  $(OBJS_tsd2a)
	$(CC) -o $@ $(OBJS_tsd2a) $(LDFLAGS)

gcin2tab:  $(OBJS_gcin2tab)
	$(CC) -o $@ $(OBJS_gcin2tab) $(LDFLAGS)

kbmcv:  $(OBJS_kbmcv)
	$(CC) -o $@ $(OBJS_kbmcv) $(LDFLAGS)

$(IMdkitLIB):
	$(MAKE) -C IMdkit/lib

$(im-srv):
	$(MAKE) -C im-srv

ibin:
	   install $(PROGS) $(bindir)

install:
	install -d $(datadir)/icons
	install gcin.png $(datadir)/icons
	install -d $(GCIN_ICON_DIR_i)
	install -m 644 icons/* $(GCIN_ICON_DIR_i)
	install -d $(bindir)
	$(MAKE) -C data install
	$(MAKE) -C im-client install
	$(MAKE) -C gtk-im install
	if [ $(prefix) = /usr/local ]; then \
	   install -m 644 gcin.png /usr/share/icons; \
	   install -d $(DOC_DIR); \
	   install -m 644 README Changelog $(DOC_DIR); \
	   install $(PROGS) $(bindir); \
	else \
	   install -d $(DOC_DIR_i); \
	   install -m 644 README Changelog $(DOC_DIR_i); \
	   install -s $(PROGS) $(bindir); \
	fi
	$(MAKE) -C scripts install
	$(MAKE) -C menu install
clean:
	$(MAKE) -C IMdkit clean
	$(MAKE) -C data clean
	$(MAKE) -C scripts clean
	$(MAKE) -C im-srv clean
	$(MAKE) -C im-client clean
	$(MAKE) -C gtk-im clean
	rm -f *.o *~ *.E *.db config.mak tags core.* $(PROGS) $(PROGS_CV) $(DATA) .depend gcin.spec menu/*~

.depend:
	$(CC) $(CFLAGS) -MM *.c > $@

config.mak: VERSION.gcin configure
	./configure

gcin.spec:	gcin.spec.in
	rm -f $@
	sed -e "s/__gcin_version__/$(GCIN_VERSION)/" < $< > $@

include .depend
