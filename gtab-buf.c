#include "gcin.h"
#include "gtab.h"
#include "gcin-conf.h"
#include "gcin-endian.h"
#include "pho.h"

typedef struct {
  char ch[CH_SZ];
} CH;

typedef struct {
  char *ch;
  int chN;  // UTF-8 size, not bytes
  char **sel;
  int selN;
  u_char flag;
  char psta; // phrase start index
} GEDIT;

GEDIT gbuf[80];
short gbufN;
short gbuf_cursor;


void insert_gbuf_cursor(char **sel, int selN)
{
  memmove(&gbuf[gbuf_cursor+1], &gbuf[gbuf_cursor], sizeof(GEDIT) * (gbufN - gbuf_cursor));
  bzero(&gbuf[gbuf_cursor], sizeof(GEDIT));
  gbuf[gbuf_cursor].sel = sel;
  gbuf[gbuf_cursor].selN = selN;
  gbuf_cursor++;
  gbufN++;
}

static void parse_gbuf_rec(int start, int plen, short *r_match_phr_N, short *r_no_match_ch_N, int use_count)
{
  for(plen=1; start + plen <= gbufN && plen <= MAX_PHRASE_LEN; plen++) {
    int tN = 1;

    int i;
    for(i=0; i < plen; i++) {
      tN *= gbuf[start+i].selN;
    }

    int has_ge = FALSE;
    for(i=0; i < tN; i++) {
      short couter[MAX_PHRASE_LEN];
      char tt[MAX_PHRASE_LEN * CH_SZ + 1];

      tt[0]=0;
      int t=i;
      int j;
      for(j=0; j < plen; j++) {
        int v = t % gbuf[start+j].selN;
        t /= gbuf[start+j].selN;
        strcat(tt, gbuf[start+j].sel[v]);
      }

      usecount_t uc;
      int eq_N;
      int  ge_N = find_match(tt, strlen(tt), &eq_N, &uc);
      if (ge_N)
        ge_N = TRUE;
    }

    if (!has_ge)  // no loger phrases found
      break;


  }
}


void parse_gbuf()
{
}
