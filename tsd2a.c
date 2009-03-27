/*
	Copyright (C) 1995	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/

#include <stdio.h>
#include <sys/types.h>
#include "pho.h"

int phcount;

void prph(u_short kk)
{
  u_int k1,k2,k3,k4;
  static u_char phchars[8];
  int phcharsN=0;

  k4=(kk&7);
  kk>>=3;
  k3=(kk&15)<<1;
  kk>>=4;
  k2=(kk&3)<<1;
  kk>>=2;
  k1=(kk&31)<<1;

  if (k1) {
    memcpy(phchars, &pho_chars[0][k1], 2);
    phcharsN+=2;
  }

  if (k2) {
    memcpy(&phchars[phcharsN], &pho_chars[1][k2], 2);
    phcharsN+=2;
  }

  if (k3)  {
    memcpy(&phchars[phcharsN], &pho_chars[2][k3], 2);
    phcharsN+=2;
  }

  if (k4)
    phchars[phcharsN++] = k4 + '0';

  phchars[phcharsN] = 0;

  printf("%s", phchars);
}

main(int argc, char **argv)
{
  FILE *fp;
  phokey_t phbuf[MAX_PHRASE_LEN];
  u_char s[128];
  u_char chbuf[128][2];
  int i, len;
  u_char tt[3], phlen;
  u_char clen, usecount;

  if (argc <= 1) {
    printf("%s: file name expected\n", argv[0]);
    exit(1);
  }
  if ((fp=fopen(argv[1],"r"))==NULL) {
    printf("Cannot open %s", argv[1]);
    exit(-1);
  }

  while (!feof(fp)) {
    fread(&clen,1,1,fp);
    fread(&usecount,1,1,fp);
    fread(phbuf,sizeof(phokey_t), clen, fp);
    fread(chbuf,2,clen,fp);

    for(i=0;i<clen;i++) {
      printf("%c%c", chbuf[i][0],chbuf[i][1]);
    }

    printf(" ");
    for(i=0;i<clen;i++) {
      prph(phbuf[i]);
      if (i!=clen-1)
        printf(" ");
    }

    printf(" %d\n", usecount);
  }

  fclose(fp);
  return 0;
}
