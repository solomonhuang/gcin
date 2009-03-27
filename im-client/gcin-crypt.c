#include "gcin-protocol.h"

static int __gcin_rand__(u_int *next)
{
  *next = *next * 1103515245 + 12345;
  return((unsigned)(*next/65536) % 32768);
}

void __gcin_enc_mem(u_char *p, int n,
                    GCIN_PASSWD *passwd, u_int *seed)
{
  int i;

  for(i=0; i < n; i++) {
    int v = __gcin_rand__(seed) % __GCIN_PASSWD_N_;
    p[i]^=passwd->passwd[v];
  }
}

