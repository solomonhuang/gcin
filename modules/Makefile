include ../config.mak
include ../suffixes-rule

CFLAGS+=-I.. -I../IMdkit/include

ifeq ($(USE_ANTHY),Y)
GCIN_MODULE+=anthy-module.so
endif

ifeq ($(USE_CHEWING),Y)
GCIN_MODULE+=chewing-module.so
CHEWING_DATADIR=$(shell pkg-config --variable=datadir chewing)
CFLAGS += -DCHEWING_DATADIR=\"$(CHEWING_DATADIR)\"
endif

all:	$(GCIN_MODULE)

anthy_module_so = anthy.pico
anthy-module.so: $(anthy_module_so)
	$(CCLD) $(SO_FLAGS) -o $@ $(anthy_module_so) $(LDFLAGS) -lanthy
	ln -sf $(PWD)/$@ data

chewing_module_so = chewing.pico
chewing-module.so: $(chewing_module_so)
	$(CCLD) $(SO_FLAGS) -o $@ $(chewing_module_so) $(LDFLAGS) -lchewing -I/usr/include/chewing
	ln -sf $(PWD)/$@ data

install:
	install $(GCIN_MODULE) $(gcinlibdir)
