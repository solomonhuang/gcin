include ../config.mak

SCRIPTS=gcin-user-setup gcin-utf8-setup

all:	$(SCRIPTS)

install:
	install -d $(GCIN_SCRIPT_DIR_i)
	install -m 755 $(SCRIPTS) $(GCIN_SCRIPT_DIR_i)
clean:
	rm -f *~ tags core.*
