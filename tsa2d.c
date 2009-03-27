/*
	Copyright (C) 1995	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include "pho.h"
#include "gcin.h"

typedef struct {
	u_char ch[2];
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
  int res,i;
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
  return memcmp(&bf[cha],&bf[chb],lena*2);
}

static int shiftb[]={9,7,3,0};

int lookup(u_char *s)
{
  int i;
  char tt[3], *pp;

  if (*s < 128)
    return *s-'0';

  tt[0]=s[0];
  tt[1]=s[1];
  tt[2]=0;

  for(i=0;i<3;i++)
    if (pp=strstr(pho_chars[i],tt))
      break;

  if (i==3)
    return 0;

  return (((pp-pho_chars[i])>>1) << shiftb[i]);
}

void prph(u_short kk)
{
  u_int k1,k2,k3,k4;

  k4=(kk&7)<<1;
  kk>>=3;
  k3=(kk&15)<<1;
  kk>>=4;
  k2=(kk&3)<<1;
  kk>>=2;
  k1=(kk&31)<<1;
  printf("%c%c%c%c%c%c%c%c",
          pho_chars[0][k1], pho_chars[0][k1+1],
          pho_chars[1][k2], pho_chars[1][k2+1],
          pho_chars[2][k3], pho_chars[2][k3+1],
          pho_chars[3][k4], pho_chars[3][k4+1]);
}


main(int argc, char **argv)
{
  FILE *fp,*fw;
  u_char s[1024];
  u_char chbuf[80][2];
  u_short phbuf[80];
  int i,j,k1,k2,k3,k4,num,idx,len, ofs, dupcou;
  u_short kk;
  u_char tt[3], uu[3],phlen;
  int hashidx[TSIN_HASH_N];
  u_char clen, llen;
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

    if (s[len-1]=='\n')
      s[--len]=0;

    if (len==0)
      continue;

    i=0;
    int chbufN=0;
    while (s[i]!=' ' && i<len) {
      memcpy(chbuf[chbufN],&s[i],2);
      i+=2;
      chbufN++;
    }

    i++;
    int phbufN=0;
    while (i<len && phbufN < chbufN && s[i]!=' ') {
      kk=0;
      while (s[i]!=' ' && i<len) {
        kk|=lookup(&s[i]);
        if (s[i]&128)
          i+=2;
        else
          i++;
      }

      i++;
      phbuf[phbufN++]=kk;
    }

    if (phbufN!=chbufN) {
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
    memcpy(&bf[ofs],chbuf,(int)clen*2);
    ofs+=clen * 2;
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
    clen=sizeof(phokey_t)*len + 2*len + 1 + 1;
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
    u_short kk,jj;

    idx=sidx[i];
    idx+=2;
    memcpy(&kk,&sf[idx],2);
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
