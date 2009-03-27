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
  if (phidx) {
    free(phidx); phidx = NULL;
  }

  if (fph) {
    fclose(fph); fph = NULL;
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




gboolean save_phrase_to_db(phokey_t *phkeys, char *utf8str, int len)
{
  int mid, ord = 0, ph_ofs, hashno, i;
  FILE *fw;
  u_char tbuf[MAX_PHRASE_LEN*(sizeof(phokey_t)+CH_SZ) +2],
         sbuf[MAX_PHRASE_LEN*(sizeof(phokey_t)+CH_SZ) +2];

  tbuf[0]=len;
  tbuf[1]=0;  // usecount

  memcpy(&tbuf[2], phkeys, sizeof(phokey_t) * len);
  memcpy(&tbuf[sizeof(phokey_t)*len + 2], utf8str, CH_SZ*len);

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

  int tlen = sbuf[0]*CH_SZ;
//  dbg("tlen:%d  ord:%d  %s\n", tlen, ord, utf8str);
  if (!ord && !memcmp(&sbuf[sbuf[0]*sizeof(phokey_t)+1+1], utf8str, tlen)) {
//    bell();
    dbg("Phrase already exists\n");
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

  fwrite(tbuf, 1, (sizeof(phokey_t)+CH_SZ)*len+1+1, fph);
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
  u_char len, usecount;
  u_char pho[sizeof(phokey_t) * MAX_PHRASE_LEN];
  len = 0;

  fread(&len, 1, 1, fph);
  if (len > MAX_PHRASE_LEN || len <=0)
    return 0;
  fread(&usecount, 1, 1,fph); // use count
  fread(pho, sizeof(phokey_t), len, fph);
  fread(str, CH_SZ, len, fph);
  str[len * CH_SZ] = 0;

  return len * CH_SZ;
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
