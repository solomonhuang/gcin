OPTFLAGS=-g

include config.mak

.SUFFIXES:	.c .o .E .pico

gcin_tsin_o = tsin.o tsin-util.o win0.o win1.o tsin-parse.o
gcin_pho_o = win-pho.o pho.o pho-util.o pho-sym.o table-update.o pho-dbg.o
gcin_gtab_o = gtab.o win-gtab.o gtab-util.o gtab-list.o gtab-buf.o

GCIN_SO= gcin1.so gcin2.so

OBJS=gcin.o eve.o util.o gcin-conf.o gcin-settings.o locale.o gcin-icon.o \
     gcin-switch.o gcin-exec-script.o $(GCIN_SO) pho-play.o \
     $(gcin_pho_o) $(gcin_gtab_o) gcin-common.o

OBJS_TSLEARN=tslearn.o util.o gcin-conf.o pho-util.o tsin-util.o gcin-send.o pho-sym.o \
             table-update.o locale.o gcin-settings.o gcin-common.o
OBJS_JUYIN_LEARN=juyin-learn.o locale.o util.o pho-util.o pho-sym.o \
                 gcin-settings.o gcin-conf.o table-update.o pinyin.o
OBJS_sim2trad=sim2trad.o util.o gcin2.so locale.o gcin-conf.o
OBJS_phod2a=phod2a.o pho-util.o gcin-conf.o pho-sym.o table-update.o pho-dbg.o locale.o \
             gcin-settings.o util.o
OBJS_tsa2d32=tsa2d32.o gcin-send.o util.o pho-sym.o gcin-conf.o locale.o pho-lookup.o
OBJS_phoa2d=phoa2d.o pho-sym.o gcin-send.o gcin-conf.o locale.o pho-lookup.o
OBJS_kbmcv=kbmcv.o pho-sym.o util.o locale.o
OBJS_tsd2a=tsd2a.o pho-sym.o pho-dbg.o locale.o util.o
OBJS_tsd2a32=tsd2a32.o pho-sym.o pho-dbg.o locale.o util.o
OBJS_gcin2tab=gcin2tab.o gtab-util.o util.o locale.o
OBJS_gtab_merge=gtab-merge.o gtab-util.o util.o locale.o
OBJS_gcin_steup=gcin-setup.o gcin-conf.o util.o gcin-send.o gcin-settings.o \
	gcin-setup-list.o gcin-switch.o locale.o gcin-setup-pho.o about.o \
	gcin-icon.o gcin-setup-gtab.o gtab-list.o gcin-exec-script.o
OBJS_gcin_setup_tab=gcin-setup-tab.o gcin-conf.o util.o gcin-send.o gcin-settings.o \
	gcin-switch.o about.o gcin-icon.o gtab-list.o gcin-exec-script.o

OBJS_gcin_gb_toggle = gcin-gb-toggle.o gcin-conf.o util.o gcin-send.o
OBJS_gcin_kbm_toggle = gcin-kbm-toggle.o gcin-conf.o util.o gcin-send.o
OBJS_gcin_message = gcin-message.o gcin-conf.o util.o gcin-send.o
OBJS_pin_juyin = pin-juyin.o util.o pho-lookup.o locale.o pho-sym.o


#WALL=-Wall
CFLAGS= $(WALL) $(OPTFLAGS) $(GTKINC) -I./IMdkit/include -DDEBUG="0$(GCIN_DEBUG)" \
        -DGCIN_TABLE_DIR=\"$(GCIN_TABLE_DIR)\" \
        -DGCIN_OGG_DIR=\"$(GCIN_OGG_DIR)\" \
        -DDOC_DIR=\"$(DOC_DIR)\" \
        -DGCIN_ICON_DIR=\"$(GCIN_ICON_DIR)\" -DGCIN_VERSION=\"$(GCIN_VERSION)\" \
        -DGCIN_SCRIPT_DIR=\"$(GCIN_SCRIPT_DIR)\" -DGCIN_BIN_DIR=\"$(GCIN_BIN_DIR)\" \
        -DSYS_ICON_DIR=\"$(SYS_ICON_DIR)\" -DFREEBSD=$(FREEBSD) -DMAC_OS=$(MAC_OS)
ifeq ($(USE_XIM),Y)
IMdkitLIB = IMdkit/lib/libXimd.a
CFLAGS += -DUSE_XIM=1
OBJS+=IC.o
endif

ifeq ($(MAC_OS),1)
EXTRA_LDFLAGS=-bind_at_load
endif

ifeq ($(USE_TRAY),Y)
CFLAGS += -DTRAY_ENABLED=1
OBJS += tray.o eggtrayicon.o
endif

ifeq ($(USE_I18N),Y)
CFLAGS += -DGCIN_i18n_message=1
endif

ifeq ($(USE_TSIN),Y)
CFLAGS += -DUSE_TSIN=1
OBJS += $(gcin_tsin_o)
endif

im-srv = im-srv/im-srv.a

.c.E:
	$(CC) $(CFLAGS) -E -o $@ $<
.c.pico:
	$(CC) $(CFLAGS) -c -fpic -o $@ $<

PROGS=gcin tsd2a tsd2a32 tsa2d32 phoa2d phod2a tslearn gcin-setup gcin2tab \
	juyin-learn sim2trad gcin-gb-toggle gcin-message gtab-merge gcin-setup-tab \
	gcin-kbm-toggle
PROGS_SYM=trad2sim
PROGS_CV=kbmcv pin-juyin

all:	$(PROGS) trad2sim $(DATA) $(PROGS_CV) gcin.spec
	$(MAKE) -C data
	$(MAKE) -C im-client
	$(MAKE) -C gtk-im
ifeq ($(USE_I18N),Y)
	$(MAKE) -C po
endif
	if [ $(QT_IM) = 'Y' ]; then $(MAKE) -C qt-im; fi

gcin:   $(OBJS) $(IMdkitLIB) $(im-srv)
	LD_RUN_PATH=.:$(gcinlibdir) \
	$(CC) $(EXTRA_LDFLAGS) -o $@ $(OBJS) $(IMdkitLIB) $(im-srv) -lXtst $(LDFLAGS) -L/usr/X11R6/lib
	rm -f core.*
	ln -sf $@ $@.test

tslearn:        $(OBJS_TSLEARN)
	$(CC) -o $@ $(OBJS_TSLEARN) $(LDFLAGS)

juyin-learn:        $(OBJS_JUYIN_LEARN)
	$(CC) -o $@ $(OBJS_JUYIN_LEARN) $(LDFLAGS)
	rm -f core.*
sim2trad:        $(OBJS_sim2trad)
	LD_RUN_PATH=.:$(gcinlibdir) \
	$(CC) -o $@ $(OBJS_sim2trad) $(LDFLAGS)
	rm -f core.*
trad2sim:	sim2trad
	ln -sf sim2trad trad2sim

gcin-setup:     $(OBJS_gcin_steup)
	rm -f core.*
	$(CC) -o $@ $(OBJS_gcin_steup) $(LDFLAGS)

gcin-setup-tab: $(OBJS_gcin_setup_tab)
	rm -f core.*
	$(CC) -o $@ $(OBJS_gcin_setup_tab) $(LDFLAGS)

phoa2d: $(OBJS_phoa2d)
	$(CC) -o $@ $(OBJS_phoa2d) $(LDFLAGS)

phod2a: $(OBJS_phod2a)
	$(CC) -o $@ $(OBJS_phod2a) $(LDFLAGS)

tsa2d:  $(OBJS_tsa2d)
	$(CC) -o $@ $(OBJS_tsa2d) $(LDFLAGS)

tsa2d32:  $(OBJS_tsa2d32)
	$(CC) -o $@ $(OBJS_tsa2d32) $(LDFLAGS)

tsd2a:  $(OBJS_tsd2a)
	$(CC) -o $@ $(OBJS_tsd2a) $(LDFLAGS)

tsd2a32:  $(OBJS_tsd2a32)
	$(CC) -o $@ $(OBJS_tsd2a32) $(LDFLAGS)

gcin2tab:  $(OBJS_gcin2tab)
	$(CC) -o $@ $(OBJS_gcin2tab) $(LDFLAGS)
	rm -f data/*.gtab

gtab-merge:  $(OBJS_gtab_merge)
	$(CC) -o $@ $(OBJS_gtab_merge) $(LDFLAGS)

kbmcv:  $(OBJS_kbmcv)
	$(CC) -o $@ $(OBJS_kbmcv) $(LDFLAGS)

gcin-gb-toggle:	$(OBJS_gcin_gb_toggle)
	$(CC) -o $@ $(OBJS_gcin_gb_toggle) $(LDFLAGS)

gcin-kbm-toggle:	$(OBJS_gcin_kbm_toggle)
	$(CC) -o $@ $(OBJS_gcin_kbm_toggle) $(LDFLAGS)

gcin-message:	$(OBJS_gcin_message)
	$(CC) -o $@ $(OBJS_gcin_message) $(LDFLAGS)

pin-juyin:	$(OBJS_pin_juyin)
	$(CC) -o $@ $(OBJS_pin_juyin) $(LDFLAGS)

gcin1_so= intcode.pico win-int.pico win-message.pico phrase.pico win-sym.pico win-inmd-switch.pico pinyin.pico win-pho-near.pico win-kbm.pico
gcin1.so: $(gcin1_so)
	$(CC) $(SO_FLAGS) -o $@ $(gcin1_so) $(LDFLAGS)

gcin2_so= t2s-lookup.pico
gcin2.so: $(gcin2_so)
	$(CC) $(SO_FLAGS) -o $@ $(gcin2_so) $(LDFLAGS)

### making the following as .so actuall makes the RSS larger
gcin_gtab_so = gtab.pico win-gtab.pico gtab-util.pico
gcin-gtab.so: $(gcin_gtab_so)
	$(CC) $(SO_FLAGS) -o $@  $(gcin_gtab_so) $(LDFLAGS)

gcin_tsin_so = tsin.pico tsin-util.pico win0.pico win1.pico win-pho-near.pico tsin-parse.pico
gcin-tsin.so: $(gcin_tsin_so)
	$(CC) -shared -o $@  $(gcin_tsin_so) $(LDFLAGS)

gcin_pho_so=win-pho.pico pho.pico pho-util.pico pho-sym.pico table-update.pico pho-dbg.pico
gcin-pho.so: $(gcin_pho_so)
	$(CC) -shared -fPIC -o $@ $(gcin_pho_so) $(LDFLAGS)

$(IMdkitLIB):
	$(MAKE) -C IMdkit/lib

$(im-srv):
	$(MAKE) -C im-srv


ibin:
	install $(PROGS) $(bindir); \
	rm -f $(bindir)/trad2sim; ln -sf sim2trad $(bindir)/trad2sim
	install $(GCIN_SO) $(gcinlibdir)

install:
	install -d $(datadir)/icons
	install gcin.png $(datadir)/icons
	$(MAKE) -C icons install
	install -d $(gcinlibdir)
	install $(GCIN_SO) $(gcinlibdir)
	install -d $(bindir)
	$(MAKE) -C data install
	$(MAKE) -C im-client install
	$(MAKE) -C gtk-im install
	if [ $(QT_IM) = 'Y' ]; then $(MAKE) -C qt-im install; fi
	if [ $(prefix) = /usr/local ]; then \
	   install -m 644 gcin.png /usr/share/icons; \
	   install -d $(DOC_DIR); \
	   install -m 644 README Changelog $(DOC_DIR); \
	   install $(PROGS) $(bindir); \
	   rm -f $(bindir)/trad2sim; ln -sf sim2trad $(bindir)/trad2sim; \
	else \
	   install -d $(DOC_DIR_i); \
	   install -m 644 README Changelog $(DOC_DIR_i); \
	   install -s $(PROGS) $(bindir); \
	   rm -f $(bindir)/trad2sim; ln -sf sim2trad $(bindir)/trad2sim; \
	fi
	$(MAKE) -C scripts install
	$(MAKE) -C menu install
	$(MAKE) -C man install
ifeq ($(USE_I18N),Y)
	$(MAKE) -C po install
endif
clean:
	$(MAKE) -C IMdkit clean
	$(MAKE) -C data clean
	$(MAKE) -C scripts clean
	$(MAKE) -C im-srv clean
	$(MAKE) -C im-client clean
	$(MAKE) -C gtk-im clean
	$(MAKE) -C qt-im clean
	$(MAKE) -C man clean
	$(MAKE) -C menu clean
	$(MAKE) -C po clean
	rm -f *.o *.E *.db *.pico *.so config.mak tags $(PROGS) $(PROGS_CV) \
	$(DATA) .depend gcin.spec trad2sim gcin.spec.tmp gcin.log
	find . '(' -name '.ted*' -o -name '*~' -o -name 'core.*' -o -name 'vgcore.*' ')' -exec rm {} \;

.depend:
	$(CC) $(CFLAGS) -MM *.c > $@

config.mak: VERSION.gcin configure
	./configure

gcin.spec:	gcin.spec.in
	rm -f $@
	sed -e "s/__gcin_version__/$(GCIN_VERSION)/" < $< > $@

include .depend
