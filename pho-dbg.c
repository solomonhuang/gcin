#include "gcin.h"
#include "pho.h"

void prph(phokey_t kk)
{
  u_int k[4];

  k[3]=(kk&7);
  kk>>=3;
  k[2]=(kk&15) * PHO_CHAR_LEN;
  kk>>=4;
  k[1]=(kk&3) * PHO_CHAR_LEN;
  kk>>=2;
  k[0]=(kk&31) * PHO_CHAR_LEN;

  int i;

  for(i=0; i < 3; i++) {
    int j;

    if (!k[i])
      continue;

    utf8_putchar(&pho_chars[i][k[i]]);
  }

  if (k[3])
    printf("%d", k[3]);
}
