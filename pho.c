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

static char typ_pho_len[]={5, 2, 4, 3};

gboolean same_query_show_pho_win();

gboolean pho_has_input()
{
  return typ_pho[0] || typ_pho[1] || typ_pho[2] || typ_pho[3] || same_query_show_pho_win();
}

phokey_t pho2key(char typ_pho[])
{
  phokey_t key=typ_pho[0];
  int i;

  for(i=1; i < 4; i++) {
    key =  typ_pho[i] | (key << typ_pho_len[i]) ;
  }

  return key;
}

void key_typ_pho(phokey_t phokey, char rtyp_pho[])
{
  rtyp_pho[3] = phokey & 7;
  phokey >>= 3;
  rtyp_pho[2] = phokey & 0xf;
  phokey >>=4;
  rtyp_pho[1] = phokey & 0x3;
  phokey >>=2;
  rtyp_pho[0] = phokey;
}


void mask_key_typ_pho(phokey_t *key)
{
  if (!typ_pho[0]) *key &= ~(31<<9);
  if (!typ_pho[1]) *key &= ~(3<<7);
  if (!typ_pho[2]) *key &= ~(15<<3);
  if (!typ_pho[3]) *key &= ~(7);
}

#define TKBM 0
#define MIN_M_PHO 5

static void find_match_phos(u_char mtyp_pho[4], int *mcount, int newkey)
{
      int vv;
      phokey_t key = pho2key(typ_pho);

      mask_key_typ_pho(&key);
#if TKBM
      dbg("-------------------- %d --", typ_pho[3]);
      prph(key);
      dbg("\n");
#endif
      for (vv = hash_pho[typ_pho[0]]; vv < hash_pho[typ_pho[0]+1]; vv++) {
        phokey_t ttt=idx_pho[vv].key;

        if (newkey!=' ' && !typ_pho[3])
          mask_key_typ_pho(&ttt);

        if (ttt > key)
          break;

        int count = 0;

        int i;
        for(i=idx_pho[vv].start; i < idx_pho[vv+1].start; i++) {
          if (utf8_sz(ch_pho[i].ch) > 1) {
#if 0
            utf8_putchar(ch_pho[i].ch);
            dbg(" ");
#endif
            count++;
          }
        }

        if (*mcount < count) {
          *mcount = count;
          memcpy(mtyp_pho, typ_pho, sizeof(typ_pho));
#if TKBM
          dbg("count %d\n", count);
#endif
          if (*mcount > MIN_M_PHO)
            break;
        }
      }
}


gboolean inph_typ_pho(int newkey)
{
  int i;
  int insert = -1;


  int max_in_idx;

  for(max_in_idx=3; max_in_idx>=0 && !typ_pho[max_in_idx]; max_in_idx--);

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


//  dbg("newkey %c\n", newkey);

  int mcount = 0;
  u_char mtyp_pho[4];

  int a;

  for(a=0; a < 3; a++) {
    char num = phkbm.phokbm[(int)inph[0]][a].num;
    char typ = phkbm.phokbm[(int)inph[0]][a].typ;

    if (typ == 3)
      continue;

    if (num) {
      if (typ==2 && typ_pho[0] && !typ_pho[2])
        typ_pho[0] = 0;
      typ_pho[(int)typ] = num;
#if TKBM
      dbg("%d num %d\n",a, num);
#endif
      find_match_phos(mtyp_pho, &mcount, newkey);
    }

    for(i=0; i < 3; i++) {
      char num = phkbm.phokbm[(int)inph[2]][i].num;
      char typ = phkbm.phokbm[(int)inph[2]][i].typ;

      if (!num)
        break;

      if (typ!=2)
        continue;

      typ_pho[(int)typ] = num;

      find_match_phos(mtyp_pho, &mcount, newkey);

      if (mcount > MIN_M_PHO) {
        return TRUE;
      }
    }


    find_match_phos(mtyp_pho, &mcount, newkey);

    if (mcount > MIN_M_PHO) {
      return TRUE;
    }
  }

  if (mcount) {
    memcpy(typ_pho, mtyp_pho, sizeof(typ_pho));
    return TRUE;
  }

  return FALSE;
}


void clrin_pho()
{
  bzero(typ_pho,sizeof(typ_pho));
  bzero(inph,sizeof(inph));
  maxi=ityp3_pho=0;
  cpg=0;

  if (gcin_pop_up_win && !same_query_show_pho_win())
    hide_win_pho();
}

void disp_pho(int index, char *phochar);
void clr_in_area_pho()
{
  int i;

  clrin_pho();

  for(i=0; i < 4; i++)
    disp_pho(i, "  ");
}


static void disp_in_area_pho()
{
  int i;

  for(i=0;i<4;i++)
    disp_pho(i, &pho_chars[i][typ_pho[i]*3]);
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

gboolean get_start_stop_idx(phokey_t key, int *start_i, int *stop_i)
{
  int typ_pho0 = key >> 9;
  int vv=hash_pho[typ_pho0];

  while (vv<idxnum_pho) {
    if (idx_pho[vv].key>=key) break;
    else
      vv++;
  }

  if (vv >= idxnum_pho || idx_pho[vv].key != key)
    return FALSE;

  *start_i=idx_pho[vv].start;
  *stop_i=idx_pho[vv+1].start;

  return TRUE;
}

// given the pho key & the big5 char, return the idx in ch_pho

int ch_key_to_ch_pho_idx(phokey_t phkey, char *utf8)
{
  int start_i, stop_i;

  get_start_stop_idx(phkey, &start_i, &stop_i);

  int i;
  for(i=start_i; i<stop_i; i++) {
    int u8len = utf8_sz(ch_pho[i].ch);
    if (!memcmp(ch_pho[i].ch, utf8, u8len)) {
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
  if (ch_pho[ch_idx].count == 32767) {
    int i;

    for(i=start_i; i < stop_i; i++) {
      if (ch_pho[i].count < 0)
        continue;

      ch_pho[i].count /= 2;
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
gboolean is_gtab_query_mode();
void set_gtab_target_displayed();

void putkey_pho(u_short key, int idx)
{
  sendkey_b5(ch_pho[idx].ch);
  char tt[512];
  lookup_gtab(ch_pho[idx].ch, tt);

  inc_pho_count(key, idx);

  clr_in_area_pho();
  ClrSelArea();

  if (is_gtab_query_mode())
    set_gtab_target_displayed();
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

void init_tab_pho()
{
  if (!ch_pho) {
    load_tab_pho_file();
  }

  show_win_pho();
  clr_in_area_pho();
}

gboolean shift_char_proc(KeySym key, int kbstate);

int feedkey_pho(KeySym xkey, int kbstate)
{
  int ctyp = 0;
  static unsigned int vv, ii;
  static phokey_t key;
  char *pp=NULL;
  char kno;
  int i,j,jj=0,kk=0;
  char out_buffer[(CH_SZ+2) * 10 + 4];
  int out_bufferN;

  if ((kbstate & ShiftMask)) {
    return shift_char_proc(xkey, kbstate);
  }


  if (xkey >= 'A' && xkey <='Z')
    xkey+=0x20;

  switch (xkey) {
    case XK_Escape:
      if (!typ_pho[0] &&!typ_pho[1] &&!typ_pho[2] &&!typ_pho[3])
        return 0;
      ClrSelArea();
      clr_in_area_pho();
      if (is_gtab_query_mode())
        close_gtab_pho_win();
      return 1;
    case XK_BackSpace:
      ityp3_pho=0;
      for(j=3;j>=0;j--) if (typ_pho[j]) {
        typ_pho[j]=0;
        if (!typ_pho[0]&&!typ_pho[1]&&!typ_pho[2]&&!typ_pho[3]) {
          ClrSelArea();
          clr_in_area_pho();
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
      if (!typ_pho[0] && !typ_pho[1] && !typ_pho[2]) {
        if (current_CS->b_half_full_char)
          return full_char_proc(xkey);
        return 0;
      }
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
        int len = u8cpy(&out_buffer[out_bufferN], ch_pho[ii].ch);
        out_bufferN+=len;
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
//        dbg("start_idx: %d\n", start_idx);
        putkey_pho(key, start_idx);
      }

//      cpg=0;
  }

  inph_typ_pho(xkey);

  if (gcin_pop_up_win)
    show_win_pho();


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
      int len = u8cpy(&out_buffer[out_bufferN], ch_pho[ii].ch);
      out_bufferN+=len;
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
      int len = u8cpy(&out_buffer[out_bufferN], ch_pho[ii].ch);
      out_bufferN+=len;

      ii++;
      i++;
    }
    maxi=i;
  }

  out_buffer[out_bufferN] = 0;
  disp_pho_sel(out_buffer);

  return 1;
}

static char typ_pho_no_to_xkey(int typ, u_char num)
{
  int i, j;

  for(i=' '; i < 127; i++)
    for(j=0; j < 3; j++)
      if (phkbm.phokbm[i][j].typ == typ && phkbm.phokbm[i][j].num == num)
        return i;

  return 0;
}


void start_gtab_pho_query(char *utf8)
{
  phokey_t phokeys[32];
  int phokeysN, i;

  phokeysN = utf8_pho_keys(utf8, phokeys);
  if (phokeysN <= 0)
    return;

  char rtyp_pho[4];
  bzero(rtyp_pho, sizeof(rtyp_pho));
  key_typ_pho(phokeys[0], rtyp_pho);

  char xkeys[4];
  bzero(xkeys, sizeof(xkeys));

  for(i=0; i < 4; i++) {
    if (!rtyp_pho[i])
      continue;

    xkeys[i] = typ_pho_no_to_xkey(i, rtyp_pho[i]);
  }

  if (!xkeys[3])
    xkeys[3] = ' ';

  for(i=0; i < 4; i++) {
    feedkey_pho(xkeys[i], 0);
  }
}
