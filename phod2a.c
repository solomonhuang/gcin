/*
	Copyright (C) 1994,1995	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/


#include "gcin.h"
#include "pho.h"
#include <sys/stat.h>
#include <stdlib.h>

PHO_ITEM *ch_pho;

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


u_short hash_pho[23];

void prph(u_short kk)
{
  u_int k1,k2,k3,k4;
        k4=(kk&7)<<1;
        kk>>=3;
        k3=(kk&15)<<1;
        kk>>=4;
        k2=(kk&3)<<1;
        kk>>=2;
        k1=(kk&31)<<1;
        if (k1) printf("%c%c", pho_chars[0][k1], pho_chars[0][k1+1]);
        if (k2) printf("%c%c", pho_chars[1][k2], pho_chars[1][k2+1]);
        if (k3) printf("%c%c", pho_chars[2][k3], pho_chars[2][k3+1]);
        if (k4) printf("%d", k4>>1);
}

int main()
{
  FILE *fr;
  int i,cou;
  unsigned int ttt;

  pho_load();

  for(i=0; i < idxnum_pho; i++) {
    u_short key = idx_pho[i].key;
    int frm = idx_pho[i].start;
    int to = idx_pho[i+1].start;

    int j;
    for(j=frm; j < to; j++) {

      prph(key);
      dbg(" %c%c %d\n", ch_pho[j].ch[0], ch_pho[j].ch[1], ch_pho[j].count);
    }
  }
}
