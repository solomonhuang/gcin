include ../config.mak

.SUFFIXES:	.kbm .cin .gtab

DATA=pho.tab et.kbm et-asdf.kbm zo.kbm zo-asdf.kbm ibm.kbm tsin tsin.idx cj.gtab ar30.gtab
DATAKEEP=symbol-table gtab.list phrase.table

all:	$(DATA)

tsin:	tsin.src
	export GCIN_NO_RELOAD=; ../tsa2d $<

tsin.idx:	tsin.src
	export GCIN_NO_RELOAD=; ../tsa2d $<

pho.tab:	 pho.tab.src
	export GCIN_NO_RELOAD=; ../phoa2d

et.kbm:
	../kbmcv   et

et-asdf.kbm:
	../kbmcv   et-asdf

zo.kbm:
	../kbmcv   zo

zo-asdf.kbm:
	../kbmcv   zo-asdf

ibm.kbm:
	../kbmcv   ibm

.cin.gtab:
	../gcin2tab $<

install:
	install -d $(GCIN_TABLE_DIR)
	install -m 644 $(DATA) $(DATAKEEP) $(GCIN_TABLE_DIR)
clean:
	rm -f *~ tags core.* $(DATA) *.gtab
