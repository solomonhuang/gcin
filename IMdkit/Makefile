
all: libXimd.a Xi18n_sample/sampleIM

libXimd.a:
	make -C lib

Xi18n_sample/sampleIM:
	cd Xi18n_sample; make

clean:
	rm -f *.bak *~
	make -C lib clean
	make -C Xi18n_sample clean

install:
	@echo nothing to be installed.

