include ../config.mak

.SUFFIXES:	.kbm .cin .gtab

DATA=pho.tab et.kbm zo.kbm tsin tsin.idx cj.gtab ar30.gtab
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

zo.kbm:
	../kbmcv   zo

.cin.gtab:
	../gcin2tab $<

install:
	install -m 644 $(DATA) $(DATAKEEP) $(GCIN_TABLE_DIR)
clean:
	rm -f *~ tags core.* $(DATA) *.gtab.*
