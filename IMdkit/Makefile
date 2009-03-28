include ../config.mak

all: libXimd.a Xi18n_sample/sampleIM

libXimd.a:
	$(MAKE) -C lib

Xi18n_sample/sampleIM:
	$(MAKE) -C Xi18n_sample

clean:
	rm -f *.bak *~
	$(MAKE) -C lib clean
	$(MAKE) -C Xi18n_sample clean

install:
	@echo nothing to be installed.

