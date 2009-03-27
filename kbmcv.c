/*
	Copyright (C) 1995	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/

#include "gcin.h"
#include "pho.h"

int lookup(u_char *s, char *num, char *typ)
{
  int i;
  char tt[CH_SZ+1], *pp;

  if (*s < 128)
    return *s-'0';

  int len = utf8_sz(s);

  bchcpy(tt, s);
  tt[len]=0;

  for(i=0;i<4;i++) {
    if ((pp=strstr(pho_chars[i], tt)))
      break;
  }

  if (!pp)
    return 0;

  *typ=i;
  *num=(pp - pho_chars[i])/CH_SZ;

  return 1;
}

void swap_char(char *a, char *b)
{
  char t;

  t = *a;
  *a = *b;
  *b = t;
}

int main(int argc, char **argv)
{
  FILE *fp;
  char s[128];
  int i,len;
  PHOKBM phkb;
  char num, typ, chk;
  char fnamesrc[40];
  char fnameout[40];

  if (argc < 2) {
    puts("file name expected");
    exit(1);
  }

  bzero(&phkb,sizeof(phkb));
  strcpy(fnameout,argv[1]);

  char *p;
  if ((p=strchr(fnameout, '.')))
    *p = 0;

  strcpy(fnamesrc,fnameout);
  strcat(fnamesrc,".kbmsrc");
  strcat(fnameout,".kbm");

  if ((fp=fopen(fnamesrc,"r"))==NULL) {
    printf("Cannot open %s\n", fnamesrc);
    exit(1);
  }

  fgets(s,sizeof(s),fp);
  len=strlen(s);
  s[len-1]=0;
  strcpy(phkb.selkey, s);
  phkb.selkeyN = strlen(s);

  while (!feof(fp)) {
    fgets(s,sizeof(s),fp);
    len=strlen(s);

    if (!len)
      break;

    if (s[len-1]=='\n')
      s[--len]=0;

    if (!len)
      break;

    if (!lookup(s, &num, &typ))
      p_err("err found");

    int utf8sz = utf8_sz(s);
    chk=s[utf8sz + 1];

    if (chk>='A' && chk<='Z')
      chk+=32;

    for(i=0;i<3;i++) {
      if (!phkb.phokbm[(int)chk][i].num) {
        phkb.phokbm[(int)chk][i].num=num;
        phkb.phokbm[(int)chk][i].typ=typ;

        // printf("%c %d %d\n", chk, num, typ);
        break;
      }
    }
  }
  fclose(fp);


#if 0
  if (phkb.phokbm['c'][0].num && phkb.phokbm['c'][1].num) {
    swap_char(&phkb.phokbm['c'][0].num, &phkb.phokbm['c'][1].num);
    swap_char(&phkb.phokbm['c'][0].typ, &phkb.phokbm['c'][1].typ);
  }
#endif
  if ((fp=fopen(fnameout,"w"))==NULL) {
    printf("Cannot create %s\n", fnameout);
    exit(1);
  }

  fwrite(&phkb,sizeof(phkb),1,fp);
  fclose(fp);
  exit(0);
}
