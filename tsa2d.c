/*
	Copyright (C) 1995	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include "gcin.h"
#include "pho.h"

typedef struct {
	u_char ch[CH_SZ];
	u_short ph;
} ITEM;


int *phidx, *sidx, phcount;
int bfsize, phidxsize;
u_char *bf;
u_char *sf;

static int qcmp(const void *a, const void *b)
{
  int idxa=*((int *)a);
  int idxb=*((int *)b);
  u_char lena,lenb, len;
  int cha, chb;
  int i;
  u_short ka,kb;

  lena=bf[idxa]; idxa+=2;
  lenb=bf[idxb]; idxb+=2;
  cha=idxa + lena * sizeof(phokey_t);
  chb=idxb + lenb * sizeof(phokey_t);
  len=Min(lena,lenb);

  for(i=0;i<len;i++) {
    memcpy(&ka, &bf[idxa], sizeof(phokey_t));
    memcpy(&kb, &bf[idxb], sizeof(phokey_t));
    if (ka > kb) return 1;
    if (kb > ka) return -1;
    idxa+=2;
    idxb+=2;
  }

  if (lena > lenb)
    return 1;
  if (lena < lenb)
    return -1;

  int tlena = utf8_tlen(&bf[cha], lena);
  int tlenb = utf8_tlen(&bf[chb], lenb);

  if (tlena > tlenb)
    return 1;
  if (tlena < tlenb)
    return -1;

  return memcmp(&bf[cha], &bf[chb], tlena);
}

static int shiftb[]={9,7,3,0};

int lookup(u_char *s)
{
  int i;
  char tt[3], *pp;

  if (*s < 128)
    return *s-'0';

  int len = utf8_sz(s);

  bchcpy(tt, s);
  tt[len]=0;


  for(i=0;i<3;i++)
    if ((pp=strstr(pho_chars[i],tt)))
      break;

  if (i==3)
    return 0;

  return (((pp-pho_chars[i])/CH_SZ) << shiftb[i]);
}


int main(int argc, char **argv)
{
  FILE *fp,*fw;
  u_char s[1024];
  u_char chbuf[MAX_PHRASE_LEN * CH_SZ];
  u_short phbuf[80];
  int i,j,idx,len, ofs;
  u_short kk;
  int hashidx[TSIN_HASH_N];
  u_char clen;
  gboolean reload = getenv("GCIN_NO_RELOAD")==NULL;

  if (reload)
    gtk_init(&argc, &argv);

  if (argc > 1) {
    if ((fp=fopen(argv[1], "r"))==NULL) {
       printf("Cannot open %s\n", argv[1]);
       exit(-1);
    }
  } else
    fp=stdin;

  bfsize=300000;
  if (!(bf=(u_char *)malloc(bfsize))) {
    puts("malloc err");
    exit(1);
  }

  phidxsize=18000;
  if (!(phidx=(int *)malloc(phidxsize*4))) {
    puts("malloc err");
    exit(1);
  }

  int lineCnt=0;
  phcount=ofs=0;
  while (!feof(fp)) {
    char usecount=0;

    lineCnt++;

    fgets(s,sizeof(s),fp);
    len=strlen(s);
    if (s[0]=='#')
      continue;

    if (s[len-1]=='\n')
      s[--len]=0;

    if (len==0)
      continue;

    i=0;
    int chbufN=0;
    int charN = 0;
    while (s[i]!=' ' && i<len) {
      int len = utf8_sz(&s[i]);

      memcpy(&chbuf[chbufN], &s[i], len);

      i+=len;
      chbufN+=len;
      charN++;
    }

    i++;
    int phbufN=0;
    while (i<len && phbufN < charN && s[i]!=' ') {
      kk=0;

      while (s[i]!=' ' && i<len) {
        kk |= lookup(&s[i]);

        if (s[i]&128)
          i += CH_SZ;
        else
          i++;
      }

      i++;
      phbuf[phbufN++]=kk;
    }

    if (phbufN!=charN) {
      fprintf(stderr, "Line %d problem in phbufN!=chbufN %d != %d\n",
        lineCnt, phbufN, chbufN);
      exit(-1);
    }

    clen=phbufN;

    while (i<len && s[i]==' ')
      i++;

    if (i==len)
      usecount = 0;
    else
      usecount = atoi(&s[i]);


    /*      printf("len:%d\n", clen); */
    phidx[phcount++]=ofs;
    memcpy(&bf[ofs++],&clen,1);
    memcpy(&bf[ofs++],&usecount,1);
    memcpy(&bf[ofs],phbuf, clen * sizeof(phokey_t));
    ofs+=clen * sizeof(phokey_t);
    memcpy(&bf[ofs], chbuf, chbufN);
    ofs+=chbufN;
    if (ofs+100 >= bfsize) {
      bfsize+=65536;
      if (!(bf=(u_char *)realloc(bf,bfsize))) {
        puts("realloc err");
        exit(1);
      }
    }
    if (phcount+100 >= phidxsize) {
      phidxsize+=1000;
      if (!(phidx=(int *)realloc(phidx,phidxsize*4))) {
        puts("realloc err");
        exit(1);
      }
    }
  }
  fclose(fp);

  /* dumpbf(bf,phidx); */

  puts("Sorting ....");
  qsort(phidx,phcount,4,qcmp);

  if (!(sf=(u_char *)malloc(bfsize))) {
    puts("malloc err");
    exit(1);
  }

  if (!(sidx=(int *)malloc(phidxsize*4))) {
    puts("malloc err");
    exit(1);
  }


  // delete duplicate
  ofs=0;
  j=0;
  bzero(s,sizeof(s));
  for(i=0;i<phcount;i++) {
    idx = phidx[i];
    sidx[j]=ofs;
    len=bf[idx];
    int tlen = utf8_tlen(&bf[idx + 1 + 1 + sizeof(phokey_t)*len], len);
    clen=sizeof(phokey_t)*len + tlen + 1 + 1;

    if (memcmp(s, &bf[idx], clen)) {
      memcpy(&sf[ofs], &bf[idx], clen);
      memcpy(s, &bf[idx], clen);
    } else
      continue;

    j++;
    ofs+=clen;
  }

  phcount=j;


  for(i=0;i<256;i++)
    hashidx[i]=-1;

  for(i=0;i<phcount;i++) {
    phokey_t kk,jj;

    idx=sidx[i];
    idx+=2;
    memcpy(&kk, &sf[idx], sizeof(phokey_t));
    jj=kk;
    kk>>=TSIN_HASH_SHIFT;
    if (hashidx[kk] < 0) {
      hashidx[kk]=i;
    }
  }

  if (hashidx[0]==-1)
    hashidx[0]=0;

  hashidx[TSIN_HASH_N-1]=phcount;
  for(i=TSIN_HASH_N-2;i>=0;i--) {
    if (hashidx[i]==-1)
      hashidx[i]=hashidx[i+1];
  }

  for(i=1; i< TSIN_HASH_N; i++) {
    if (hashidx[i]==-1)
      hashidx[i]=hashidx[i-1];
  }

  printf("Writing data tsin %d\n", ofs);
  if ((fw=fopen("tsin","w"))==NULL) {
    puts("create err");
    exit(-1);
  }

  fwrite(sf,1,ofs,fw);
  fclose(fw);

  puts("Writing data tsin.idx");
  if ((fw=fopen("tsin.idx","w"))==NULL) {
    puts("create err");
    exit(-1);
  }

  fwrite(&phcount,4,1,fw);
  fwrite(hashidx,1,sizeof(hashidx),fw);
  fwrite(sidx,4,phcount,fw);
  printf("%d phrases\n",phcount);

  fclose(fw);

  if (reload)
    send_gcin_message(GDK_DISPLAY(), "reload");

  exit(0);
}
