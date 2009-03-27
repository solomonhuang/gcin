#include "gcin.h"
#include "pho.h"
#include <sys/stat.h>
#include <stdlib.h>


char phofname[128]="";
extern char *TableDir;
u_short idxnum_pho;
PHO_IDX idx_pho[1403];
int ch_pho_ofs;
PHO_ITEM *ch_pho;
int ch_phoN;

void pho_load()
{
#ifndef  NO_PRIVATE_TSIN
  if (!phofname[0]) {
    char tt[128];

    get_gcin_dir(tt);
    strcat(tt,"/pho.tab");
    strcpy(phofname,tt);

    char vv[128];
    if (access(phofname, W_OK) < 0){
      sprintf(vv,"cp %s/pho.tab  %s\n", TableDir, tt);
      system(vv);
    }

#else
    strcat(strcpy(phofname, TableDir), "pho.tab");
#endif
  }


  FILE *fr;

  if ((fr=fopen(phofname,"r"))==NULL)
    p_err("err %s\n", phofname);

  struct stat st;
  fstat(fileno(fr), &st);

  int count = st.st_size / sizeof(PHO_ITEM);

  fread(&idxnum_pho,sizeof(u_short),1,fr);
  fread(&idxnum_pho,sizeof(u_short),1,fr);
  fread(idx_pho, sizeof(PHO_IDX), idxnum_pho, fr);

  ch_pho_ofs = ftell(fr);

  if (ch_pho)
    free(ch_pho);

  if (!(ch_pho=(PHO_ITEM *)malloc(sizeof(PHO_ITEM) * count))) {
    p_err("malloc error");
  }

  ch_phoN=fread(ch_pho,sizeof(PHO_ITEM), count, fr);
//  dbg("ch_phoN:%d  %d\n", ch_phoN, idxnum_pho);
  fclose(fr);

  idx_pho[idxnum_pho].key=0xffff;
  idx_pho[idxnum_pho].start=ch_phoN;
}


void free_pho_mem()
{
  if (ch_pho)
    free(ch_pho);
}

int big5_pho_chars(char *big5, phokey_t *phkeys)
{
  int ofs=0;
  int phkeysN=0;

  do {
    for(; ofs < ch_phoN; ofs++)
      if (!memcmp(big5, ch_pho[ofs].ch, 2))
        break;

    if (ofs==ch_phoN)
      goto ret;

    int i;
    for(i=0; i < idxnum_pho; i++) {
      if (idx_pho[i].start<= ofs && ofs < idx_pho[i+1].start) {
        phkeys[phkeysN++] = idx_pho[i].key;
        break;
      }
    }

    ofs++;
  } while (ofs < ch_phoN);

ret:
  return phkeysN;
}
