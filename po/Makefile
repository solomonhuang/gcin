include ../config.mak

all:	gmo

gmo:
	$(MAKE) -C en_US
	$(MAKE) -C zh_CN

pot:
	xgettext --from-code=UTF-8 --keyword=_ --keyword=N_ --keyword=_L -o ./gcin.pot ../*.c* ../*/*.c ../*/*.cpp

install:
	install -d $(datadir)/locale/en_US/LC_MESSAGES
	install -d $(datadir)/locale/zh_CN/LC_MESSAGES
	install -m 644 en_US/gcin.gmo $(datadir)/locale/en_US/LC_MESSAGES/gcin.mo
	install -m 644 zh_CN/gcin.gmo $(datadir)/locale/zh_CN/LC_MESSAGES/gcin.mo

clean:
	$(MAKE) -C en_US clean
	$(MAKE) -C zh_CN clean
	rm -f ./gcin.pot
