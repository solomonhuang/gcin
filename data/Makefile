include ../config.mak

.SUFFIXES:	.kbmsrc .kbm .cin .gtab

DATA=pho.tab tsin.idx \
     cj.gtab ar30.gtab dayi3.gtab pinyin.gtab jyutping.gtab cj5.gtab \
     et.kbm et-asdf.kbm et26.kbm et26-asdf.kbm zo.kbm zo-asdf.kbm \
     ibm.kbm tsin hsu.kbm pho-huge.tab simplex.gtab kana-nippon.gtab \
     greek.gtab cj-punc.gtab latin-letters.gtab
DATAKEEP=symbol-table gtab.list phrase.table

all:	$(DATA)

tsin:	tsin.src ../tsa2d
	GCIN_NO_RELOAD=; export GCIN_NO_RELOAD; ../tsa2d $<

tsin.idx:	tsin.src
	GCIN_NO_RELOAD=; export GCIN_NO_RELOAD; ../tsa2d $<

pho.tab:	 pho.tab.src ../phoa2d
	GCIN_NO_RELOAD=; export GCIN_NO_RELOAD; ../phoa2d $<

pho-huge.tab:	 pho-huge.tab.src ../phoa2d
	GCIN_NO_RELOAD=; export GCIN_NO_RELOAD; ../phoa2d $<

.cin.gtab:      ../gcin2tab
	../gcin2tab $<

.kbmsrc.kbm:	../kbmcv
	../kbmcv $<

install:
	install -d $(GCIN_TABLE_DIR_i)
	install -m 644 $(DATA) $(DATAKEEP) $(GCIN_TABLE_DIR_i)
#	cd $(GCIN_TABLE_DIR_i); bzip2 -f *.gtab
clean:
	rm -f *~ tags core.* $(DATA) *.gtab
