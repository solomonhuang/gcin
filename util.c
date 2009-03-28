#include "gcin.h"


void p_err(char *fmt,...)
{
  va_list args;

  va_start(args, fmt);
  fprintf(stderr,"gcin:");
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr,"\n");

  if (getenv("GCIN_ERR_COREDUMP"))
    abort();

  exit(-1);
}

static FILE *out_fp;

static void init_out_fp()
{
  if (!out_fp) {
    if (getenv("GCIN_DBG_TMP") || 0) {
      char fname[64];
      sprintf(fname, "/tmp/gcindbg-%d-%d", getuid(), getpid());
      out_fp = fopen(fname, "w");
    }

    if (!out_fp)
      out_fp = stdout;
  }
}


void dbg(char *fmt,...)
{
  va_list args;

  init_out_fp();

  va_start(args, fmt);
  vfprintf(out_fp, fmt, args);
  fflush(out_fp);
  va_end(args);
}

#if !CLIENT_LIB
void dbg_time(char *fmt,...)
{
  va_list args;
  time_t t;

  init_out_fp();

  time(&t);
  struct tm *ltime = localtime(&t);
  dbg("%02d:%02d:%02d ", ltime->tm_hour, ltime->tm_min, ltime->tm_sec);

  va_start(args, fmt);
  vfprintf(out_fp, fmt, args);
  fflush(out_fp);
  va_end(args);
}
#endif


void *zmalloc(int n)
{
  void *p =  malloc(n);
  bzero(p, n);
  return p;
}
