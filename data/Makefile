include ../config.mak

.SUFFIXES:	.kbmsrc .kbm .cin .gtab
CFLAGS= -DUNIX=1 $(WALL) $(OPTFLAGS) $(GTKINC) -I.. -DDEBUG="0$(GCIN_DEBUG)" \


DATA=pho.tab2 tsin32.idx \
     cj.gtab ar30.gtab ar30-big.gtab dayi3.gtab pinyin.gtab jyutping.gtab cj5.gtab \
     et.kbm et26.kbm zo.kbm pinyin.kbm dvorak.kbm mitac.kbm colemak.kbm \
     ibm.kbm tsin32 hsu.kbm pho-huge.tab2 simplex.gtab symbols.gtab russian.gtab \
     greek.gtab cj-punc.gtab latin-letters.gtab tsin-1.src pin-juyin.xlt \
     t2s.dat s2t.dat cj543.gtab
DATAKEEP=symbol-table gtab.list phrase.table phrase-ctrl.table

all:	$(DATA)

tsin32:	tsin.src ../tsa2d32
	GCIN_NO_RELOAD=; export GCIN_NO_RELOAD;export LD_LIBRARY_PATH=../im-client;../tsa2d32 $<

tsin32.idx:	tsin32

pho.tab2:	 pho.tab2.src ../phoa2d
	GCIN_NO_RELOAD=;export GCIN_NO_RELOAD;export LD_LIBRARY_PATH=../im-client;../phoa2d $<

pho-huge.tab2:	 pho-huge.tab2.src ../phoa2d
	GCIN_NO_RELOAD=;export GCIN_NO_RELOAD;export LD_LIBRARY_PATH=../im-client;../phoa2d $<

t2s.dat:	t2s-file t2s-file.table
	./t2s-file
s2t.dat:	t2s-file

pin-juyin.xlt:	../pin-juyin pin-juyin.src
	../pin-juyin

extr1:	extr1.c
	$(CC) $(CFLAGS) $< ../util.o ../locale.o -o $@ $(LDFLAGS)

t2s-file:	t2s-file.c
	$(CC) $(CFLAGS) $< ../util.o ../locale.o -o $@ $(LDFLAGS)

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
