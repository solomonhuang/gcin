#include <string.h>
#include "gcin.h"
#include "pho.h"
#include "tsin.h"
#include "gcin-conf.h"

typedef struct {
  int start;
  float score;
  TSIN_PARSE best[MAX_PH_BF_EXT+1];
} CACHE ;

static CACHE *cache;
static int cacheN;

static CACHE *cache_lookup(int start)
{
  int i;

  for(i=0; i < cacheN; i++)
    if (cache[i].start == start)
      return &cache[i];
  return NULL;
}

static void add_cache(int start, float score, TSIN_PARSE *out)
{
  cache[cacheN].start = start;
  cache[cacheN].score = score;
  memcpy(cache[cacheN].best, out, sizeof(TSIN_PARSE) * (c_len - start));
  cacheN++;
}

float tsin_parse_recur(int start, TSIN_PARSE *out)
{
  int plen;
  float bestscore = 0;

  for(plen=1; start + plen <= c_len && plen <= MAX_PHRASE_LEN; plen++) {
    if (plen > 1 && (chpho[start+plen-1].flag & FLAG_CHPHO_PHRASE_USER_HEAD))
      break;
    phokey_t pp[MAX_PHRASE_LEN + 1];
    int sti, edi;
    TSIN_PARSE pbest[MAX_PH_BF_EXT+1];
    float pbestscore = 0;
    int remlen;

    bzero(pbest, sizeof(TSIN_PARSE) * c_len);

    pbest[0].len = 1;
    pbest[0].start = start;
    utf8cpy(pbest[0].str, chpho[start].ch);
#if 0
    dbg("hh "); utf8_putchar(chpho[start].ch); dbg("\n");
#endif

    extract_pho(start, plen, pp);

    if (!tsin_seek(pp, plen, &sti, &edi)) {
      if (plen > 1)
        break;
      goto next;
    }

    for (;sti < edi; sti++) {
      phokey_t mtk[MAX_PHRASE_LEN];
      char mtch[MAX_PHRASE_LEN*CH_SZ+1];
      char match_len;
      usecount_t usecount;

      load_tsin_entry(sti, &match_len, &usecount, mtk, mtch);

      if (match_len < plen)
        continue;

      if (check_fixed_mismatch(start, mtch, plen))
        continue;

      if (usecount < 0)
        usecount = 0;

      int i;
      for(i=0;i < plen;i++)
        if (mtk[i]!=pp[i])
          break;

      if (i < plen)
        continue;

      float score;

      score = plen;
      if (match_len > plen) {
        continue;
      }

      score = (float)usecount + 200 * plen * plen;

      if (pbestscore >= score)
        continue;

      pbest[0].len = plen;
      pbestscore = score;
      utf8cpyN(pbest[0].str, mtch, plen);
      pbest[0].flag |= FLAG_TSIN_PARSE_PHRASE;
#if 0
      utf8_putcharn(mtch, plen);
      dbg("   plen %d score:%d,%d  usecount:%d  start:%d sti:%d\n",
        plen, pbestscore, score, usecount, start, sti);
#endif
    }

#if 0
    // no match phrase was found
    if (pbestscore == 0.0) {
      if (plen > 1)
        break;
    }
#endif

next:
    remlen = c_len - (start + plen);

    if (remlen) {
      int next = start + plen;
      int uc;
      CACHE *pca;

      if (pca = cache_lookup(next)) {
        uc = pca->score;
        memcpy(&pbest[1], pca->best, (c_len - next) * sizeof(TSIN_PARSE));
      } else {
        uc = tsin_parse_recur(next, &pbest[1]);
        add_cache(next, uc, &pbest[1]);
      }

      pbestscore += uc;
    }

    if (bestscore < pbestscore) {
      bestscore = pbestscore;
      memcpy(out, pbest, sizeof(TSIN_PARSE) * (c_len - start));
    }
  }

  return bestscore;
}


void tsin_parse(TSIN_PARSE out[])
{
  if (c_len <= 1)
    return;

  cache = tmalloc(CACHE, c_len);
  cacheN = 0;

  tsin_parse_recur(0, out);

  int i, ofsi;

  for(i=0; i < c_len; i++)
    chpho[i].flag &= ~FLAG_CHPHO_PHRASE_HEAD;

  for(ofsi=i=0; out[i].len; i++) {
    int j, ofsj;
    int psta = ofsi;

    if (out[i].flag & FLAG_TSIN_PARSE_PHRASE)
        chpho[ofsi].flag |= FLAG_CHPHO_PHRASE_HEAD;

    for(ofsj=j=0; j < out[i].len; j++) {
      ofsj += u8cpy(chpho[ofsi].ch, &out[i].str[ofsj]);

      if (out[i].flag & FLAG_TSIN_PARSE_PHRASE)
        chpho[ofsi].psta = psta;

      ofsi++;
    }
  }

  if (chpho[c_len-1].psta>=0) {
    ph_sta = chpho[c_len-1].psta;
  }

#if 0
  dbg("%d ph_sta: %d\n",i, ph_sta);
#endif

  disp_ph_sta();

  free(cache); cache = NULL;
}
