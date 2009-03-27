/*
	Copyright (C) 1995	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/

#include "gcin.h"
#include "pho.h"

int phcount;

main(int argc, char **argv)
{
  FILE *fp;
  phokey_t phbuf[MAX_PHRASE_LEN];
  u_char chbuf[128][CH_SZ];
  int i;
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
    fread(chbuf, CH_SZ, clen, fp);

    for(i=0;i<clen;i++) {
      int len=utf8_sz(&chbuf[i][0]);

      int j;
      for(j=0; j < len; j++)
        printf("%c", chbuf[i][j]);
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
