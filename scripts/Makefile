include ../config.mak

SCRIPTS=gcin-user-setup gcin-utf8-setup modify-XIM

all:	$(SCRIPTS)

install:
	if [ $(prefix) = /usr/local ]; then \
	   sh modify-XIM; \
	fi
	install -d $(GCIN_SCRIPT_DIR_i)
	install -m 755 $(SCRIPTS) $(GCIN_SCRIPT_DIR_i)
clean:
	rm -f *~ tags core.*
