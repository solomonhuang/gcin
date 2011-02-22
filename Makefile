OPTFLAGS=-g

include config.mak

.SUFFIXES:	.c .o .E .pico .cpp

gcin_tsin_o = tsin.o tsin-util.o win0.o win1.o tsin-parse.o
gcin_pho_o = win-pho.o pho.o pho-util.o pho-sym.o table-update.o pho-dbg.o
gcin_gtab_o = gtab.o win-gtab.o gtab-util.o gtab-list.o gtab-buf.o

GCIN_SO= gcin1.so gcin2.so

OBJS=gcin.o eve.o util.o gcin-conf.o gcin-settings.o locale.o gcin-icon.o \
     gcin-switch.o gcin-exec-script.o $(GCIN_SO) pho-play.o cache.o gtk_bug_fix.o \
     $(gcin_pho_o) $(gcin_gtab_o) gcin-common.o phrase.o t2s-lookup.o gtab-use-count.o \
     win-save-phrase.o unix-exec.o pho-kbm-name.o statistic.o tsin-scan.o

OBJS_TSLEARN=tslearn.o util.o gcin-conf.o pho-util.o tsin-util.o gcin-send.o pho-sym.o \
             table-update.o locale.o gcin-settings.o gcin-common.o gcin-icon.o pho-dbg.o
OBJS_JUYIN_LEARN=juyin-learn.o locale.o util.o pho-util.o pho-sym.o \
                 gcin-settings.o gcin-conf.o table-update.o pinyin.o gcin-icon.o pho-dbg.o
OBJS_sim2trad=sim2trad.o util.o gcin2.so locale.o gcin-conf.o gcin-icon.o
OBJS_phod2a=phod2a.o pho-util.o gcin-conf.o pho-sym.o table-update.o pho-dbg.o locale.o \
             gcin-settings.o util.o
OBJS_tsa2d32=tsa2d32.o gcin-send.o util.o pho-sym.o gcin-conf.o locale.o pho-lookup.o
OBJS_phoa2d=phoa2d.o pho-sym.o gcin-send.o gcin-conf.o locale.o pho-lookup.o util.o
OBJS_kbmcv=kbmcv.o pho-sym.o util.o locale.o
OBJS_tsd2a32=tsd2a32.o pho-sym.o pho-dbg.o locale.o util.o gtab-dbg.o
OBJS_gcin2tab=gcin2tab.o gtab-util.o util.o locale.o
OBJS_gtab_merge=gtab-merge.o gtab-util.o util.o locale.o
OBJS_gcin_steup=gcin-setup.o gcin-conf.o util.o gcin-send.o gcin-settings.o \
	gcin-setup-list.o gcin-switch.o locale.o gcin-setup-pho.o about.o \
	gcin-icon.o gcin-setup-gtab.o gtab-list.o gcin-exec-script.o pho-kbm-name.o

OBJS_gcin_gb_toggle = gcin-gb-toggle.o gcin-conf.o util.o gcin-send.o
OBJS_gcin_kbm_toggle = gcin-kbm-toggle.o gcin-conf.o util.o gcin-send.o
OBJS_gcin_exit = gcin-exit.o gcin-conf.o util.o gcin-send.o
OBJS_gcin_message = gcin-message.o gcin-conf.o util.o gcin-send.o
OBJS_pin_juyin = pin-juyin.o util.o pho-lookup.o locale.o pho-sym.o

OBJS_tsin2gtab_phrase = tsin2gtab-phrase.o gcin-conf.o util.o locale.o \
	pho-dbg.o pho-sym.o gtab-dbg.o

#WALL=-Wall
CFLAGS= -DUNIX=1 $(WALL) $(OPTFLAGS) $(GTKINC) -I./IMdkit/include -I./im-client -DDEBUG="0$(GCIN_DEBUG)" \
        -DGCIN_TABLE_DIR=\"$(GCIN_TABLE_DIR)\" \
        -DGCIN_OGG_DIR=\"$(GCIN_OGG_DIR)\" \
        -DDOC_DIR=\"$(DOC_DIR)\" \
        -DGCIN_ICON_DIR=\"$(GCIN_ICON_DIR)\" \
        -DGCIN_SCRIPT_DIR=\"$(GCIN_SCRIPT_DIR)\" -DGCIN_BIN_DIR=\"$(GCIN_BIN_DIR)\" \
        -DSYS_ICON_DIR=\"$(SYS_ICON_DIR)\" -DFREEBSD=$(FREEBSD) -DMAC_OS=$(MAC_OS) \
        -DG_DISABLE_SINGLE_INCLUDES -DG_DISABLE_DEPRECATED \
        -DGDK_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED \
        -DGTK_DISABLE_SINGLE_INCLUDES -DGTK_DISABLE_DEPRECATED
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
OBJS += tray.o eggtrayicon.o tray-win32.o
endif

ifeq ($(USE_TRAY),N)
GCIN_SO += tray-win32.o
endif

ifeq ($(USE_I18N),Y)
CFLAGS += -DGCIN_i18n_message=1
endif

ifeq ($(USE_TSIN),Y)
CFLAGS += -DUSE_TSIN=1
OBJS += $(gcin_tsin_o)
endif

ifeq ($(USE_ANTHY),Y)
CFLAGS += -DUSE_ANTHY=1
LDFLAGS += -ldl
ifeq ($(USE_TSIN),N)
OBJS += $(gcin_tsin_o)
endif
endif

ifeq ($(USE_GCB),Y)
CFLAGS += -DUSE_GCB=1
OBJS += gcb.o
endif

OBJ_IMSRV=im-addr.o im-dispatch.o im-srv.o gcin-crypt.o

.c.E:
	$(CC) $(CFLAGS) -E -o $@ $<

.cpp.E:
	$(CCX) $(CFLAGS) -E -o $@ $<

.cpp.o:
	$(CCX) $(CFLAGS) -c $<

.c.pico:
	$(CC) $(CFLAGS) -c -fpic -o $@ $<
.cpp.pico:
	$(CCX) $(CFLAGS) -c -fpic -o $@ $<

PROGS=gcin tsd2a32 tsa2d32 phoa2d phod2a tslearn gcin-setup gcin2tab \
	juyin-learn sim2trad gcin-gb-toggle gcin-message gtab-merge \
	gcin-kbm-toggle tsin2gtab-phrase gcin-exit
PROGS_SYM=trad2sim
PROGS_CV=kbmcv pin-juyin

all:	$(PROGS) trad2sim $(DATA) $(PROGS_CV) gcin.spec
	$(MAKE) -C data
	$(MAKE) -C gtk-im
ifeq ($(USE_I18N),Y)
	$(MAKE) -C po
endif
	if [ $(GTK3_IM) = 'Y' ]; then $(MAKE) -C gtk3-im; fi
	if [ $(QT_IM) = 'Y' ]; then $(MAKE) -C qt-im; fi
	if [ $(QT4_IM) = 'Y' ]; then $(MAKE) -C qt4-im; fi

#gcc_ld_run_path=-Wl,-rpath,$(gcin_ld_run_path)

gcin:   $(OBJS) $(IMdkitLIB) $(OBJ_IMSRV)
	$(CCLD) $(EXTRA_LDFLAGS) $(gcc_ld_run_path) -o $@ $(OBJS) $(IMdkitLIB) $(OBJ_IMSRV) -lXtst $(LDFLAGS) -L/usr/X11R6/lib
	rm -f core.* vgcore.*
	ln -sf $@ $@.test

gcin-nocur:   $(OBJS) $(IMdkitLIB) $(OBJ_IMSRV)
	$(CCLD) -Wl,-rpath,$(gcinlibdir) $(EXTRA_LDFLAGS) -o $@ $(OBJS) $(IMdkitLIB) $(OBJ_IMSRV) -lXtst $(LDFLAGS) -L/usr/X11R6/lib
	rm -f core.*

tslearn:        $(OBJS_TSLEARN)
	$(CCLD) $(gcc_ld_run_path) -o $@ $(OBJS_TSLEARN) -L./im-client -lgcin-im-client $(LDFLAGS)

juyin-learn:        $(OBJS_JUYIN_LEARN)
	$(CCLD) -o $@ $(OBJS_JUYIN_LEARN) $(LDFLAGS)
	rm -f core.*
sim2trad:        $(OBJS_sim2trad)
	$(CC) $(gcc_ld_run_path) -o $@ $(OBJS_sim2trad) $(LDFLAGS)
	rm -f core.*
trad2sim:	sim2trad
	ln -sf sim2trad trad2sim

gcin-setup:     $(OBJS_gcin_steup) im-client/libgcin-im-client.so
	rm -f core.*
	$(CCLD) $(gcc_ld_run_path) -o $@ $(OBJS_gcin_steup) -L./im-client -lgcin-im-client $(LDFLAGS)

phoa2d: $(OBJS_phoa2d) im-client/libgcin-im-client.so
	$(CCLD) $(gcc_ld_run_path) -o $@ $(OBJS_phoa2d) -L./im-client -lgcin-im-client $(LDFLAGS)

phod2a: $(OBJS_phod2a)
	$(CCLD) -lX11 -o $@ $(OBJS_phod2a) $(LDFLAGS)

tsa2d32:  $(OBJS_tsa2d32) im-client/libgcin-im-client.so
	$(CCLD) $(gcc_ld_run_path) -o $@ $(OBJS_tsa2d32) -L./im-client -lgcin-im-client $(LDFLAGS)

tsd2a:  $(OBJS_tsd2a)
	$(CCLD) -o $@ $(OBJS_tsd2a) $(LDFLAGS)

tsd2a32:  $(OBJS_tsd2a32)
	$(CCLD) -o $@ $(OBJS_tsd2a32) $(LDFLAGS)

gcin2tab:  $(OBJS_gcin2tab)
	$(CCLD) -o $@ $(OBJS_gcin2tab) $(LDFLAGS)
	rm -f data/*.gtab

gtab-merge:  $(OBJS_gtab_merge)
	$(CCLD) -o $@ $(OBJS_gtab_merge) $(LDFLAGS)

kbmcv:  $(OBJS_kbmcv)
	$(CCLD) -o $@ $(OBJS_kbmcv) $(LDFLAGS)

gcin-gb-toggle:	$(OBJS_gcin_gb_toggle)
	$(CCLD) $(gcc_ld_run_path) -o $@ $(OBJS_gcin_gb_toggle) -L./im-client -lgcin-im-client $(LDFLAGS)

gcin-kbm-toggle:	$(OBJS_gcin_kbm_toggle)
	$(CCLD) $(gcc_ld_run_path) -o $@ $(OBJS_gcin_kbm_toggle) -L./im-client -lgcin-im-client $(LDFLAGS)

gcin-exit:	$(OBJS_gcin_exit)
	$(CCLD) $(gcc_ld_run_path) -o $@ $(OBJS_gcin_exit) -L./im-client -lgcin-im-client $(LDFLAGS)

gcin-message:	$(OBJS_gcin_message)
	$(CCLD) $(gcc_ld_run_path) -o $@ $(OBJS_gcin_message) -L./im-client -lgcin-im-client $(LDFLAGS)

pin-juyin:	$(OBJS_pin_juyin)
	$(CCLD) -o $@ $(OBJS_pin_juyin) $(LDFLAGS)

tsin2gtab-phrase:       $(OBJS_tsin2gtab_phrase)
	$(CCLD) -o $@ $(OBJS_tsin2gtab_phrase) $(LDFLAGS)

im-client/libgcin-im-client.so:
	$(MAKE) -C im-client

gcin-version.h:	VERSION.gcin
	echo '#define GCIN_VERSION "'`cat VERSION.gcin`'"' > gcin-version.h

gcin1_so += intcode.pico win-int.pico win-message.pico win-sym.pico \
win-inmd-switch.pico pinyin.pico win-pho-near.pico win-kbm.pico

ifeq ($(USE_ANTHY),Y)
gcin1_so += anthy.pico
endif

gcin1.so: $(gcin1_so) pho.o tsin.o eve.o gtab.o win-sym.o
	$(CCLD) $(SO_FLAGS) -o $@ $(gcin1_so) $(LDFLAGS)

gcin2_so= t2s-lookup.pico
gcin2.so: $(gcin2_so) gcin-conf.o
	$(CCLD) $(SO_FLAGS) -o $@ $(gcin2_so) $(LDFLAGS)

#gtk_bug_fix.so: gtk_bug_fix.pico
#	$(CC) $(SO_FLAGS) -o $@ gtk_bug_fix.pico

### making the following as .so actuall makes the RSS larger
gcin_gtab_so = gtab.pico win-gtab.pico gtab-util.pico
gcin-gtab.so: $(gcin_gtab_so)
	$(CC) $(SO_FLAGS) -o $@  $(gcin_gtab_so) $(LDFLAGS)

gcin_tsin_so = tsin.pico tsin-util.pico win0.pico win1.pico win-pho-near.pico tsin-parse.pico
gcin-tsin.so: $(gcin_tsin_so)
	$(CCLD) -shared -o $@  $(gcin_tsin_so) $(LDFLAGS)

gcin_pho_so=win-pho.pico pho.pico pho-util.pico pho-sym.pico table-update.pico pho-dbg.pico
gcin-pho.so: $(gcin_pho_so)
	$(CC) -shared -fPIC -o $@ $(gcin_pho_so) $(LDFLAGS)

$(IMdkitLIB):
	$(MAKE) -C IMdkit/lib

ibin:	gcin-nocur
	install $(PROGS) $(bindir); \
	cp gcin-nocur $(bindir)/gcin; \
	rm -f $(bindir)/trad2sim; ln -sf sim2trad $(bindir)/trad2sim
	install $(GCIN_SO) $(gcinlibdir)

install:
	install -d $(datadir)/icons
	install -m 644 gcin.png $(datadir)/icons
	$(MAKE) -C icons install
	install -d $(gcinlibdir)
	install $(GCIN_SO) $(gcinlibdir)
	install -d $(bindir)
	$(MAKE) -C data install
	$(MAKE) -C im-client install
	$(MAKE) -C gtk-im install
	if [ $(GTK3_IM) = 'Y' ]; then $(MAKE) -C gtk3-im install; fi
	if [ $(QT_IM) = 'Y' ]; then $(MAKE) -C qt-im install; fi
	if [ $(QT4_IM) = 'Y' ]; then $(MAKE) -C qt4-im install; fi
	if [ $(prefix) = /usr/local ]; then \
	   install -m 644 gcin.png /usr/share/icons; \
	   install -d $(DOC_DIR); \
	   install -m 644 README.html Changelog.html $(DOC_DIR); \
	   install $(PROGS) $(bindir); \
	   rm -f $(bindir)/trad2sim; ln -sf sim2trad $(bindir)/trad2sim; \
	else \
	   install -d $(DOC_DIR_i); \
	   install -m 644 README.html Changelog.html $(DOC_DIR_i); \
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
	$(MAKE) -C im-client clean
	$(MAKE) -C gtk-im clean
ifeq ($(GTK3_IM), Y)
	$(MAKE) -C gtk3-im clean
endif
	$(MAKE) -C qt-im clean
	$(MAKE) -C qt4-im clean
	$(MAKE) -C man clean
	$(MAKE) -C menu clean
	$(MAKE) -C po clean
	rm -f *.o *.E *.db *.pico *.so config.mak tags $(PROGS) gcin-nocur $(PROGS_CV) \
	$(DATA) .depend gcin.spec trad2sim gcin.spec.tmp gcin.log
	find . '(' -name '.ted*' -o -name '*~' -o -name 'core.*' -o -name 'vgcore.*' ')' -exec rm {} \;

.depend:
	$(CCX) $(CFLAGS) -MM *.cpp > $@

config.mak: VERSION.gcin configure
	./configure

gcin.spec:	gcin.spec.in
	rm -f $@
	sed -e "s/__gcin_version__/$(GCIN_VERSION)/" < $< > $@

include .depend
