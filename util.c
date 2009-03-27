#include "gcin.h"


void p_err(char *fmt,...)
{
  va_list args;

  va_start(args, fmt);
  fprintf(stderr,"gcin:");
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr,"\n");
  exit(-1);
}

static FILE *out_fp;

void dbg(char *fmt,...)
{
  va_list args;
  if (!out_fp) {
    if (getenv("GCIN_DBG_TMP")) {
      char fname[64];
      sprintf(fname, "/tmp/gcindbg-%d-%d", getuid(), getpid());
      out_fp = fopen(fname, "w");
    }

    if (!out_fp)
      out_fp = stdout;
  }

  va_start(args, fmt);
  vfprintf(out_fp, fmt, args);
  fflush(out_fp);
  va_end(args);
}

void *zmalloc(int n)
{
  void *p =  malloc(n);
  bzero(p, n);
  return p;
}
