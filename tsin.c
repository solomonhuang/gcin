/*
	Copyright (C) 2004	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/
#include <string.h>

#include "gcin.h"
#include "pho.h"
#include "tsin.h"
#include "gcin-conf.h"

extern gboolean b_hsu_kbm;

extern PHO_ITEM *ch_pho;
extern char *pho_chars[];

extern int ityp3_pho;
extern u_char typ_pho[];
extern char inph[];

extern u_short idxnum_pho;
extern PHO_IDX idx_pho[];
extern u_short hash_pho[];
extern PHOKBM phkbm;

extern char tsidxfname[64];
extern int hashidx[TSIN_HASH_N];

typedef struct {
  phokey_t pho;
  char ch[CH_SZ];
  char och[CH_SZ];
  char ph1ch[CH_SZ]; // char selected by 1st pho
  u_char flag;
  char psta; // phrase start index
} CHPHO;

enum {
  FLAG_CHPHO_FIXED=1,    // user selected the char, so it should not be change
  FLAG_CHPHO_PHRASE_HEAD=2,
  FLAG_CHPHO_PHRASE_VOID=4
};

static CHPHO chpho[MAX_PH_BF_EXT];
static int c_idx, c_len, ph_sta=-1, ph_sta_last=-1;
static int sel_pho;
static gboolean eng_ph=TRUE;
static int save_frm, save_to;
static int current_page;
static int startf;
static gboolean full_match;
static gboolean tsin_half_full;

static struct {
  phokey_t phokey[MAX_PHRASE_LEN];
  int phidx;
  char str[MAX_PHRASE_LEN*CH_SZ+1];
  int len;
} pre_sel[10];
int pre_selN;

gboolean save_phrase_to_db2(CHPHO *chph, int len);

void disp_char(int index, u_char *ch);

static void disp_char_chbuf(int idx)
{
  disp_char(idx, chpho[idx].ch);
}

static void init_chpho_i(int i)
{
  chpho[i].ch[0]=' ';
  chpho[i].ch[1]=0;
  chpho[i].flag=0;
  chpho[i].psta=-1;
}

void clr_tsin_cursor(int index);

static void clrcursor()
{
  clr_tsin_cursor(c_idx);
}

static int last_cursor_idx=0;
void set_cursor_tsin(int index);

static void drawcursor()
{
  clr_tsin_cursor(last_cursor_idx);
  last_cursor_idx = c_idx;

  if (c_idx == c_len) {
    if (tsin_half_full || eng_ph) {
      disp_char(c_idx,"  ");
      set_cursor_tsin(c_idx);
    } else {
      disp_char(c_idx, " ");
      set_cursor_tsin(c_idx);
    }
  }
  else {
    set_cursor_tsin(c_idx);
  }

//  gdk_flush();
}

static void chpho_extract(CHPHO *chph, int len, phokey_t *pho, char *ch, char *och)
{
   int i;
   int ofs=0, oofs=0;

   for(i=0; i < len; i++) {
      pho[i] = chph[i].pho;
      int u8len = u8cpy(&ch[ofs], chph[i].ch);
      ofs+=u8len;

      if (och) {
        int ou8len = u8cpy(&och[oofs], chph[i].och);
        oofs+=ou8len;
      }
   }
}


void inc_pho_count(phokey_t key, int ch_idx);
int ch_key_to_ch_pho_idx(phokey_t phkey, char *big5);
void inc_dec_tsin_use_count(phokey_t *pho, char *ch, int prlen, int N);

static void putbuf(int len)
{
  u_char tt[CH_SZ * (MAX_PH_BF_EXT+1) + 1];
  int i,idx;


#if 1
  // update phrase reference count
  if (len >= 2) {
    for(i=0; i < len; i++) {
      if (!BITON(chpho[i].flag, FLAG_CHPHO_PHRASE_HEAD)) {
        continue;
      }

      int j;
      for(j=i+1; j < len; j++)
        if (chpho[j].psta != i)
          break;

      int phrlen = j - i;
#if 0
      dbg("phrlen %d ", phrlen);
      int k;
      for(k=i; k < j; k++)
        utf8_putchar(chpho[k].ch);
      puts("");
      for(k=i; k < j; k++)
        utf8_putchar(chpho[k].och);
      puts("");
#endif

      if (phrlen < 2)
        continue;

      phokey_t pho[MAX_PHRASE_LEN];
      char ch[MAX_PHRASE_LEN * CH_SZ];
      char och[MAX_PHRASE_LEN * CH_SZ];

      chpho_extract(&chpho[i], phrlen, pho, ch, och);

      if (chpho[i].flag & FLAG_CHPHO_PHRASE_VOID)
        inc_dec_tsin_use_count(pho, och, phrlen, TRUE);
      else
        inc_dec_tsin_use_count(pho, ch, phrlen, FALSE);
    }
  }
#endif

  for(idx=i=0;i<len;i++) {
    int len = utf8_sz(chpho[i].ch);

    if (chpho[i].pho) {
      int pho_idx = ch_key_to_ch_pho_idx(chpho[i].pho, chpho[i].ch);
      if (pho_idx >= 0)
        inc_pho_count(chpho[i].pho, pho_idx);
    }

    memcpy(&tt[idx], chpho[i].ch, len);
    idx += len;
  }

  tt[idx]=0;
  send_text(tt);
}


void hide_char(int index);

static void prbuf()
{
  int i;

//  dbg("prbuf\n");

  for(i=0; i < c_len; i++) {
    disp_char_chbuf(i);
  }

  if (c_len < MAX_PH_BF_EXT)
    hide_char(c_len);

  if (c_len+1 < MAX_PH_BF_EXT)
    hide_char(c_len+1);

  drawcursor();
}


static int orig_disp_in_area_x = -1;
void disp_tsin_pho(int index, char *pho);

static void disp_in_area_pho_tsin()
{
  int i;

  for(i=0;i<4;i++) {
    disp_tsin_pho(i, &pho_chars[i][typ_pho[i] * CH_SZ]);
  }
}


void clrin_pho();
void clear_chars_all();

static void restore_ai()
{
  if (sel_pho)
    return;

  clear_chars_all();
  clrin_pho();
  disp_in_area_pho_tsin();
  prbuf();
}

static void clear_disp_ph_sta();
static void clear_match()
{
  ph_sta=-1;
  clear_disp_ph_sta();
//  bzero(psta, sizeof(psta));
//  dbg("clear_match\n");
}

static void clr_ch_buf()
{
  int i;
  for(i=0; i < MAX_PH_BF_EXT; i++) {
    init_chpho_i(i);
  }

  clear_match();
}


void disp_tsin_eng_pho(int eng_pho);

void show_stat()
{
  disp_tsin_eng_pho(eng_ph);
}

void load_tsin_db();

void load_tsin_entry(int idx, u_char *len, char *usecount, phokey_t *pho,
                    u_char *ch);

#if 0
void nputs(u_char *s, u_char len)
{
  char tt[16];

  memcpy(tt, s, len*CH_SZ);
  tt[len*CH_SZ]=0;
  dbg("%s", tt);
}


static void dump_tsidx(int i)
{
  phokey_t pho[MAX_PHRASE_LEN];
  u_char ch[MAX_PHRASE_LEN*CH_SZ];
  char usecount;
  u_char len;

  load_tsin_entry(i, &len, &usecount, pho, ch);

  int j;
  for(j=0; j < len; j++) {
    prph(pho[j]);
    dbg(" ");
  }

  nputs(ch, len);
  dbg("\n");
}


static void dump_tsidx_all()
{
  int i;

  for(i=0; i < phcount; i++) {
    dump_tsidx(i);
  }

  dbg("************************************************\n");
  for(i=0; i < 254; i++) {
    dbg("%d]%d ", i, hashidx[i]);
    dump_tsidx(hashidx[i]);
  }
}

#endif


void load_tab_pho_file();
void show_win0();

void init_tab_pp(int usenow)
{
  if (!ch_pho)
    load_tab_pho_file();

  if (phcount && gwin0) {
disp_prom:
    show_stat();
    restore_ai();
    show_win0();
    return;
  }

  load_tsin_db();

  clr_ch_buf();
  show_win0();

//  dump_tsidx_all();
  goto disp_prom;
}

gboolean save_phrase_to_db2(CHPHO *chph, int len);

static void save_phrase()
{
  int tt, i;
  u_char len;

  if (save_frm==save_to)
    return;

  if (save_frm > save_to) {
    tt=save_frm;
    save_frm=save_to;
    save_to=tt;
  }

  if (save_to==c_len)
    save_to--;

  len= save_to- save_frm+ 1;
  if (len < 2 || len > MAX_PHRASE_LEN)
    return;

  for(i=save_frm;i<=save_to;i++)
    if (!chpho[i].pho)
      return;

  if (!save_phrase_to_db2(&chpho[save_frm], len)) {
    bell();
  }

  clrcursor();
  ph_sta=-1;
  c_idx=c_len;
  drawcursor();
  return;
}

#define PH_SHIFT_N (MAX_PH_BF - 1)
void compact_win0_x();

static void shift_ins()
{
   int j;

   if (!c_idx && c_len >= PH_SHIFT_N) {
     c_len--;
   }
   else
   if (c_len >= PH_SHIFT_N) {
     int ofs;

     ofs = 1;
     putbuf(ofs);

     ph_sta-=ofs;
     for(j=0; j < c_len - ofs; j++) {
       chpho[j] = chpho[j+ofs];
     }
     c_idx-=ofs;
     c_len-=ofs;
     prbuf();
   }

   c_len++;
   if (c_idx < c_len-1) {
     for(j=c_len; j>=c_idx; j--) {
       chpho[j+1] = chpho[j];
     }

     chpho[c_len].ch[0]=' ';
    /*    prbuf(); */
   }

   compact_win0_x();
}


static void put_b5_char(char *b5ch, phokey_t key)
{
   shift_ins();

   bchcpy(chpho[c_idx].ch, b5ch);
   bchcpy(chpho[c_idx].och, b5ch);
   bchcpy(chpho[c_idx].ph1ch, b5ch);

   disp_char_chbuf(c_idx);

   chpho[c_idx].pho=key;
   c_idx++;

   if (c_idx < c_len) {
     prbuf();
   }
}


#define MAX_PHRASE_SEL_N (9)

static u_char selstr[MAX_PHRASE_SEL_N][MAX_PHRASE_LEN * CH_SZ], sellen[MAX_PHRASE_SEL_N];
static int selidx[MAX_PHRASE_SEL_N];

static u_short phrase_count;
static u_short pho_count;

static gboolean chpho_eq_pho(int idx, phokey_t *phos, int len)
{
  int i;

  for(i=0; i < len; i++)
    if (chpho[idx+i].pho != phos[i])
       return FALSE;

  return TRUE;
}


static void get_sel_phrase()
{
  int sti,edi,j;
  phokey_t key, stk[MAX_PHRASE_LEN];
  u_char len, mlen, stch[MAX_PHRASE_LEN * CH_SZ + 1];

  mlen=c_len-c_idx;

  if (mlen > MAX_PHRASE_LEN)
    mlen=MAX_PHRASE_LEN;

  key=chpho[c_idx].pho;
  j= key >> TSIN_HASH_SHIFT;

  if (j >= TSIN_HASH_N)
    return;

  sti=hashidx[j];
  edi=hashidx[j+1];
  phrase_count = 0;

  while (sti < edi && phrase_count < MAX_PHRASE_SEL_N) {
    char usecount=0;
    load_tsin_entry(sti, &len, &usecount, stk, stch);

    if (len > mlen) {
      sti++;
      continue;
    }


    if (chpho_eq_pho(c_idx, stk, len)) {
      sellen[phrase_count]=len;
      selidx[phrase_count]=sti;
      memcpy(selstr[phrase_count++], stch, CH_SZ*len);
    }

    sti++;
  }
}



static void get_sel_pho()
{
  phokey_t key;

  if (c_idx==c_len)
    key=chpho[c_idx-1].pho;
  else
    key=chpho[c_idx].pho;

  if (!key)
    return;

  int i=hash_pho[key>>9];
  phokey_t ttt;

  while (i<idxnum_pho) {
    ttt=idx_pho[i].key;
    if (ttt>=key)
      break;
    i++;
  }

  if (ttt!=key) {
    return;
  }

  startf = idx_pho[i].start;
  pho_count = idx_pho[i+1].start - startf;
//  dbg("pho_count %d\n", pho_count);
}


void clear_sele();
void set_sele_text(int i, char *text, int len);
void disp_selections(int idx);
void disp_arrow_up(), disp_arrow_down();

static void disp_current_sel_page()
{
  int i;

  clear_sele();

  for(i=0; i < phkbm.selkeyN; i++) {
    int idx = current_page + i;

    if (idx < phrase_count) {
      int tlen = utf8_tlen(selstr[i], sellen[i]);
      set_sele_text(i, selstr[i], tlen);
    } else
    if (idx < phrase_count + pho_count) {
      int v = idx - phrase_count + startf;
      set_sele_text(i, ch_pho[v].ch, CH_SZ);
    } else
      break;
  }

  if (current_page + phkbm.selkeyN < phrase_count + pho_count) {
    disp_arrow_down();
  }

  if (current_page > 0)
    disp_arrow_up();

  disp_selections(c_idx);
}

static int fetch_user_selection(int val, char **seltext)
{
  int idx = current_page + val;
  int len = 0;

  if (idx < phrase_count) {
    len = sellen[idx];

    *seltext = selstr[idx];
  } else
  if (idx < phrase_count + pho_count) {
    int v = idx - phrase_count + startf;

    len = 1;
    *seltext = ch_pho[v].ch;
  }

  return len;
}


int phokey_t_seq(phokey_t *a, phokey_t *b, int len);

static void extract_pho(int chpho_idx, int plen, phokey_t *pho)
{
  int i;

  for(i=0; i < plen; i++) {
    pho[i] = chpho[chpho_idx + i].pho;
  }
}


gboolean tsin_seek(phokey_t *pho, int plen, int *r_sti, int *r_edi);

static u_char scanphr(int chpho_idx, int plen)
{
  if (plen >= MAX_PHRASE_LEN)
    return 0;


  phokey_t pp[MAX_PHRASE_LEN];
  extract_pho(chpho_idx, plen, pp);
  int sti, edi;

#if 0
  dbg("scanphr %d\n", plen);

  int t;
  for(t=0; t < plen; t++)
    prph(pp[t]);
  puts("");
#endif

  if (!tsin_seek(pp, plen, &sti, &edi))
    return 0;


  pre_selN = 0;
  int maxlen=0;

  while (sti < edi && pre_selN < 10) {
    phokey_t mtk[MAX_PHRASE_LEN];
    u_char mtch[MAX_PHRASE_LEN*CH_SZ+1];
    u_char match_len;
    char usecount;

    load_tsin_entry(sti, &match_len, &usecount, mtk, mtch);


    sti++;
    if (plen > match_len) {
      continue;
    }

    int i;
    for(i=0;i < plen;i++) {
      if (mtk[i]!=pp[i])
        break;
    }

    if (i < plen)
      continue;

#define VOID_PHRASE_N -1

    if (match_len == 2 && usecount < VOID_PHRASE_N)
      continue;

#if 0
    dbg("nnn ");
    nputs(mtch, match_len);
    dbg("\n");
#endif

#if 1
    int j;
    char *p = mtch;

    for(j=0; j < plen; j++) {
      int u8sz = utf8_sz(p);
      if (!(chpho[chpho_idx+j].flag & FLAG_CHPHO_FIXED))
        continue;

      if (memcmp(chpho[chpho_idx+j].ch, p, u8sz))
        break;

      p+= u8sz;
    }

    if (j < plen)
      continue;
#endif

    if (maxlen < match_len)
      maxlen = match_len;

    pre_sel[pre_selN].len = match_len;
    pre_sel[pre_selN].phidx = sti - 1;
    memcpy(pre_sel[pre_selN].str, mtch, match_len*CH_SZ);
    memcpy(pre_sel[pre_selN].phokey, mtk, match_len*sizeof(phokey_t));
    pre_selN++;

  }

  return maxlen;
}

static void disp_pre_sel_page()
{

  int i;

  if (!tsin_phrase_pre_select) {
    return;
  }

  if (pre_selN==0 || (pre_selN==1 && pre_sel[0].len==2))
    return;

  clear_sele();

  for(i=0; i < Min(phkbm.selkeyN, pre_selN); i++) {
    int tlen = utf8_tlen(pre_sel[i].str, pre_sel[i].len);

    set_sele_text(i, pre_sel[i].str, tlen);
  }

  disp_selections(ph_sta);
}


static void raise_phr(int c)
{
  int i,j,tmp;
  FILE *fp;

  if (!c)
    return;

  i=selidx[c];
  j=selidx[0];
  tmp=phidx[i];
  phidx[i]=phidx[j];
  phidx[j]=tmp;
  selidx[0]=tmp;

  if ((fp=fopen(tsidxfname,"r+"))==NULL) {
    dbg("%s modify err", tsidxfname);
    return;
  }

  fseek(fp,sizeof(phcount)+sizeof(hashidx) + j * sizeof(int), SEEK_SET);
  fwrite(&phidx[j],4,(c+1),fp);
  fclose(fp);
}

void hide_selections_win();
static void close_selection_win()
{
  hide_selections_win();
  current_page=sel_pho=0;
  pre_selN = 0;
}


static void clear_ch_buf_sel_area()
{
  clear_chars_all();
  c_len=c_idx=0; ph_sta=-1;
  full_match = FALSE;
  clr_ch_buf();
  drawcursor();
  clear_disp_ph_sta();
}


static void clear_tsin_buffer()
{
  clear_ch_buf_sel_area();
  close_selection_win();
  pre_selN = 0;
}

void clr_in_area_pho_tsin();
void close_win_pho_near();

static void tsin_reset_in_pho()
{
  clrin_pho();
  prbuf();
  clr_in_area_pho_tsin();
  close_selection_win();
  pre_selN = 0;
  drawcursor();

  close_win_pho_near();
}

gboolean flush_tsin_buffer()
{
  tsin_reset_in_pho();

  if (c_len) {
    putbuf(c_len);
    compact_win0_x();
    clear_ch_buf_sel_area();
    clear_tsin_buffer();
    return 1;
  }

  return 0;
}

void show_button_pho(gboolean bshow);

void tsin_set_eng_ch(int nmod)
{
  eng_ph = nmod;
  show_stat();
  drawcursor();

  show_button_pho(eng_ph);
}

void tsin_toggle_eng_ch()
{
  tsin_set_eng_ch(!eng_ph);
}

void tsin_toggle_half_full()
{
#if 0
  if (!eng_ph) {
    tsin_half_full^=1;
    drawcursor();
  } else {
    flush_tsin_buffer();
  }
#else
    tsin_half_full^=1;
    drawcursor();
#endif
}


#if 0
static char ochars[]="<,>.?/:;\"'{[}]_-+=|\\~`";
#else
static char ochars[]="<,>.?/:;\"'{[}]_-+=|\\";
#endif

static void hide_pre_sel()
{
  pre_selN = 0;
  hide_selections_win();
}


void clear_tsin_line();

static void clear_disp_ph_sta()
{
  clear_tsin_line();
}

void draw_underline(int index);

static void disp_ph_sta()
{
  clear_disp_ph_sta();

  if (ph_sta < 0)
    return;

  int i;

  for(i=ph_sta; i < c_idx; i++) {
    draw_underline(i);
  }
}



void ch_pho_cpy(CHPHO *pchpho, char *utf8, phokey_t *phos, int len)
{
  int i;

  for(i=0; i < len; i++) {
    int len = u8cpy(pchpho[i].ch, utf8);
    utf8+=len;
    pchpho[i].pho = phos[i];
  }
}


void set_chpho_ch(CHPHO *pchpho, char *utf8, int len)
{
  int i;

  for(i=0; i < len; i++) {
    int u8len = u8cpy(pchpho[i].ch, utf8);
    utf8+=u8len;
  }
}


void set_chpho_ch2(CHPHO *pchpho, char *utf8, int len)
{
  int i;

  for(i=0; i < len; i++) {
    int u8len = u8cpy(pchpho[i].ch, utf8);
    u8cpy(pchpho[i].och, utf8);
    utf8+=u8len;
  }
}


gboolean add_to_tsin_buf(char *str, phokey_t *pho, int len)
{
    int i;

    if (c_idx < 0 || c_len + len >= MAX_PH_BF_EXT)
      return 0;

    char *pp = str;
    for(i=0; i < len; i++) {
      if (pho[i])
        continue;

      phokey_t tpho[32];
      tpho[0]=0;
      int u8len = utf8_sz(pp);

      utf8_pho_keys(pp, tpho);
      pho[i] = tpho[0];
      pp += u8len;
    }


    if (c_idx < c_len) {
      for(i=c_len-1; i >= c_idx; i--) {
        chpho[i+len] = chpho[i];
      }
    }

    ch_pho_cpy(&chpho[c_idx], str, pho, len);

    if (c_idx == c_len)
      c_idx +=len;

    c_len+=len;

    clrin_pho();
    disp_in_area_pho_tsin();

    prbuf();
#if 1
    for(i=1;i < len; i++) {
      chpho[c_idx+i].psta= c_idx;
    }
#endif

    if (len > 1)
      chpho[c_idx].flag |= FLAG_CHPHO_PHRASE_HEAD;

    drawcursor();
    disp_ph_sta();
    hide_pre_sel();
    ph_sta=-1;
    return TRUE;
}

static void set_phrase_link(int idx, int len)
{
    int j;

    if (len < 2)
      return;

    for(j=1;j < len; j++) {
      chpho[idx+j].psta=idx;
    }

    chpho[idx].flag |= FLAG_CHPHO_PHRASE_HEAD;
}

// should be used only if it is a real phrase
gboolean add_to_tsin_buf_phsta(char *str, phokey_t *pho, int len)
{
    int idx = ph_sta < 0 && c_idx==c_len ? ph_sta_last : ph_sta;
#if 0
    dbg("idx:%d  ph_sta:%d ph_sta_last:%d c_idx:%d  c_len:%d\n",
       idx, ph_sta, ph_sta_last, c_idx, c_len);
#endif
    if (idx < 0)
      return 0;

    if (idx + len >= MAX_PH_BF_EXT)
      flush_tsin_buffer();

    ch_pho_cpy(&chpho[idx], str, pho, len);
    set_chpho_ch2(&chpho[idx], str, len);

    c_len=c_idx=idx + len;

    clrin_pho();
    disp_in_area_pho_tsin();

    prbuf();

    set_phrase_link(idx, len);

    drawcursor();
    disp_ph_sta();
    hide_pre_sel();
    ph_sta=-1;
    return 1;
}


void add_to_tsin_buf_str(char *str)
{
  char *pp = str;
  char *endp = pp+strlen(pp);
  int N = 0;


  while (*pp) {
    int u8sz = utf8_sz(pp);
    N++;
    pp += u8sz;

    if (pp >= endp) // bad utf8 string
      break;
  }


  phokey_t pho[MAX_PHRASE_LEN];
  bzero(pho, sizeof(pho));
  add_to_tsin_buf(str, pho, N);
}


static gboolean pre_sel_handler(KeySym xkey)
{
  static char shift_sele[]="!@#$%^&*()asdfghjkl:";
  static char noshi_sele[]="1234567890asdfghjkl;";
  char *p;

  if (!pre_selN || !tsin_phrase_pre_select)
    return 0;

  if (isupper(xkey))
    xkey = xkey - 'A' + 'a';

//  dbg("pre_sel_handler aa\n");

  if (!(p=strchr(shift_sele, xkey)))
    return 0;

  int c = p - shift_sele;
  char noshi = noshi_sele[c];

  if (!(p=strchr(phkbm.selkey, noshi)))
    return 0;

  c = p - phkbm.selkey;

  int len = pre_sel[c].len;

  if (c >= pre_selN)
    return 0;

  int j, eqlenN=0, current_ph_idx;

  for(j=0; j < pre_selN; j++) {
    if (pre_sel[j].len != len || phokey_t_seq(pre_sel[j].phokey, pre_sel[c].phokey, len))
      continue;

    if (j==c)
      current_ph_idx = eqlenN;

    selidx[eqlenN++]=pre_sel[j].phidx;
  }

//    dbg("eqlenN:%d %d\n", eqlenN, current_ph_idx);

  if (eqlenN > 1) {
    raise_phr(current_ph_idx);
  }

  full_match = FALSE;
  gboolean b_added = add_to_tsin_buf_phsta(pre_sel[c].str, pre_sel[c].phokey, len);

  return b_added;
}


static gboolean pre_punctuation(KeySym xkey)
{
  static char shift_punc[]="<>?:\"{}!";
  static char chars[]="，。？：；『』！";

  char *p;

  if ((p=strchr(shift_punc, xkey))) {
    int c = p - shift_punc;
    phokey_t key=0;

    return add_to_tsin_buf(&chars[c*CH_SZ], &key, 1);
  }

  return 0;
}

gboolean inph_typ_pho(char newkey);

static gboolean b_shift_pressed;


int feedkey_pp(KeySym xkey, int kbstate)
{
  char ctyp=0;
  static u_int ii;
  static u_short key;
  int i,k,pst;
  u_char match_len;
  int shift_m=kbstate&ShiftMask;
  int j,jj,kk, idx;
  char kno;

   b_shift_pressed = FALSE;

   if (kbstate & (Mod1Mask|Mod1Mask)) {
       return 0;
   }

   close_win_pho_near();

   switch (xkey) {
     case XK_Escape:
        tsin_reset_in_pho();
        return 1;
     case XK_Return:
     case XK_KP_Enter:
        if (shift_m) {
          save_frm=c_idx;
          save_to=c_len-1;
          save_phrase();
          return 1;
        } else
          return flush_tsin_buffer();
     case XK_Home:
        close_selection_win();
        if (!c_len)
          return 0;
        clrcursor();
        c_idx=0;
        drawcursor();
        return 1;
     case XK_End:
        close_selection_win();
        if (!c_len)
          return 0;
        clrcursor();
        c_idx=c_len;
        drawcursor();
        return 1;
     case XK_Left:
        close_selection_win();
        if (c_idx) {
          clrcursor();
          c_idx--;
          drawcursor();
          return 1;
        }
        // Thanks to PCMan.bbs@bbs.sayya.org for the suggestion
        if (c_len)
          return 1;
        return 0;
     case XK_Right:
        close_selection_win();
        if (c_idx < c_len) {
          clrcursor();
          c_idx++;
          drawcursor();
          return 1;
        }

        // Thanks to PCMan.bbs@bbs.sayya.org for the suggestion
        if (c_len)
          return 1;

        return 0;
     case XK_Caps_Lock:
        if (tsin_chinese_english_toggle_key == TSIN_CHINESE_ENGLISH_TOGGLE_KEY_CapsLock) {
          close_selection_win();
          tsin_toggle_eng_ch();
          return 1;
        } else
          return 0;
     case XK_Tab:
        if (tsin_chinese_english_toggle_key == TSIN_CHINESE_ENGLISH_TOGGLE_KEY_Tab) {
          close_selection_win();
          tsin_toggle_eng_ch();
          return 1;
        } else {
          if (c_len) {
            flush_tsin_buffer();
            return 1;
          }
          return 0;
        }
     case XK_Shift_L:
     case XK_Shift_R:
        b_shift_pressed = TRUE;
        break;
     case XK_Delete:
        if (c_idx == c_len)
          return 0;

        close_selection_win();
        clear_disp_ph_sta();
        ityp3_pho=0;
        pre_selN = 0;

        for(j=3;j>=0;j--)
          if (typ_pho[j]) {
            typ_pho[j]=0;
            disp_in_area_pho_tsin();
            return 1;
          }

        clrcursor();
        pst=k=chpho[c_idx].psta;

        for(k=c_idx;k<c_len;k++) {
          chpho[k] = chpho[k+1];
//          memcpy(ch_obuf[k], ch_obuf[k+1], CH_SZ);
          chpho[k].psta=chpho[k+1].psta-1;
        }

        c_len--;
        init_chpho_i(c_len);

        prbuf();

        compact_win0_x();

        if (!c_idx) {
          clear_match();
        }
        else {
          k=c_idx-1;
          pst=chpho[k].psta;

          while (k>0 && chpho[k].psta==pst)
            k--;

          if (chpho[k].psta!=pst)
            k++;

          match_len= c_idx - k;
          if (!(match_len=scanphr(k, match_len)))
            ph_sta=-1;
          else
            ph_sta=k;

//          if (ph_sta < 0 || c_idx - ph_sta < 2)
            pre_selN = 0;
        }

        disp_ph_sta();
        return 1;
     case XK_BackSpace:
        close_selection_win();
        clear_disp_ph_sta();
        ityp3_pho=0;
        pre_selN = 0;

        for(j=3;j>=0;j--)
          if (typ_pho[j]) {
            typ_pho[j]=0;
            inph[j]=0;
            disp_in_area_pho_tsin();
            return 1;
          }

        if (!c_idx)
          return 0;

        clrcursor();
        c_idx--;
        pst=k=chpho[c_idx].psta;

        for(k=c_idx;k<c_len;k++) {
          chpho[k]=chpho[k+1];
//          memcpy(ch_obuf[k], ch_obuf[k+1], CH_SZ);
          chpho[k].psta=chpho[k+1].psta-1;
        }

        c_len--;
        init_chpho_i(c_len);
        prbuf();
        compact_win0_x();

        if (!c_idx) {
          clear_match();
        }
        else {
          k=c_idx-1;
          pst=chpho[k].psta;

          while (k>0 && chpho[k].psta==pst)
            k--;

          if (chpho[k].psta!=pst)
            k++;

          match_len= c_idx - k;
          if (!(match_len=scanphr(k, match_len)))
            ph_sta=-1;
          else
            ph_sta=k;

//          if (ph_sta < 0 || c_idx - ph_sta < 2)
            pre_selN = 0;
        }

        disp_ph_sta();
        return 1;
     case XK_Up:
       if (!sel_pho) {
         if (c_len && c_idx == c_len) {
           int idx = c_len-1;
           phokey_t pk = chpho[idx].pho;

           if (pk) {
             void create_win_pho_near(phokey_t pho);
             create_win_pho_near(pk);
           }

           return 1;
         }
         return 0;
       }

       current_page = current_page - phkbm.selkeyN;
       if (current_page < 0)
         current_page = 0;

       disp_current_sel_page();
       return 1;
     case XK_space:
       if (tsin_space_opt == TSIN_SPACE_OPT_FLUSH_BUFFER && c_len && eng_ph
           && (ityp3_pho || (!typ_pho[0] && !typ_pho[1] && !typ_pho[2])) ) {
         flush_tsin_buffer();
         return 1;
       }
     case XK_Down:
       if (!eng_ph && xkey == XK_space)
           goto asc_char;

       if (!ityp3_pho && (typ_pho[0]||typ_pho[1]||typ_pho[2]) && xkey==XK_space) {
         ctyp=3;
         kno=0;

         inph_typ_pho(xkey);
         goto llll1;
       }

change_char:
       if (!c_len)
         return 0;

       idx = c_idx==c_len ? c_idx - 1 : c_idx;
       if (!chpho[idx].pho)
         return 1;

       if (!sel_pho) {
         get_sel_phrase();
         get_sel_pho();
         sel_pho=1;
         current_page = 0;
       } else {
         current_page = current_page + phkbm.selkeyN;
         if (current_page >= phrase_count + pho_count)
           current_page = 0;
       }

       disp_current_sel_page();
       return 1;
     case '\'':  // single quote
       if (phkbm.phokbm[xkey][0].num)
         goto other_keys;
       else {
         phokey_t key = 0;
         return add_to_tsin_buf("、", &key, 1);
       }
     case XK_q:
     case XK_Q:
       if (b_hsu_kbm && eng_ph)
         goto change_char;
     default:
other_keys:
       if ((kbstate & ControlMask)) {
         if (xkey==XK_u) {
           clear_tsin_buffer();
           return 1;
         } else
           return 0;
       }

       char *pp;

       char xkey_lcase = xkey;
       if ('A' <= xkey && xkey <= 'Z')
          xkey_lcase = tolower(xkey);

       if ((pp=strchr(phkbm.selkey,xkey_lcase)) && sel_pho) {
         int c=pp-phkbm.selkey;
         char *sel_text;
         int len = fetch_user_selection(c, &sel_text);

         if (len > 1) {
           int cpsta = chpho[c_idx].psta;
           if (cpsta >= 0) {
//             dbg("psta %d\n", cpsta);
             chpho[cpsta].flag |= FLAG_CHPHO_PHRASE_VOID;
             set_chpho_ch(&chpho[c_idx], sel_text, len);
           } else
             set_chpho_ch2(&chpho[c_idx], sel_text, len);

           chpho[c_idx].flag &= ~FLAG_CHPHO_PHRASE_VOID;
           set_phrase_link(c_idx, len);
           raise_phr(c);
         } else
         if (len == 1) { // single chinese char
           i= c_idx==c_len?c_idx-1:c_idx;
           key=chpho[i].pho;
#if 0
           set_chpho_ch2(&chpho[i], sel_text, 1);
#else
           set_chpho_ch(&chpho[i], sel_text, 1);
#endif
           if (i && chpho[i].psta == i-1 && !(chpho[i-1].flag & FLAG_CHPHO_FIXED)) {
             set_chpho_ch(&chpho[i-1], chpho[i-1].ph1ch, 1);
             chpho[i-1].flag |= FLAG_CHPHO_PHRASE_VOID;
           }

           chpho[i].flag |= FLAG_CHPHO_FIXED;
         }

         if (len) {
           prbuf();
           current_page=sel_pho=ityp3_pho=0;
           if (len == 1) {
             hide_selections_win();
             ph_sta = -1;
             goto restart;
           }
           else
             ph_sta=-1;

           hide_selections_win();
         }
         return 1;
       }

       sel_pho=current_page=0;
   }

   if (!eng_ph || shift_m || (xkey <= XK_KP_9 && xkey >= XK_KP_0) ||
        xkey == XK_KP_Subtract || xkey == XK_KP_Add || xkey == XK_KP_Multiply ||
        xkey == XK_KP_Divide
       ) {
asc_char:
        if (shift_m) {
          if (pre_sel_handler(xkey)) {
            return 1;
          }

          if (eng_ph && pre_punctuation(xkey))
            return 1;
        }

        if (xkey <= XK_KP_9 && xkey >= XK_KP_0)
          xkey=xkey-XK_KP_0+'0';
        else {
          switch (xkey) {
            case XK_KP_Add:
              xkey = '+';
              break;
            case XK_KP_Subtract:
              xkey = '-';
              break;
            case XK_KP_Multiply:
              xkey = '*';
              break;
            case XK_KP_Divide:
              xkey = '/';
              break;
          }
        }

//        dbg("xkey: %c\n", xkey);

        if (shift_m && eng_ph)  {
          char *ppp=strchr(ochars,xkey);

          if (!(kbstate&LockMask) && ppp && !((ppp-ochars) & 1))
            xkey=*(ppp+1);
          if (kbstate&LockMask && islower(xkey))
            xkey-=0x20;
          else
            if (!(kbstate&LockMask) && isupper(xkey))
              xkey+=0x20;
        } else {
          if (!eng_ph && tsin_chinese_english_toggle_key == TSIN_CHINESE_ENGLISH_TOGGLE_KEY_CapsLock) {
            if (shift_m) {
              if (islower(xkey))
                xkey-=0x20;
            } else
            if (isupper(xkey))
              xkey+=0x20;
          }
        }


        if (xkey > 127)
          return 1;

        u_char tt=xkey;
        shift_ins();

        if (tsin_half_full) {
          bchcpy(chpho[c_idx].ch, half_char_to_full_char(xkey));
        } else {
//          dbg("%c\n", tt);
          chpho[c_idx].ch[0]=tt;
        }

        phokey_t tphokeys[32];
        tphokeys[0]=0;
        utf8_pho_keys(chpho[c_idx].ch, tphokeys);

        disp_char_chbuf(c_idx);
        chpho[c_idx].pho=tphokeys[0];
        c_idx++;
        if (c_idx < c_len)
          prbuf();

        orig_disp_in_area_x = -1;

        drawcursor();
        return 1;
   } else { /* pho */
     if (xkey > 127)
       return 0;

     if (xkey >= 'A' && xkey <='Z')
       xkey+=0x20;

     inph_typ_pho(xkey);

     if (typ_pho[3])
       ctyp = 3;

llll1:
     jj=0;
     kk=1;
llll2:
     if (ctyp==3)
       ityp3_pho=1;  /* last key is entered */

     disp_in_area_pho_tsin();

     key = pho2key(typ_pho);

     int vv=hash_pho[typ_pho[0]];
     phokey_t ttt=0xffff;
     while (vv<idxnum_pho) {
       ttt=idx_pho[vv].key;
       if (!typ_pho[0]) ttt &= ~(31<<9);
       if (!typ_pho[1]) ttt &= ~(3<<7);
       if (!typ_pho[2]) ttt &= ~(15<<3);
       if (!typ_pho[3]) ttt &= ~(7);
       if (ttt>=key) break;
       else
       vv++;
     }

     if (ttt > key || (ityp3_pho && idx_pho[vv].key!=key) ) {
       while (jj<4) {
         while(kk<3)
         if (phkbm.phokbm[(int)inph[jj]][kk].num ) {
           if (kk) {
             ctyp=phkbm.phokbm[(int)inph[jj]][kk-1].typ;
             typ_pho[(int)ctyp]=0;
           }
           kno=phkbm.phokbm[(int)inph[jj]][kk].num;
           ctyp=phkbm.phokbm[(int)inph[jj]][kk].typ;
           typ_pho[(int)ctyp]=kno;
           kk++;
           goto llll2;
         } else kk++;
         jj++;
         kk=1;
       }

       bell(); ityp3_pho=typ_pho[3]=0;
       disp_in_area_pho_tsin();
       return 1;
     }

     if (key==0 || !ityp3_pho)
       return 1;

     ii=idx_pho[vv].start;
     start_idx=ii;
   } /* pho */

   put_b5_char(ch_pho[start_idx].ch, key);

   disp_ph_sta();
   clrin_pho();
   clr_in_area_pho_tsin();
   drawcursor();
   hide_pre_sel();

   if (ph_sta < 0) {
restart:
     if ((match_len=scanphr(c_idx-1,1)))
       ph_sta=c_idx-1;

//     dbg("scanphr c_idx:%d match_len:%d\n", c_idx, match_len);
//     chpho[c_idx-1].psta = c_idx-1;
     pre_selN=0;
     disp_ph_sta();
     return 1;
   } else {
     int mdist = c_idx - ph_sta;
     int max_match_phrase_len;

//     dbg("match_len:%d mdist %d = c_idx:%d - ph_sta:%d\n", match_len,  mdist, c_idx, ph_sta);

     while (ph_sta < c_idx) {
//       dbg("ph_sta:%d\n", ph_sta);
       if ((max_match_phrase_len = scanphr(ph_sta, c_idx - ph_sta))) {
//         dbg("max_match_phrase_len: %d\n", max_match_phrase_len);
         break;
       } else
       if (full_match) {  // tstr: 選擇視窗
//         dbg("last full_match\n");
         full_match = FALSE;
         ph_sta = -1;
         goto restart;
       }

       ph_sta++;
     }

     mdist = c_idx - ph_sta;

     if (ph_sta==c_idx) {
//       dbg("uuu no match ....\n");
       clear_match();
       return 1;
     }

     if (!pre_selN) {
//       dbg("no match found ..\n");
       clear_match();
       goto restart;
     }

//     dbg("iiiiiiii ph_sta:%d %d  max_match_phrase_len:%d\n", ph_sta, c_idx, max_match_phrase_len);
     disp_ph_sta();

     if (mdist == 1) {
//       dbg("single len match\n");
       chpho[c_idx-1].psta = c_idx-1;
       pre_selN=0;
       return 1;
     }

     disp_pre_sel_page();

     full_match = FALSE;

     for(i=0; i < pre_selN; i++) {
       if (pre_sel[i].len != mdist)
         continue;
       int ofs=0;

       for(j=0; j < mdist; j++) {
          int clensel = utf8_sz(&pre_sel[i].str[ofs]);
          int clen = utf8_sz(chpho[ph_sta+j].ch);

          if (clensel != clen)
            continue;

          if ((chpho[ph_sta+j].flag & FLAG_CHPHO_FIXED) &&
             memcmp(chpho[ph_sta+j].ch, &pre_sel[i].str[ofs], clen))
             break;
          ofs+=clen;
       }

       if (j < mdist)
         continue;

       ch_pho_cpy(&chpho[ph_sta], pre_sel[i].str, pre_sel[i].phokey, mdist);
       if (chpho[ph_sta].psta < 0)
         set_chpho_ch2(&chpho[ph_sta], pre_sel[i].str, mdist);


       int j;
       for(j=0;j < mdist; j++) {
         if (j)
           chpho[ph_sta+j].psta = ph_sta;
         disp_char(ph_sta+j, chpho[ph_sta+j].ch);
       }

       full_match = TRUE;
       chpho[ph_sta].flag |= FLAG_CHPHO_PHRASE_HEAD;

       if (mdist==max_match_phrase_len) { // tstr: 選擇視窗
//         dbg("full match .......... %d\n", ph_sta);
         ph_sta_last = ph_sta;
         ph_sta = -1;
         if (pre_selN == 1)
           pre_selN = 0;
       }

       return 1;
     }
   }

   return 1;
}


int feedkey_pp_release(KeySym xkey, int kbstate)
{
  switch (xkey) {
     case XK_Shift_L:
     case XK_Shift_R:
        if (b_shift_pressed && tsin_chinese_english_toggle_key == TSIN_CHINESE_ENGLISH_TOGGLE_KEY_Shift) {
          b_shift_pressed = FALSE;
          close_selection_win();
          tsin_toggle_eng_ch();
          return 1;
        } else
          return 0;
     default:
        return 0;
  }
}


void tsin_remove_last()
{
  if (!c_len)
    return;
  c_len--;
  c_idx--;
}


gboolean save_phrase_to_db2(CHPHO *chph, int len)
{
   phokey_t pho[MAX_PHRASE_LEN];
   char ch[MAX_PHRASE_LEN * CH_SZ];

   chpho_extract(chph, len, pho, ch, NULL);

   return save_phrase_to_db(pho, ch, len, 1);
}
