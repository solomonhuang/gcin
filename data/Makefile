include ../config.mak

DATA=pho.tab et.kbm zo.kbm tsin tsin.idx
DATAKEEP=symbol-table

all:	$(DATA)

tsin:	tsin.src
	../tsa2d $<

tsin.idx:	tsin.src
	../tsa2d $<

pho.tab:	 pho.tab.src
	../phoa2d

et.kbm:
	../kbmcv   et

zo.kbm:
	../kbmcv   zo

install:
	install -m 644 $(DATA) $(DATAKEEP) $(GCIN_TABLE_DIR)
clean:
	rm -f *~ tags core.* $(DATA)
