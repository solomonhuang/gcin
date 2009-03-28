#include "gcin.h"
#include "pho.h"

void prph(phokey_t kk)
{
  u_int k[4];
  phokey_t okk = kk;

  k[3]=(kk&7);
  kk>>=3;
  k[2]=(kk&15) * PHO_CHAR_LEN;
  kk>>=4;
  k[1]=(kk&3) * PHO_CHAR_LEN;
  kk>>=2;
  k[0]=(kk&31) * PHO_CHAR_LEN;


  if (k[0]==BACK_QUOTE_NO*PHO_CHAR_LEN) {
    utf8_putchar(&pho_chars[0][k[0]]);
    printf("%c", okk & 0x7f);
  } else {
    int i;
    for(i=0; i < 3; i++) {
      if (!k[i])
        continue;

      utf8_putchar(&pho_chars[i][k[i]]);
    }

    if (k[3])
      printf("%d", k[3]);
  }
}
