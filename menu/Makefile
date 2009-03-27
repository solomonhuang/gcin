MENUS = gcin gcin-setup

include ../config.mak

all:

install:
	if [ $(prefix) = /usr/local ]; then \
	   install -d /usr/lib/menu; \
	   install -m 644 $(MENUS) /usr/lib/menu; \
	   which update-menus >& /dev/null && update-menus; \
	else \
	   install -d $(libdir)/menu; \
	   install -m 644 $(MENUS) $(libdir)/menu; \
	fi
clean:
	rm -f $(MENUS) *~
