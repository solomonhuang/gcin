/*
	Copyright (C) 2004	 Edward Liu, Hsin-Chu, Taiwan
*/

#include <sys/stat.h>
#include "gcin.h"
#include "gtab.h"
#include "pho.h"
#include "gcin-conf.h"
#include <regex.h>

typedef enum {
  SAME_PHO_QUERY_none = 0,
  SAME_PHO_QUERY_gtab_input = 1,
  SAME_PHO_QUERY_pho_select = 2,
} SAME_PHO_QUERY;

static SAME_PHO_QUERY same_pho_query_state = SAME_PHO_QUERY_none;

extern char *TableDir;

#define MAX_SEL_BUF ((CH_SZ + 1) * MAX_SELKEY + 1)


INMD inmd[MAX_GTAB_NUM_KEY+1];

INMD *cur_inmd;
static gboolean last_full, more_pg, wild_mode, m_pg_mode, spc_pressed;
static char seltab[MAX_SELKEY][MAX_CIN_PHR];
static u_short defselN, exa_match;
static KeySym inch[MAX_TAB_KEY_NUM64];
static int ci;
static u_short last_idx, pg_idx;
static u_short wild_page;
static int sel1st_i = MAX_SELKEY - 1;

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

#define tblch(i) (cur_inmd->key64 ? cur_inmd->tbl64[i].ch:cur_inmd->tbl[i].ch)
#define Max_tab_key_num (cur_inmd->key64 ? MAX_TAB_KEY_NUM64 : MAX_TAB_KEY_NUM)

void lookup_gtab(char *ch, char out[])
{
  char *tbuf[32];
  int tbufN=0;

  if (!cur_inmd)
    return;

  int i;
  for(i=0; i < cur_inmd->DefChars; i++) {
    if (bchcmp(tblch(i), ch))
      continue;

    u_int64_t key = CONVT2(cur_inmd, i);

    int j;

    int tlen=0;
    char t[CH_SZ * MAX_TAB_KEY_NUM64 + 1];

    for(j=Max_tab_key_num - 1; j>=0; j--) {

      int sh = j * KeyBits;
      int k = (key >> sh) & 0x3f;

      if (!k)
        break;
      int len;
      char *keyname;

      if (cur_inmd->keyname_lookup) {
        len = 1;
        keyname = &cur_inmd->keyname_lookup[k];
      } else {
        keyname = &cur_inmd->keyname[k * CH_SZ];
        len = (*keyname & 0x80) ? CH_SZ : 2;
      }
//      dbg("uuuuuuuuuuuu %d %x len:%d\n", k, cur_inmd->keyname[k], len);
      memcpy(&t[tlen], keyname, len);
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
    free(inp->tbl64); inp->tbl64 = NULL;
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

static void ClrIn()
{
  bzero(inch,sizeof(inch));
  bzero(seltab,sizeof(seltab));
  m_pg_mode=pg_idx=more_pg=wild_mode=wild_page=last_idx=defselN=exa_match=
  spc_pressed=ci=0;

  sel1st_i=MAX_SELKEY-1;
}

void clear_gtab_in_area();

static void ClrInArea()
{
  clear_gtab_in_area();
  last_idx = 0;
}


void hide_win_pho();

void close_gtab_pho_win()
{
  if (same_pho_query_state != SAME_PHO_QUERY_none) {
    same_pho_query_state = SAME_PHO_QUERY_none;
    hide_win_pho();
  }
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
  struct TableHead2 th2;

  current_CS->b_half_full_char = FALSE;

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

  fread(ttt, 1, strlen(gtab64_header)+1, fp);

  gboolean read_th2 = FALSE;

  if (!strcmp(ttt, gtab64_header)) {
    inp->max_keyN = 10;
    inp->key64 = TRUE;
    read_th2 = TRUE;
    dbg("it's a 64-bit .gtab\n");
  } else
  if (!strcmp(ttt, gtab32_ver2_header)) {
    inp->max_keyN = 5;
    read_th2 = TRUE;
    dbg("it's a 32-bit ver2 .gtab\n");
  }
  else {
    inp->max_keyN = 5;
    fseek(fp, 0, SEEK_SET);
  }

  strcpy(uuu,ttt);

  fread(&th,1,sizeof(th),fp);
  if (read_th2) {
    dbg("read_th2\n");
    fread(&th2,1,sizeof(th2),fp);
    memcpy(inp->endkey, th2.endkey, sizeof(th2.endkey));
  }

  fread(ttt, 1, th.KeyS, fp);
  fread(inp->keyname, CH_SZ, th.KeyS, fp);
  memcpy(&inp->keyname[61*CH_SZ], "？", CH_SZ);  /* for wild card */
  memcpy(&inp->keyname[62*CH_SZ], "＊", CH_SZ);

  // for boshiamy
  extern u_char *fullchar[];
  gboolean all_full_ascii = TRUE;
  char keyname_lookup[MAX_GTAB_KEYS];

  bzero(keyname_lookup, sizeof(keyname_lookup));
  for(i=1; i < th.KeyS; i++) {
    char *keyname = &inp->keyname[i*CH_SZ];
    int len = utf8_sz(keyname);
    int j;

    if (len==1 && utf8_sz(keyname + 1)) { // array30
      all_full_ascii = FALSE;
      break;
    }

#define FULLN (127 - ' ')

    for(j=0; j < FULLN; j++)
      if (!memcmp(fullchar[j], keyname, len)) {
        break;
      }

    if (j==FULLN) {
      dbg("all_full_ascii %d\n", j);
      all_full_ascii = FALSE;
      break;
    }

    keyname_lookup[i] = ' ' + j;
  }


  if (all_full_ascii) {
    inp->keyname_lookup = malloc(sizeof(char) * MAX_GTAB_KEYS);
    memcpy(inp->keyname_lookup, keyname_lookup, MAX_GTAB_KEYS);
  }

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

  if (inp->key64) {
    if (inp->tbl64) {
      dbg("free %x\n", inp->tbl64);
      free(inp->tbl64);
    }

    if ((inp->tbl64=tmalloc(ITEM64, th.DefC))==NULL) {
      p_err("malloc err");
    }

    fread(inp->tbl64, sizeof(ITEM64), th.DefC, fp);
  } else {
    if (inp->tbl) {
      dbg("free %x\n", inp->tbl);
      free(inp->tbl);
    }

    if ((inp->tbl=tmalloc(ITEM, th.DefC))==NULL) {
      p_err("malloc err");
    }

    fread(inp->tbl,sizeof(ITEM),th.DefC, fp);
  }

  dbg("chars %d\n", th.DefC);

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

  inp->max_keyN = 5;

  if (usenow) {
    cur_inmd=inp;
//    reset_inp();
    set_gtab_input_method_name(inp->cname);
    DispInArea();
  }


  dbg("key64: %d\n", inp->key64);


#if 0
  for(i='A'; i < 127; i++)
    printf("%d] %c %d\n", i, i, inp->keymap[i]);
#endif
#if 0
  for(i=0; i < 100; i++) {
    u_char *ch = tblch(i);
    dbg("%d] %c%c%c\n", i, ch[0], ch[1], ch[2]);
  }
#endif
}

static char match_phrase[MAX_PHRASE_STR_LEN];
static int part_matched_len;
gboolean find_match(char *str, int len, char *match_str, int match_strs_max);
void start_gtab_pho_query(char *utf8);

static void clear_phrase_match_buf()
{
   part_matched_len = 0;
   match_phrase[0]=0;
}

static void putstr_inp(u_char *p)
{
  if (strlen(p) > CH_SZ || p[0] < 128) {
    send_text(p);
  }
  else {
    char tt[512];

    if (same_pho_query_state == SAME_PHO_QUERY_gtab_input) {
      same_pho_query_state = SAME_PHO_QUERY_pho_select;
      start_gtab_pho_query(p);

      ClrIn();
      ClrSelArea();
      ClrInArea();
      return;
    }

    if (gtab_disp_key_codes || wild_mode)
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
        if (find_match(match_phrase, part_matched_len + CH_SZ, NULL, 0)) {
#if DPHR
          dbg("cat match_phrase %s\n", match_phrase);
#endif
          part_matched_len += CH_SZ;
        } else {
          strcpy(match_phrase, p);
          if (find_match(match_phrase, CH_SZ, NULL, 0)) {
#if DPHR
            dbg("single match_phrase %s\n", match_phrase);
#endif
            part_matched_len = CH_SZ;
          }
          else {
#if DPHR
            dbg("no match\n");
#endif
            clear_phrase_match_buf();
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

#define MAX_MATCH_STRS 1024

  char match_arr[CH_SZ * MAX_MATCH_STRS + 1];

  int N = find_match(match_phrase, part_matched_len, match_arr, MAX_MATCH_STRS);
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

static u_int vmask[]=
{ 0,
  0x3f<<24,  (0x3f<<24)|(0x3f<<18), (0x3f<<24)|(0x3f<<18)|(0x3f<<12),
 (0x3f<<24)|(0x3f<<18)|(0x3f<<12)|(0x3f<<6),
 (0x3f<<24)|(0x3f<<18)|(0x3f<<12)|(0x3f<<6)|0x3f
};

#define KKK ((u_int64_t)0x3f)


static u_int64_t vmask64[]=
{ 0,
  (KKK<<54),
  (KKK<<54)|(KKK<<48),
  (KKK<<54)|(KKK<<48)|(KKK<<42),
  (KKK<<54)|(KKK<<48)|(KKK<<42)|(KKK<<36),
  (KKK<<54)|(KKK<<48)|(KKK<<42)|(KKK<<36)|(KKK<<30),
  (KKK<<54)|(KKK<<48)|(KKK<<42)|(KKK<<36)|(KKK<<30)|(KKK<<24),
  (KKK<<54)|(KKK<<48)|(KKK<<42)|(KKK<<36)|(KKK<<30)|(KKK<<24)|(KKK<<18),
  (KKK<<54)|(KKK<<48)|(KKK<<42)|(KKK<<36)|(KKK<<30)|(KKK<<24)|(KKK<<18)|(KKK<<12),
  (KKK<<54)|(KKK<<48)|(KKK<<42)|(KKK<<36)|(KKK<<30)|(KKK<<24)|(KKK<<18)|(KKK<<12)|(KKK<<6),
  (KKK<<54)|(KKK<<48)|(KKK<<42)|(KKK<<36)|(KKK<<30)|(KKK<<24)|(KKK<<18)|(KKK<<12)|(KKK<<6)|KKK
};

#define KEY_N (cur_inmd->max_keyN)

void wildcard()
{
  int i,t,match, wild_ofs=0;
  u_int64_t  kk;
  int found=0;
  regex_t reg;

  ClrSelArea();
  bzero(seltab,sizeof(seltab));
  /* printf("wild %d %d %d %d\n", inch[0], inch[1], inch[2], inch[3]); */
  defselN=0;
  char regstr[16];
  int regstrN=0;

  regstr[regstrN++]=' ';

  for(i=0; i < KEY_N; i++) {
    if (!inch[i])
      break;
    if (inch[i] == 62) {
      regstr[regstrN++]='.';
      regstr[regstrN++]='*';
    } else
    if (inch[i] == 61) {
      regstr[regstrN++]='.';
    } else {
      char c = inch[i] + '0';         // start from '0'
      if (strchr("*.\()[]", c))
      regstr[regstrN++] = '\\';
      regstr[regstrN++]=c;
    }
  }

  regstr[regstrN++]=' ';
  regstr[regstrN]=0;

//  dbg("regstr %s\n", regstr);

  if (regcomp(&reg, regstr, 0)) {
    dbg("regcomp failed\n");
    return;
  }

  char tt[MAX_SEL_BUF];
  tt[0]=0;

  for(t=0; t< cur_inmd->DefChars && defselN < strlen(cur_inmd->selkey); t++) {
    kk=CONVT2(cur_inmd, t);
    match=1;
    char ts[32];
    int tsN=0;

    ts[tsN++]= ' ';

    for(i=0; i < KEY_N; i++) {
      char c = (kk >> (LAST_K_bitN - i*6)) & 0x3f;
      if (!c)
        break;
      ts[tsN++] = c + '0';
    }

    ts[tsN++]= ' ';
    ts[tsN]=0;

    if (!regexec(&reg, ts, 0, 0, 0)) {
      if (wild_ofs >= wild_page) {
        b1_cat(tt, cur_inmd->selkey[defselN]);
        bch_cat(tt, tblch(t));
        bchcpy(seltab[defselN++], tblch(t));
        b1_cat(tt, ' ');
      } else
        wild_ofs++;

      found=1;
    }
  } /* for t */

  regfree(&reg);
  disp_gtab_sel(tt);

  if (!found)
    bell();
}

static char *ptr_selkey(KeySym key)
{
  if (key>= XK_KP_0 && key<= XK_KP_9)
    key-= XK_KP_0 - '0';
  return strchr(cur_inmd->selkey, key);
}

static void load_phr(int j, char *tt)
{
  int len;
  char *ch = tblch(j);

  int phrno=((int)(ch[0])<<8)|ch[1];

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

void init_gtab_pho_query_win();
int feedkey_pho(KeySym xkey);

void set_gtab_target_displayed()
{
  close_gtab_pho_win();
}

gboolean is_gtab_query_mode()
{
  return same_pho_query_state == SAME_PHO_QUERY_pho_select;
}

void reset_gtab_all()
{
  if (!cur_inmd)
    return;

  ClrIn();
  ClrSelArea();
  ClrInArea();
}

static void disp_selection(gboolean phrase_selected)
{
  ClrSelArea();

  char tt[MAX_SEL_BUF];
  tt[0]=0;

  int ofs;

  if (exa_match && (gtab_space_auto_first & GTAB_space_auto_first_any)) {
    strcat(tt, seltab[0]);
    strcat(tt, " ");
    ofs = 1;
  } else {
    ofs = 0;
  }

  int i;
  for(i=ofs; i<10 + ofs; i++) {
    if (seltab[i][0]) {
      b1_cat(tt, cur_inmd->selkey[i - ofs]);

      if (phrase_selected && i==sel1st_i) {
        strcat(tt, "<span foreground=\"blue\">");
        strcat(strcat(tt, seltab[i]), " ");
        strcat(tt, "</span>");
      } else {
        strcat(strcat(tt, seltab[i]), " ");
      }
    } else {
      extern gboolean b_use_full_space;

      if (b_use_full_space)
         strcat(tt, " 　 ");
      else
         strcat(tt, "   ");
    }
  }

  if (gtab_pre_select || wild_mode || spc_pressed || last_full)
    disp_gtab_sel(tt);
}

static gboolean has_wild_card()
{
  int i;

  for(i=0; i < cur_inmd->MaxPress; i++)
    if (inch[i]>60) {
      return TRUE;
    }

  return FALSE;
}

static void proc_wild_disp()
{
   DispInArea();
   wild_page = 0;
   wildcard();
   disp_selection(0);
}


gboolean feedkey_gtab(KeySym key, int kbstate)
{
  int i,j=0;
  static int s1,e1;
  int inkey;
  char *pselkey= NULL;
  gboolean phrase_selected = FALSE;

//  dbg("uuuuu %x %x\n", key, kbstate);

  if (!cur_inmd)
    return 0;

  if (kbstate & (Mod1Mask|ControlMask)) {
      return 0;
  }


  if (same_pho_query_state == SAME_PHO_QUERY_pho_select)
    return feedkey_pho(key);

  if (current_CS->b_half_full_char) {
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

  gboolean has_wild = FALSE;

  switch (key) {
    case XK_BackSpace:
      clear_phrase_match_buf();
      last_idx=0;

      if (ci==0)
        return 0;

      if (ci>0)
        inch[--ci]=0;

      if (has_wild_card()) {
        proc_wild_disp();
        return 1;
      }


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
    case XK_Return:
      clear_phrase_match_buf();
      return 0;
    case XK_Escape:
      close_gtab_pho_win();
      if (ci) {
        reset_gtab_all();
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
      has_wild = has_wild_card();

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
      } else
      if (!has_wild) {
//        dbg("iii %d  defselN:%d   %d\n", sel1st_i, defselN, cur_inmd->M_DUP_SEL);
        if (gtab_space_auto_first == GTAB_space_auto_first_any && seltab[0][0] &&
            sel1st_i==MAX_SELKEY-1 && exa_match<=cur_inmd->M_DUP_SEL) {
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

      if (has_wild) {
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
//        if (ci==cur_inmd->MaxPress)
        {
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
    case '`':
      same_pho_query_state = SAME_PHO_QUERY_gtab_input;
      init_gtab_pho_query_win();
      return 1;
    default:
      if (key>=XK_KP_0 && key<=XK_KP_9) {
        if (!spc_pressed)
          return 0;
      }

      char *pendkey = strchr(cur_inmd->endkey, key);

      // for array30
      if (!ci) {
        if (cur_inmd->endkey[0] && pendkey)
          return 0;
      }

      pselkey=ptr_selkey(key);


      if ((key < 32 || key > 0x7e) && (gtab_full_space_auto_first||spc_pressed)) {
//        dbg("sel1st_i:%d  '%c'\n", sel1st_i, seltab[sel1st_i][0]);
        if (seltab[sel1st_i][0])
          putstr_inp(seltab[sel1st_i]);  /* select 1st */

        return 0;
      }


      inkey=cur_inmd->keymap[key];

#if 1 // for dayi, testcase :  6 space keypad6
      if ((spc_pressed||(wild_mode && (!inkey ||pendkey))) && pselkey) {
        int vv = pselkey - cur_inmd->selkey;

        if ((gtab_space_auto_first & GTAB_space_auto_first_any) && !wild_mode)
          vv++;

        if (vv<0)
          vv=9;

        if (seltab[vv][0]) {
          putstr_inp(seltab[vv]);
          return 1;
        }
      }
#endif

//      dbg("iii %x\n", pselkey);
      if (seltab[sel1st_i][0] && !wild_mode &&
           (gtab_full_space_auto_first||spc_pressed)) {
        putstr_inp(seltab[sel1st_i]);  /* select 1st */
      }

#if 0
      if (wild_mode)
        goto XXXX;
#endif
      if (key > 0x7f)
        return 0;

      spc_pressed=0;

      // for cj & boshiamy to input digits
      if (!ci && !inkey)
          return 0;

      if (wild_mode && inkey>=1 && ci< cur_inmd->MaxPress) {
        inch[ci++]=inkey;
        proc_wild_disp();
        return 1;
      }

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
        for(i=0; i < MAX_TAB_KEY_NUM64; i++)
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
#if 1
      if (!inkey && pselkey && defselN)
        goto YYYY;
#endif
      if (!inkey && !pselkey)
        return 0;
  } /* switch */


  if (ci==0) {
    ClrSelArea();
    ClrInArea();
    ClrIn();
    return 1;
  }

  char *pendkey = strchr(cur_inmd->endkey, key);

  DispInArea();

  static u_int64_t val; // needs static
  val=0;

  for(i=0; i < Max_tab_key_num; i++)
    val|= (u_int64_t)inch[i] << (KeyBits * (Max_tab_key_num - 1 - i));

//  dbg("--------- %d val %llx\n", Max_tab_key_num, val);
#if 1
  if (last_idx)
    s1=last_idx;
  else
#endif
    s1=cur_inmd->idx1[inch[0]];

  e1=cur_inmd->idx1[inch[0]+1];
  u_int64_t vmaskci = cur_inmd->key64 ? vmask64[ci]:vmask[ci];

  while ((CONVT2(cur_inmd, s1) & vmaskci) != val &&
          CONVT2(cur_inmd, s1) < val &&  s1<e1)
    s1++;

  last_idx=s1;
#if 0
  dbg("inch %d %d   val:%x\n", inch[0], inch[1], val);
  u_char *tbl_ch = tblch(s1);
  dbg("s1:%d e1:%d key:%llx ci:%d vmask[ci]:%llx ch:%c%c%c and:%x\n", s1, e1, CONVT2(cur_inmd, s1),
     ci, vmaskci, tbl_ch[0], tbl_ch[1], tbl_ch[2], CONVT2(cur_inmd, s1) & vmask[ci]);

  dbg("pselkey:%x  %d  defselN:%d\n", pselkey,
       (CONVT2(cur_inmd, s1) & vmask[ci])!=val,
       defselN);
#endif

XXXX:
  vmaskci = cur_inmd->key64 ? vmask64[ci]:vmask[ci];

  if ((CONVT2(cur_inmd, s1) & vmaskci)!=val || (wild_mode && defselN) ||
                  ((ci==cur_inmd->MaxPress||spc_pressed) && defselN &&
      (pselkey && ( pendkey || spc_pressed)) ) ) {
YYYY:
    if ((pselkey || wild_mode) && defselN) {
      int vv = pselkey - cur_inmd->selkey;

      if ((gtab_space_auto_first & GTAB_space_auto_first_any) && !wild_mode &&
          (!cur_inmd->use_quick || ci!=2))
        vv++;

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

  if (ci < cur_inmd->MaxPress && !spc_pressed && !strchr(cur_inmd->endkey, key)) {
    int shiftb=(KEY_N - 1 -ci) * KeyBits;

    exa_match=0;
    bzero(seltab, sizeof(seltab));
    while (CONVT2(cur_inmd, j)==val && exa_match <= cur_inmd->M_DUP_SEL) {
      u_char *tbl_ch = tblch(j);

      if (tbl_ch[0] >= 0x80)
        bchcpy(seltab[exa_match++], tbl_ch);

      j++;
    }


    defselN=exa_match;

    if (defselN > cur_inmd->M_DUP_SEL)
      defselN--;

    if (gtab_disp_partial_match)
    while((CONVT2(cur_inmd, j) & vmask[ci])==val && j<e1) {
      int fff=cur_inmd->keycol[(CONVT2(cur_inmd, j)>>shiftb) & 0x3f];
      u_char *tbl_ch = tblch(j);

      if (!(seltab[fff][0]) ||
           (bchcmp(seltab[fff], tbl_ch)>0 && fff > exa_match)) {
        if (tbl_ch[0] >= 0x80) {
          bchcpy(seltab[fff], tbl_ch);
          defselN++;
        }
      }

      j++;
    }
  } else {
next_pg:
    defselN=more_pg=0;
    bzero(seltab,sizeof(seltab));
    if (strchr(cur_inmd->endkey, key))
      spc_pressed = 1;

    while(CONVT2(cur_inmd, j)==val && defselN<cur_inmd->M_DUP_SEL && j<e1) {
      u_char *tbl_ch = tblch(j);

      if (tbl_ch[0] < 0x80)
        load_phr(j, seltab[defselN]);
      else {
        bchcpy(seltab[defselN], tbl_ch);
      }

      j++; defselN++;

      if (ci == cur_inmd->MaxPress || spc_pressed) {
//        dbg("sel1st_i %d %d %d\n", ci, cur_inmd->MaxPress, spc_pressed);
        sel1st_i=0;

        if (gtab_auto_select_by_phrase && !(gtab_space_auto_first & GTAB_space_auto_first_any))
          phrase_selected = set_sel1st_i();
      }
    }

    if (j<e1 && CONVT2(cur_inmd, j)==val && defselN==cur_inmd->M_DUP_SEL) {
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
  disp_selection(phrase_selected);

  return 1;
}
