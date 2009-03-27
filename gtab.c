/*
	Copyright (C) 2004	 Edward Liu, Hsin-Chu, Taiwan
*/

#include <sys/stat.h>
#include "gcin.h"
#include "gtab.h"
#include "pho.h"
#include "gcin-conf.h"

extern char *TableDir;

#define MAX_SEL_BUF ((CH_SZ + 1) * MAX_SELKEY + 1)

u_long CONVT(char *s);

INMD inmd[MAX_GTAB_NUM_KEY+1];

INMD *cur_inmd;
static int spc_pressed=0;
static char seltab[MAX_SELKEY][MAX_CIN_PHR];
static int defselN=0;
static KeySym inch[MAX_TAB_KEY_NUM];
static int ci;
static int last_full, last_idx;
static int more_pg, pg_idx, m_pg_mode;
static int sel1st_i = MAX_SELKEY - 1;
static int wild_mode;
static int wild_page;

#define gtab_full_space_auto_first (gtab_space_auto_first & (GTAB_space_auto_first_any|GTAB_space_auto_first_full))

/* for array30-like quick code */
static char keyrow[]=
	      "qwertyuiop"
	      "asdfghjkl;"
	      "zxcvbnm,./";

int key_col(char cha)
{
  char *p = strchr(keyrow, cha);

  if (!p)
    return 0;

  return (p - keyrow)%10;
}

static int qcmp_strlen(const void *aa, const void *bb)
{
  char *a = *((char **)aa), *b = *((char **)bb);

  return strlen(a) - strlen(b);
}


void lookup_gtab(char *ch, char out[])
{
  char *tbuf[32];
  int tbufN=0;

  if (!cur_inmd)
    return;

  int i;

  for(i=0; i < cur_inmd->DefChars; i++) {
    if (bchcmp(cur_inmd->tbl[i].ch, ch))
      continue;
    u_long key = CONVT(cur_inmd->tbl[i].key);

    int j;

    int tlen=0;
    char t[CH_SZ * MAX_GTAB_ITEM_KEY_LEN + 1];
    for(j=MAX_TAB_KEY_NUM-1; j>=0; j--) {

      int sh = j * KeyBits;
      int k = (key >> sh) & 0x3f;

      if (!k)
        break;

      int len = (cur_inmd->keyname[k * CH_SZ] & 0x80) ? CH_SZ : 2;
//      dbg("uuuuuuuuuuuu %d %x len:%d\n", k, cur_inmd->keyname[k], len);
      memcpy(&t[tlen], &cur_inmd->keyname[k * CH_SZ], len);
      tlen+=len;
    }

    t[tlen]=0;

    tbuf[tbufN] = strdup(t);
    tbufN++;
  }


  qsort(tbuf, tbufN, sizeof(char *), qcmp_strlen);
  out[0]=0;

  for(i=0; i < tbufN; i++) {
#define MAX_DISP_MATCH 40
    if (strlen(out) < MAX_DISP_MATCH) {
      strcat(out, tbuf[i]);
      if (i < tbufN-1)
        strcat(out, " |");
    }

    free(tbuf[i]);
  }

  void set_key_codes_label(char *s);
  set_key_codes_label(out);
  void set_key_codes_label_pho(char *s);
  set_key_codes_label_pho(out);
}

void free_gtab()
{
  int i;

  for(i=0; i < 10; i++) {
    INMD *inp = &inmd[i];
    free(inp->tbl); inp->tbl = NULL;
    free(inp->phridx); inp->phridx = NULL;
    free(inp->phrbuf); inp->phrbuf = NULL;
  }
}


char *b1_cat(char *s, char c)
{
  char t[2];
  t[0]=c;
  t[1]=0;

  return strcat(s, t);
}


char *bch_cat(char *s, char *ch)
{
  char t[CH_SZ + 1];
  memcpy(t, ch, CH_SZ);
  t[CH_SZ]=0;

  return strcat(s, t);
}


void minimize_win_gtab();
void disp_gtab_sel(char *s);

static void ClrSelArea()
{
  disp_gtab_sel("");
  minimize_win_gtab();
}


void disp_gtab(int index, char *gtabchar);
static void ClrInArea()
{
  int i;

  disp_gtab(0, " ");

  for(i=1; i < cur_inmd->MaxPress; i++) {
    disp_gtab(i, " ");
  }

  last_idx=0;
}

static void ClrIn()
{
  bzero(inch,sizeof(inch));
  bzero(seltab,sizeof(seltab));
  m_pg_mode=pg_idx=more_pg=wild_mode=wild_page=last_idx=defselN=
  spc_pressed=ci=0;

  sel1st_i=MAX_SELKEY-1;
}

static void DispInArea()
{
  int i;

  for(i=0;i<ci;i++) {
    disp_gtab(i, &cur_inmd->keyname[inch[i] * CH_SZ]);
  }

  for(; i < cur_inmd->MaxPress; i++) {
    disp_gtab(i, "  ");
  }
}


void load_gtab_list()
{
  char ttt[128];
  FILE *fp;

//  strcat(strcpy(ttt,TableDir),"/gtab.list");
  get_sys_table_file_name("gtab.list", ttt);

  if ((fp=fopen(ttt, "r"))==NULL)
    exit(-1);

  dbg("load_gtab_list %s\n", ttt);

  while (!feof(fp)) {
    char name[32];
    char key[32];
    char file[32];

    name[0]=0;
    key[0]=0;
    file[0]=0;

    fscanf(fp, "%s %s %s", name, key, file);
    if (strlen(name) < 1)
      break;
    if (name[0]=='#')
      continue;

    int keyidx = atoi(key);
    if (keyidx < 0 || keyidx>=10)
      p_err("bad key value %s in %s\n", key, ttt);
    strcpy(inmd[keyidx].filename, file);
    strcpy(inmd[keyidx].cname, name);
  }

  fclose(fp);
}

void set_gtab_input_method_name(char *s);

void init_gtab(int inmdno, int usenow)
{
  FILE *fp;
  char ttt[128],uuu[128];
  int i;
  INMD *inp=&inmd[inmdno];
  struct TableHead th;


  if (!inmd[inmdno].filename[0] || !strcmp(inmd[inmdno].filename,"-"))
    return;

  strcat(strcpy(ttt,TableDir),"/");
  strcat(ttt, inmd[inmdno].filename);

  struct stat st;
  int rval = stat(ttt, &st);

  if (rval < 0) {
    dbg("init_tab:1 err open %s\n", ttt);
    return;
  }

  if (st.st_mtime == inp->file_modify_time) {
//    dbg("unchanged\n");
    set_gtab_input_method_name(inp->cname);
    cur_inmd=inp;
    return;    /* table is already loaded */
  }

  inp->file_modify_time = st.st_mtime;

  if ((fp=fopen(ttt, "r"))==NULL) {
    p_err("init_tab:2 err open %s", ttt);
  }

  dbg("gtab file %s\n", ttt);

  strcpy(uuu,ttt);

  fread(&th,1,sizeof(th),fp);
  fread(ttt,1,th.KeyS,fp);
  fread(inp->keyname, CH_SZ, th.KeyS, fp);
  memcpy(&inp->keyname[61*CH_SZ], "？", CH_SZ);  /* for wild card */
  memcpy(&inp->keyname[62*CH_SZ], "＊", CH_SZ);
  inp->KeyS=th.KeyS;
  inp->MaxPress=th.MaxPress;
  inp->DefChars=th.DefC;
  strcpy(inp->selkey,th.selkey);
  inp->M_DUP_SEL=th.M_DUP_SEL;

  dbg("MaxPress:%d\n", th.MaxPress);

  for(i=0;i<th.KeyS;i++) {
    inp->keymap[(int)ttt[i]]=i;
    inp->keymap[toupper(ttt[i])]=i;
    inp->keycol[i]=key_col(ttt[i]);
  }


  inp->keymap[(int)'?']=61;
  inp->keymap[(int)'*']=62;
  fread(inp->idx1, sizeof(gtab_idx1_t), th.KeyS+1,fp);
  /* printf("chars: %d\n",th.DefC); */
  dbg("inmdno: %d th.KeyS:%d\n", inmdno, th.KeyS);

  if (inp->tbl) {
    dbg("free %x\n", inp->tbl);
    free(inp->tbl);
  }

  if ((inp->tbl=tmalloc(ITEM, th.DefC))==NULL) {
    p_err("malloc err");
  }

  fread(inp->tbl,sizeof(ITEM),th.DefC, fp);

  memcpy(&inp->qkeys, &th.qkeys, sizeof(th.qkeys));
  inp->use_quick= th.qkeys.quick1[1][0][0] > 0;  // only array 30 use this

  fread(&inp->phrnum, sizeof(int), 1, fp);
  dbg("inp->phrnum: %d\n", inp->phrnum);
  if (inp->phridx)
    free(inp->phridx);
  inp->phridx = tmalloc(int, inp->phrnum);
  fread(inp->phridx, sizeof(int), inp->phrnum, fp);

#if 0
  for(i=0; i < inp->phrnum; i++)
    dbg("inp->phridx %d %d\n", i, inp->phridx[i]);
#endif

  int nbuf = 0;
  if (inp->phrnum)
    nbuf = inp->phridx[inp->phrnum-1];

  if (inp->phrbuf)
    free(inp->phrbuf);
  inp->phrbuf = malloc(nbuf);
  fread(inp->phrbuf, 1, nbuf, fp);

  fclose(fp);

  if (usenow) {
    cur_inmd=inp;
//    reset_inp();
    set_gtab_input_method_name(inp->cname);
    DispInArea();
  }
}

static char match_phrase[MAX_PHRASE_STR_LEN];
static int part_matched_len;
gboolean find_match(char *str, int len, char *match_str);

static void putstr_inp(u_char *p)
{
  if (strlen(p) > CH_SZ)
    send_text(p);
  else {
    char tt[512];

    lookup_gtab(p, tt);
    sendkey_b5(p);

    if (gtab_auto_select_by_phrase && !(gtab_space_auto_first & GTAB_space_auto_first_any)) {
      if (part_matched_len < strlen(match_phrase) &&
          !memcmp(&match_phrase[part_matched_len], p, CH_SZ)) {
          part_matched_len+=CH_SZ;
#if DPHR
          dbg("inc\n");
#endif
      } else {
        memcpy(&match_phrase[part_matched_len], p, CH_SZ);
        match_phrase[part_matched_len + CH_SZ] = 0;
#if DPHR
        dbg("%d   zzz %s\n",part_matched_len, match_phrase);
#endif
        if (find_match(match_phrase, part_matched_len + CH_SZ, NULL)) {
#if DPHR
          dbg("cat match_phrase %s\n", match_phrase);
#endif
          part_matched_len += CH_SZ;
        } else {
          strcpy(match_phrase, p);
          if (find_match(match_phrase, CH_SZ, NULL)) {
#if DPHR
            dbg("single match_phrase %s\n", match_phrase);
#endif
            part_matched_len = CH_SZ;
          }
          else {
#if DPHR
            dbg("no match\n");
#endif
            part_matched_len = 0;
            match_phrase[0]=0;
          }
        }
      }
    } // gtab_auto_select_by_phrase
  }

  ClrIn();
  ClrSelArea();
  ClrInArea();
}


static gboolean set_sel1st_i()
{
  if (!part_matched_len)
    return FALSE;

  char match_arr[512];

  int N = find_match(match_phrase, part_matched_len, match_arr);
  int i;

#if 0
  match_arr[N*CH_SZ] = 0;
  dbg("iii N:%d %s '%c%c%c'\n", N, match_arr, match_arr[0], match_arr[1], match_arr[2]);
#endif

  for(i=0; i < N; i++) {
    int j;

    for(j=0; j < MAX_SELKEY; j++) {
      if (!seltab[j][0])
        continue;
      int len = strlen(seltab[j]);
      if (len!=CH_SZ)
        continue;
      if (!memcmp(seltab[j], &match_arr[i*CH_SZ], CH_SZ)) {
        sel1st_i = j;

//        dbg("%d\n", j);
        return TRUE;
      }
    }
  }

  return FALSE;
}


#define swap(a,b) { tt=a; a=b; b=tt; }

static u_long vmask[]=
{ 0,
  0x3f<<24,  (0x3f<<24)|(0x3f<<18), (0x3f<<24)|(0x3f<<18)|(0x3f<<12),
 (0x3f<<24)|(0x3f<<18)|(0x3f<<12)|(0x3f<<6),
 (0x3f<<24)|(0x3f<<18)|(0x3f<<12)|(0x3f<<6)|0x3f
};


/* for strict match, use stack */
void wildcard()
{
  int i,t,vv,match, wild_ofs=0;
  u_long kk;
  int found=0;

  ClrSelArea();
  bzero(seltab,sizeof(seltab));
  /* printf("wild %d %d %d %d\n", inch[0], inch[1], inch[2], inch[3]); */
  defselN=0;

  char tt[MAX_SEL_BUF];
  tt[0]=0;

  for(t=0; t< cur_inmd->DefChars && defselN<10; t++) {
    ITEM it = cur_inmd->tbl[t];

    kk=CONVT(it.key);
    match=1;

    for(i=0;i<ci&&match && kk;) {
      switch (inch[i]) {
        case 61: /* ? */
          kk=(kk<<6) & vmask[4];
          i++;
          break;
        case 62: /* * */
          if (i==ci-1) {
            kk=0;
            i++;
            break;
          }

          if (inch[i+1]=='*' || inch[i+1]=='?') {
            i+=2;
            continue;
          }

          do {
            vv=(kk>>24)&0x3f;
            kk=(kk<<6) & vmask[4];
          } while (vv && vv!=inch[i+1]);

          if (vv!=inch[i+1]) match=0;

          if (i==ci-2 && kk)
            continue;

          i+=2;
          continue;
        default:
          if (inch[i]!=((kk>>24)&0x3f)) {
            match=0;
            break;
          }

          kk=(kk<<6) & vmask[4];
          i++;
      } /* switch */
    }

    if (i==ci && match && !(kk&vmask[4])) {
      if (wild_ofs >= wild_page) {
        b1_cat(tt, (defselN+1)%10 + '0');
        bch_cat(tt, it.ch);
        bchcpy(seltab[defselN++],it.ch);
        b1_cat(tt, ' ');
      } else
        wild_ofs++;

      found=1;
    }
  } /* for t */

  disp_gtab_sel(tt);

  if (!found)
    bell();
}

static char *ptr_selkey(char key)
{
  return strchr(cur_inmd->selkey, key);
}

static void load_phr(int j, char *tt)
{
  int len;

  int phrno=((int)(cur_inmd->tbl[j].ch[0])<<8)|
                  cur_inmd->tbl[j].ch[1];

  int ofs = cur_inmd->phridx[phrno], ofs1 = cur_inmd->phridx[phrno+1];

//  dbg("load_phr   %d  %d %d\n", phrno, ofs, ofs1);
  len = ofs1 - ofs;

  if (len > 128 || len <= 0) {
    dbg("phrae error %d\n", len);
    strcpy(tt,"err");
    return;
  }

  memcpy(tt, cur_inmd->phrbuf + ofs, len);
  tt[len]=0;
}



gboolean feedkey_gtab(KeySym key, int kbstate)
{
  int i,j;
  static int s1,e1;
  int inkey;
  int exa_match;
  char *pselkey= NULL;
  gboolean phrase_selected = FALSE;

  if (!cur_inmd)
    return 0;

  if (kbstate & (Mod1Mask|ControlMask)) {
      return 0;
  }

  if (current_IC->b_half_full_char) {
     char *s = half_char_to_full_char(key);
     if (!s)
       return 0;
     char tt[CH_SZ+1];

     memcpy(tt, s, CH_SZ); tt[CH_SZ]=0;
     send_text(tt);
     return 1;
  }

  if ((kbstate & ShiftMask) && key!='*' && key!='?') {
    char tt[2];

    if (key >= 127)
      return 0;

    if (kbstate & LockMask) {
      if (key >= 'a' && key <= 'z')
        key-=0x20;
    } else {
      if (key >= 'A' && key <= 'Z')
        key+=0x20;
    }

    tt[0] = key;
    tt[1] = 0;
    send_text(tt);
    return 1;
  }


  switch (key) {
    case XK_BackSpace:
#if     DELETE_K
    case XK_Delete:
#endif
      last_idx=0;
      if (ci==0) return 0;
      if (ci>0) inch[--ci]=0;
      wild_mode=spc_pressed=0;
      if (ci==1 && cur_inmd->use_quick) {
        int i;

        bzero(seltab,sizeof(seltab));
        for(i=0;i<10;i++)
          memcpy(seltab[i], &cur_inmd->qkeys.quick1[inch[0]-1][i][0], CH_SZ);

        defselN=10;
        DispInArea();
        goto Disp_opt;
      }
      break;
    case XK_Escape:
      if (ci) {
        ClrSelArea();
        ClrInArea();
        ClrIn();
        return 1;
      } else
        return 0;
    case '<':
      if (wild_mode) {
        if (wild_page>=10) wild_page-=10;
        wildcard();
        return 1;
      }
      return 0;
    case ' ':
      if (wild_mode) {
        if (defselN==10)
          wild_page+=10;
        else
          wild_page=0;
        wildcard();
        return 1;
      } else
      if (more_pg) {
        j=pg_idx;
        goto next_pg;
      } else
      if (ci==0) {
        if (last_full) {
          last_full=0;
          return 1;
        }
        return 0;
      } else {
//        dbg("iii %d  defselN:%d   %d\n", sel1st_i, defselN, cur_inmd->M_DUP_SEL);
        if (gtab_space_auto_first == GTAB_space_auto_first_any && seltab[0][0] &&
            sel1st_i==MAX_SELKEY-1 && defselN<=cur_inmd->M_DUP_SEL) {
          sel1st_i = 0;
        }

        if (seltab[sel1st_i][0]) {
//          dbg("last_full %d %d\n", last_full,spc_pressed);
          if (gtab_full_space_auto_first || spc_pressed) {
            putstr_inp((u_char *)&seltab[sel1st_i]);  /* select 1st */
            return 1;
          }
        }
      }

      last_full=0;
      spc_pressed=1;

      for(i=0;i<5;i++)
        if (inch[i]>60) {
          wild_page=0;
          wild_mode=1;
          wildcard();
          return 1;
        }

      break;
    case '?':
    case '*':
      if (ci< cur_inmd->MaxPress) {
        inkey=cur_inmd->keymap[key];
        inch[ci++]=inkey;
        DispInArea();
        if (ci==cur_inmd->MaxPress) {
          wild_page=0;
          wild_mode=1;
          wildcard();
        }
        return 1;
      }
      return 0;
    case XK_Shift_L:
    case XK_Shift_R:
    case XK_Control_R:
    case XK_Control_L:
    case XK_Alt_L:
    case XK_Alt_R:
    case XK_Caps_Lock:
      return 0;
    default:
      if (key>=XK_KP_0 && key<=XK_KP_9)
        key-=XK_KP_0-'0';

      if ((key < 32 || key > 0x7e) && (gtab_full_space_auto_first||spc_pressed)) {
//        dbg("sel1st_i:%d  '%c'\n", sel1st_i, seltab[sel1st_i][0]);

        if (seltab[sel1st_i][0])
          putstr_inp(seltab[sel1st_i]);  /* select 1st */

        return 0;
      }

      pselkey=ptr_selkey(key);

      if (pselkey && !ci)
        return 0;

      if (!pselkey && seltab[sel1st_i][0] && !wild_mode &&
           (gtab_full_space_auto_first||spc_pressed)) {
        putstr_inp(seltab[sel1st_i]);  /* select 1st */
      }

      if (wild_mode)
        goto XXXX;

      if (key > 0x7f)
        return 0;

      inkey=cur_inmd->keymap[key];
      spc_pressed=0;

      if (inkey>=1 && ci< cur_inmd->MaxPress) {
        inch[ci++]=inkey;
        last_full=0;
        if (ci==1 && cur_inmd->use_quick) {
          int i;
          for(i=0;i<10;i++)
            bchcpy(seltab[i], &cur_inmd->qkeys.quick1[inkey-1][i][0]);

          defselN=10;
          DispInArea();
          goto Disp_opt;
        }
      } else
      if (ci == cur_inmd->MaxPress && !pselkey) {
        bell();
        return 1;
      }

      if (inkey) {
        for(i=0;i<5;i++)
          if (inch[i]>60) {
            DispInArea();
            if (ci==cur_inmd->MaxPress) {
              wild_mode=1;
              wild_page=0;
              wildcard();
            }

            return 1;
          }
      }
#if 0
      if (pselkey && current_IC->in_method==8 && inch[0]==23 && ci==3)
        goto YYYY;
#endif
      if (!inkey && pselkey && defselN)
        goto YYYY;
      if (!inkey && !pselkey)
        return 0;
  } /* switch */

  if (ci==0) {
    ClrSelArea();
    ClrInArea();
    ClrIn();
    return 1;
  }

  DispInArea();

  static u_long val; // needs static
  val=0;

  for(i=0; i < MAX_TAB_KEY_NUM; i++)
    val|= inch[i] << (KeyBits * (MAX_TAB_KEY_NUM - 1 - i));

#if 1
  if (last_idx)
    s1=last_idx;
  else
#endif
    s1=cur_inmd->idx1[inch[0]];

  e1=cur_inmd->idx1[inch[0]+1];
  while ((CONVT(cur_inmd->tbl[s1].key)&vmask[ci]) != val &&
          CONVT(cur_inmd->tbl[s1].key)<val &&  s1<e1)
    s1++;

  last_idx=s1;
#if 0
  dbg("inch %d %d   val:%x\n", inch[0], inch[1], val);
  dbg("s1:%d e1:%d key:%x ci:%d vmask[ci]:%x ch:%c%c and:%x\n", s1, e1, CONVT(cur_inmd->tbl[s1].key),
     ci, vmask[ci], cur_inmd->tbl[s1].ch[0], cur_inmd->tbl[s1].ch[1],
     (CONVT(cur_inmd->tbl[s1].key)&vmask[ci]));
#endif

XXXX:
  if ((CONVT(cur_inmd->tbl[s1].key) & vmask[ci])!=val || (wild_mode && defselN) ||
                  ((ci==cur_inmd->MaxPress||spc_pressed) && defselN && pselkey)) {
YYYY:
    if ((pselkey || wild_mode) && defselN) {
      int vv=pselkey - cur_inmd->selkey;

      if (vv<0)
        vv=9;

      if (seltab[vv][0]) {
        putstr_inp(seltab[vv]);
        return 1;
      }
    }

    if (pselkey && !defselN)
      return 0;

    if (gtab_dup_select_bell)
      bell();

    if (ci>0)
      inch[--ci]=0;

    last_idx=0;
    DispInArea();
    return 1;
  }


refill:

  j=s1;

  if (ci < cur_inmd->MaxPress && !spc_pressed ) {
    int shiftb=(4-ci) * KeyBits;

    defselN=0;
    bzero(seltab, sizeof(seltab));
    while (CONVT(cur_inmd->tbl[j].key)==val && defselN <= cur_inmd->M_DUP_SEL) {
      if (cur_inmd->tbl[j].ch[0] >= 0x80)
        bchcpy(seltab[defselN++],cur_inmd->tbl[j].ch);

      j++;
    }

//    dbg("rrr %d\n", defselN);

    exa_match=defselN-1;

    if (gtab_disp_partial_match)
    while((CONVT(cur_inmd->tbl[j].key)&vmask[ci])==val && j<e1) {
      int fff=cur_inmd->keycol[(CONVT(cur_inmd->tbl[j].key)>>shiftb) & 0x3f];

      if (!(seltab[fff][0]) ||
           (bchcmp(seltab[fff],cur_inmd->tbl[j].ch)>0 && fff > exa_match)) {
        if (cur_inmd->tbl[j].ch[0] >= 0x80) {
          bchcpy(seltab[fff], cur_inmd->tbl[j].ch);
//          defselN++;
        }
      }

      j++;
    }
  } else {
next_pg:
    defselN=more_pg=0;
    bzero(seltab,sizeof(seltab));

    while(CONVT(cur_inmd->tbl[j].key)==val && defselN<cur_inmd->M_DUP_SEL && j<e1) {
      if (cur_inmd->tbl[j].ch[0]<0x80)
        load_phr(j++, seltab[defselN++]);
      else
        bchcpy(&seltab[defselN++], cur_inmd->tbl[j++].ch);

      if (ci == cur_inmd->MaxPress || spc_pressed) {
//        dbg("sel1st_i %d %d %d\n", ci, cur_inmd->MaxPress, spc_pressed);
        sel1st_i=0;

        if (gtab_auto_select_by_phrase && !(gtab_space_auto_first & GTAB_space_auto_first_any))
          phrase_selected = set_sel1st_i();
      }
    }

    if (j<e1 && CONVT(cur_inmd->tbl[j].key)==val && defselN==cur_inmd->M_DUP_SEL) {
      pg_idx=j;
      more_pg=1;
      m_pg_mode=1;
    } else if (m_pg_mode) {
      pg_idx=s1;
      more_pg=1;
    }

    if (defselN==1 && !more_pg) {
      if (ci==cur_inmd->MaxPress)
        last_full=1;

      if (spc_pressed || gtab_press_full_auto_send) {
        putstr_inp(seltab[0]);
        return 1;
      }
    } else
    if (!defselN) {
      bell();
      spc_pressed=0;
      goto refill;
    } else
    if (!more_pg) {
      if (gtab_dup_select_bell && (gtab_full_space_auto_first || spc_pressed))
        bell();
    }
  }

Disp_opt:

  ClrSelArea();

  char tt[MAX_SEL_BUF];
  tt[0]=0;

  for(i=0;i<10;i++) {
    if (seltab[i][0]) {
      b1_cat(tt, (i+1)%10 + '0');

      if (phrase_selected && i==sel1st_i) {
        strcat(tt, "<span foreground=\"blue\">");
        strcat(strcat(tt, seltab[i]), " ");
        strcat(tt, "</span>");
      } else {
        strcat(strcat(tt, seltab[i]), " ");
      }
    } else
      strcat(tt, " 　 ");
  }

  if (gtab_pre_select || spc_pressed || last_full)
    disp_gtab_sel(tt);

  return 1;
}
