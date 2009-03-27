include ../config.mak

.SUFFIXES:	.kbmsrc .kbm .cin .gtab

DATA=pho.tab tsin.idx \
     cj.gtab ar30.gtab \
     et.kbm et-asdf.kbm et26.kbm et26-asdf.kbm zo.kbm zo-asdf.kbm \
     ibm.kbm tsin hsu.kbm
DATAKEEP=symbol-table gtab.list phrase.table

all:	$(DATA)

tsin:	tsin.src
	export GCIN_NO_RELOAD=; ../tsa2d $<

tsin.idx:	tsin.src
	export GCIN_NO_RELOAD=; ../tsa2d $<

pho.tab:	 pho.tab.src
	export GCIN_NO_RELOAD=; ../phoa2d

.cin.gtab:
	../gcin2tab $<

.kbmsrc.kbm:	../kbmcv
	../kbmcv $<

install:
	install -d $(GCIN_TABLE_DIR)
	install -m 644 $(DATA) $(DATAKEEP) $(GCIN_TABLE_DIR)
clean:
	rm -f *~ tags core.* $(DATA) *.gtab
