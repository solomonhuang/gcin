include ../config.mak

.SUFFIXES:	.kbmsrc .kbm .cin .gtab

DATA=pho.tab tsin32.idx \
     cj.gtab ar30.gtab ar30-big.gtab dayi3.gtab pinyin.gtab jyutping.gtab cj5.gtab \
     et.kbm et-asdf.kbm et26.kbm et26-asdf.kbm zo.kbm zo-asdf.kbm \
     pinyin.kbm pinyin-asdf.kbm dvorak.kbm mitac.kbm \
     ibm.kbm tsin32 hsu.kbm pho-huge.tab simplex.gtab kana-nippon.gtab symbols.gtab \
     greek.gtab cj-punc.gtab latin-letters.gtab tsin-1.src pin-juyin.xlt \
     t2s.dat s2t.dat
DATAKEEP=symbol-table gtab.list phrase.table phrase-ctrl.table

all:	$(DATA)

tsin32:	tsin.src ../tsa2d32
	GCIN_NO_RELOAD=; export GCIN_NO_RELOAD; ../tsa2d32 $<

tsin32.idx:	tsin32

pho.tab:	 pho.tab.src ../phoa2d
	GCIN_NO_RELOAD=; export GCIN_NO_RELOAD; ../phoa2d $<

pho-huge.tab:	 pho-huge.tab.src ../phoa2d
	GCIN_NO_RELOAD=; export GCIN_NO_RELOAD; ../phoa2d $<

t2s.dat:	t2s-file t2s-file.table
	./t2s-file
s2t.dat:	t2s-file

pin-juyin.xlt:	../pin-juyin pin-juyin.src
	../pin-juyin

extr1:	extr1.c
	cc $< ../util.o ../locale.o -o $@ $(LDFLAGS)

t2s-file:	t2s-file.c
	cc $< ../util.o ../locale.o -o $@ $(LDFLAGS)

tsin-1.src:	extr1
	./extr1 > $@

.cin.gtab:      ../gcin2tab
	../gcin2tab $<

.kbmsrc.kbm:	../kbmcv
	../kbmcv $<

install:
	install -d $(GCIN_TABLE_DIR_i)
	install -m 644 $(DATA) $(DATAKEEP) $(GCIN_TABLE_DIR_i)
#	cd $(GCIN_TABLE_DIR_i); bzip2 -f *.gtab
clean:
	rm -f *~ tags core.* $(DATA) *.gtab extr1 t2s-file tsin-1.src tsin-gtabidx* tsin-ch-idx
