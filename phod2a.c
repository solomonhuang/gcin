/*
	Copyright (C) 1994,1995	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/


#include "gcin.h"
#include "pho.h"
#include "gcin-conf.h"


int main(int argc, char **argv)
{
  int i;

  load_setttings();

  if (argc > 1) {
    p_err("Currently only support ~/.gcin/pho.tab");
  }

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

  return 0;
}
