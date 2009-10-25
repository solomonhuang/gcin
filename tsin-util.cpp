/*
	Copyright (C) 1995-2008	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/

#include "gcin.h"
#include "pho.h"
#include "tsin.h"


int hashidx[TSIN_HASH_N];
int *phidx;
FILE *fph;
int phcount;

#if UNIX
#define FNAME_MAX 64
#else
#define FNAME_MAX 128
#endif

char tsidxfname[FNAME_MAX]="";
int ts_gtabN;
static int *ts_gtab_hash;
#define HASHN 256

static int a_phcount;
char tsfname[FNAME_MAX];

void get_gcin_user_or_sys_fname(char *name, char fname[]);

#if USE_TSIN
void load_tsin_db()
{
  if (!tsfname[0]) {
    get_gcin_user_or_sys_fname("tsin32", tsfname);
  }

  strcpy(tsidxfname, tsfname);
  strcat(tsidxfname, ".idx");

  FILE *fr;

  if ((fr=fopen(tsidxfname,"rb"))==NULL) {
    p_err("Cannot open %s\n", tsidxfname);
  }

  fread(&phcount,4,1,fr);
#if     0
  printf("phcount:%d\n",phcount);
#endif
  a_phcount=phcount+256;
  fread(&hashidx,1,sizeof(hashidx),fr);

  if (phidx)
    free(phidx);

  if ((phidx=tmalloc(int, a_phcount))==NULL)
    p_err("malloc err pp 1");

  fread(phidx,4, phcount, fr);
  fclose(fr);

  if (fph)
    fclose(fph);

  dbg("tsfname: %s\n", tsfname);

  if ((fph=fopen(tsfname,"rb+"))==NULL)
    p_err("Cannot open %s", tsfname);
}
#endif


#if USE_TSIN
void free_tsin()
{
  if (fph) {
    fclose(fph); fph = NULL;
  }

  if (phidx) {
    free(phidx); phidx = NULL;
  }
}
#endif


static int phseq(u_char *a, u_char *b)
{
  u_char lena, lenb, mlen;
  int i;
  phokey_t ka,kb;

  lena=*(a++); lenb=*(b++);
  a+=sizeof(usecount_t); b+=sizeof(usecount_t);   // skip usecount

  mlen=Min(lena,lenb);

  for(i=0;i<mlen; i++) {
    memcpy(&ka, a, sizeof(phokey_t));
    memcpy(&kb,b, sizeof(phokey_t));
    if (ka > kb) return 1;
    if (ka < kb) return -1;
    a+=sizeof(phokey_t);
    b+=sizeof(phokey_t);
  }

  if (lena > lenb) return 1;
  if (lena < lenb) return -1;
  return 0;
}

void inc_dec_tsin_use_count(phokey_t *pho, char *ch, int N, gboolean b_dec);

static gboolean saved_phrase;

gboolean save_phrase_to_db(phokey_t *phkeys, char *utf8str, int len, usecount_t usecount)
{
  int mid, ord = 0, ph_ofs, hashno, i;
  FILE *fw;
  u_char tbuf[MAX_PHRASE_LEN*(sizeof(phokey_t)+CH_SZ) + 1 + sizeof(usecount_t)],
         sbuf[MAX_PHRASE_LEN*(sizeof(phokey_t)+CH_SZ) + 1 + sizeof(usecount_t)];

  saved_phrase = TRUE;

  tbuf[0]=len;
  memcpy(&tbuf[1], &usecount, sizeof(usecount));  // usecount
  int tlen = utf8_tlen(utf8str, len);
#if 0
  dbg("tlen %d  '", tlen);
  for(i=0; i < tlen; i++)
    putchar(utf8str[i]);
  dbg("'\n");
#endif

  memcpy(&tbuf[1 + sizeof(usecount_t)], phkeys, sizeof(phokey_t) * len);
  memcpy(&tbuf[sizeof(phokey_t)*len + 1 + sizeof(usecount_t)], utf8str, tlen);

  hashno=phkeys[0] >> TSIN_HASH_SHIFT;
  if (hashno >= TSIN_HASH_N)
    return FALSE;

  for(mid=hashidx[hashno]; mid<hashidx[hashno+1]; mid++) {
    ph_ofs=phidx[mid];
    fseek(fph, ph_ofs, SEEK_SET);
    fread(sbuf,1,1,fph);
    fread(&sbuf[1], sizeof(usecount_t), 1, fph); // use count
    fread(&sbuf[1+sizeof(usecount_t)], 1, (sizeof(phokey_t) + CH_SZ) * sbuf[0], fph);
    if ((ord=phseq(sbuf,tbuf)) >=0)
      break;
  }

//  dbg("tlen:%d  ord:%d  %s\n", tlen, ord, utf8str);
  if (!ord && !memcmp(&sbuf[sbuf[0]*sizeof(phokey_t)+1+sizeof(usecount_t)], utf8str, tlen)) {
//    bell();
    dbg("Phrase already exists\n");
    inc_dec_tsin_use_count(phkeys, utf8str, len, FALSE);
    return FALSE;
  }

  for(i=phcount;i>=mid;i--)
    phidx[i+1]=phidx[i];

  fseek(fph,0,SEEK_END);
  ph_ofs=ftell(fph);
  phidx[mid]=ph_ofs;
  phcount++;
  if (phcount>=a_phcount) {
    a_phcount+=256;
    if (!(phidx=trealloc(phidx, int, a_phcount*4))) {
      p_err("tsin.c:realloc err");
    }
  }

  fwrite(tbuf, 1, sizeof(phokey_t)*len + tlen + 1+ sizeof(usecount_t), fph);
  fflush(fph);

  if (hashidx[hashno]>mid)
    hashidx[hashno]=mid;

  hashno++;

  for(;hashno<256;hashno++)
    hashidx[hashno]++;

  if ((fw=fopen(tsidxfname,"wb"))==NULL) {
    dbg("%s create err", tsidxfname);
    return FALSE;
  }

  fwrite(&phcount,4,1,fw);
  fwrite(&hashidx,sizeof(hashidx),1,fw);
  fwrite(phidx,4,phcount,fw);
  fclose(fw);

  return TRUE;
}


int *ts_gtab;

int read_tsin_phrase(char *str, usecount_t *usecount)
{
  u_char len;
  u_char pho[sizeof(phokey_t) * MAX_PHRASE_LEN];
  len = 0;

  fread(&len, 1, 1, fph);
  if (len > MAX_PHRASE_LEN || len <=0)
    return 0;
  fread(usecount, sizeof(usecount_t), 1, fph); // use count
  fread(pho, sizeof(phokey_t), len, fph);

  int i;
  int tlen = 0;

  for(i=0; i < len; i++) {
    fread(&str[tlen], 1, 1, fph);
    int sz = utf8_sz(&str[tlen]);
    fread(&str[tlen+1], 1, sz-1, fph);
    tlen+=sz;
  }

  str[tlen] = 0;

  return tlen;
}


#if USE_TSIN
int load_ts_gtab(int idx, char *tstr, usecount_t *usecount)
{
  int ofs = ts_gtab[idx];

  fseek(fph, ofs, SEEK_SET);
  return read_tsin_phrase(tstr, usecount);
}
#endif

typedef struct {
  char ts[MAX_PHRASE_STR_LEN];
  int ofs;
} TS_TMP;

static int qcmp_ts_gtab(const void *aa, const void *bb)
{
  TS_TMP *a = (TS_TMP *)aa, *b = (TS_TMP *)bb;

  return strcmp(a->ts, b->ts);
}

#include <sys/stat.h>

#if USE_TSIN
void build_ts_gtab(int rebuild)
{
  if (!phidx)
    load_tsin_db();

  if (!ts_gtab_hash)
    ts_gtab_hash = tmalloc(int, HASHN+1);

  char fname[256];

  get_gcin_user_or_sys_fname("tsin-gtabidx2", fname);

  struct stat st_gtab, st_tsin32;
  FILE *fp;

//  dbg("%s %s\n", fname, tsfname);

#if 1
  if (!rebuild && !stat(fname, &st_gtab) && !stat(tsfname, &st_tsin32) &&
      st_tsin32.st_mtime < st_gtab.st_mtime) {

    if (fp=fopen(fname, "rb")) {
      printf(".......... from %s\n", fname);
      fread(&ts_gtabN, sizeof(ts_gtabN), 1, fp);
      fread(ts_gtab_hash, sizeof(int), HASHN+1, fp);
      ts_gtab = tmalloc(int, ts_gtabN);
      fread(ts_gtab, sizeof(int), ts_gtabN, fp);
      fclose(fp);
      return;
    }
  }
#endif

//  puts("oooooooooooooooooooooo");

  fseek(fph,0,SEEK_SET);

  if (ts_gtab) {
    free(ts_gtab);
    ts_gtab = NULL;
  }

  TS_TMP *tstmp=NULL;
  int tstmpN=0;
  usecount_t usecount;


  while (!feof(fph)) {
    if (!(tstmp=trealloc(tstmp, TS_TMP, tstmpN + 1)))
      p_err("tsin.c:realloc err");

    tstmp[tstmpN].ofs = ftell(fph);

    if (!read_tsin_phrase(tstmp[tstmpN].ts, &usecount))
      break;

    tstmpN++;
  }

  qsort(tstmp, tstmpN, sizeof(TS_TMP), qcmp_ts_gtab);

  int i;
  for(i=0; i <= HASHN;i++)
    ts_gtab_hash[i] = -1;

  ts_gtabN = tstmpN;
  ts_gtab = tmalloc(int, ts_gtabN);

  for(i=0; i < tstmpN; i++) {
    ts_gtab[i] = tstmp[i].ofs;
    usecount_t uc;
    u_char tstr[MAX_CIN_PHR];
    int tlen = load_ts_gtab(i, (char *)tstr, &uc);

    if (ts_gtab_hash[tstr[0]] < 0)
      ts_gtab_hash[tstr[0]] = i;
  }

  if (ts_gtab_hash[0]==-1)
    ts_gtab_hash[0]=0;

  ts_gtab_hash[HASHN]=ts_gtabN-1;
  for(i=HASHN-1;i>=0;i--)
    if (ts_gtab_hash[i] < 0)
      ts_gtab_hash[i] = ts_gtab_hash[i+1];

  for(i=1; i < HASHN; i++)
    if (ts_gtab_hash[i] < 0)
      ts_gtab_hash[i] = ts_gtab_hash[i-1];


  if (fp=fopen(fname, "wb")) {
    fwrite(&ts_gtabN, sizeof(ts_gtabN), 1, fp);
    fwrite(ts_gtab_hash, sizeof(int), HASHN+1, fp);
    fwrite(ts_gtab, sizeof(int), ts_gtabN, fp);
    fclose(fp);
  }

  free(tstmp);
}
#endif



#if USE_TSIN
int find_match(char *str, int *eq_N, usecount_t *usecount)
{
  int len = strlen(str);
  int ge_N = 0;

  *eq_N  = 0;
  *usecount = 0;

  if (!ts_gtabN || saved_phrase) {
    saved_phrase = FALSE;
    build_ts_gtab(0);
  }
#if 0
  int bottom = 0;
  int top = ts_gtabN - 1;
#else
  int hashi = ((u_char *)str)[0];
  int bottom=ts_gtab_hash[hashi];
  int top=ts_gtab_hash[hashi+1];
#endif
  int mid, tlen;
  char tstr[MAX_PHRASE_STR_LEN];
  usecount_t uc;
#if 0
  dbg("bot top  %s %d %d %d\n", str, ts_gtabN, bottom, top);
#endif
  do {
    mid = (bottom + top) /2;

//    dbg("tstr:%s  %d %d %d\n", tstr, bottom, mid, top);
    tlen = load_ts_gtab(mid, tstr, usecount);

    if (!tlen) {  // error in db
      dbg("error in db %s %d\n", str, mid);
      build_ts_gtab(1);
#if 1
      return 0;
#else
	exit(0);
#endif
    }

    int r = strncmp(str, tstr, len);

    if (r < 0) {
      top = mid - 1;
    }
    else
    if (r > 0) {
      bottom = mid + 1;
    } else {
      bottom = mid;
      int i;

      for(i=mid; i>=0; i--) {
        tlen = load_ts_gtab(i, tstr, &uc);

//	if (!strcmp(str,"水"))
//        printf("< %s %s\n", str, tstr);

        if (strncmp(str, tstr, len))
          break;

        ge_N++;
        if (strcmp(str, tstr))
          continue;

        (*eq_N)++;
        if (!*usecount)
          *usecount = uc;

        return ge_N;
      }

      for(i=mid+1; i<ts_gtabN; i++) {
        tlen = load_ts_gtab(i, tstr, &uc);

//	if (!strcmp(str,"水"))
//        printf("> %s %s\n", str, tstr);

        if (strncmp(str, tstr, len))
          break;

        ge_N++;
        if (strcmp(str, tstr))
          continue;

        (*eq_N)++;
        if (!*usecount)
          *usecount = uc;

        return ge_N;
      }

      return ge_N;
    }

  } while (bottom <= top);

//  dbg("%d %d\n", bottom, top);
  return 0;
}
#endif

void inc_gtab_usecount(char *str)
{
  int len = strlen(str);
  if (!len)
    return;

//  printf("inc %s\n", str);

  if (!ts_gtabN)
    build_ts_gtab(0);

  int bottom = 0;
  int top = ts_gtabN - 1;
  int mid, tlen;
  char tstr[MAX_PHRASE_STR_LEN];
  usecount_t uc;

  do {
    mid = (bottom + top) /2;

//    dbg("tstr:%s  %d %d %d\n", tstr, bottom, mid, top);
    tlen = load_ts_gtab(mid, tstr, &uc);

    if (!tlen) {  // error in db
      dbg("inc_gtab_usecount error in db\n");
      build_ts_gtab(1);
      return;
    }

    int r = strcmp(str, tstr);

    if (r < 0)
      top = mid - 1;
    else
    if (r > 0) {
      bottom = mid + 1;
    } else {
      uc++;
      u_char len;
      int ofs = ts_gtab[mid];
      fseek(fph, ofs, SEEK_SET);
      fread(&len, 1, 1, fph);
      fwrite(&uc, sizeof(usecount_t), 1, fph); // use count
      fflush(fph);
      return;
    }
  } while (bottom <= top);

  printf("inc_gtab_usecount failed %s\n", str);
  return;
}


void load_tsin_entry(int idx, char *len, usecount_t *usecount, phokey_t *pho,
                    u_char *ch)
{
  *usecount = 0;

  if (idx >= phcount) {
    load_tsin_db(); // probably db changed, reload;
    *len = 0;
    return;
  }

  int ph_ofs=phidx[idx];

  fseek(fph, ph_ofs, SEEK_SET);
  fread(len, 1, 1, fph);

  if (*len > MAX_PHRASE_LEN || *len <= 0) {
    dbg("err: tsin db changed reload");
    load_tsin_db(); // probably db changed, reload;
    *len = 0;
    return;
  }

  fread(usecount, sizeof(usecount_t), 1, fph); // use count
  fread(pho, sizeof(phokey_t), (int)(*len), fph);
  if (ch)
    fread(ch, CH_SZ, (int)(*len), fph);
}


int phokey_t_seq(phokey_t *a, phokey_t *b, int len)
{
  int i;

  for (i=0;i<len;i++) {
    if (a[i] > b[i]) return 1;
    else
    if (a[i] < b[i]) return -1;
  }

  return 0;
}

// ***  r_sti<=  range  < r_edi
gboolean tsin_seek(phokey_t *pho, int plen, int *r_sti, int *r_edi)
{
  int mid, cmp;
  phokey_t ss[MAX_PHRASE_LEN], stk[MAX_PHRASE_LEN];
  u_char mlen, stch[MAX_PHRASE_LEN * CH_SZ];
  char len;
  usecount_t usecount;
  int hashi= *pho >> TSIN_HASH_SHIFT;

  if (hashi >= TSIN_HASH_N)
    return FALSE;

  int top=hashidx[hashi];
  int bot=hashidx[hashi+1];

  if (top>=phcount)
    return FALSE;

  while (top <= bot) {
    mid=(top+bot)/ 2;
    load_tsin_entry(mid, &len, &usecount, ss, stch);

    if (len > plen)
      mlen=plen;
    else
      mlen=len;

    cmp=phokey_t_seq(ss, pho, mlen);

    if (!cmp && len < plen)
      cmp=-2;

    if (cmp>0)
      bot=mid-1;
    else
    if (cmp<0)
      top=mid+1;
    else
      break;
  }

  if (cmp) {
//    dbg("no match %d\n", cmp);
    return FALSE;
  }

  // seek to the first match because binary search is used
  int sti;
  for(sti = mid; sti>=0; sti--) {
    load_tsin_entry(sti, &len, &usecount, stk, stch);

    if (len >= plen && !phokey_t_seq(stk, pho, plen))
      continue;
    break;
  }
  sti++;

  // seek to the tail
  int edi;
  for(edi = mid; edi < phcount; edi++) {
    load_tsin_entry(edi, &len, &usecount, stk, stch);

    if (len >= plen && !phokey_t_seq(stk, pho, plen))
      continue;
    break;
  }

  *r_sti = sti;
  *r_edi = edi;

  return TRUE;
}

// och : orginal och;
void inc_dec_tsin_use_count(phokey_t *pho, char *ch, int N, gboolean b_dec)
{
  int sti, edi;

  if (!tsin_seek(pho, N, &sti, &edi))
    return;

  int idx;
  int tlen = utf8_tlen(ch, N);

#if 0
  dbg("otlen %d  ", tlen);
  int i;
  for(i=0; i < tlen; i++)
    putchar(ch[i]);
  puts("");
#endif

  for(idx=sti; idx < edi; idx++) {
    char len;
    usecount_t usecount, n_usecount;
    phokey_t phi[MAX_PHRASE_LEN];
    char stch[MAX_PHRASE_LEN * CH_SZ];

    load_tsin_entry(idx, &len, &usecount, phi, (u_char *)stch);
    n_usecount = usecount;

    if (len!=N || phokey_t_seq(phi, pho, N))
      break;
#if 0
    for(i=0; i < tlen; i++)
      putchar(stch[i]);
    dbg(" ppp\n");
#endif
    if (!utf8_str_eq(stch, ch, N))
      continue;
#if 0
    dbg("found match\n");
#endif
    int ph_ofs=phidx[idx];
    fseek(fph, ph_ofs + 1, SEEK_SET);

    if (b_dec) {
      if (usecount > -127)
        n_usecount--;
//      dbg("dec %d\n", n_usecount);
    } else {
      if (usecount < 0x3fffffff)
        n_usecount++;
//      dbg("inc %d\n", n_usecount);
    }

    if (n_usecount != usecount) {
      fwrite(&n_usecount, sizeof(usecount_t), 1, fph); // use count
      fflush(fph);
    }
  }
}
