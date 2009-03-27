#include "gcin.h"
#include "pho.h"

void prph(phokey_t kk)
{
  u_int k[4];

  k[3]=(kk&7) * CH_SZ;
  kk>>=3;
  k[2]=(kk&15) * CH_SZ;
  kk>>=4;
  k[1]=(kk&3) * CH_SZ;
  kk>>=2;
  k[0]=(kk&31) * CH_SZ;

  int i;

  for(i=0; i < 3; i++) {
    int j;

    if (!k[i])
      continue;

    for(j=0; j < CH_SZ; j++) {
      putchar(pho_chars[i][k[i]+j]);
    }
  }

  if (k[3])
    printf("%d", k[3]/CH_SZ);
}
