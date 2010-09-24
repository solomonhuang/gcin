include ../config.mak

SCRIPTS=gcin-user-setup modify-XIM utf8-edit gtab.append_prepare html-browser tsin-gtab-import tsin-gtab-edit

all:	$(SCRIPTS)

install:
	if [ $(prefix) = /usr/local ]; then \
	   sh modify-XIM; \
	fi
	install -d $(GCIN_SCRIPT_DIR_i)
	install -m 755 $(SCRIPTS) $(GCIN_SCRIPT_DIR_i)
clean:
	rm -f *~ tags core.*
