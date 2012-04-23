/*
	Copyright (C) 1995-2008	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/

#include "gcin.h"
#include "pho.h"
#include "tsin.h"
#include "gtab.h"
#include "gst.h"
#include "lang.h"
#include <sys/stat.h>

//int hashidx[TSIN_HASH_N];
TSIN_HANDLE tsin_hand;

int ph_key_sz; // bytes
gboolean tsin_is_gtab;
static int tsin_hash_shift;

#define PHIDX_SKIP  (sizeof(tsin_hand.phcount) + sizeof(tsin_hand.hashidx))

char *current_tsin_fname;
time_t current_modify_time;
int ts_gtabN;
static int *ts_gtab_hash;
#define HASHN 256

void get_gcin_user_or_sys_fname(char *name, char fname[]);


static void get_modify_time(TSIN_HANDLE *ptsin_hand)
{
  struct stat st;
  if (!fstat(fileno(ptsin_hand->fph), &st)) {
    ptsin_hand->modify_time = st.st_mtime;
  }
}

void load_tsin_db_ex(TSIN_HANDLE *ptsin_hand, char *infname, gboolean is_gtab_i, gboolean read_only, gboolean use_idx)
{
  char tsidxfname[512];
  char *fmod = read_only?"rb":"rb+";
//  dbg("cur %s %s\n", infname, current_tsin_fname);

  if (ptsin_hand==&tsin_hand && current_tsin_fname && !strcmp(current_tsin_fname, infname))
    return;

  strcpy(tsidxfname, infname);
  strcat(tsidxfname, ".idx");

//  dbg("tsidxfname %s\n", tsidxfname);

  FILE *fp_phidx = ptsin_hand->fp_phidx, *fph = ptsin_hand->fph;

  if (use_idx) {
    if ((fp_phidx=fopen(tsidxfname, fmod))==NULL) {
      p_err("load_tsin_db_ex A Cannot open '%s'\n", tsidxfname);
    }
    ptsin_hand->fp_phidx = fp_phidx;

    fread(&ptsin_hand->phcount,4,1, fp_phidx);
    fread(&ptsin_hand->hashidx,1,sizeof(ptsin_hand->hashidx), fp_phidx);
#if     0
  printf("phcount:%d\n",phcount);
#endif
    ptsin_hand->a_phcount=ptsin_hand->phcount+256;
  }


  if (fph)
    fclose(fph);

  dbg("tsfname: %s\n", infname);

  if ((fph=fopen(infname, fmod))==NULL)
    p_err("load_tsin_db0 B Cannot open '%s'", infname);
  ptsin_hand->fph = fph;

  free(current_tsin_fname);
  current_tsin_fname = strdup(infname);


  get_modify_time(ptsin_hand);

  if (is_gtab_i) {
    TSIN_GTAB_HEAD head;
    fread(&head, sizeof(head), 1, fph);
    if (head.keybits*head.maxkey > 32) {
      ph_key_sz = 8;
      tsin_hash_shift = TSIN_HASH_SHIFT_64;
    }
    else {
      ph_key_sz = 4;
      tsin_hash_shift = TSIN_HASH_SHIFT_32;
    }
  } else {
    ph_key_sz = 2;
    tsin_hash_shift = TSIN_HASH_SHIFT;
  }
  tsin_is_gtab = is_gtab_i;
}


void load_tsin_db0(char *infname, gboolean is_gtab_i)
{
  load_tsin_db_ex(&tsin_hand, infname, is_gtab_i, FALSE, TRUE);
}

void free_tsin_ex(TSIN_HANDLE *ptsin_hand)
{
  free(current_tsin_fname); current_tsin_fname=NULL;

  if (ptsin_hand->fph) {
    fclose(ptsin_hand->fph); ptsin_hand->fph = NULL;
  }

  if (ptsin_hand->fp_phidx) {
    fclose(ptsin_hand->fp_phidx); ptsin_hand->fp_phidx=NULL;
  }
}

void free_tsin()
{
  free_tsin_ex(&tsin_hand);
}

extern gboolean is_chs;
void load_tsin_db()
{
  char tsfname[512];
  char *fname = tsin32_f;

  get_gcin_user_or_sys_fname(fname, tsfname);
  load_tsin_db0(tsfname, FALSE);
}

static void seek_fp_phidx(TSIN_HANDLE *ptsin_hand, int i)
{
  fseek(ptsin_hand->fp_phidx, PHIDX_SKIP + i*sizeof(int), SEEK_SET);
}

void reload_tsin_db()
{
  char tt[512];
  if (!current_tsin_fname)
    return;

  strcpy(tt, current_tsin_fname);
  free(current_tsin_fname); current_tsin_fname = NULL;
  load_tsin_db0(tt, tsin_is_gtab);
}

inline static int get_phidx(TSIN_HANDLE *ptsin_hand, int i)
{
  seek_fp_phidx(ptsin_hand, i);
  int t;
  fread(&t, sizeof(int), 1, ptsin_hand->fp_phidx);

  if (tsin_is_gtab)
    t += sizeof(TSIN_GTAB_HEAD);

  return t;
}


inline int phokey_t_seq16(phokey_t *a, phokey_t *b, int len)
{
  int i;

  for (i=0;i<len;i++) {
    if (a[i] > b[i]) return 1;
    else
    if (a[i] < b[i]) return -1;
  }

  return 0;
}


inline int phokey_t_seq32(u_int *a, u_int *b, int len)
{
  int i;

  for (i=0;i<len;i++) {
    if (a[i] > b[i]) return 1;
    else
    if (a[i] < b[i]) return -1;
  }

  return 0;
}


inline int phokey_t_seq64(u_int64_t *a, u_int64_t *b, int len)
{
  int i;

  for (i=0;i<len;i++) {
    if (a[i] > b[i]) return 1;
    else
    if (a[i] < b[i]) return -1;
  }

  return 0;
}


static int phokey_t_seq(void *a, void *b, int len)
{
  if (ph_key_sz==2)
    return phokey_t_seq16((phokey_t *)a, (phokey_t *)b, len);
  if (ph_key_sz==4)
    return phokey_t_seq32((u_int *)a, (u_int *)b, len);
  if (ph_key_sz==8)
    return phokey_t_seq64((u_int64_t*)a, (u_int64_t*)b, len);
  return 0;
}


static int phseq(u_char *a, u_char *b)
{
  u_char lena, lenb, mlen;

  lena=*(a++); lenb=*(b++);
  a+=sizeof(usecount_t); b+=sizeof(usecount_t);   // skip usecount

  mlen=Min(lena,lenb);
  u_int64_t ka[MAX_PHRASE_LEN], kb[MAX_PHRASE_LEN];

  memcpy(ka, a, ph_key_sz * mlen);
  memcpy(kb, b, ph_key_sz * mlen);

  int d = phokey_t_seq(a, b, mlen);
  if (d)
    return d;

  if (lena > lenb) return 1;
  if (lena < lenb) return -1;
  return 0;
}

void inc_dec_tsin_use_count(void *pho, char *ch, int N);

static gboolean saved_phrase;


static void reload_if_modified()
{
  struct stat st;
  if (fstat(fileno(tsin_hand.fph), &st) || tsin_hand.modify_time != st.st_mtime) {
    reload_tsin_db();
  }
}


gboolean save_phrase_to_db(void *phkeys, char *utf8str, int len, usecount_t usecount)
{
  reload_if_modified();

  int mid, ord = 0, ph_ofs, hashno;
  u_char tbuf[MAX_PHRASE_LEN*(sizeof(u_int64_t)+CH_SZ) + 1 + sizeof(usecount_t)],
         sbuf[MAX_PHRASE_LEN*(sizeof(u_int64_t)+CH_SZ) + 1 + sizeof(usecount_t)];

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

  dbg("save_phrase_to_db '%s'  tlen:%d\n", utf8str, tlen);

  memcpy(&tbuf[1 + sizeof(usecount_t)], phkeys, ph_key_sz * len);
  memcpy(&tbuf[ph_key_sz*len + 1 + sizeof(usecount_t)], utf8str, tlen);

  if (ph_key_sz==2)
    hashno= *((phokey_t *)phkeys) >> TSIN_HASH_SHIFT;
  else if (ph_key_sz==4)
    hashno= *((u_int *)phkeys) >> TSIN_HASH_SHIFT_32;
  else
    hashno= *((u_int64_t *)phkeys) >> TSIN_HASH_SHIFT_64;

//  dbg("hashno %d\n", hashno);

  if (hashno >= TSIN_HASH_N)
    return FALSE;

  for(mid=tsin_hand.hashidx[hashno]; mid<tsin_hand.hashidx[hashno+1]; mid++) {
    ph_ofs=get_phidx(&tsin_hand, mid);

    fseek(tsin_hand.fph, ph_ofs, SEEK_SET);
    fread(sbuf,1,1,tsin_hand.fph);
    fread(&sbuf[1], sizeof(usecount_t), 1, tsin_hand.fph); // use count
    fread(&sbuf[1+sizeof(usecount_t)], 1, (ph_key_sz + CH_SZ) * sbuf[0], tsin_hand.fph);
    if ((ord=phseq(sbuf,tbuf)) > 0)
      break;

    if (!ord && !memcmp(&sbuf[sbuf[0]*ph_key_sz+1+sizeof(usecount_t)], utf8str, tlen)) {
//    bell();
      dbg("Phrase already exists\n");
      inc_dec_tsin_use_count(phkeys, utf8str, len);
      return FALSE;
    }
  }

  int wN = tsin_hand.phcount - mid;

//  dbg("wN %d  phcount:%d mid:%d\n", wN, phcount, mid);

  if (wN > 0) {
    int *phidx = tmalloc(int, wN);
    seek_fp_phidx(&tsin_hand, mid);
    fread(phidx, sizeof(int), wN, tsin_hand.fp_phidx);
    seek_fp_phidx(&tsin_hand, mid+1);
    fwrite(phidx, sizeof(int), wN, tsin_hand.fp_phidx);
    free(phidx);
  }

  fseek(tsin_hand.fph,0,SEEK_END);

  ph_ofs=ftell(tsin_hand.fph);
  if (tsin_is_gtab)
    ph_ofs -= sizeof(TSIN_GTAB_HEAD);

//  dbg("ph_ofs %d  ph_key_sz:%d\n", ph_ofs, ph_key_sz);
  seek_fp_phidx(&tsin_hand, mid);
  fwrite(&ph_ofs, sizeof(int), 1, tsin_hand.fp_phidx);
  tsin_hand.phcount++;

  fwrite(tbuf, 1, ph_key_sz*len + tlen + 1+ sizeof(usecount_t), tsin_hand.fph);
  fflush(tsin_hand.fph);

  if (tsin_hand.hashidx[hashno]>mid)
    tsin_hand.hashidx[hashno]=mid;

  for(hashno++; hashno<TSIN_HASH_N; hashno++)
    tsin_hand.hashidx[hashno]++;

  rewind(tsin_hand.fp_phidx);
  fwrite(&tsin_hand.phcount, sizeof(tsin_hand.phcount), 1, tsin_hand.fp_phidx);
  fwrite(&tsin_hand.hashidx,sizeof(tsin_hand.hashidx),1, tsin_hand.fp_phidx);
  fflush(tsin_hand.fp_phidx);


  get_modify_time(&tsin_hand);

//  dbg("ofs %d\n", get_phidx(mid));

  return TRUE;
}


#include <sys/stat.h>


void load_tsin_entry0_ex(TSIN_HANDLE *ptsin_hand, char *len, usecount_t *usecount, void *pho, u_char *ch)
{
  *usecount = 0;
  *len = 0;
  fread(len, 1, 1, ptsin_hand->fph);

  if (*len > MAX_PHRASE_LEN || *len <= 0) {
    dbg("err: tsin db changed reload len:%d\n", *len);
    if (ptsin_hand==&tsin_hand)
      reload_tsin_db(); // probably db changed, reload;
    *len = 0;
    return;
  }

  fread(usecount, sizeof(usecount_t), 1, ptsin_hand->fph); // use count
  fread(pho, ph_key_sz, (int)(*len), ptsin_hand->fph);
  if (ch) {
    fread(ch, CH_SZ, (int)(*len), ptsin_hand->fph);
    int tlen = utf8_tlen((char *)ch, *len);
    ch[tlen]=0;
  }
}

void load_tsin_entry_ex(TSIN_HANDLE *ptsin_hand, int idx, char *len, usecount_t *usecount, void *pho, u_char *ch)
{
  *usecount = 0;

  if (idx >= ptsin_hand->phcount) {
    reload_tsin_db(); // probably db changed, reload;
    *len = 0;
    return;
  }

  int ph_ofs=get_phidx(ptsin_hand, idx);
//  dbg("idx %d:%d\n", idx, ph_ofs);

  fseek(ptsin_hand->fph, ph_ofs , SEEK_SET);
  load_tsin_entry0_ex(ptsin_hand, len, usecount, pho, ch);
}

void load_tsin_entry(int idx, char *len, usecount_t *usecount, void *pho, u_char *ch)
{
  load_tsin_entry_ex(&tsin_hand, idx, len, usecount, pho, ch);
}

// tone_mask : 1 -> pho has tone
void mask_tone(phokey_t *pho, int plen, char *tone_mask)
{
  int i;
//  dbg("mask_tone\n");
  if (!tone_mask)
    return;

  for(i=0; i < plen; i++) {
   if (!tone_mask[i])
    pho[i] &= (~7);
  }
}


// ***  r_sti<=  range  < r_edi
gboolean tsin_seek_ex(TSIN_HANDLE *ptsin_hand, void *pho, int plen, int *r_sti, int *r_edi, char *tone_mask)
{
  int mid, cmp;
  u_int64_t ss[MAX_PHRASE_LEN], stk[MAX_PHRASE_LEN];
  char len;
  usecount_t usecount;
  int hashi;

#if 0
  dbg("tsin_seek %d\n", plen);
#endif

#if 0
  if (tone_mask)
    mask_tone((phokey_t *)pho, plen, tone_mask);
#endif

  if (ph_key_sz==2)
    hashi= *((phokey_t *)pho) >> TSIN_HASH_SHIFT;
  else if (ph_key_sz==4)
    hashi= *((u_int *)pho) >> TSIN_HASH_SHIFT_32;
  else
    hashi= *((u_int64_t *)pho) >> TSIN_HASH_SHIFT_64;

  if (hashi >= TSIN_HASH_N) {
//    dbg("hashi >= TSIN_HASH_N\n");
    return FALSE;
  }

  int top=ptsin_hand->hashidx[hashi];
  int bot=ptsin_hand->hashidx[hashi+1];

  if (top>=ptsin_hand->phcount) {
//    dbg("top>=phcount\n");
    return FALSE;
  }

  while (top <= bot) {
    mid=(top+bot)/ 2;
    load_tsin_entry_ex(ptsin_hand, mid, &len, &usecount, ss, NULL);

    u_char mlen;
    if (len > plen)
      mlen=plen;
    else
      mlen=len;

//    prphs(ss, mlen);
//    mask_tone((phokey_t *)ss, mlen, tone_mask);

#if DBG || 0
    int j;
    dbg("> ");
    prphs(ss, len);
    dbg("\n");
#endif

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

  if (cmp && !tone_mask) {
//    dbg("no match %d\n", cmp);
    return FALSE;
  }

//  dbg("<--\n");
  // seek to the first match because binary search is used
  gboolean found=FALSE;

  int sti;
  for(sti = mid; sti>=0; sti--) {
    load_tsin_entry_ex(ptsin_hand, sti, &len, &usecount, stk, NULL);

    u_char mlen;
    if (len > plen)
      mlen=plen;
    else
      mlen=len;
#if 0
    prphs(stk, len);
#endif
    mask_tone((phokey_t *)stk, mlen, tone_mask);

    int v = phokey_t_seq(stk, pho, plen);
    if (!v)
      found = TRUE;

#if 0
    int j;
    dbg("%d] %d*> ", sti, mlen);
    prphs(stk, len);
    dbg(" v:%d\n", v);
#endif

    if ((!tone_mask && !v && len>=plen) ||
        (tone_mask && v>0 || !v && len >= plen))
      continue;
    break;
  }
  sti++;

  // seek to the tail

  if (tone_mask) {
    int top=ptsin_hand->hashidx[hashi];
    int bot=ptsin_hand->hashidx[hashi+1];

    if (top>=ptsin_hand->phcount) {
  //    dbg("top>=phcount\n");
      return FALSE;
    }

    phokey_t tpho[MAX_PHRASE_LEN];

    int i;
    for(i=0; i < plen; i++)
      tpho[i]=((phokey_t*)pho)[i] | 7;

    while (top <= bot) {
      mid=(top+bot)/ 2;
      load_tsin_entry_ex(ptsin_hand, mid, &len, &usecount, ss, NULL);

      u_char mlen;
      if (len > plen)
        mlen=plen;
      else
        mlen=len;

  //    prphs(ss, mlen);

#if DBG || 0
      int j;
      dbg("> ");
      prphs(ss, len);
      dbg("\n");
#endif

      cmp=phokey_t_seq(ss, tpho, mlen);

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
  }

  int edi;
  for(edi = mid; edi < ptsin_hand->phcount; edi++) {
    load_tsin_entry_ex(ptsin_hand, edi, &len, &usecount, stk, NULL);

    u_char mlen;
    if (len > plen)
      mlen=plen;
    else
      mlen=len;
#if 0
    prphs(stk, len);
#endif
    mask_tone((phokey_t *)stk, mlen, tone_mask);

    int v = phokey_t_seq(stk, pho, plen);
    if (!v)
      found = TRUE;
#if 0
    dbg("edi%d -> ", edi);
    prphs(stk, len);
    dbg(" v:%d\n", v);
#endif

    if ((!tone_mask && !v && len >= plen)
      || (tone_mask && v<0 || !v && len >= plen))
      continue;
    break;
  }

#if 0
  dbg("sti%d edi:%d found:%d\n", sti, edi, found);
#endif

  *r_sti = sti;
  *r_edi = edi;

  return edi > sti;
}


gboolean tsin_seek(void *pho, int plen, int *r_sti, int *r_edi, char *tone_mask)
{
  return tsin_seek_ex(&tsin_hand, pho, plen, r_sti, r_edi, tone_mask);
}

void inc_dec_tsin_use_count(void *pho, char *ch, int N)
{
  int sti, edi;

  reload_if_modified();

//  dbg("inc_dec_tsin_use_count '%s'\n", ch);

  if (!tsin_seek(pho, N, &sti, &edi, NULL))
    return;

  int idx;
  int tlen = strlen(ch);

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
    u_int64_t phi[MAX_PHRASE_LEN];
    char stch[MAX_PHRASE_LEN * CH_SZ * 2];

    load_tsin_entry(idx, &len, &usecount, phi, (u_char *)stch);
    n_usecount = usecount;

    if (len!=N || phokey_t_seq(phi, pho, N))
      break;
#if 0
    for(i=0; i < tlen; i++)
      putchar(stch[i]);
    dbg(" ppp\n");
#endif

//	dbg("stch %s\n", stch);
    if (strcmp(stch, ch))
      continue;
#if 0
    dbg("found match\n");
#endif
    int ph_ofs=get_phidx(&tsin_hand, idx);

    fseek(tsin_hand.fph, ph_ofs + 1, SEEK_SET);

    if (usecount < 0x3fffffff)
      n_usecount++;

    if (n_usecount != usecount) {
      fwrite(&n_usecount, sizeof(usecount_t), 1, tsin_hand.fph); // use count
      fflush(tsin_hand.fph);
    }
  }


  get_modify_time(&tsin_hand);
}
