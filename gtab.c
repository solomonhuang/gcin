/*
	Copyright (C) 2004-2008	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/

#include <sys/stat.h>
#include <regex.h>
#include "gcin.h"
#include "gtab.h"
#include "pho.h"
#include "gcin-conf.h"
#include "gcin-endian.h"

typedef enum {
  SAME_PHO_QUERY_none = 0,
  SAME_PHO_QUERY_gtab_input = 1,
  SAME_PHO_QUERY_pho_select = 2,
} SAME_PHO_QUERY;

static GTAB_space_pressed_E _gtab_space_auto_first;
static SAME_PHO_QUERY same_pho_query_state = SAME_PHO_QUERY_none;
static int S1, E1;
extern char *TableDir;

INMD *cur_inmd;
static gboolean last_full, more_pg, wild_mode, spc_pressed, invalid_spc;
static char seltab[MAX_SELKEY][MAX_CIN_PHR];
static short defselN, exa_match;
static KeySym inch[MAX_TAB_KEY_NUM64_6];
static int ci;
static int last_idx, pg_idx;
static int wild_page;
static int sel1st_i = MAX_SELKEY - 1;
static int total_matchN;

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

  for(i=0; i < MAX_TAB_KEY_NUM64_6; i++)
    if (inch[i])
      return TRUE;

  if (same_query_show_pho_win())
    return TRUE;

  return FALSE;
}

#define tblch2(inm, i) (inm->key64 ? inm->tbl64[i].ch:inm->tbl[i].ch)
#define tblch(i) tblch2(cur_inmd, i)

static int load_phr_ch(INMD *inm, u_char *ch, char *tt)
{
  int phrno =((int)(ch[0])<<16)|((int)ch[1]<<8)|ch[2];
  int ofs = inm->phridx[phrno], ofs1 = inm->phridx[phrno+1];

//  dbg("load_phr   j:%d %d %d %d\n", j, phrno, ofs, ofs1);
  int len = ofs1 - ofs;

  if (len > MAX_CIN_PHR || len <= 0) {
    dbg("phrae error %d\n", len);
    strcpy(tt,"err");
    return 0;
  }

  memcpy(tt, inm->phrbuf + ofs, len);
  tt[len]=0;
  return len;
}

static void load_phr(int j, char *tt)
{
  u_char *ch = tblch(j);

  load_phr_ch(cur_inmd, ch, tt);
}

static int qcmp_strlen(const void *aa, const void *bb)
{
  char *a = *((char **)aa), *b = *((char **)bb);

  return strlen(a) - strlen(b);
}

#define Max_tab_key_num1(inm) (inm->key64 ? MAX_TAB_KEY_NUM64 : MAX_TAB_KEY_NUM)
#define Max_tab_key_num Max_tab_key_num1(cur_inmd)
void set_key_codes_label(char *s, int better);
void set_page_label(char *s);

static void clear_page_label()
{
  set_page_label("");
}

void lookup_gtabn(char *ch, char *out)
{
  char outbuf[512];
  char *tbuf[128];
  int tbufN=0;
  INMD *tinmd = &inmd[default_input_method];
  int n = utf8_str_N(ch);
  gboolean phrase = n > 1 || !(ch[0] & 0x80);

  if (!tinmd->DefChars)
    tinmd = cur_inmd;

  if (!tinmd)
    return;

  gboolean need_disp = FALSE;

  if (!out) {
    out = outbuf;
    need_disp = TRUE;
  }

  out[0]=0;

  int min_klen = 100;

  int i;
  for(i=0; i < tinmd->DefChars; i++) {
    char *chi = tblch2(tinmd, i);

    if (phrase) {
      if ((chi[0] & 0x80))
        continue;
      char tstr[512];
      load_phr_ch(tinmd, chi, tstr);
      if (strcmp(tstr, ch))
        continue;
    } else {
      if (!(chi[0] & 0x80))
        continue;
      if (!utf8_eq(chi, ch))
        continue;
    }

    u_int64_t key = CONVT2(tinmd, i);

    int j;

    int tlen=0, klen=0;
    char t[CH_SZ * 10 + 1];

    for(j=Max_tab_key_num1(tinmd) - 1; j>=0; j--) {

      int sh = j * KeyBits;
      int k = (key >> sh) & tinmd->kmask;

      if (!k)
        break;
      int len;
      char *keyname;

      if (tinmd->keyname_lookup) {
        len = 1;
        keyname = &tinmd->keyname_lookup[k];
      } else {
        keyname = &tinmd->keyname[k * CH_SZ];
        len = (*keyname & 0x80) ? utf8_sz(keyname) : strlen(keyname);
      }
//      dbg("uuuuuuuuuuuu %d %x len:%d\n", k, tinmd->keyname[k], len);
      memcpy(&t[tlen], keyname, len);
      tlen+=len;
      klen++;
    }

    if (klen < min_klen)
      min_klen = klen;

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

  if (!out[0] || !need_disp)
    return;


  set_key_codes_label(out, ci > min_klen);
  void set_key_codes_label_pho(char *s);
  set_key_codes_label_pho(out);
}

void lookup_gtab(char *ch)
{
  char tt[CH_SZ+1];
  utf8cpy(tt, ch);
  lookup_gtabn(tt, NULL);
}


void lookup_gtab_out(char *ch, char *out)
{
  char tt[CH_SZ+1];
  utf8cpy(tt, ch);
  lookup_gtabn(tt, out);
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

static clr_seltab()
{
  bzero(seltab,sizeof(seltab));
}

static void ClrIn()
{
  bzero(inch,sizeof(inch));
  clr_seltab();
  total_matchN=pg_idx=more_pg=wild_mode=wild_page=last_idx=defselN=exa_match=
  spc_pressed=ci=invalid_spc=0;

  sel1st_i=MAX_SELKEY-1;

  clear_gtab_in_area();
  last_idx = 0;

  if (gcin_pop_up_win && !same_query_show_pho_win())
    hide_win_gtab();

  clear_gtab_input_error_color();
  clear_page_label();
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
//    dbg("inch:%d ", inch[i]);
//    utf8_putchar(&cur_inmd->keyname[inch[i] * CH_SZ]);
//    puts("");
  }

  for(; i < cur_inmd->MaxPress; i++) {
    disp_gtab(i, "  ");
  }
}

void set_gtab_input_method_name(char *s);
void case_inverse(int *xkey, int shift_m);

time_t file_mtime(char *fname)
{
  struct stat st;

  if (stat(fname, &st) < 0)
    return 0;

  return st.st_mtime;
}


void init_gtab(int inmdno)
{
  FILE *fp;
  char ttt[128],uuu[128];
  int i;
  INMD *inp=&inmd[inmdno];
  struct TableHead th;

//  current_CS->b_half_full_char = FALSE;

  if (!inmd[inmdno].filename || !strcmp(inmd[inmdno].filename,"-")) {
    dbg("filename is empty\n");
    return;
  }

  get_gcin_user_fname(inmd[inmdno].filename, ttt);
  time_t mtime = file_mtime(ttt);

  if (!mtime) {
    strcat(strcpy(ttt,TableDir),"/");
    strcat(ttt, inmd[inmdno].filename);
    if (!(mtime = file_mtime(ttt))) {
      dbg("init_tab:1 err open %s\n", ttt);
      return;
    }
  }

  char append[64], append_user[128];
  strcat(strcpy(append, inmd[inmdno].filename), ".append");
  get_gcin_user_fname(append, append_user);
  time_t mtime_append = file_mtime(append_user);

  if (mtime_append) {
    char append_user_gtab[128];

    strcat(strcpy(append_user_gtab, append_user), ".gtab");
    time_t mtime_append_gtab = file_mtime(append_user_gtab);

    if (mtime_append_gtab < mtime || mtime_append_gtab < mtime_append) {
      char exe[256];

      sprintf(exe, GCIN_BIN_DIR"/gtab-merge %s %s %s", ttt, append_user, append_user_gtab);
      dbg("exe %s\n", exe);
#if 1
      system(exe);
#endif
      mtime_append_gtab = file_mtime(append_user_gtab);
    }

    if (mtime_append_gtab) {
      strcpy(ttt, append_user_gtab);
      mtime = mtime_append_gtab;
    }
  }

  if (mtime == inp->file_modify_time) {
//    dbg("unchanged\n");
    set_gtab_input_method_name(inp->cname);
    cur_inmd=inp;

    if (gtab_space_auto_first == GTAB_space_auto_first_none)
      _gtab_space_auto_first = cur_inmd->space_style;
    else
      _gtab_space_auto_first = gtab_space_auto_first;

    return;    /* table is already loaded */
  }

  inp->file_modify_time = mtime;

  if ((fp=fopen(ttt, "r"))==NULL)
    p_err("init_tab:2 err open %s", ttt);

  dbg("gtab file %s\n", ttt);


  strcpy(uuu,ttt);

  fread(&th,1,sizeof(th),fp);

  if (th.keybits<6 || th.keybits>7)
    th.keybits = 6;

  inp->keybits = th.keybits;
  dbg("keybits:%d\n", th.keybits);

#if NEED_SWAP
  swap_byte_4(&th.version);
  swap_byte_4(&th.flag);
  swap_byte_4(&th.space_style);
  swap_byte_4(&th.KeyS);
  swap_byte_4(&th.MaxPress);
  swap_byte_4(&th.M_DUP_SEL);
  swap_byte_4(&th.DefC);
#endif

  if (th.MaxPress*th.keybits > 32) {
    inp->max_keyN = 64 / th.keybits;
    inp->key64 = TRUE;
    dbg("it's a 64-bit .gtab\n");
  } else {
    inp->max_keyN = 32 / th.keybits;
  }

  free(inp->endkey);
  inp->endkey = strdup(th.endkey);

  if (th.flag & FLAG_GTAB_SYM_KBM)
    dbg("symbol kbm\n");

  fread(ttt, 1, th.KeyS, fp);
  dbg("KeyS %d\n", th.KeyS);

  if (inp->keyname)
    free(inp->keyname);
  inp->keyname = tmalloc(char, (th.KeyS + 3) * CH_SZ);
  fread(inp->keyname, CH_SZ, th.KeyS, fp);
  inp->WILD_QUES=th.KeyS+1;
  inp->WILD_STAR=th.KeyS+2;
  utf8cpy(&inp->keyname[inp->WILD_QUES*CH_SZ], "？");  /* for wild card */
  utf8cpy(&inp->keyname[inp->WILD_STAR*CH_SZ], "＊");

  char *keyname = &inp->keyname[1 * CH_SZ];


  // for boshiamy
  extern u_char *fullchar[];
  gboolean all_full_ascii = TRUE;
  char keyname_lookup[256];

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
    dbg("all_full_ascii\n");
    int mkeys = 1<< th.keybits;
    free(inp->keyname_lookup);
    inp->keyname_lookup = malloc(sizeof(char) * mkeys);
    memcpy(inp->keyname_lookup, keyname_lookup, mkeys);
  }

  inp->KeyS=th.KeyS;
  inp->MaxPress=th.MaxPress;
  inp->DefChars=th.DefC;
  strcpy(inp->selkey,th.selkey);
  inp->M_DUP_SEL=th.M_DUP_SEL;
  inp->space_style=th.space_style;
  inp->flag=th.flag;
  free(inp->cname);
  inp->cname = strdup(th.cname);

//  dbg("MaxPress:%d  M_DUP_SEL:%d\n", th.MaxPress, th.M_DUP_SEL);

  free(inp->keymap);
  inp->keymap = tzmalloc(char, 128);

  if (!(th.flag & FLAG_GTAB_SYM_KBM)) {
    inp->keymap[(int)'?']=inp->WILD_QUES;
    if (!strchr(th.selkey, '*'))
      inp->keymap[(int)'*']=inp->WILD_STAR;
  }

  free(inp->keycol);
  inp->keycol=tzmalloc(char, th.KeyS+1);
  for(i=0;i<th.KeyS;i++) {
    dbg("%c", ttt[i]);
    inp->keymap[(int)ttt[i]]=i;
//    dbg("%d %d %c\n", i, inp->keymap[(int)ttt[i]], ttt[i]);
    if (!BITON(inp->flag, FLAG_KEEP_KEY_CASE))
      inp->keymap[toupper(ttt[i])]=i;
    inp->keycol[i]=key_col(ttt[i]);
  }
  dbg("\n");

  free(inp->idx1);
  inp->idx1 = tmalloc(gtab_idx1_t, th.KeyS+1);
  fread(inp->idx1, sizeof(gtab_idx1_t), th.KeyS+1, fp);
#if NEED_SWAP
  for(i=0; i <= th.KeyS+1; i++)
    swap_byte_4(&inp->idx1[i]);
#endif
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
#if NEED_SWAP
    for(i=0; i < th.DefC; i++) {
      swap_byte_8(&inp->tbl64[i].key);
    }
#endif
  } else {
    if (inp->tbl) {
      dbg("free %x\n", inp->tbl);
      free(inp->tbl);
    }

    if ((inp->tbl=tmalloc(ITEM, th.DefC))==NULL) {
      p_err("malloc err");
    }

    fread(inp->tbl,sizeof(ITEM),th.DefC, fp);
#if NEED_SWAP
    for(i=0; i < th.DefC; i++) {
      swap_byte_4(&inp->tbl[i].key);
    }
#endif
  }

  dbg("chars %d\n", th.DefC);

  memcpy(&inp->qkeys, &th.qkeys, sizeof(th.qkeys));
  inp->use_quick= th.qkeys.quick1[1][0][0] > 0;  // only array 30 use this

  fread(&inp->phrnum, sizeof(int), 1, fp);
#if NEED_SWAP
    swap_byte_4(&inp->phrnum);
    for(i=0; i < inp->phrnum; i++) {
      swap_byte_4(&inp->phrnum);
    }
#endif
  dbg("inp->phrnum: %d\n", inp->phrnum);
  free(inp->phridx);
  inp->phridx = tmalloc(int, inp->phrnum);
  fread(inp->phridx, sizeof(int), inp->phrnum, fp);
#if NEED_SWAP
    for(i=0; i < inp->phrnum; i++) {
      swap_byte_4(&inp->phridx[i]);
    }
#endif

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

  cur_inmd=inp;
//    reset_inp();
  set_gtab_input_method_name(inp->cname);
  DispInArea();

  dbg("key64: %d\n", inp->key64);

  if (gtab_space_auto_first == GTAB_space_auto_first_none)
    _gtab_space_auto_first = th.space_style;
  else
    _gtab_space_auto_first = gtab_space_auto_first;

  inp->last_k_bitn = (((cur_inmd->key64 ? 64:32) / inp->keybits) - 1) * inp->keybits;
  inp->kmask = (1 << th.keybits) - 1;


#if 0
  for(i='A'; i < 127; i++)
    printf("%d] %c %d\n", i, i, inp->keymap[i]);
#endif
#if 0
  for(i=0; i < Min(100,th.DefC) ; i++) {
    u_char *ch = tblch(i);
    dbg("%d] %x %c%c%c\n", i, *((int *)inp->tbl[i].key), ch[0], ch[1], ch[2]);
  }
#endif
}

#if 0
static char match_phrase[MAX_PHRASE_STR_LEN];
static int part_matched_len;
#endif

void start_gtab_pho_query(char *utf8);

#if 0
static void clear_phrase_match_buf()
{
   part_matched_len = 0;
   match_phrase[0]=0;
}
#endif

static void putstr_inp(u_char *p)
{
  int plen = strlen(p);
  usecount_t usecount;

  clear_page_label();

  if (!wild_mode && gtab_hide_row2 || !gtab_disp_key_codes)
    set_key_codes_label(NULL, 0);

  char_play(p);

  int to_tsin = (cur_inmd->flag & FLAG_GTAB_SYM_KBM) && default_input_method==6;

  if (utf8_str_N(p) > 1  || p[0] < 128) {
    if (gtab_disp_key_codes && !gtab_hide_row2 || wild_mode)
      lookup_gtabn(p, NULL);
#if USE_TSIN
    if (to_tsin) {
      add_to_tsin_buf_str(p);
    }
    else
#endif
      send_text(p);
  }
  else {
    if (same_pho_query_state == SAME_PHO_QUERY_gtab_input) {
      same_pho_query_state = SAME_PHO_QUERY_pho_select;
      start_gtab_pho_query(p);

      ClrIn();
      ClrSelArea();
      return;
    }

    if (gtab_disp_key_codes && !gtab_hide_row2 || wild_mode)
      lookup_gtab(p);

    if (to_tsin)
      add_to_tsin_buf_str(p);
    else
      sendkey_b5(p);

#if 0
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
        if (find_match(match_phrase, part_matched_len + plen, NULL, 0, &usecount)) {
#if DPHR
          dbg("cat match_phrase %s\n", match_phrase);
#endif
          part_matched_len += plen;
        } else {
          strcpy(match_phrase, p);
          if (find_match(match_phrase, plen, NULL, 0, &usecount)) {
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
#endif

  }

  ClrIn();
  ClrSelArea();

  if ((cur_inmd->flag & FLAG_GTAB_SYM_KBM)) {
    extern int win_kbm_inited, b_show_win_kbm;
    init_in_method(default_input_method);
    if (win_kbm_inited && !b_show_win_kbm)
      hide_win_kbm();
  }
}


static gboolean set_sel1st_i()
{
#if 0
  if (!part_matched_len)
    return FALSE;
  usecount_t usecount;

#define MAX_MATCH_STRS 1024

  char match_arr[CH_SZ * MAX_MATCH_STRS + 1];
  find_match(match_phrase, part_matched_len, match_arr, MAX_MATCH_STRS, &usecount);

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
#else
  return FALSE;
#endif
}




#define swap(a,b) { tt=a; a=b; b=tt; }

static u_int vmask[]=
{ 0,
 (0x3f<<24),
 (0x3f<<24)|(0x3f<<18),
 (0x3f<<24)|(0x3f<<18)|(0x3f<<12),
 (0x3f<<24)|(0x3f<<18)|(0x3f<<12)|(0x3f<<6),
 (0x3f<<24)|(0x3f<<18)|(0x3f<<12)|(0x3f<<6)|0x3f
};


static u_int vmask_7[]=
{ 0,
 (0x7f<<21),
 (0x7f<<21)|(0x7f<<14),
 (0x7f<<21)|(0x7f<<14)|(0x7f<<7),
 (0x7f<<21)|(0x7f<<14)|(0x7f<<7)|0x7f,
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


#define KKK7 ((u_int64_t)0x7f)

static u_int64_t vmask64_7[]=
{ 0,
 (KKK7<<56),
 (KKK7<<56)|(KKK7<<49),
 (KKK7<<56)|(KKK7<<49)|(KKK7<<42),
 (KKK7<<56)|(KKK7<<49)|(KKK7<<42)|(KKK7<<35),
 (KKK7<<56)|(KKK7<<49)|(KKK7<<42)|(KKK7<<35)|(KKK7<<28),
 (KKK7<<56)|(KKK7<<49)|(KKK7<<42)|(KKK7<<35)|(KKK7<<28)|(KKK7<<21),
 (KKK7<<56)|(KKK7<<49)|(KKK7<<42)|(KKK7<<35)|(KKK7<<28)|(KKK7<<21)|(KKK7<<14),
 (KKK7<<56)|(KKK7<<49)|(KKK7<<42)|(KKK7<<35)|(KKK7<<28)|(KKK7<<21)|(KKK7<<14)|(KKK7<<7),
 (KKK7<<56)|(KKK7<<49)|(KKK7<<42)|(KKK7<<35)|(KKK7<<28)|(KKK7<<21)|(KKK7<<14)|(KKK7<<7)|KKK7,
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

gboolean cmp_inmd_idx(regex_t *reg, int idx)
{
  u_int64_t kk=CONVT2(cur_inmd, idx);
  char ts[32];
  int tsN=0;

  ts[tsN++]= ' ';

  int i;
  for(i=0; i < KEY_N; i++) {
    char c = (kk >> (LAST_K_bitN - i*cur_inmd->keybits)) & cur_inmd->kmask;
    if (!c)
      break;
    ts[tsN++] = c + '0';
  }

  ts[tsN++]= ' ';
  ts[tsN]=0;

  return regexec(reg, ts, 0, 0, 0);
}

static int page_len()
{
  return (_gtab_space_auto_first & GTAB_space_auto_first_any) ?
  cur_inmd->M_DUP_SEL+1:cur_inmd->M_DUP_SEL;
}

static void page_no_str(char tstr[])
{
  if (wild_mode) {
    int pgN = (total_matchN + cur_inmd->M_DUP_SEL - 1) / cur_inmd->M_DUP_SEL;
    if (pgN < 2)
      return;
    sprintf(tstr, "%d/%d", wild_page /cur_inmd->M_DUP_SEL + 1, pgN);
  } else {
    int pgN = (E1 - S1 + page_len() - 1) /page_len();

    if (pgN < 2)
      return;

    sprintf(tstr, "%d/%d", (pg_idx - S1)/page_len()+1, pgN);
  }
}

char *htmlspecialchars(char *s, char out[])
{
  struct {
    char c;
    char *str;
  } chs[]= {{'>',"gt"}, {'<',"lt"}, {'&',"amp"}};
  int chsN=sizeof(chs)/sizeof(chs[0]);

  int outn=0;
  while (*s) {
    int sz = utf8_sz(s);
    int i;
    for(i=0; i<chsN; i++)
      if (chs[i].c==*s)
        break;
    if (i==chsN) {
      memcpy(&out[outn],s, sz);
      outn+=sz;
      s+=sz;
    }
    else {
      out[outn++]='&';
      int len=strlen(chs[i].str);
      memcpy(&out[outn], chs[i].str, len);
      outn+=len;
      out[outn++]=';';
      s++;
    }
  }

  out[outn]=0;
  return out;
}

static void disp_selection(gboolean phrase_selected)
{
  char pgstr[32];
  pgstr[0]=0;
  page_no_str(pgstr);

  if (!gtab_vertical_select) {
    if (more_pg)
      set_page_label(pgstr);
    else
      clear_page_label();
  }


  char tt[(MAX_CIN_PHR + 4) * MAX_SELKEY + 80];
  tt[0]=0;
  char uu[MAX_CIN_PHR];

  int ofs;
  if (!wild_mode && exa_match && (_gtab_space_auto_first & GTAB_space_auto_first_any)) {
    strcat(tt, htmlspecialchars(seltab[0], uu));
    if (gtab_vertical_select)
      strcat(tt, "\n");
    else
      strcat(tt, " ");
    ofs = 1;
  } else {
    ofs = 0;
  }

  int i;
  for(i=ofs; i< cur_inmd->M_DUP_SEL + ofs; i++) {
    if (seltab[i][0]) {
      char selback[MAX_CIN_PHR+16];
      htmlspecialchars(seltab[i], selback);

      utf8cpy(uu, &cur_inmd->selkey[i - ofs]);
      char vvv[16];
      char www[512];
      sprintf(www, "<span foreground=\"%s\">%s</span>", gcin_sel_key_color, htmlspecialchars(uu, vvv));
      strcat(tt, www);

      if (gtab_vertical_select)
        strcat(tt, ". ");

      if (phrase_selected && i==sel1st_i) {
        strcat(tt, "<span foreground=\"red\">");
        strcat(strcat(tt, selback), " ");
        strcat(tt, "</span>");
      } else {
        char uu[MAX_CIN_PHR];

        if (gtab_vertical_select) {
          utf8cpy_bytes(uu, selback, 60);
          strcat(tt, uu);
        } else {
          char *p = selback;
          static char *skip[]={"http://", "ftp://", "https://", NULL};

          int j;
          for(j=0; skip[j]; j++)
            if (!strncmp(seltab[i], skip[j], strlen(skip[j]))) {
              p+=strlen(skip[j]);
              break;
            }

          utf8cpy_bytes(uu, p, 6 * 3);
          strcat(strcat(tt, uu), " ");
        }
      }

      if (gtab_vertical_select)
        strcat(tt, "\n");
    } else {
      extern gboolean b_use_full_space;

      if (!gtab_vertical_select && gtab_disp_partial_match) {
         if (b_use_full_space)
            strcat(tt, " 　 ");
         else
            strcat(tt, "   ");
      }
    }
  }

  if (gtab_vertical_select && pgstr[0]) {
    char tstr2[16];
    sprintf(tstr2, "(%s)", pgstr);
    strcat(tt, tstr2);
  }

  int len = strlen(tt);
  if (len && tt[len-1] == '\n')
    tt[len-1] = 0;

  if (gtab_pre_select || wild_mode || spc_pressed || last_full) {
    disp_gtab_sel(tt);
  }
}


void wildcard()
{
  int i,t, wild_ofs=0;
  int found=0;
  regex_t reg;

  ClrSelArea();
  clr_seltab();
  /* printf("wild %d %d %d %d\n", inch[0], inch[1], inch[2], inch[3]); */
  defselN=0;
  char regstr[32];
  int regstrN=0;

  regstr[regstrN++]=' ';

  for(i=0; i < KEY_N; i++) {
    if (!inch[i])
      break;
    if (inch[i] == cur_inmd->WILD_STAR) {
      regstr[regstrN++]='.';
      regstr[regstrN++]='*';
    } else
    if (inch[i] == cur_inmd->WILD_QUES) {
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

  for(t=0; t< cur_inmd->DefChars && defselN < cur_inmd->M_DUP_SEL; t++) {
    if (cmp_inmd_idx(&reg, t))
      continue;

    if (wild_ofs >= wild_page) {
      load_seltab(t, defselN);
      defselN++;
    } else
      wild_ofs++;

    found=1;
  } /* for t */


  if (!found) {
    bell_err();
  } else
  if (!wild_page) {
    total_matchN = 0;

    for(t=0; t< cur_inmd->DefChars; t++)
      if (!cmp_inmd_idx(&reg, t))
        total_matchN++;

  }

  if (total_matchN > cur_inmd->M_DUP_SEL)
    more_pg = 1;

  regfree(&reg);
  disp_selection(FALSE);
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


static gboolean has_wild_card()
{
  int i;

  for(i=0; i < cur_inmd->MaxPress; i++)
    if (inch[i]>= cur_inmd->WILD_QUES) {
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

    send_ascii(key);
    return TRUE;
}


gboolean feedkey_gtab(KeySym key, int kbstate)
{
  int i,j=0;
  int inkey;
  char *pselkey= NULL;
  gboolean phrase_selected = FALSE;
  char seltab_phrase[MAX_SELKEY];
  gboolean is_keypad = FALSE;
  gboolean shift_m = (kbstate & ShiftMask) > 0;

  bzero(seltab_phrase, sizeof(seltab_phrase));

//  dbg("uuuuu %x %x\n", key, kbstate);

  if (!cur_inmd)
    return 0;

  if (kbstate & (Mod1Mask|ControlMask)) {
    return 0;
  }


  if (same_pho_query_state == SAME_PHO_QUERY_pho_select)
    return feedkey_pho(key, 0);


  if (gtab_capslock_in_eng && (kbstate&LockMask)) {
    if (key < 0x20 || key>=0x7f)
      return 0;

    if (gcin_capslock_lower) {
      case_inverse(&key, shift_m);
      send_ascii(key);
      return 1;
    } else
      return 0;
  }


  int lcase = tolower(key);
  int ucase = toupper(key);
  if (key < 127 && cur_inmd->keymap[key]) {
     if (key < 'A' || key > 'z' || key > 'Z'  && key < 'a' )
       goto shift_proc;
     if (cur_inmd->keymap[lcase] != cur_inmd->keymap[ucase])
       goto next;

  }


shift_proc:
  if (shift_m && !strchr(cur_inmd->selkey, key) && !more_pg &&
       key!='*' && (key!='?' || gtab_shift_phrase_key && !ci)) {
    if (gtab_shift_phrase_key)
      return feed_phrase(key, kbstate);
    else {
      if (!cur_inmd->keymap[key] || (lcase != ucase &&
           cur_inmd->keymap[lcase]==cur_inmd->keymap[ucase]))
        return shift_char_proc(key, kbstate);
    }
  }


  gboolean has_wild = FALSE;

  switch (key) {
    case XK_BackSpace:
#if 0
      clear_phrase_match_buf();
#endif
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
        clr_seltab();
        for(i=0;i<cur_inmd->M_DUP_SEL;i++)
          utf8cpy(seltab[i], cur_inmd->qkeys.quick1[inch[0]-1][i]);

        defselN=cur_inmd->M_DUP_SEL;
        DispInArea();
        goto Disp_opt;
      } else
      if (ci==2 && cur_inmd->use_quick) {
        int i;
        clr_seltab();
        for(i=0;i<cur_inmd->M_DUP_SEL;i++)
          utf8cpy(seltab[i], cur_inmd->qkeys.quick2[inch[0]-1][inch[1]-1][i]);

        defselN=cur_inmd->M_DUP_SEL;
        DispInArea();
        goto Disp_opt;
      }

      break;
    case XK_Return:
#if 0
      clear_phrase_match_buf();
#endif
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
    case XK_Prior:
    case XK_KP_Subtract:
      if (wild_mode) {
        if (wild_page >= cur_inmd->M_DUP_SEL) wild_page-=cur_inmd->M_DUP_SEL;
        wildcard();
        return 1;
      } else
      if (more_pg) {
        pg_idx -= page_len();
        if (pg_idx < S1)
          pg_idx = S1;

        goto next_pg;
      }
      return 0;
    case XK_Next:
    case XK_KP_Add:
      if (more_pg) {
next_page:
        pg_idx += page_len();
        if (pg_idx >=E1)
          pg_idx = S1;
        goto next_pg;
      } else
        return 0;
    case ' ':
      if (invalid_spc && gtab_invalid_key_in)
        ClrIn();

      if (!gtab_invalid_key_in && spc_pressed && invalid_spc) {
        ClrIn();
        return 1;
      }

      has_wild = has_wild_card();

//      dbg("wild_mode:%d more_pg:%d ci:%d  has_wild:%d\n", wild_mode, more_pg, ci, has_wild);

      if (wild_mode) {
        // request from tetralet
        if (!wild_page && total_matchN < cur_inmd->M_DUP_SEL) {
          sel1st_i = 0;
          goto direct_select;
        }

        wild_page += cur_inmd->M_DUP_SEL;
        if (wild_page >= total_matchN)
          wild_page=0;

        wildcard();
        return 1;
      } else
      if (more_pg && !(_gtab_space_auto_first & GTAB_space_auto_first_any)) {
        goto next_page;
      } else
      if (ci==0) {
        if (current_CS->b_half_full_char)
          return full_char_proc(key);

        return 0;
      } else
      if (!has_wild) {
//        dbg("iii %d  defselN:%d   %d\n", sel1st_i, defselN, cur_inmd->M_DUP_SEL);
        if (_gtab_space_auto_first == GTAB_space_auto_first_any && seltab[0][0] &&
            sel1st_i==MAX_SELKEY-1) {
          sel1st_i = 0;
        }

        if (_gtab_space_auto_first == GTAB_space_auto_first_nofull && exa_match > 1)
          bell();

        if (seltab[sel1st_i][0]) {
//          dbg("last_full %d %d\n", last_full,spc_pressed);
          if (gtab_full_space_auto_first || spc_pressed) {
direct_select:
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
      if (!gtab_que_wild_card)
        return 0;
    case '*':
      inkey=cur_inmd->keymap[key];
      if ((inkey && (inkey!=cur_inmd->WILD_STAR && inkey!=cur_inmd->WILD_QUES)) || ptr_selkey(key)) {
//        dbg("%d %d\n", inkey, cur_inmd->WILD_STAR);
        goto next;
      }
      if (ci< cur_inmd->MaxPress) {
        inch[ci++]=inkey;
        DispInArea();

        if (gcin_pop_up_win)
          show_win_gtab();

        total_matchN = 0;
        wild_page=0;
        wild_mode=1;
        wildcard();
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
      clear_gtab_input_error_color();

      if (invalid_spc && gtab_invalid_key_in) {
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
        for(i=0; i < MAX_TAB_KEY_NUM64_6; i++)
          if (inch[i]>=cur_inmd->WILD_QUES) {
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

        if (defselN) {
          goto YYYY;
        }
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

#if 1
  if (last_idx)
    S1=last_idx;
  else
#endif
    S1=cur_inmd->idx1[inch[0]];

//  dbg("--------- ch:%d %d val %llx  S1:%d\n", inch[0], Max_tab_key_num, val, S1);

  int oE1=cur_inmd->idx1[inch[0]+1];
  u_int64_t vmaskci;
  if (cur_inmd->keybits==6)
    vmaskci = cur_inmd->key64 ? vmask64[ci]:vmask[ci];
  else
    vmaskci = cur_inmd->key64 ? vmask64_7[ci]:vmask_7[ci];

  while ((CONVT2(cur_inmd, S1) & vmaskci) != val &&
          CONVT2(cur_inmd, S1) < val &&  S1<oE1)
    S1++;

  pg_idx=last_idx=S1;

#if 0
  dbg("MaxPress:%d vmaskci:%x ci:%d  !=%d\n", cur_inmd->MaxPress, vmaskci, ci,
  ((CONVT2(cur_inmd, S1) & vmaskci)!=val));
#endif

  if ((CONVT2(cur_inmd, S1) & vmaskci)!=val || (wild_mode && defselN) ||
                  ((ci==cur_inmd->MaxPress||spc_pressed) && defselN &&
      (pselkey && ( pendkey || spc_pressed)) ) ) {
YYYY:

    if ((pselkey || wild_mode) && defselN) {
      int vv = pselkey - cur_inmd->selkey;

      if ((_gtab_space_auto_first & GTAB_space_auto_first_any) && !wild_mode
          && exa_match && (!cur_inmd->use_quick || ci!=2))
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
      } else {
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

  j=S1;
  while(CONVT2(cur_inmd, j)==val && j<oE1)
    j++;

  E1 = j;
  total_matchN = E1 - S1;
  pg_idx = S1;

  more_pg = 0;
  if (total_matchN > page_len()) {
    if ((_gtab_space_auto_first & GTAB_space_auto_first_any) || spc_pressed || pendkey ||
      ci==cur_inmd->MaxPress && (_gtab_space_auto_first & GTAB_space_auto_first_full))
      more_pg = 1;
  }


  if (ci < cur_inmd->MaxPress && !spc_pressed && !pendkey && !more_pg) {
    j = S1;
    exa_match=0;
    clr_seltab();
    int match_cnt=0;

    while (CONVT2(cur_inmd, j)==val && exa_match <= page_len()) {
      seltab_phrase[exa_match] = load_seltab(j, exa_match);
      match_cnt++;
      exa_match++;
      j++;
    }

    defselN=exa_match;

    if (defselN > page_len())
      defselN--;

    int shiftb=(KEY_N - 1 -ci) * KeyBits;

//    if (gtab_disp_partial_match)
    while((CONVT2(cur_inmd, j) & vmaskci)==val && j<oE1) {
      int fff=cur_inmd->keycol[(CONVT2(cur_inmd, j)>>shiftb) & cur_inmd->kmask];
      u_char *tbl_ch = tblch(j);

      if (gtab_disp_partial_match && (!seltab[fff][0] || seltab_phrase[fff] ||
           (bchcmp(seltab[fff], tbl_ch)>0 && fff > exa_match))) {
        seltab_phrase[fff] = load_seltab(j, fff);
        defselN++;
      }

      match_cnt++;
#if 0
      dbg("jj %d", fff); utf8_putchar(seltab[fff]); dbg("\n");
#endif
      j++;
    }

    if (gtab_unique_auto_send) {
      char *first_str=NULL, nonemptyN=0;
      for(i=0; i < page_len(); i++) {
        if (!seltab[i][0])
          continue;
        if (!first_str)
          first_str = seltab[i];
      }

      if (match_cnt==1 && first_str) {
        putstr_inp(first_str);
        return 1;
      }
    }
  } else {
//    dbg("more %d %d\n", more_pg,  total_matchN);
next_pg:
    defselN=0;
    clr_seltab();
    if (pendkey)
      spc_pressed = 1;


    if (gtab_edit_buffer) {
      insert_gbuf_cursor(S1, E1);
    } else {
      j = pg_idx;
      while(j<E1 && CONVT2(cur_inmd, j)==val && defselN < page_len()) {
        load_seltab(j, defselN);

        j++; defselN++;

        if (ci == cur_inmd->MaxPress || spc_pressed) {
  //        dbg("sel1st_i %d %d %d\n", ci, cur_inmd->MaxPress, spc_pressed);
          sel1st_i=0;

          if (gtab_auto_select_by_phrase && !(_gtab_space_auto_first & GTAB_space_auto_first_any))
            phrase_selected = set_sel1st_i();
        }
      }
    }

    exa_match = defselN;

    if (ci==cur_inmd->MaxPress)
      last_full=1;

    if (defselN==1 && !more_pg) {
      if (spc_pressed || (gtab_press_full_auto_send && last_full) || gtab_unique_auto_send) {
        putstr_inp(seltab[0]);
        return 1;
      }
    } else
    if (!defselN) {
      bell_err();
//      spc_pressed=0;

//      if (gtab_invalid_key_in)
      {
        invalid_spc = TRUE;
        return TRUE;
      }

      return TRUE;
    } else
    if (!more_pg) {
      if (gtab_dup_select_bell && (gtab_disp_partial_match || gtab_pre_select)) {
        if (spc_pressed || gtab_full_space_auto_first || last_full && gtab_press_full_auto_send)
          bell();
      }
    }
  }

Disp_opt:
  if (gtab_disp_partial_match || gtab_pre_select || ((exa_match > 1 || more_pg) &&
    (spc_pressed || gtab_press_full_auto_send ||
    (ci==cur_inmd->MaxPress && (_gtab_space_auto_first & GTAB_space_auto_first_full))) ) ) {
       disp_selection(phrase_selected);
  }

  return 1;
}
