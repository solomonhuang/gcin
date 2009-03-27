/*
	Copyright (C) 1995	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/

#include "gcin.h"
#include "pho.h"
#include "tsin.h"


int hashidx[TSIN_HASH_N];
int *phidx;
FILE *fph;
int phcount;
char tsidxfname[64]="";

static int a_phcount;
static char tsfname[64]="";

void load_tsin_db()
{
  if (!tsfname[0]) {
    if (!getenv("GCIN_TABLE_DIR"))
      get_gcin_user_fname("tsin", tsfname);
    else
      get_sys_table_file_name("tsin", tsfname);
  }


  strcpy(tsidxfname, tsfname);
  strcat(tsidxfname, ".idx");

  FILE *fr;

  if ((fr=fopen(tsidxfname,"r"))==NULL) {
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

  if ((fph=fopen(tsfname,"r+"))==NULL)
    p_err("Cannot open %s", tsfname);
}


void free_tsin()
{
  if (fph) {
    fclose(fph); fph = NULL;
  }

  if (phidx) {
    free(phidx); phidx = NULL;
  }
}


static int phseq(u_char *a, u_char *b)
{
  u_char lena, lenb, mlen;
  int i;
  phokey_t ka,kb;

  lena=*(a++); lenb=*(b++);
  a++; b++;   // skip usecount

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


gboolean save_phrase_to_db(phokey_t *phkeys, char *utf8str, int len, int usecount)
{
  int mid, ord = 0, ph_ofs, hashno, i;
  FILE *fw;
  u_char tbuf[MAX_PHRASE_LEN*(sizeof(phokey_t)+CH_SZ) +2],
         sbuf[MAX_PHRASE_LEN*(sizeof(phokey_t)+CH_SZ) +2];

  tbuf[0]=len;
  tbuf[1]=usecount;  // usecount
  int tlen = utf8_tlen(utf8str, len);
#if 0
  dbg("tlen %d  '", tlen);
  for(i=0; i < tlen; i++)
    putchar(utf8str[i]);
  dbg("'\n");
#endif

  memcpy(&tbuf[2], phkeys, sizeof(phokey_t) * len);
  memcpy(&tbuf[sizeof(phokey_t)*len + 2], utf8str, tlen);

  hashno=phkeys[0] >> TSIN_HASH_SHIFT;
  if (hashno >= TSIN_HASH_N)
    return FALSE;

  for(mid=hashidx[hashno]; mid<hashidx[hashno+1]; mid++) {
//    u_char usecount;

    ph_ofs=phidx[mid];
    fseek(fph, ph_ofs, SEEK_SET);
    fread(sbuf,1,1,fph);
    fread(&sbuf[1], 1, 1,fph); // use count
    fread(&sbuf[2], 1, (sizeof(phokey_t) + CH_SZ) * sbuf[0] + 2, fph);
    if ((ord=phseq(sbuf,tbuf)) >=0)
      break;
  }

//  dbg("tlen:%d  ord:%d  %s\n", tlen, ord, utf8str);
  if (!ord && !memcmp(&sbuf[sbuf[0]*sizeof(phokey_t)+1+1], utf8str, tlen)) {
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

  fwrite(tbuf, 1, sizeof(phokey_t)*len + tlen + 1+1, fph);
  fflush(fph);

  if (hashidx[hashno]>mid)
    hashidx[hashno]=mid;

  hashno++;

  for(;hashno<256;hashno++)
    hashidx[hashno]++;

  if ((fw=fopen(tsidxfname,"w"))==NULL) {
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
int ts_gtabN;

int read_tsin_phrase(char *str)
{
  u_char len;
  char usecount;
  u_char pho[sizeof(phokey_t) * MAX_PHRASE_LEN];
  len = 0;

  fread(&len, 1, 1, fph);
  if (len > MAX_PHRASE_LEN || len <=0)
    return 0;
  fread(&usecount, 1, 1,fph); // use count
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

typedef struct {
  char ts[MAX_PHRASE_STR_LEN];
  int ofs;
} TS_TMP;

static int qcmp_ts_gtab(const void *aa, const void *bb)
{
  TS_TMP *a = (TS_TMP *)aa, *b = (TS_TMP *)bb;

  return strcmp(a->ts, b->ts);
}


void build_ts_gtab()
{
  load_tsin_db();

  fseek(fph,0,SEEK_SET);

  if (ts_gtab) {
    free(ts_gtab);
    ts_gtab = NULL;
  }

  TS_TMP *tstmp=NULL;
  int tstmpN=0;

  while (!feof(fph)) {
    if (!(tstmp=trealloc(tstmp, TS_TMP, tstmpN + 1)))
      p_err("tsin.c:realloc err");

    tstmp[tstmpN].ofs = ftell(fph);

    if (!read_tsin_phrase(tstmp[tstmpN].ts))
      break;

    tstmpN++;
  }

  qsort(tstmp, tstmpN, sizeof(TS_TMP), qcmp_ts_gtab);

  ts_gtabN = tstmpN;
  ts_gtab = tmalloc(int, ts_gtabN);

  int i;
  for(i=0; i < tstmpN; i++) {
    ts_gtab[i] = tstmp[i].ofs;
  }

  free(tstmp);
}


static int load_ts_gtab(int idx, char *tstr)
{
  int ofs = ts_gtab[idx];

  fseek(fph, ofs, SEEK_SET);
  return read_tsin_phrase(tstr);
}

// len is in CH_SZ
int find_match(char *str, int len, char *match_chars, int match_chars_max)
{
  if (!len)
    return 0;

  if (!ts_gtabN)
    build_ts_gtab();

  int bottom = 0;
  int top = ts_gtabN - 1;
  int mid, tlen;
  char tstr[MAX_PHRASE_STR_LEN];
  int matchN=0;

  do {
    mid = (bottom + top) /2;

//    dbg("tstr:%s  %d %d %d\n", tstr, bottom, mid, top);
    tlen = load_ts_gtab(mid, tstr);

    if (!tlen) {  // error in db
      dbg("error in db\n");
      build_ts_gtab();
      return 0;
    }

    int r = strncmp(str, tstr, len);

    if (r < 0) {
      top = mid - 1;
    }
    else
    if (r > 0 || strlen(tstr)==len) {
      bottom = mid + 1;
    } else {
      strcpy(str, tstr);

      if (!match_chars)
        return 1;

      bottom = mid;
      int i;

      for(i=mid; i>=0; i--) {
        tlen = load_ts_gtab(i, tstr);

        if (strncmp(str, tstr, len) || tlen <= len)
          break;

        if (matchN >= match_chars_max)
          break;

        memcpy(&match_chars[matchN * CH_SZ], &tstr[len], CH_SZ);
//        dbg("zzz %c%c%c\n", match_chars[0], match_chars[1], match_chars[2]);
        matchN++;
        match_chars[matchN * CH_SZ] = 0;

//        dbg("iiiiii '%s' %d %d %s\n", tstr, tlen, len, match_chars);
      }

      for(i=mid+1; i< ts_gtabN; i++) {
        tlen = load_ts_gtab(i, tstr);

        if (strncmp(str, tstr, len) || tlen <= len)
          break;

        if (matchN >= match_chars_max)
          break;

        memcpy(&match_chars[matchN * CH_SZ], &tstr[len], CH_SZ);
        matchN++;
      }

      return matchN;
    }

  } while (bottom <= top);

//  dbg("%d %d\n", bottom, top);
  return 0;
}


void load_tsin_entry(int idx, u_char *len, char *usecount, phokey_t *pho,
                    u_char *ch)
{
  if (idx >= phcount) {
    load_tsin_db(); // probably db changed, reload;
    *len = 0;
    return;
  }

  int ph_ofs=phidx[idx];

  fseek(fph, ph_ofs, SEEK_SET);
  fread(len, 1, 1, fph);

  if (*len >= MAX_PHRASE_LEN) {
    load_tsin_db(); // probably db changed, reload;
    *len = 0;
    return;
  }

  fread(usecount, 1, 1,fph); // use count
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


gboolean tsin_seek(phokey_t *pho, int plen, int *r_sti, int *r_edi)
{
  int mid, cmp;
  phokey_t ss[MAX_PHRASE_LEN], stk[MAX_PHRASE_LEN];
  u_char len, mlen, stch[MAX_PHRASE_LEN * CH_SZ];
  char usecount;
  int sti, edi;
  int i= *pho >> TSIN_HASH_SHIFT;

  if (i >= TSIN_HASH_N)
    return FALSE;

  int top=hashidx[i];
  int bot=edi=hashidx[i+1];

  while (top <= bot) {
    mid=(top+bot)/ 2;
    sti++;
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
  for(;mid>=0;mid--) {
    load_tsin_entry(mid, &len, &usecount, stk, stch);

    if (len >= plen && !phokey_t_seq(stk, pho, plen))
      continue;
    break;
  }

  mid++;
  sti = mid;

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
  dbg("otlen %d\n", tlen);
  int i;
  for(i=0; i < tlen; i++)
    putchar(ch[i]);
  puts("");
#endif


  for(idx=sti; idx < edi; idx++) {
    char len, usecount, n_usecount;
    phokey_t phi[MAX_PHRASE_LEN];
    char stch[MAX_PHRASE_LEN * CH_SZ];

    load_tsin_entry(idx, &len, &usecount, phi, stch);
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
    dbg("found match ");
#endif
    int ph_ofs=phidx[idx];
    fseek(fph, ph_ofs + 1, SEEK_SET);

    if (b_dec) {
      if (usecount > -127)
        n_usecount--;
//      dbg("dec %d\n", n_usecount);
    } else {
      if (usecount < 126)
        n_usecount++;
//      dbg("inc %d\n", n_usecount);
    }

    if (n_usecount != usecount) {
      fwrite(&n_usecount, 1, 1, fph); // use count
      fflush(fph);
    }
  }
}
