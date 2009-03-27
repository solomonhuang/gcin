/*
	Copyright (C) 1995	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/

#include "gcin.h"
#include "pho.h"

int phcount;

int main(int argc, char **argv)
{
  FILE *fp;
  phokey_t phbuf[MAX_PHRASE_LEN];
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

    int tlen = 0;

    for(i=0;i<clen;i++) {
      char ch[CH_SZ];

      int n = fread(ch, 1, 1, fp);
      if (n<=0)
        goto stop;

      int len=utf8_sz(ch);

      fread(&ch[1], 1, len-1, fp);

      int j;
      for(j=0; j < len; j++)
        printf("%c", ch[j]);
    }

    printf(" ");
    for(i=0;i<clen;i++) {
      prph(phbuf[i]);
      if (i!=clen-1)
        printf(" ");
    }

    printf(" %d\n", usecount);
  }

stop:
  fclose(fp);
  return 0;
}
