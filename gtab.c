/*
	Copyright (C) 2004	 Edward Liu, Hsin-Chu, Taiwan
*/

#include <sys/stat.h>
#include <regex.h>
#include "gcin.h"
#include "gtab.h"
#include "pho.h"
#include "gcin-conf.h"

typedef enum {
  SAME_PHO_QUERY_none = 0,
  SAME_PHO_QUERY_gtab_input = 1,
  SAME_PHO_QUERY_pho_select = 2,
} SAME_PHO_QUERY;

static GTAB_space_pressed_E _gtab_space_auto_first;
static SAME_PHO_QUERY same_pho_query_state = SAME_PHO_QUERY_none;

extern char *TableDir;

#define MAX_SEL_BUF ((CH_SZ + 1) * MAX_SELKEY + 1)


INMD inmd[MAX_GTAB_NUM_KEY+1];

INMD *cur_inmd;
static gboolean last_full, more_pg, wild_mode, m_pg_mode, spc_pressed, invalid_spc;
static char seltab[MAX_SELKEY][MAX_CIN_PHR];
static short defselN, exa_match;
static KeySym inch[MAX_TAB_KEY_NUM64];
static int ci;
static int last_idx, pg_idx;
static int wild_page;
static int sel1st_i = MAX_SELKEY - 1;

#define gtab_full_space_auto_first (_gtab_space_auto_first & (GTAB_space_auto_first_any|GTAB_space_auto_first_full))

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

gboolean same_query_show_pho_win()
{
  return same_pho_query_state != SAME_PHO_QUERY_none;
}

gboolean gtab_has_input()
{
  int i;

  for(i=0; i < MAX_TAB_KEY_NUM64; i++)
    if (inch[i])
      return TRUE;

  if (same_query_show_pho_win())
    return TRUE;

  return FALSE;
}


static int qcmp_strlen(const void *aa, const void *bb)
{
  char *a = *((char **)aa), *b = *((char **)bb);

  return strlen(a) - strlen(b);
}

#define tblch(i) (cur_inmd->key64 ? cur_inmd->tbl64[i].ch:cur_inmd->tbl[i].ch)
#define Max_tab_key_num (cur_inmd->key64 ? MAX_TAB_KEY_NUM64 : MAX_TAB_KEY_NUM)
void set_key_codes_label(char *s);

void lookup_gtab(char *ch, char out[])
{
  char *tbuf[32];
  int tbufN=0;

  if (!cur_inmd)
    return;

  int i;
  for(i=0; i < cur_inmd->DefChars; i++) {
    char *chi = tblch(i);

    if (!(chi[0] & 0x80))
      continue;
    if (!utf8_eq(chi, ch))
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
        len = (*keyname & 0x80) ? utf8_sz(keyname) : strlen(keyname);
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

  set_key_codes_label(out);
  void set_key_codes_label_pho(char *s);
  set_key_codes_label_pho(out);
}

void free_gtab()
{
  int i;

  for(i=0; i < MAX_GTAB_NUM_KEY; i++) {
    INMD *inp = &inmd[i];
    free(inp->tbl); inp->tbl = NULL;
    free(inp->tbl64); inp->tbl64 = NULL;
    free(inp->phridx); inp->phridx = NULL;
    free(inp->phrbuf); inp->phrbuf = NULL;
    free(inp->keyname_lookup); inp->keyname_lookup = NULL;
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
  int len = u8cpy(t, ch);
  t[len]=0;

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
void clear_gtab_input_error_color();


static void ClrIn()
{
  bzero(inch,sizeof(inch));
  bzero(seltab,sizeof(seltab));
  m_pg_mode=pg_idx=more_pg=wild_mode=wild_page=last_idx=defselN=exa_match=
  spc_pressed=ci=invalid_spc=0;

  sel1st_i=MAX_SELKEY-1;

  clear_gtab_in_area();
  last_idx = 0;

  if (gcin_pop_up_win && !same_query_show_pho_win())
    hide_win_gtab();

  clear_gtab_input_error_color();
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

char gtab_list[]="gtab.list";

void load_gtab_list()
{
  char ttt[128];
  FILE *fp;

  get_gcin_user_fname(gtab_list, ttt);

  if ((fp=fopen(ttt, "r"))==NULL) {
    get_sys_table_file_name(gtab_list, ttt);
    if ((fp=fopen(ttt, "r"))==NULL)
      p_err("cannot open %s", ttt);
  }

  dbg("load_gtab_list %s\n", ttt);

  while (!feof(fp)) {
    char line[256];
    char name[32];
    char key[32];
    char file[32];
    char icon[128];

    name[0]=0;
    key[0]=0;
    file[0]=0;
    icon[0]=0;

    fgets(line, sizeof(line), fp);

    sscanf(line, "%s %s %s %s", name, key, file, icon);
    if (strlen(name) < 1)
      break;
    if (name[0]=='#')
      continue;

    int keyidx = gcin_switch_keys_lookup(key[0]);
    if (keyidx < 0)
      p_err("bad key value %s in %s\n", key, ttt);

    free(inmd[keyidx].filename);
    inmd[keyidx].filename = strdup(file);

    free(inmd[keyidx].cname);
    inmd[keyidx].cname = strdup(name);

    free(inmd[keyidx].icon);
    if (strlen(icon))
      inmd[keyidx].icon = strdup(icon);
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

//  current_CS->b_half_full_char = FALSE;

  if (!inmd[inmdno].filename[0] || !strcmp(inmd[inmdno].filename,"-")) {
    dbg("filename is empty\n");
    return;
  }

  struct stat st;
  get_gcin_user_fname(inmd[inmdno].filename, ttt);
  int rval = stat(ttt, &st);

  if (rval < 0) {
    strcat(strcpy(ttt,TableDir),"/");
    strcat(ttt, inmd[inmdno].filename);

    int rval = stat(ttt, &st);

    if (rval < 0) {
      dbg("init_tab:1 err open %s\n", ttt);
      return;
    }
  }

  if (st.st_mtime == inp->file_modify_time) {
//    dbg("unchanged\n");
    set_gtab_input_method_name(inp->cname);
    cur_inmd=inp;

    if (gtab_space_auto_first == GTAB_space_auto_first_none)
      _gtab_space_auto_first = cur_inmd->space_style;
    else
      _gtab_space_auto_first = gtab_space_auto_first;

    return;    /* table is already loaded */
  }

  inp->file_modify_time = st.st_mtime;
#if 0
  if (strstr(ttt, ".bz2")) {
    char uu[256];
    strcat(strcpy(uu, "bunzip2 -c "), ttt);

    if ((fp=popen(uu, "r"))==NULL) {
      p_err("init_tab:2 err popen %s", uu);
    }
  } else
#endif
  {
    if ((fp=fopen(ttt, "r"))==NULL) {
      p_err("init_tab:2 err open %s", ttt);
    }
  }

  dbg("gtab file %s\n", ttt);


  strcpy(uuu,ttt);

  fread(&th,1,sizeof(th),fp);

  if (th.MaxPress > 5) {
    inp->max_keyN = 10;
    inp->key64 = TRUE;
    dbg("it's a 64-bit .gtab\n");
  } else {
    inp->max_keyN = 5;
  }

  free(inp->endkey);
  inp->endkey = strdup(th.endkey);

  fread(ttt, 1, th.KeyS, fp);
  dbg("KeyS %d\n", th.KeyS);

  fread(inp->keyname, CH_SZ, th.KeyS, fp);
#define WILD_QUES 64
#define WILD_STAR 65
  utf8cpy(&inp->keyname[WILD_QUES*CH_SZ], "？");  /* for wild card */
  utf8cpy(&inp->keyname[WILD_STAR*CH_SZ], "＊");

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
    free(inp->keyname_lookup);
    inp->keyname_lookup = malloc(sizeof(char) * MAX_GTAB_KEYS);
    memcpy(inp->keyname_lookup, keyname_lookup, MAX_GTAB_KEYS);
  }

  inp->KeyS=th.KeyS;
  inp->MaxPress=th.MaxPress;
  inp->DefChars=th.DefC;
  strcpy(inp->selkey,th.selkey);
  inp->M_DUP_SEL=th.M_DUP_SEL;
  inp->space_style=th.space_style;
  inp->flag=th.flag;

  dbg("MaxPress:%d  M_DUP_SEL:%d\n", th.MaxPress, th.M_DUP_SEL);

  free(inp->keymap);
  inp->keymap = tzmalloc(char, 128);

  for(i=0;i<th.KeyS;i++) {
    inp->keymap[(int)ttt[i]]=i;
    if (!BITON(inp->flag, FLAG_KEEP_KEY_CASE))
      inp->keymap[toupper(ttt[i])]=i;
    inp->keycol[i]=key_col(ttt[i]);
  }

  inp->keymap[(int)'?']=WILD_QUES;
  inp->keymap[(int)'*']=WILD_STAR;


  free(inp->idx1);
  inp->idx1 = tmalloc(gtab_idx1_t, th.KeyS+1);
  fread(inp->idx1, sizeof(gtab_idx1_t), th.KeyS+1, fp);
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

  if (gtab_space_auto_first == GTAB_space_auto_first_none)
    _gtab_space_auto_first = th.space_style;
  else
    _gtab_space_auto_first = gtab_space_auto_first;


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
  int plen = strlen(p);

  set_key_codes_label("");

  if (utf8_str_N(p) > 1  || p[0] < 128) {
    send_text(p);
  }
  else {
    char tt[512];

    if (same_pho_query_state == SAME_PHO_QUERY_gtab_input) {
      same_pho_query_state = SAME_PHO_QUERY_pho_select;
      start_gtab_pho_query(p);

      ClrIn();
      ClrSelArea();
      return;
    }

    if (gtab_disp_key_codes && !gtab_hide_row2 || wild_mode)
      lookup_gtab(p, tt);

    sendkey_b5(p);


    if (gtab_auto_select_by_phrase && !(_gtab_space_auto_first & GTAB_space_auto_first_any)) {
      if (part_matched_len < strlen(match_phrase) &&
          !memcmp(&match_phrase[part_matched_len], p, plen)) {
          part_matched_len+=plen;
#if DPHR
          dbg("inc\n");
#endif
      } else {
        memcpy(&match_phrase[part_matched_len], p, plen);
        match_phrase[part_matched_len + plen] = 0;
#if DPHR
        dbg("%d   zzz %s\n",part_matched_len, match_phrase);
#endif
        if (find_match(match_phrase, part_matched_len + plen, NULL, 0)) {
#if DPHR
          dbg("cat match_phrase %s\n", match_phrase);
#endif
          part_matched_len += plen;
        } else {
          strcpy(match_phrase, p);
          if (find_match(match_phrase, plen, NULL, 0)) {
#if DPHR
            dbg("single match_phrase %s\n", match_phrase);
#endif
            part_matched_len = plen;
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
}


static gboolean set_sel1st_i()
{
  if (!part_matched_len)
    return FALSE;

#define MAX_MATCH_STRS 1024

  char match_arr[CH_SZ * MAX_MATCH_STRS + 1];
  find_match(match_phrase, part_matched_len, match_arr, MAX_MATCH_STRS);

  int ofs = 0;
  char *pp = match_arr;

  while (*pp) {
    int j;

    for(j=1; j < MAX_SELKEY; j++) {
      if (!seltab[j][0])
        continue;
      int len = strlen(seltab[j]);

      if (utf8_str_N(seltab[j])!=1)
        continue;

      if (!memcmp(seltab[j], pp, len)) {
        sel1st_i = j;
        return TRUE;
      }
    }

    pp+=utf8_sz(pp);
  }

  return FALSE;
}


static void load_phr(int j, char *tt)
{
  int len;
  u_char *ch = tblch(j);
  int phrno =((int)(ch[0])<<16)|((int)ch[1]<<8)|ch[2];

  int ofs = cur_inmd->phridx[phrno], ofs1 = cur_inmd->phridx[phrno+1];

//  dbg("load_phr   j:%d %d %d %d\n", j, phrno, ofs, ofs1);
  len = ofs1 - ofs;

  if (len > 128 || len <= 0) {
    dbg("phrae error %d\n", len);
    strcpy(tt,"err");
    return;
  }

  memcpy(tt, cur_inmd->phrbuf + ofs, len);
  tt[len]=0;
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

static gboolean load_seltab(int tblidx, int seltabidx)
{
  u_char *tbl_ch = tblch(tblidx);
  if (tbl_ch[0] < 0x80) {
    load_phr(tblidx, seltab[seltabidx]);
    return TRUE;
  }

  int len = u8cpy(seltab[seltabidx], tbl_ch);
  seltab[seltabidx][len] = 0;

  return FALSE;
}

void set_gtab_input_error_color();
static void bell_err()
{
  bell();
  set_gtab_input_error_color();
}

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
    if (inch[i] == WILD_STAR) {
      regstr[regstrN++]='.';
      regstr[regstrN++]='*';
    } else
    if (inch[i] == WILD_QUES) {
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

  for(t=0; t< cur_inmd->DefChars && defselN < cur_inmd->M_DUP_SEL; t++) {
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

        load_seltab(t, defselN);

        strcat(tt, seltab[defselN]);
        defselN++;

        b1_cat(tt, ' ');
      } else
        wild_ofs++;

      found=1;
    }
  } /* for t */

  regfree(&reg);
  disp_gtab_sel(tt);

  if (!found) {
    bell_err();
  }
}

static char *ptr_selkey(KeySym key)
{
  if (key>= XK_KP_0 && key<= XK_KP_9)
    key-= XK_KP_0 - '0';
  return strchr(cur_inmd->selkey, key);
}


void init_gtab_pho_query_win();
int feedkey_pho(KeySym xkey, int state);

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
}

static void disp_selection(gboolean phrase_selected)
{
  ClrSelArea();

  char tt[1024];
  tt[0]=0;

  int ofs;

  if (exa_match && (_gtab_space_auto_first & GTAB_space_auto_first_any) && !more_pg) {
    strcat(tt, seltab[0]);
    strcat(tt, " ");
    ofs = 1;
  } else {
    ofs = 0;
  }

  int i;
  for(i=ofs; i< cur_inmd->M_DUP_SEL + ofs; i++) {
    if (seltab[i][0]) {
      b1_cat(tt, cur_inmd->selkey[i - ofs]);
//      dbg("yy %d '%s'\n", strlen(seltab[i]), seltab[i]);

      if (phrase_selected && i==sel1st_i) {
        strcat(tt, "<span foreground=\"blue\">");
        strcat(strcat(tt, seltab[i]), " ");
        strcat(tt, "</span>");
      } else {
        strcat(strcat(tt, seltab[i]), " ");
      }
    } else {
      extern gboolean b_use_full_space;

      if (gtab_disp_partial_match) {
         if (b_use_full_space)
            strcat(tt, " 　 ");
         else
            strcat(tt, "   ");
      }
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

gboolean full_char_proc(KeySym keysym);

gboolean shift_char_proc(KeySym key, int kbstate)
{
    char tt[2];

    if (key >= 127)
      return FALSE;

    if (kbstate & LockMask) {
      if (key >= 'a' && key <= 'z')
        key-=0x20;
    } else {
      if (key >= 'A' && key <= 'Z')
        key+=0x20;
    }


    if (current_CS->b_half_full_char)
      return full_char_proc(key);


    tt[0] = key;
    tt[1] = 0;
    send_text(tt);
    return TRUE;
}


gboolean feedkey_gtab(KeySym key, int kbstate)
{
  int i,j=0;
  static int s1,e1;
  int inkey;
  char *pselkey= NULL;
  gboolean phrase_selected = FALSE;
  char seltab_phrase[MAX_SELKEY];
  gboolean is_keypad = FALSE;

  bzero(seltab_phrase, sizeof(seltab_phrase));


//  dbg("uuuuu %x %x\n", key, kbstate);

  if (!cur_inmd)
    return 0;

  if (kbstate & (Mod1Mask|ControlMask)) {
      return 0;
  }


  if (same_pho_query_state == SAME_PHO_QUERY_pho_select)
    return feedkey_pho(key, 0);


  if (key < 127 && cur_inmd->keymap[key]) {
     if (key < 'A' || key > 'z' || key > 'Z'  && key < 'a' )
       goto shift_proc;
     char lcase = tolower(key);
     char ucase = toupper(key);
     if (cur_inmd->keymap[lcase] != cur_inmd->keymap[ucase])
       goto next;

     if (gtab_capslock_in_eng && (kbstate&LockMask))
       return 0;
  }


shift_proc:
  if ((kbstate & ShiftMask) && key!='*' && (key!='?' || gtab_shift_phrase_key && !ci)) {
    if (gtab_shift_phrase_key)
      return feed_phrase(key, kbstate);
    else
      return shift_char_proc(key, kbstate);
  }


  gboolean has_wild = FALSE;

  switch (key) {
    case XK_BackSpace:
      clear_phrase_match_buf();
      last_idx=0;
      spc_pressed=0;
      sel1st_i=MAX_SELKEY-1;
      clear_gtab_input_error_color();

      if (ci==0)
        return 0;

      if (ci>0)
        inch[--ci]=0;

      if (has_wild_card()) {
        proc_wild_disp();
        return 1;
      }


      wild_mode=0;
      invalid_spc = FALSE;
      if (ci==1 && cur_inmd->use_quick) {
        int i;

        bzero(seltab,sizeof(seltab));
        for(i=0;i<10;i++)
          utf8cpy(seltab[i], cur_inmd->qkeys.quick1[inch[0]-1][i]);

        defselN=10;
        DispInArea();
        goto Disp_opt;
      } else
      if (ci==2 && cur_inmd->use_quick) {
        int i;

        bzero(seltab,sizeof(seltab));
        for(i=0;i<10;i++)
          utf8cpy(seltab[i], cur_inmd->qkeys.quick2[inch[0]-1][inch[1]-1][i]);

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
      } else {
        ClrIn();
        return 0;
      }
    case '<':
      if (wild_mode) {
        if (wild_page >= cur_inmd->M_DUP_SEL) wild_page-=cur_inmd->M_DUP_SEL;
        wildcard();
        return 1;
      }
      return 0;
    case ' ':
      if (invalid_spc) {
        ClrIn();
      }

      has_wild = has_wild_card();

      if (wild_mode) {
        if (defselN == cur_inmd->M_DUP_SEL)
          wild_page+= cur_inmd->M_DUP_SEL;
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

        if (current_CS->b_half_full_char)
          return full_char_proc(key);

        return 0;
      } else
      if (!has_wild) {
//        dbg("iii %d  defselN:%d   %d\n", sel1st_i, defselN, cur_inmd->M_DUP_SEL);
        if (_gtab_space_auto_first == GTAB_space_auto_first_any && !more_pg && seltab[0][0] &&
            sel1st_i==MAX_SELKEY-1 && exa_match<=cur_inmd->M_DUP_SEL+1) {
          sel1st_i = 0;
        }

        if (_gtab_space_auto_first == GTAB_space_auto_first_nofull && exa_match > 1)
          bell();

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

        if (gcin_pop_up_win)
          show_win_gtab();

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
      if (!cur_inmd->keymap[key]) {
        same_pho_query_state = SAME_PHO_QUERY_gtab_input;
        if (gcin_pop_up_win)
          show_win_gtab();
        init_gtab_pho_query_win();
        return 1;
      }
    default:
next:

      if (invalid_spc) {
        ClrIn();
      }
      if (key>=XK_KP_0 && key<=XK_KP_9) {
        if (!ci)
          return FALSE;
        if (!strncmp(cur_inmd->filename, "dayi", 4)) {
          key = key - XK_KP_0 + '0';
          is_keypad = TRUE;
        }
        else
          return 0;
      }

      char *pendkey = strchr(cur_inmd->endkey, key);

      pselkey=ptr_selkey(key);


      if (!pselkey && (key < 32 || key > 0x7e) && (gtab_full_space_auto_first || spc_pressed)) {
//        dbg("%x %x sel1st_i:%d  '%c'\n", pselkey, key, sel1st_i, seltab[sel1st_i][0]);
        if (seltab[sel1st_i][0])
          putstr_inp(seltab[sel1st_i]);  /* select 1st */

        return 0;
      }


      inkey=cur_inmd->keymap[key];

//      dbg("spc_pressed %d %d %d\n", spc_pressed, last_full, cur_inmd->MaxPress);

#if 1 // for dayi, testcase :  6 space keypad6
      if (( (spc_pressed||last_full||is_keypad) ||(wild_mode && (!inkey ||pendkey))) && pselkey) {
        int vv = pselkey - cur_inmd->selkey;

        if ((_gtab_space_auto_first & GTAB_space_auto_first_any) && !wild_mode)
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
           (gtab_full_space_auto_first||spc_pressed||last_full) ) {
        putstr_inp(seltab[sel1st_i]);  /* select 1st */
      }

#if 0
      if (wild_mode)
        goto XXXX;
#endif
      if (key > 0x7f) {
        return 0;
      }

      spc_pressed=0;

      // for cj & boshiamy to input digits
      if (!ci && !inkey) {
        if (current_CS->b_half_full_char)
          return full_char_proc(key);
        else
          return 0;
      }


      if (wild_mode && inkey>=1 && ci< cur_inmd->MaxPress) {
        inch[ci++]=inkey;
        if (gcin_pop_up_win)
          show_win_gtab();
        proc_wild_disp();
        return 1;
      }

      if (inkey>=1 && ci< cur_inmd->MaxPress) {
        inch[ci++]=inkey;
        if (gcin_pop_up_win)
          show_win_gtab();
        last_full=0;

        if (cur_inmd->use_quick && !pendkey) {
          if (ci==1) {
            int i;
            for(i=0;i < cur_inmd->M_DUP_SEL; i++) {
              utf8cpy(seltab[i], &cur_inmd->qkeys.quick1[inkey-1][i]);
            }

            defselN=cur_inmd->M_DUP_SEL;
            DispInArea();
            goto Disp_opt;
          } else
          if (ci==2 && !pselkey) {
            int i;
            for(i=0;i < cur_inmd->M_DUP_SEL; i++) {
              utf8cpy(seltab[i], &cur_inmd->qkeys.quick2[inch[0]-1][inkey-1][i]);
            }

            defselN=cur_inmd->M_DUP_SEL;
            DispInArea();
            goto Disp_opt;
          }
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
      } else {
        if (!pselkey) {
          if (current_CS->b_half_full_char)
            return full_char_proc(key);
          else
            return 0;
        }

        if (defselN)
          goto YYYY;
     }
  } /* switch */


  if (ci==0) {
    ClrSelArea();
    ClrIn();
    return 1;
  }

  invalid_spc = FALSE;
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

  vmaskci = cur_inmd->key64 ? vmask64[ci]:vmask[ci];

  if ((CONVT2(cur_inmd, s1) & vmaskci)!=val || (wild_mode && defselN) ||
                  ((ci==cur_inmd->MaxPress||spc_pressed) && defselN &&
      (pselkey && ( pendkey || spc_pressed)) ) ) {
YYYY:
    if ((pselkey || wild_mode) && defselN) {
      int vv = pselkey - cur_inmd->selkey;

      if ((_gtab_space_auto_first & GTAB_space_auto_first_any) && !wild_mode &&
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

    if (gtab_invalid_key_in) {
      if (spc_pressed) {
        bell_err();
        invalid_spc = TRUE;
//        dbg("invalid_spc\n");
      }
      else {
        seltab[0][0]=0;
        ClrSelArea();
      }
    } else {
      if (gtab_dup_select_bell)
        bell();

      if (ci>0)
        inch[--ci]=0;
    }

    last_idx=0;
    DispInArea();
    return 1;
  }


refill:

  j=s1;

  if (ci < cur_inmd->MaxPress && !spc_pressed && !pendkey) {
    int shiftb=(KEY_N - 1 -ci) * KeyBits;

    exa_match=0;
    bzero(seltab, sizeof(seltab));
    while (CONVT2(cur_inmd, j)==val && exa_match <= cur_inmd->M_DUP_SEL) {
      seltab_phrase[exa_match] = load_seltab(j, exa_match);
      exa_match++;

      j++;
    }


    defselN=exa_match;

    if (defselN > cur_inmd->M_DUP_SEL)
      defselN--;

    if (gtab_disp_partial_match)
    while((CONVT2(cur_inmd, j) & vmask[ci])==val && j<e1) {
      int fff=cur_inmd->keycol[(CONVT2(cur_inmd, j)>>shiftb) & 0x3f];
      u_char *tbl_ch = tblch(j);

      if (!seltab[fff][0] || seltab_phrase[fff] ||
           (bchcmp(seltab[fff], tbl_ch)>0 && fff > exa_match)) {
#if 0
        if (tbl_ch[0] >= 0x80) {
          bchcpy(seltab[fff], tbl_ch);
          defselN++;
        }
        else
        if (!seltab[fff][0]) {
          load_phr(j, seltab[fff]);
          seltab_phrase[fff] = TRUE;
        }
#endif
        if (!(seltab_phrase[fff] = load_seltab(j, fff)))
          defselN++;
      }

      j++;
    }
  } else {
next_pg:
    defselN=more_pg=0;
    bzero(seltab,sizeof(seltab));

    if (pendkey)
      spc_pressed = 1;

    while(CONVT2(cur_inmd, j)==val && defselN<cur_inmd->M_DUP_SEL && j<e1) {
      load_seltab(j, defselN);

      j++; defselN++;

      if (ci == cur_inmd->MaxPress || spc_pressed) {
//        dbg("sel1st_i %d %d %d\n", ci, cur_inmd->MaxPress, spc_pressed);
        sel1st_i=0;

        if (gtab_auto_select_by_phrase && !(_gtab_space_auto_first & GTAB_space_auto_first_any))
          phrase_selected = set_sel1st_i();
      }
    }

    exa_match = defselN;

    if (j<e1 && CONVT2(cur_inmd, j)==val &&
        ((_gtab_space_auto_first & GTAB_space_auto_first_any) && defselN==cur_inmd->M_DUP_SEL+1 ||
         !(_gtab_space_auto_first & GTAB_space_auto_first_any) && defselN==cur_inmd->M_DUP_SEL)) {
      pg_idx=j;
      more_pg=1;
      m_pg_mode=1;
    } else if (m_pg_mode) {
      pg_idx=s1;
      more_pg=1;
    }

    if (ci==cur_inmd->MaxPress)
      last_full=1;

    if (defselN==1 && !more_pg) {
      if (spc_pressed || (gtab_press_full_auto_send && last_full)) {
        putstr_inp(seltab[0]);
        return 1;
      }
    } else
    if (!defselN) {
      bell_err();
      spc_pressed=0;

      if (gtab_invalid_key_in) {
        invalid_spc = TRUE;
        return TRUE;
      }
      goto refill;
    } else
    if (!more_pg) {
      if (gtab_dup_select_bell && (gtab_disp_partial_match || gtab_pre_select)) {
        if (spc_pressed || gtab_full_space_auto_first || last_full && gtab_press_full_auto_send)
          bell();
      }
    }
  }

Disp_opt:
  if (gtab_disp_partial_match || gtab_pre_select || (exa_match > 1 && (spc_pressed || gtab_press_full_auto_send)) ) {
       disp_selection(phrase_selected);
  }

  return 1;
}
