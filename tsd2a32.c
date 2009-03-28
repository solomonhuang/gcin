// gcin

#include "gcin.h"
#include "pho.h"

int phcount;

int main(int argc, char **argv)
{
  FILE *fp;
  phokey_t phbuf[MAX_PHRASE_LEN];
  int i;
  u_char clen;
  usecount_t usecount;
  gboolean pr_usecount = TRUE;
  char *fname;

  if (argc <= 1) {
    printf("%s: file name expected\n", argv[0]);
    exit(1);
  }

  fname = argv[1];
  if (!strcmp(argv[1], "-nousecount")) {
    fname = argv[2];
    pr_usecount = FALSE;
  }


  if ((fp=fopen(fname,"r"))==NULL) {
    printf("Cannot open %s", argv[1]);
    exit(-1);
  }

  while (!feof(fp)) {
    fread(&clen,1,1,fp);
    fread(&usecount, sizeof(usecount_t), 1,fp);
    if (!pr_usecount)
      usecount = 0;

    fread(phbuf,sizeof(phokey_t), clen, fp);

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
