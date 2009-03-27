/*
	Copyright (C) 1995	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/

#include "gcin.h"
#include "pho.h"

#define b2cpy(a,b) memcpy(a,b,2)

int hashidx[TSIN_HASH_N];
int *phidx;
FILE *fph;
int phcount, a_phcount;
char tsfname[64]="";
char tsidxfname[64]="";

void load_tsin_db()
{
  if (!tsfname[0]) {
    char tt[128];

#ifndef  NO_PRIVATE_TSIN
    get_gcin_conf_fname("tsin", tsfname);
#else
    get_sys_table_file_name("tsin", tsfname);
#endif
  }

  strcpy(tsidxfname,tsfname);
  strcat(tsidxfname,".idx");

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
  u_short ka,kb;

  lena=*(a++); lenb=*(b++);
  a++; b++;   // skip usecount

  mlen=Min(lena,lenb);

  for(i=0;i<mlen; i++) {
    memcpy(&ka,a,2);
    memcpy(&kb,b,2);
    if (ka > kb) return 1;
    if (ka < kb) return -1;
    a+=2; b+=2;
  }

  if (lena > lenb) return 1;
  if (lena < lenb) return -1;
  return 0;
}


gboolean save_phrase_to_db(phokey_t *phkeys, char *big5str, int len)
{
  int tt, ofs, top,bottom, mid, ord, ph_ofs, hashno, hashno_end, i;
  FILE *fw;
  u_char tbuf[MAX_PHRASE_LEN*4+2], sbuf[MAX_PHRASE_LEN*4+2], ch[MAX_PHRASE_LEN*2+1];

  tbuf[0]=len;
  tbuf[1]=0;  // usecount
  memcpy(&tbuf[2], phkeys, sizeof(phokey_t)* len);
  memcpy(&tbuf[sizeof(phokey_t)*len + 2], big5str, 2*len);

  hashno=phkeys[0] >> TSIN_HASH_SHIFT;
  if (hashno >= TSIN_HASH_N)
    return FALSE;

  for(mid=hashidx[hashno]; mid<hashidx[hashno+1]; mid++) {
    u_char usecount;

    ph_ofs=phidx[mid];
    fseek(fph, ph_ofs, SEEK_SET);
    fread(sbuf,1,1,fph);
    fread(&sbuf[1], 1, 1,fph); // use count
    fread(&sbuf[2], 1, (sizeof(phokey_t) + 2) * sbuf[0] + 2, fph);
    if ((ord=phseq(sbuf,tbuf)) >=0)
      break;
  }

  tt=sbuf[0]*2;
  if (!ord && !memcmp(&sbuf[tt+1+1], big5str, tt)) {
//    bell();
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

  fwrite(tbuf,1,4*len+1+1,fph);
  fflush(fph);

  if (hashidx[hashno]>mid) hashidx[hashno]=mid;
  hashno++;
  for(;hashno<256;hashno++) hashidx[hashno]++;

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
