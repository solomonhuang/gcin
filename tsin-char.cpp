#include "gcin.h"
#include "pho.h"
#include "tsin.h"

extern int ts_gtabN;
typedef struct {
  char ch[CH_SZ];
  u_int bits;
} CH_ENT;

static CH_ENT *chs;
static int chsN;

static int qcmp_ch(const void *aa, const void *bb)
{
  CH_ENT *a = (CH_ENT *)aa;
  CH_ENT *b = (CH_ENT *)bb;

  return memcmp(a->ch, b->ch, sizeof(CH_SZ));
}

static CH_ENT *find(char *ch)
{
  char t[CH_SZ];
  bzero(t, CH_SZ);

  u8cpy(t, ch);
  int bottom = 0;
  int top = chsN-1;

  do {
    int mid = (bottom + top) /2;
    int r = memcmp(t, chs[mid].ch, CH_SZ);

    if (r < 0)
      top = mid - 1;
    else
    if (r > 0)
      bottom = mid + 1;
    else
      return &chs[mid];
  } while (bottom <= top);

  return NULL;
}

#include <sys/stat.h>

void build_ts_gtab(int rebuild);
void get_gcin_user_or_sys_fname(char *name, char fname[]);
int load_ts_gtab(int idx, char *tstr, usecount_t *usecount);

static void build_chs()
{
  if (!ts_gtabN)
    build_ts_gtab(0);

  char fname[256];
  get_gcin_user_or_sys_fname("tsin-ch-idx", fname);
  struct stat st_gtab, st_tsin32;
  FILE *fp;
  extern char tsfname[];

#if 1
  if (!stat(fname, &st_gtab) && !stat(tsfname, &st_tsin32) &&
      st_tsin32.st_mtime < st_gtab.st_mtime) {

    if (fp=fopen(fname, "rb")) {
      printf("............... from %s\n", fname);
      fread(&chsN, sizeof(chsN), 1, fp);
      chs = tmalloc(CH_ENT, chsN);
      fread(chs, sizeof(CH_ENT), chsN, fp);
      fclose(fp);
      return;
    }
  }
#endif

  int i;
  char str[MAX_CIN_PHR];
  usecount_t uc;

  for(i=0;i < ts_gtabN; i++) {
    load_ts_gtab(i, str, &uc);
    char *p=str;

    while (*p) {
      chs = trealloc(chs, CH_ENT, chsN+1);
      bzero(&chs[chsN], sizeof(CH_ENT));
      int sz = u8cpy(chs[chsN].ch, p);
      p+=sz;
      chsN++;
    }
  }

  qsort(chs, chsN, sizeof(CH_ENT), qcmp_ch);
#if 0
  printf("chsN:%d\n", chsN);
#endif
  int nchsN=1;
  for(i=1; i<chsN; i++)
    if (qcmp_ch(&chs[i], &chs[i-1]))
      chs[nchsN++]=chs[i];

  chsN = nchsN;
  chs = trealloc(chs, CH_ENT, chsN);
#if 0
  printf("chsN:%d\n", chsN);
#endif
  for(i=0;i < ts_gtabN; i++) {
    load_ts_gtab(i, str, &uc);
    char *p=str;

    int cidx = 0;
    while (*p) {
      int sz = utf8_sz(p);
      CH_ENT *ce = find(p);
      if (!ce)
        p_err("err found %s", p);

      ce->bits |= 1<<cidx;

      cidx++;
      p+=sz;
    }
  }

  if (fp=fopen(fname, "wb")) {
    fwrite(&chsN, sizeof(chsN), 1, fp);
    fwrite(chs, sizeof(CH_ENT), chsN, fp);
    fclose(fp);
  }
}


int ch_pos_find(char *ch, int pos)
{
//  utf8_putchar(ch);

  if (!chsN)
    build_chs();

  CH_ENT *p = find(ch);

  if (!p) {
//    puts("0");
    return 0;
  }

  int v = p->bits & (1<<pos);
//  printf("%d\n", v);
  return v;
}
