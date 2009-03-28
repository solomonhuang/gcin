#include <string.h>

#include "gcin.h"
#include "pho.h"
#include "tsin.h"
#include "gcin-conf.h"

float tsin_parse_recur(int start, TSIN_PARSE *out)
{
  int plen;
  float bestscore = 0;

  for(plen=1; start + plen <= c_len && plen <= MAX_PHRASE_LEN; plen++) {
    phokey_t pp[MAX_PHRASE_LEN + 1];
    int sti, edi;
    TSIN_PARSE pbest[MAX_PH_BF_EXT+1];
    float pbestscore = 0;
    int remlen;

    bzero(pbest, sizeof(TSIN_PARSE) * c_len);

    pbest[0].len = 1;
    pbest[0].start = start;
    utf8cpy(pbest[0].str, chpho[start].ch);
//    utf8_putchar(chpho[start].ch); dbg("\n");
    pbestscore = 0;

    extract_pho(start, plen, pp);
    if (!tsin_seek(pp, plen, &sti, &edi)) {
      if (plen > 1)
        break;

      goto next;
    }

    for (;sti < edi; sti++) {
      phokey_t mtk[MAX_PHRASE_LEN];
      char mtch[MAX_PHRASE_LEN*CH_SZ+1];
      u_char match_len;
      usecount_t usecount;

      load_tsin_entry(sti, &match_len, &usecount, mtk, mtch);

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

      int score;

      score = plen;
      if (match_len > plen) {
        if (start + plen == c_len)
          pbest[0].flag |= FLAG_TSIN_PARSE_PARTIAL;

        if (pbestscore < score) {
#if 0
          dbg("start:%d flag:%x %d part plen %d  sc:%d,%d ",
            start, pbest[0].flag, sti, plen, pbestscore, score);
          utf8_putchar(chpho[start].ch); dbg(" %d\n", pbest[0].len);
#endif
          pbestscore = score;
        }

        continue;
      }

      score = usecount + 50 * plen * plen;

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


next:
    remlen = c_len - (start + plen);

    if (remlen) {
      int uc = tsin_parse_recur(start + plen, &pbest[1]);

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
#if 0
    dbg("%d] %s  %d  flag:%x\n", i,  out[i].str, out[i].len, out[i].flag);
#endif
  }

#if 0
  dbg("i:%d\n", i);
#endif
  i--;
  if ((out[i].flag & FLAG_TSIN_PARSE_PHRASE) && c_len - out[i].start > 1) {
    ph_sta = out[i].start;
  }
#if 0
  else {
    for(; i>=0 ; i--)
      if (out[i].flag & FLAG_TSIN_PARSE_PARTIAL)
        break;

    if (i < 0)
      ph_sta = -1;
    else
      ph_sta = out[i].start;
  }
#endif

#if 0
  dbg("%d ph_sta: %d\n",i, ph_sta);
#endif
}
