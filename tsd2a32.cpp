// gcin

#include "gcin.h"
#include "pho.h"

int phcount;
void prph2(FILE *fp, phokey_t kk);

#if WIN32
void init_gcin_program_files();
 #pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif

int main(int argc, char **argv)
{
  FILE *fp;
  phokey_t phbuf[MAX_PHRASE_LEN];
  int i;
  u_char clen;
  usecount_t usecount;
  gboolean pr_usecount = TRUE;
  char *fname;
  char *fname_out = NULL;

  if (argc <= 1) {
    printf("%s: file name expected\n", argv[0]);
    exit(1);
  }


  for(i=1; i < argc;) {
	if (!strcmp(argv[i], "-nousecount")) {
		i++;
	    pr_usecount = FALSE;
	} else
	if (!strcmp(argv[i], "-o")) {
		if (i==argc-1)
			p_err("-o need out file name");
		fname_out = argv[i+1];
		i+=2;
	} else
		fname = argv[i++];
  }

  FILE *fp_out;

  if (!fname_out) {
	  fp_out = stdout;
  }
  else {
	  dbg("use %s\n", fname_out);
	  fp_out = fopen(fname_out, "w");
	  if (!fp_out)
		  p_err("cannot create %s\n", fname_out);
  }

  if ((fp=fopen(fname,"rb"))==NULL) {
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
        fprintf(fp_out, "%c", ch[j]);
    }

    fprintf(fp_out, " ");
    for(i=0;i<clen;i++) {
      prph2(fp_out, phbuf[i]);
      if (i!=clen-1)
        fprintf(fp_out, " ");
    }

    fprintf(fp_out, " %d\n", usecount);
  }

stop:
  fclose(fp);
  fclose(fp_out);
  return 0;
}
