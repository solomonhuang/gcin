/*
	Copyright (C) 1994,1995	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/


#include "gcin.h"
#include "pho.h"

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

void dbg(char *fmt,...)
{
  va_list args;

  va_start(args, fmt);
  vprintf(fmt, args);
  fflush(stdout);
  va_end(args);
}


int main()
{
  FILE *fr;
  int i,cou;
  unsigned int ttt;

  pho_load();

  for(i=0; i < idxnum_pho; i++) {
    phokey_t key = idx_pho[i].key;
    int frm = idx_pho[i].start;
    int to = idx_pho[i+1].start;

    int j;
    for(j=frm; j < to; j++) {

      prph(key);
      dbg(" %c%c%c %d\n", ch_pho[j].ch[0], ch_pho[j].ch[1], ch_pho[j].ch[2],
        ch_pho[j].count);
    }
  }
}
