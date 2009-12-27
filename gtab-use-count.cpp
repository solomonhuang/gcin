#include "gcin.h"
#include "gtab.h"

typedef struct {
  u_char ch[CH_SZ];
  int use_count;
} GTAB_USE_CNT;

static char gtab_use_count_file[]="gtab-use-count";
static FILE *fp_gtab_use_count;

static void init_fp()
{
  if (!fp_gtab_use_count) {
    char fname[128];
    get_gcin_user_fname(gtab_use_count_file, fname);

    if (!(fp_gtab_use_count=fopen(fname, "rb+"))) {
      if (!(fp_gtab_use_count=fopen(fname, "wb+"))) {
        dbg("cannot write to %s\n", fname);
        return;
      }
    }
  }
}


void inc_gtab_use_count(char *s)
{
  init_fp();

  GTAB_USE_CNT c;
  rewind(fp_gtab_use_count);

  utf8_putchar(s);
//  dbg("inc %d\n", ftell(fp_gtab_use_count));
  while (!feof(fp_gtab_use_count)) {
    fread(&c, sizeof(c), 1, fp_gtab_use_count);
    if (memcmp(c.ch, s, CH_SZ))
      continue;

    long ofs = ftell(fp_gtab_use_count);
//    dbg("aa %d ofs:%d sz:%d\n", c.use_count, ofs, sizeof(c));
    fseek(fp_gtab_use_count, - sizeof(c), SEEK_CUR);
//    dbg("bb %d ofs:%d\n", c.use_count, ftell(fp_gtab_use_count));

    c.use_count++;
    fwrite(&c, sizeof(c), 1, fp_gtab_use_count);
    fflush(fp_gtab_use_count);
    return;
  }

  int fofs = ftell(fp_gtab_use_count);
//  dbg("fofs: %d\n", fofs);

  int delta = fofs % sizeof(GTAB_USE_CNT);
  if (delta) // avoid incomplete write
    fseek(fp_gtab_use_count, - delta, SEEK_CUR);

  bzero(c.ch, CH_SZ);
  c.use_count = 1;
  memcpy(c.ch, s, CH_SZ);
  fwrite(&c, sizeof(c), 1, fp_gtab_use_count);
  fflush(fp_gtab_use_count);
}


int get_gtab_use_count(char *s)
{
  if (strlen(s) > utf8_sz(s)) { // only character
    return 0;
  }

  init_fp();

  GTAB_USE_CNT c;
  rewind(fp_gtab_use_count);
  while (!feof(fp_gtab_use_count)) {
    fread(&c, sizeof(c), 1, fp_gtab_use_count);
    if (!memcmp(c.ch, s, CH_SZ)) {
//      printf("zz %s %d\n", s, c.use_count);
      return c.use_count;
    }
  }

  return 0;
}
