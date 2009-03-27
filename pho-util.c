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
static char pho_tab[]="pho.tab";
void update_table_file(char *name, int version);

void pho_load()
{
  if (!phofname[0]) {
    if (!getenv("GCIN_TABLE_DIR") && phonetic_char_dynamic_sequence) {
      get_gcin_user_fname(pho_tab, phofname);

      if (access(phofname, W_OK) < 0){
        char sys_file[256], vv[256];

        get_sys_table_file_name(sys_file, pho_tab);
        sprintf(vv,"cp %s %s\n", sys_file, phofname);
        system(vv);
      }
    } else {
      get_sys_table_file_name(pho_tab, phofname);
      dbg("use system's pho, no dynamic adj\n");
    }
  }

  update_table_file(pho_tab, 2);

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

#if 0
  int i;
  for(i=0; i <ch_phoN; i++) {
    char tt[5];

    utf8cpy(tt, ch_pho[i].ch);
    dbg("oooo %s\n", tt);
  }
#endif
}


void free_pho_mem()
{
  if (ch_pho)
    free(ch_pho);
}

int utf8_pho_keys(char *big5, phokey_t *phkeys)
{
  int ofs=0;
  int phkeysN=0;

  phkeys[0];

  do {
    for(; ofs < ch_phoN; ofs++)
      if (!bchcmp(big5, ch_pho[ofs].ch))
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

char *phokey_to_str(phokey_t kk)
{
  u_int k1,k2,k3,k4;
  static u_char phchars[CH_SZ * 4 + 1];
  int phcharsN=0;

  k4=(kk&7);
  kk>>=3;
  k3=(kk&15) * CH_SZ;
  kk>>=4;
  k2=(kk&3) * CH_SZ;
  kk>>=2;
  k1=(kk&31) * CH_SZ;

  if (k1) {
    bchcpy(phchars, &pho_chars[0][k1]);
    phcharsN+=CH_SZ;
  }

  if (k2) {
    bchcpy(&phchars[phcharsN], &pho_chars[1][k2]);
    phcharsN+=CH_SZ;
  }

  if (k3)  {
    bchcpy(&phchars[phcharsN], &pho_chars[2][k3]);
    phcharsN+=CH_SZ;
  }

  if (k4)
    phchars[phcharsN++] = k4 + '0';

  phchars[phcharsN] = 0;

  return phchars;
}

void str_to_all_phokey_chars(char *b5_str, char *out)
{
  int len=strlen(b5_str);

  out[0]=0;

  int h;

  for(h=0; h < strlen(b5_str); h+=CH_SZ) {
    phokey_t phos[32];

    int n=utf8_pho_keys(&b5_str[h], phos);

    int i;
    for(i=0; i < n; i++) {
      char *pstr = phokey_to_str(phos[i]);
      strcat(out, pstr);
      if (i < n -1)
        strcat(out, " ");
    }

    if (h < len - CH_SZ)
      strcat(out, " | ");
  }
}
