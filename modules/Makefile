include ../config.mak
include ../suffixes-rule

CFLAGS+=-I.. -I../IMdkit/include
GCIN_MODULE=intcode-module.so

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

intcode_module_so = intcode.pico win-int.pico
intcode-module.so: $(intcode_module_so)
	$(CCLD) $(SO_FLAGS) -o $@ $(intcode_module_so) $(LDFLAGS)

chewing_module_obj = chewing-conf.pico
chewing_module_so = chewing.pico gcin-setup-chewing.pico
chewing-module.so: $(chewing_module_obj) $(chewing_module_so)
	$(CCLD) $(SO_FLAGS) -o $@ $(chewing_module_obj) $(chewing_module_so) $(LDFLAGS) $(shell pkg-config --cflags --libs chewing)

install:
	install $(GCIN_MODULE) $(gcinlibdir)

clean:
	rm -f *.pico *.so *.o
.depend:
	$(CCX) $(CFLAGS) -MM *.cpp > $@

include .depend
