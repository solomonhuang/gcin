#include <math.h>
#include "gcin.h"
#include "gtab.h"
#include "gcin-conf.h"
#include "gcin-endian.h"
#include "pho.h"
#include "tsin.h"
#include "tsin-parse.h"

extern gboolean gtab_buf_select;
typedef struct {
  char ch[CH_SZ];
} CH;

typedef struct {
  char *ch;
  char **sel;
  int selN;
  u_char flag, c_sel;
  char plen;
} GEDIT;

#define MAX_GBUF 80
GEDIT gbuf[MAX_GBUF+1];
short gbufN;
short gbuf_cursor;
extern int pg_idx, total_matchN;
extern char seltab[MAX_SELKEY][MAX_CIN_PHR];

void dump_gbuf()
{
  int i;

  for(i=0; i<gbufN; i++) {
    int j;
    for(j=0;j < gbuf[i].selN; j++)
      printf("%d:%s ", j, gbuf[i].sel[j]);
    puts("");
  }
}

static char *gen_buf_str()
{
  int i;
  char *out = tmalloc(char, 1);
  int outN=0;

  for(i=0;i<gbufN;i++) {
    char *t = gbuf[i].ch;
    int len = strlen(t);

    out = trealloc(out, char, outN+len+1);
    memcpy(out + outN, t, len);
    outN+=len;
  }

  out[outN] = 0;
  return out;
}

static char *gen_buf_str_disp()
{
  int i;
  char *out = tmalloc(char, 1);
  int outN=0;

  gbuf[gbufN].ch = " ";

  for(i=0;i<=gbufN;i++) {
    char spec[MAX_CIN_PHR * 2];
    htmlspecialchars(gbuf[i].ch, spec);
    char www[MAX_CIN_PHR * 2];
    char *t = spec;

    if (i==gbuf_cursor) {
      sprintf(www, "<span background=\"%s\">%s</span>", "red", spec);
      t = www;
    }

    int len = strlen(t);
    out = trealloc(out, char, outN+len+1);
    memcpy(out + outN, t, len);
    outN+=len;
  }

  out[outN] = 0;
  return out;
}


void disp_label_edit(char *str);

static void free_pgbuf(GEDIT *p)
{
  int i;
  for(i=0; i < p->selN; i++)
    free(p->sel[i]);
  free(p->sel);
}


static void free_gbuf(int idx)
{
  free_pgbuf(&gbuf[idx]);
}


static void clear_buf_all()
{
  int i;
  for(i=0;i<gbufN;i++)
    free_gbuf(i);
  gbuf_cursor = gbufN=0;
  gtab_buf_select = 0;
}

void disp_gbuf()
{
  char *bf=gen_buf_str_disp();
  disp_label_edit(bf);

  if (gbufN)
    lookup_gtabn(gbuf[gbufN-1].ch, NULL);

  free(bf);
}

void clear_gbuf_sel()
{
  gtab_buf_select = 0;
  total_matchN = 0;
  ClrSelArea();
}

int gbuf_cursor_left()
{
  if (!gbuf_cursor)
    return gbufN;
  if (gtab_buf_select)
    clear_gbuf_sel();
  ClrIn();
  gbuf_cursor--;
  disp_gbuf();
  return 1;
}


int gbuf_cursor_right()
{
  if (gbuf_cursor==gbufN)
    return gbufN;
  if (gtab_buf_select)
    clear_gbuf_sel();
  gbuf_cursor++;
  disp_gbuf();
  return 1;
}

int gbuf_cursor_home()
{
  if (!gbufN)
    return 0;
  if (gtab_buf_select)
    return 1;
  gbuf_cursor = 0;
  disp_gbuf();
  return 1;
}


int gbuf_cursor_end()
{
  if (!gbufN)
    return 0;
  if (gtab_buf_select)
    return 1;
  gbuf_cursor = gbufN;
  disp_gbuf();
  return 1;
}

int output_gbuf()
{
  if (!gbufN)
    return 0;
  char *bf=gen_buf_str();
#if 0
  printf("out %s\n", bf);
#endif
  send_text(bf);
  free(bf);


  int i;
  for(i=0; i < gbufN;) {
    char t[MAX_CIN_PHR+1];
    t[0]=0;

    int j;
    for(j=i; j < i+gbuf[i].plen; j++)
      strcat(t, gbuf[j].ch);

    if (!gbuf[i].plen)
      i++;
    else {
      inc_gtab_usecount(t);
      i+=gbuf[i].plen;
    }
  }


  clear_buf_all();
  disp_gbuf();
  ClrIn();
  return;
}


gboolean check_gtab_fixed_mismatch(int idx, char *mtch, int plen)
{
  int j;
  char *p = mtch;

  for(j=0; j < plen; j++) {
    int u8sz = utf8_sz(p);
    if (!(gbuf[idx+j].flag & FLAG_CHPHO_FIXED))
      continue;

    if (memcmp(gbuf[idx+j].ch, p, u8sz))
      break;

    p+= u8sz;
  }

  if (j < plen)
    return TRUE;

  return FALSE;
}



CACHE *cache_lookup(int start);

#define DBG 0

static int gtab_parse_recur(int start, TSIN_PARSE *out,
                     short *r_match_phr_N, short *r_no_match_ch_N)

{
  int plen;
  double bestscore = -1;
  int bestusecount = 0;
  *r_match_phr_N = 0;
  *r_no_match_ch_N = gbufN - start;


  for(plen=1; start + plen <= gbufN && plen <= MAX_PHRASE_LEN; plen++) {
    TSIN_PARSE pbest[MAX_PH_BF_EXT+1];
#define MAXV 1000
    int maxusecount = 5-MAXV;
    int remlen;
    short match_phr_N=0, no_match_ch_N = plen;

    if (plen > 1 && (gbuf[start+plen-1].flag & FLAG_CHPHO_PHRASE_USER_HEAD))
      break;

    bzero(pbest, sizeof(TSIN_PARSE) * gbufN);
    pbest[0].len = plen;
    pbest[0].start = start;
    int i, ofs;
    for(i=0; i < plen; i++)
      pbest[0].str[i] = gbuf[start + i].c_sel;

    int tN = 1;
    for(i=0; i < plen; i++)
      tN *= gbuf[start+i].selN;

#if DBG
    printf("start:%d  plen:%d\n", start, plen);
#endif
    int has_ge = FALSE;
    for(i=0; i < tN; i++) {
      unsigned char counter[MAX_PHRASE_LEN];
      char tt[MAX_CIN_PHR * 2];

      tt[0]=0;
      int t=i;
      int j;
      for(j=0; j < plen; j++) {
        int selN = gbuf[start+j].selN;
        unsigned char v = t % selN;

        counter[j] = v;
        t /= selN;

        if (selN > 10 && !ch_pos_find(gbuf[start+j].sel[v], j))
            break;
        strcat(tt, gbuf[start+j].sel[v]);
      }

      if (j < plen)
        continue;

      if (check_gtab_fixed_mismatch(start, tt, plen))
        continue;
 //     dbg("a %s\n", tt);
      usecount_t usecount;
      int eq_N;

      int  ge_N = find_match(tt, &eq_N, &usecount);
      if (ge_N)
        has_ge = TRUE;
//      dbg("b %s  ge:%d eq:%d\n", tt, ge_N, eq_N);

      if (usecount <= maxusecount)
        continue;
      if (!eq_N)
        continue;

      pbest[0].len = plen;
      maxusecount = usecount;
      memcpy(pbest[0].str, counter, plen);
      pbest[0].flag |= FLAG_TSIN_PARSE_PHRASE;

      match_phr_N = 1;
      no_match_ch_N = 0;
    }

//    printf("has_ge:%d\n", has_ge);
    if (!has_ge && plen > 1)  // no longer phrases found
      break;

    remlen =  gbufN - (start + plen);

    if (remlen) {
      int next = start + plen;
      CACHE *pca;

      short smatch_phr_N, sno_match_ch_N;
      int uc;

      if (pca = cache_lookup(next)) {
        uc = pca->usecount;
        smatch_phr_N = pca->match_phr_N;
        sno_match_ch_N = pca->no_match_ch_N;
        memcpy(&pbest[1], pca->best, (gbufN - next) * sizeof(TSIN_PARSE));
      } else {
        uc = gtab_parse_recur(next, &pbest[1], &smatch_phr_N, &sno_match_ch_N);
//        dbg("   gg %d\n", smatch_phr_N);
        add_cache(next, uc, &pbest[1], smatch_phr_N, sno_match_ch_N, gbufN);
      }

      match_phr_N += smatch_phr_N;
      no_match_ch_N += sno_match_ch_N;
      maxusecount += uc;
    }


    double score = log(maxusecount + MAXV) /
      (pow(match_phr_N, 10)+ 1.0E-6) / (pow(no_match_ch_N, 20) + 1.0E-6);

#if DBG
    dbg("st:%d plen:%d zz muse:%d ma:%d noma:%d  score:%.4e %.4e\n", start, plen,
        maxusecount, match_phr_N, no_match_ch_N, score, bestscore);
#endif
    if (score > bestscore) {
#if DBG
      dbg("is best org %.4e\n", bestscore);
#endif
      bestscore = score;
      memcpy(out, pbest, sizeof(TSIN_PARSE) * (gbufN - start));

#if DBG
      dbg("    str:%d  ", start);
      int i;
      for(i=0;  i < gbufN - start; i++) {
        int j;
        for(j=0; j < out[i].len; j++) {
          unsigned char v = out[i].str[j];
          char idx = out[i].start + j;
          char *s = gbuf[idx].sel[v];
          printf("%s", out[i].str);
        }
        puts("");
      }
      dbg("\n");
#endif

      bestusecount = maxusecount;
      *r_match_phr_N = match_phr_N;
      *r_no_match_ch_N = no_match_ch_N;
    }
  }

  if (bestusecount < 0)
    bestusecount = 0;

  return bestusecount;
}

void init_cache();
void free_cache();

void gtab_parse()
{
  int i, ofsi;
  TSIN_PARSE out[MAX_PH_BF_EXT+1];
  bzero(out, sizeof(out));


  if (gbufN <= 1)
    return;

  init_cache(gbufN);

  short smatch_phr_N, sno_match_ch_N;
  gtab_parse_recur(0, out, &smatch_phr_N, &sno_match_ch_N);
#if 0
  puts("vvvvvvvvvvvvvvvv");
  for(i=0;  i < gbufN; i++) {
    printf("%d:", out[i].len);
//    utf8_putcharn(out[i].str, out[i].len);
  }
  dbg("\n");
#endif

  for(i=0; i < gbufN; i++)
    gbuf[i].flag &= ~FLAG_CHPHO_PHRASE_HEAD;

  for(i=0; out[i].len; i++) {
    int j;
    int psta = out[i].start;

    if (out[i].flag & FLAG_TSIN_PARSE_PHRASE) {
      gbuf[psta].flag |= FLAG_CHPHO_PHRASE_HEAD;
      gbuf[psta].plen = out[i].len;
    }

    for(j=0; j < out[i].len; j++) {
      unsigned char v = out[i].str[j];
      char idx = out[i].start + j;
      if (v >=gbuf[idx].selN)
        p_err("bad v %d\n", v);

      gbuf[idx].ch = gbuf[idx].sel[v];
      gbuf[idx].c_sel = v;
    }
  }

#if 0
  puts("-----------------------------");
  for(i=0;i<gbufN;i++)
    puts(gbuf[i].ch);
#endif
  free_cache();
}

static GEDIT *cursor_gbuf()
{
  return gbuf_cursor == gbufN ? &gbuf[gbuf_cursor-1] : &gbuf[gbuf_cursor];
}


void insert_gbuf_cursor(char **sel, int selN)
{
  if (!sel || !selN)
    return;

  GEDIT *pbuf = &gbuf[gbuf_cursor];

  if (gbuf_cursor < gbufN)
    memmove(&gbuf[gbuf_cursor+1], &gbuf[gbuf_cursor], sizeof(GEDIT) * (gbufN - gbuf_cursor));

  gbuf_cursor++;
  gbufN++;

  bzero(pbuf, sizeof(GEDIT));

  free_pgbuf(pbuf);

  pbuf->ch = sel[0];
  pbuf->sel = sel;
  pbuf->selN = selN;
  pbuf->c_sel = 0;
  gtab_parse();
  disp_gbuf();

  char_play(pbuf->ch);
}


void set_gbuf_c_sel(int v)
{
  GEDIT *pbuf = cursor_gbuf();

  pbuf->c_sel = v + pg_idx;
  pbuf->ch = pbuf->sel[pbuf->c_sel];
  pbuf->flag |= FLAG_CHPHO_FIXED;
  gtab_buf_select = 0;
  disp_gtab_sel("");
  gtab_parse();
  disp_gbuf();
}

void insert_gbuf_cursor1(char *s)
{
   char **sel = tmalloc(char *, 1);
   sel[0] = strdup(s);
   insert_gbuf_cursor(sel, 1);
   clear_after_put();
}

int insert_gbuf_cursor1_not_empty(char *s)
{
   if (!gbufN || !gtab_auto_select_by_phrase)
     return 0;
   insert_gbuf_cursor1(s);
   return TRUE;
}

void insert_gbuf_cursor_char(char ch)
{
  char t[2];
  t[0]=ch;
  t[1]=0;
  insert_gbuf_cursor1(t);
}

int gtab_buf_delete()
{
  if (gbuf_cursor==gbufN)
    return 0;
  if (gtab_buf_select)
    return 1;

  free_gbuf(gbuf_cursor);
  memmove(&gbuf[gbuf_cursor], &gbuf[gbuf_cursor+1], sizeof(GEDIT) * (gbufN - gbuf_cursor -1));
  gbufN--;
  disp_gbuf();
  return 1;
}

int gtab_buf_backspace()
{
  if (!gbuf_cursor)
    return 0;

  gbuf_cursor--;
  gtab_buf_delete();

  return 1;
}

extern int more_pg;

void gtab_disp_sel()
{
  int idx = gbuf_cursor==gbufN ? gbuf_cursor-1:gbuf_cursor;
  GEDIT *pbuf=&gbuf[idx];

  int i;
  for(i=0; i < cur_inmd->M_DUP_SEL; i++) {
    int v = i + pg_idx;
    if (v >= pbuf->selN)
      break;

    strcpy(seltab[i], pbuf->sel[v]);
  }

  if (pbuf->selN > page_len())
    more_pg = 1;

  disp_selection(FALSE);
}


int show_buf_select()
{
  if (!gbufN)
    return 0;

  int idx = gbuf_cursor==gbufN ? gbuf_cursor-1:gbuf_cursor;
  GEDIT *pbuf=&gbuf[idx];
  gtab_buf_select = 1;
  total_matchN = pbuf->selN;
  pg_idx = 0;

  gtab_disp_sel();

  return 1;
}


void gbuf_next_pg()
{
  pg_idx += page_len();
  if (pg_idx >= total_matchN)
    pg_idx = 0;

  gtab_disp_sel();
}
