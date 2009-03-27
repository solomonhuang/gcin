/*
	Copyright (C) 1994,1995	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/


#include "gcin.h"
#include "pho.h"
#include <sys/stat.h>
#include <stdlib.h>

static char out_buffer[45];
int out_bufferN;

extern PHO_ITEM *ch_pho;

int ityp3_pho;
static int cpg,maxi;
int start_idx, stop_idx;
int ch_pho_ofs;
PHOKBM phkbm;
u_char typ_pho[4];
char inph[4];

#define MAX_HASH_PHO 27
u_short hash_pho[MAX_HASH_PHO+1];

phokey_t pho2key(char typ_pho[])
{
  return (u_short)typ_pho[0]<<9 | (u_short) typ_pho[1]<<7 |
	 (u_short)typ_pho[2]<<3 | typ_pho[3];
}

void clrin_pho()
{
  bzero(typ_pho,sizeof(typ_pho));
  bzero(inph,sizeof(inph));
  maxi=ityp3_pho=0;
}

#define InAreaX (0)

static void clr_in_area_pho()
{
  int i;

  for(i=0; i < 4; i++)
    disp_pho(i, "  ");
}


static void disp_in_area_pho()
{
  int i;
  extern int cursor_x;

  for(i=0;i<4;i++) {
    disp_pho(i, &pho_chars[i][typ_pho[i]*2]);
  }
}

static int qcmp_count(const void *aa, const void *bb)
{
  PHO_ITEM *a = (PHO_ITEM *)aa;
  PHO_ITEM *b = (PHO_ITEM *)bb;

  return b->count - a->count;
}

static void ClrSelArea()
{
  disp_pho_sel(" ");
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
    if (!memcmp(ch_pho[i].ch, big5, 2)) {
      return i;
    }
  }

  prph(phkey);
//  dbg("error found   %c%c", *big5, *(big5+1));
  return -1;
}


void inc_pho_count(u_short key, int ch_idx)
{
  int start_i, stop_i;

  get_start_stop_idx(key, &start_i, &stop_i);

//  dbg("start_i %d %d    %d %d\n", start_i, stop_i, start_idx, stop_idx);
  if (ch_pho[ch_idx].count == 65535) {
    int i;

    for(i=start_i; i < stop_i; i++) {
      ch_pho[start_i].count /= 2;
    }
  }

  ch_pho[ch_idx].count++;

  qsort(&ch_pho[start_i], stop_i - start_i, sizeof(PHO_ITEM), qcmp_count);

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


void putkey_pho(u_short key, int idx)
{
  sendkey_b5(ch_pho[idx].ch);
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

  get_gcin_conf_str("phonetic-keyboard", phokbm_name, "zo");
  strcat(phokbm_name, ".kbm");
  get_sys_table_file_name(phokbm_name, kbmfname);

  if ((fr=fopen(kbmfname,"r"))==NULL) {
     p_err("Cannot open %s", kbmfname);
  }

  fread(&phkbm,sizeof(phkbm),1,fr);
  fclose (fr);

#if     0
  for(i=0;i<128;i++)
  if (phkbm.phokbm[i][0][0])
          printf("%c %d %d\n", i, phkbm.phokbm[i][0][0], phkbm.phokbm[i][0][1]);
#endif
}


void init_tab_pho(int usenow)
{
  int i,cou;

  if (!ch_pho) {
    load_tab_pho_file();
  }

  create_win_pho();
  clr_in_area_pho();
  clrin_pho();
}



int feedkey_pho(KeySym xkey)
{
  static int ctyp;
  static unsigned int vv, ii, ttt;
  static u_short key;
  u_char *pp=NULL;
  char kno;
  int i,j,jj=0,kk=0;

  if (xkey >= 'A' && xkey <='Z')
    xkey+=0x20;

  switch (xkey) {
    case XK_Escape:
      if (!typ_pho[0] &&!typ_pho[1] &&!typ_pho[2] &&!typ_pho[3]) return 0;
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
      if (j<0) return 0;
       goto llll3;
    case '<':
       if (!ityp3_pho)
         return 0;
       if (cpg>=SELKEY)
         cpg-=SELKEY;
       goto proc_state;
    case ' ':
      if (!typ_pho[0] && !typ_pho[1] && !typ_pho[2])
        return 0;
      if (!ityp3_pho && xkey==' ') {
         ctyp=3;  kno=0; jj=0;
         if (typ_pho[0] && !typ_pho[1] && !typ_pho[2] && !typ_pho[3]) {
           char tch=inph[0];
           if(phkbm.phokbm[tch][1][1]==2) {
             typ_pho[0]=0;
             typ_pho[2]=phkbm.phokbm[tch][1][0];
           }
         }
         goto llll1;
       }
      ii=start_idx+cpg+SELKEY;
      if (ii < stop_idx)
        cpg+=SELKEY;
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

      while(i<SELKEY  && ii< stop_idx) {
//        xprintf("%c%c%c ", phkbm.selkey[i], ch_pho[ii].ch[0],ch_pho[ii].ch[1]);
        out_buffer[out_bufferN++] = phkbm.selkey[i];
        memcpy(&out_buffer[out_bufferN], ch_pho[ii].ch, 2);
        out_bufferN+=2;
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

      if ((pp=strchr(phkbm.selkey,xkey)) && maxi && ityp3_pho) {
        int c=pp-phkbm.selkey;
        if (c<maxi) {
          putkey_pho(key, start_idx+cpg+c);
        }
        return 1;
      }

      if (ityp3_pho && !cpg) {
        putkey_pho(key, start_idx);
      }

      cpg=0;
  }

  for(i=2;i>=0;i--)
    if (typ_pho[i])
      break;

  kno=phkbm.phokbm[xkey][0][0];
  ctyp=phkbm.phokbm[xkey][0][1];

  for(j=0;j<3;j++)
  if (phkbm.phokbm[xkey][j][1] > i) {
       kno=phkbm.phokbm[xkey][j][0];
       ctyp=phkbm.phokbm[xkey][j][1];
       break;
  }
#if    0
  printf("xkey:%c kno:%d ctyp:%d\n", xkey, kno, ctyp);
#endif
  if (!kno) return 0;
  typ_pho[ctyp]=kno;
  inph[ctyp]=xkey;
llll1:
  jj=0;
  kk=1;
llll2:
  if (ctyp==3) {
       ityp3_pho=1;  /* last key is entered */
  }
llll3:

  key = pho2key(typ_pho);

#if    0
  printf("??%d %d %d %d\n", typ_pho[0], typ_pho[1], typ_pho[2], typ_pho[3]);
#endif
  if (!key)
    return 1;

  vv=hash_pho[typ_pho[0]];
  ttt=0xffff;

  while (vv < idxnum_pho) {
    ttt=idx_pho[vv].key;
    if (!typ_pho[0]) ttt &= ~(31<<9);
    if (!typ_pho[1]) ttt &= ~(3<<7);
    if (!typ_pho[2]) ttt &= ~(15<<3);
    if (!typ_pho[3]) ttt &= ~(7);

    if (ttt>=key)
      break;
    else
      vv++;
  }

  if (ttt > key || (ityp3_pho && idx_pho[vv].key != key) ) {
    while (jj<4) {
      while(kk<3)
        if (phkbm.phokbm[inph[jj]][kk][0] ) {
          if (kk) {
            ctyp=phkbm.phokbm[inph[jj]][kk-1][1];
            typ_pho[ctyp]=0;
          }
          kno=phkbm.phokbm[inph[jj]][kk][0];
          ctyp=phkbm.phokbm[inph[jj]][kk][1];
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
    while(i<SELKEY  && ii < stop_idx) {
      out_buffer[out_bufferN++] = phkbm.selkey[i];
      memcpy(&out_buffer[out_bufferN], ch_pho[ii].ch, 2);
      out_bufferN+=2;
      out_buffer[out_bufferN++] = ' ';

      ii++;
      i=i+1;
    }

#if 0
    if (cpg)
      xprintf("<");
    else
      xprintf(" ");
#endif
    out_buffer[out_bufferN++] = cpg ? '<' : ' ';

    if (ii < stop_idx) {
/*
      if (cpg)
        xprintf("\\");
      else xprintf(" ");
        xprintf(">");
*/
      out_buffer[out_bufferN++] = cpg ? '\\' : ' ';
      out_buffer[out_bufferN++] = '>';
    } else
      cpg=0;

    maxi=i;
  } else {
    while(i<SELKEY  && ii < stop_idx) {
//      xprintf("%c%c ", ch_pho[ii].ch[0],ch_pho[ii].ch[1]);
      memcpy(&out_buffer[out_bufferN], ch_pho[ii].ch, 2);
      out_bufferN+=2;

      ii++;
      i++;
    }
    maxi=i;
  }

  out_buffer[out_bufferN] = 0;
  disp_pho_sel(out_buffer);

  return 1;
}
