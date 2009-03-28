MENUS = gcin gcin-setup

include ../config.mak

all:

install:
	if [ $(prefix) = /usr/local ]; then \
	   install gcin-setup.desktop /usr/share/control-center-2.0/capplets; \
	   install gcin-setup.desktop /usr/share/applications; \
	   install -d /usr/lib/menu; \
	   install -m 644 $(MENUS) /usr/lib/menu; \
	   which update-menus >& /dev/null && update-menus || exit 0; \
	else \
	   install -d $(libdir)/menu; \
	   install -m 644 $(MENUS) $(libdir)/menu; \
	   install -d $(datadir)/control-center-2.0/capplets; \
	   install -m 644 gcin-setup.desktop $(datadir)/control-center-2.0/capplets; \
	   install -d $(datadir)/applications; \
	   install -m 644 gcin-setup.desktop $(datadir)/applications; \
	fi
clean:
	rm -f $(MENUS) *~
