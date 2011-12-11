include ../config.mak

all:

install:
	if [ $(prefix) = /usr/local ]; then \
	   install gcin-tools.desktop /usr/share/applications; \
	   which update-menus > /dev/null 2>&1 && update-menus || exit 0; \
	else \
	   install -d $(datadir)/applications; \
	   install -m 644 gcin-tools.desktop $(datadir)/applications; \
	fi
clean:
	rm -f $(MENUS) *~
