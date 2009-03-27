/*
	Copyright (C) 1994,1995	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/


#include "gcin.h"
#include "pho.h"
#include <sys/stat.h>
#include <stdlib.h>


extern PHO_ITEM *ch_pho;

int ityp3_pho;
static int cpg, maxi;
int start_idx, stop_idx;
PHOKBM phkbm;
u_char typ_pho[4];
char inph[4];
gboolean b_hsu_kbm;


#define MAX_HASH_PHO 27
u_short hash_pho[MAX_HASH_PHO+1];

phokey_t pho2key(char typ_pho[])
{
  return (u_short)typ_pho[0]<<9 | (u_short) typ_pho[1]<<7 |
         (u_short)typ_pho[2]<<3 | typ_pho[3];
}

void mask_key_typ_pho(phokey_t *key)
{
  if (!typ_pho[0]) *key &= ~(31<<9);
  if (!typ_pho[1]) *key &= ~(3<<7);
  if (!typ_pho[2]) *key &= ~(15<<3);
  if (!typ_pho[3]) *key &= ~(7);
}



gboolean inph_typ_pho(int newkey)
{
  int i;
  int insert = -1;

  if (newkey == ' ') {
    insert = 3;
    inph[3] = newkey;

  }

  if (b_hsu_kbm) {
    if (typ_pho[1] && !typ_pho[2]) {
      for(i=0; i < 3; i++)
        if (phkbm.phokbm[(int)newkey][i].typ==2) {
          insert = 2;
          inph[2] = newkey;
          typ_pho[2] = phkbm.phokbm[(int)newkey][i].num;
        }
    }
  }

  int max_in_idx;

  for(max_in_idx=3; max_in_idx>=0 && !typ_pho[max_in_idx]; max_in_idx--);


  if (insert < 0)
  for(i=0; i < 3; i++) {
    char num = phkbm.phokbm[(int)newkey][i].num;
    int typ = phkbm.phokbm[(int)newkey][i].typ;
#define TKBM 0
    if (typ==3 && (typ_pho[0]||typ_pho[1]||typ_pho[2])) {
      inph[typ] = newkey;
      typ_pho[typ] = num;
#if TKBM
      dbg("insert0 typ %d\n", typ);
#endif
      insert = typ;
      break;
    }
  }

  // try insert mode first
  if (insert < 0)
  for(i=0; i < 3; i++) {
    char num = phkbm.phokbm[(int)newkey][i].num;
    int typ = phkbm.phokbm[(int)newkey][i].typ;

    if (num && !inph[typ] && typ>max_in_idx) {
      inph[typ] = newkey;
      typ_pho[typ] = num;
#if TKBM
      dbg("insert typ %d\n", typ);
#endif
      insert = typ;
      break;
    }
  }

  if (insert < 0) {
    // then overwrite mode
    for(i=0; i < 3; i++) {
      char num = phkbm.phokbm[newkey][i].num;
      int typ = phkbm.phokbm[newkey][i].typ;

      if (num) {
        inph[typ] = newkey;
        typ_pho[typ] = num;
        insert = typ;
        break;
      }
    }
  }


  if (insert < 0)
    return FALSE;


  if (inph[3] && inph[0]=='c' && !typ_pho[1] && !typ_pho[2] &&
      phkbm.phokbm['c'][1].num) {
    typ_pho[0] = phkbm.phokbm['c'][1].num;
  }

  // v 吃
  if (b_hsu_kbm && inph[3] && inph[0]=='v' && !typ_pho[1] && !typ_pho[2] &&
      phkbm.phokbm['v'][1].num) {
    typ_pho[0] = phkbm.phokbm['v'][1].num;
  }


  int idx = insert == 1 ? 0 : insert;

  {
    for(i=0; i < 3; i++) {
      char num = phkbm.phokbm[(int)inph[idx]][i].num;
      char typ = phkbm.phokbm[(int)inph[idx]][i].typ;

      if (!num)
        break;

      if (typ!=idx)
        continue;

//      dbg("idx:%d i:%d\n", idx, i);

      typ_pho[(int)typ] = num;

      int vv=hash_pho[typ_pho[0]];
      phokey_t ttt=0xffff;
      phokey_t key = pho2key(typ_pho);

      mask_key_typ_pho(&key);

      while (vv < idxnum_pho) {
        ttt=idx_pho[vv].key;
        mask_key_typ_pho(&ttt);

        if (ttt>=key)
          break;
        else
          vv++;
      }

      if (ttt == key) {
#if TKBM
        dbg("match idx:%d %d %d\n",idx, typ, num);
#endif
        return TRUE;
      }
    }
  }

  // forced to type 2
  if (inph[0] && (newkey == ' '|| typ_pho[3]) &&
      !typ_pho[1] && !typ_pho[2] && (!b_hsu_kbm || inph[0]!='a')) {
    int midx = phkbm.phokbm[(int)inph[0]][2].typ==2 ? 2:1;
    int mnum = phkbm.phokbm[(int)inph[0]][midx].num;

    if (mnum) {
      int mtyp = phkbm.phokbm[(int)inph[0]][midx].typ;
#if TKBM
       dbg("ooooooooooo %d %d %d\n", midx, mnum, mtyp);
#endif
      if (mtyp==2) {
        typ_pho[0]=0;
        typ_pho[mtyp] = mnum;
        return TRUE;
      }
    }
  }

  return FALSE;
}


void clrin_pho()
{
  bzero(typ_pho,sizeof(typ_pho));
  bzero(inph,sizeof(inph));
  maxi=ityp3_pho=0;
  cpg=0;
}

void disp_pho(int index, char *phochar);
static void clr_in_area_pho()
{
  int i;

  for(i=0; i < 4; i++)
    disp_pho(i, "  ");
}


static void disp_in_area_pho()
{
  int i;

  for(i=0;i<4;i++)
    disp_pho(i, &pho_chars[i][typ_pho[i]*CH_SZ]);
}

static int qcmp_count(const void *aa, const void *bb)
{
  PHO_ITEM *a = (PHO_ITEM *)aa;
  PHO_ITEM *b = (PHO_ITEM *)bb;

  return b->count - a->count;
}

void disp_pho_sel(char *s);
void minimize_win_pho();

static void ClrSelArea()
{
  disp_pho_sel(" ");
  minimize_win_pho();
}


extern char *TableDir;
extern char phofname[128];

static void get_start_stop_idx(phokey_t key, int *start_i, int *stop_i)
{
  int typ_pho0 = key >> 9;
  int vv=hash_pho[typ_pho0];

  while (vv<idxnum_pho) {
    if (idx_pho[vv].key>=key) break;
    else
      vv++;
  }

  *start_i=idx_pho[vv].start;
  *stop_i=idx_pho[vv+1].start;
}

// given the pho key & the big5 char, return the idx in ch_pho

int ch_key_to_ch_pho_idx(phokey_t phkey, char *big5)
{
  int start_i, stop_i;

  get_start_stop_idx(phkey, &start_i, &stop_i);

  int i;
  for(i=start_i; i<stop_i; i++) {
    if (!bchcmp(ch_pho[i].ch, big5)) {
      return i;
    }
  }

  prph(phkey);
//  dbg("error found   %c%c", *big5, *(big5+1));
  return -1;
}


void inc_pho_count(phokey_t key, int ch_idx)
{
  int start_i, stop_i;

  if (!phonetic_char_dynamic_sequence)
    return;

  get_start_stop_idx(key, &start_i, &stop_i);

//  dbg("start_i %d %d    %d %d\n", start_i, stop_i, start_idx, stop_idx);
  if (ch_pho[ch_idx].count == 65535) {
    int i;

    for(i=start_i; i < stop_i; i++) {
      ch_pho[start_i].count /= 2;
    }
  }

  ch_pho[ch_idx].count++;
//  dbg("count %d\n", ch_pho[ch_idx].count);

  qsort(&ch_pho[start_i], stop_i - start_i, sizeof(PHO_ITEM), qcmp_count);
#if 0
  int i;
  for(i=start_i; i < stop_i; i++) {
    dbg("uuuu %c%c%c %d\n", ch_pho[i].ch[0], ch_pho[i].ch[1],
      ch_pho[i].ch[2], ch_pho[i].count);
  }
#endif

  FILE *fw;

//  dbg("phofname %s\n", phofname);
  if ((fw=fopen(phofname,"r+"))==NULL) {
    p_err("err %s\n", phofname);
  }

  if (fseek(fw, ch_pho_ofs + sizeof(PHO_ITEM) * start_i, SEEK_SET) < 0)
    p_err("fseek err");
#if 1
  if (fwrite(&ch_pho[start_i], sizeof(PHO_ITEM), stop_i - start_i, fw) <= 0)
    p_err("fwrite err");
#endif
  fclose(fw);
}


void lookup_gtab(char *ch, char out[]);

void putkey_pho(u_short key, int idx)
{
  sendkey_b5(ch_pho[idx].ch);
  char tt[512];
  lookup_gtab(ch_pho[idx].ch, tt);

  inc_pho_count(key, idx);

  clrin_pho();
  clr_in_area_pho();
  ClrSelArea();
}


void load_tab_pho_file()
{
  pho_load();

  bzero(typ_pho,sizeof(typ_pho));

  u_int ttt=0;
  int i;
  for(i=0; i<MAX_HASH_PHO; i++) {
    if (idx_pho[ttt].key >> 9 == i)
      hash_pho[i]=ttt;
    else {
      continue;
    }

    while (ttt < idxnum_pho && idx_pho[ttt].key >> 9 == i)
      ttt++;
  }

  for(i=MAX_HASH_PHO; !hash_pho[i];  i--)
    hash_pho[i]=idxnum_pho;

  char kbmfname[MAX_GCIN_STR];
  FILE *fr;
  char phokbm_name[MAX_GCIN_STR];

  get_gcin_conf_str(PHONETIC_KEYBOARD, phokbm_name, "zo");
  if (strcmp(phokbm_name, "hsu"))
    b_hsu_kbm = FALSE;
  else
    b_hsu_kbm = TRUE;

  dbg("pho kbm: %s\n", phokbm_name);

  strcat(phokbm_name, ".kbm");
  get_sys_table_file_name(phokbm_name, kbmfname);

  if ((fr=fopen(kbmfname,"r"))==NULL) {
     p_err("Cannot open %s", kbmfname);
  }

  fread(&phkbm,sizeof(phkbm),1,fr);
  fclose (fr);

#if     0
  dbg("ooo\n");
  for(i=0;i<128;i++)
  if (phkbm.phokbm[i][0].num)
    dbg("kbm %c %d %d\n", i, phkbm.phokbm[i][0].num, phkbm.phokbm[i][1].typ);
#endif
}


void show_win_pho();

void init_tab_pho(int usenow)
{
  if (!ch_pho) {
    load_tab_pho_file();
  }

  show_win_pho();
  clr_in_area_pho();
  clrin_pho();
}



int feedkey_pho(KeySym xkey)
{
  int ctyp = 0;
  static unsigned int vv, ii;
  static phokey_t key;
  char *pp=NULL;
  char kno;
  int i,j,jj=0,kk=0;
  char out_buffer[(CH_SZ+2) * 10 + 4];
  int out_bufferN;

  if (xkey >= 'A' && xkey <='Z')
    xkey+=0x20;

  switch (xkey) {
    case XK_Escape:
      if (!typ_pho[0] &&!typ_pho[1] &&!typ_pho[2] &&!typ_pho[3])
        return 0;
      clrin_pho();
      ClrSelArea();
      clr_in_area_pho();
      return 1;
    case XK_BackSpace:
      ityp3_pho=0;
      for(j=3;j>=0;j--) if (typ_pho[j]) {
        typ_pho[j]=0;
        if (!typ_pho[0]&&!typ_pho[1]&&!typ_pho[2]&&!typ_pho[3]) {
          ClrSelArea();
          clr_in_area_pho();
          clrin_pho();
          return 1;
        }
        break;
      }

      if (j<0)
        return 0;

      goto llll3;
    case '<':
       if (!ityp3_pho)
         return 0;
       if (cpg >= phkbm.selkeyN)
         cpg -= phkbm.selkeyN;
       goto proc_state;
    case ' ':
      if (!typ_pho[0] && !typ_pho[1] && !typ_pho[2])
        return 0;
      if (!ityp3_pho && xkey==' ') {
         inph_typ_pho(xkey);
         ctyp=3;  kno=0; jj=0;

         goto llll1;
      }

      ii = start_idx+ cpg + phkbm.selkeyN;

      if (ii < stop_idx) {
        cpg += phkbm.selkeyN;
      }
      else {
          if (cpg) {
            cpg=0;
            ii=start_idx;
          } else {
            putkey_pho(key, start_idx);
          /*      maxi=ityp3_pho=0; */
            return 1;
          }
       }
      i=0;
      ClrSelArea();

      out_bufferN=0;

      while(i<phkbm.selkeyN  && ii< stop_idx) {
        out_buffer[out_bufferN++] = phkbm.selkey[i];
        bchcpy(&out_buffer[out_bufferN], ch_pho[ii].ch);
        out_bufferN+=CH_SZ;
        out_buffer[out_bufferN++] = ' ';

        ii++;
        i++;
      }

      out_buffer[out_bufferN++] = cpg ? '<' : ' ';

      if (ii < stop_idx) {
        out_buffer[out_bufferN++] = cpg ? '\\' : ' ';
        out_buffer[out_bufferN++] = '>';
      }

      out_buffer[out_bufferN] = 0;

      disp_pho_sel(out_buffer);
      maxi=i;
      return 1;
   default:
      if (xkey >= 127 || xkey < ' ')
        return 0;

      if ((pp=strchr(phkbm.selkey, xkey)) && maxi && ityp3_pho) {
        int c=pp-phkbm.selkey;

        if (c<maxi) {
          putkey_pho(key, start_idx + cpg + c);
        }
        return 1;
      }

      if (ityp3_pho && !cpg) {
        dbg("start_idx: %d\n", start_idx);
        putkey_pho(key, start_idx);
      }

//      cpg=0;
  }

  inph_typ_pho(xkey);

  if (typ_pho[3])
    ctyp = 3;

llll1:
  jj=0;
  kk=1;
llll2:
  if (ctyp == 3) {
       ityp3_pho=1;  /* last key is entered */
  }
llll3:

  key = pho2key(typ_pho);

#if    0
  dbg("typ_pho %d %d %d %d\n", typ_pho[0], typ_pho[1], typ_pho[2], typ_pho[3]);
#endif
  if (!key)
    return 1;

  vv=hash_pho[typ_pho[0]];
  phokey_t ttt=0xffff;

  while (vv < idxnum_pho) {
    ttt=idx_pho[vv].key;
    mask_key_typ_pho(&ttt);

    if (ttt>=key)
      break;
    else
      vv++;
  }

  if (ttt > key || (ityp3_pho && idx_pho[vv].key != key) ) {
    while (jj<4) {
      while(kk<3)
        if (phkbm.phokbm[(int)inph[jj]][kk].num ) {

          if (kk) {
            ctyp=phkbm.phokbm[(int)inph[jj]][kk-1].typ;
            typ_pho[ctyp]=0;
          }

          kno=phkbm.phokbm[(int)inph[jj]][kk].num;
          ctyp=phkbm.phokbm[(int)inph[jj]][kk].typ;
          typ_pho[ctyp]=kno;
          kk++;
          goto llll2;
        }
        else
          kk++;
      jj++;
      kk=1;
    }

    bell();
    ityp3_pho=typ_pho[3]=0;
    disp_in_area_pho();
    return 1;
  }

proc_state:
  disp_in_area_pho();
  start_idx = ii = idx_pho[vv].start;
  stop_idx = idx_pho[vv+1].start;

//   dbg("start_idx: %d %d\n", start_idx, stop_idx);

//  ClrSelArea();
//  gotoxy(0,MROW-2);
  ii+=cpg;

  if (ityp3_pho && stop_idx - start_idx==1) {
    putkey_pho(key, ii);
    maxi=ityp3_pho=0;
    return 1;
  }

  i=0;


  out_bufferN=0;

  if (ityp3_pho) {
    while(i< phkbm.selkeyN  && ii < stop_idx) {
      out_buffer[out_bufferN++] = phkbm.selkey[i];
      bchcpy(&out_buffer[out_bufferN], ch_pho[ii].ch);
      out_bufferN+=CH_SZ;
      out_buffer[out_bufferN++] = ' ';

      ii++;
      i=i+1;
    }

    out_buffer[out_bufferN++] = cpg ? '<' : ' ';

    if (ii < stop_idx) {
      out_buffer[out_bufferN++] = cpg ? '\\' : ' ';
      out_buffer[out_bufferN++] = '>';
    } else {
      cpg=0;
    }

    maxi=i;
  } else {
    while(i<phkbm.selkeyN  && ii < stop_idx) {
      bchcpy(&out_buffer[out_bufferN], ch_pho[ii].ch);
      out_bufferN+=CH_SZ;

      ii++;
      i++;
    }
    maxi=i;
  }

  out_buffer[out_bufferN] = 0;
  disp_pho_sel(out_buffer);

  return 1;
}
