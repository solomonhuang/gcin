#include "gcin.h"
#include "gtab.h"

/* this function is used to avoid 4-byte bus-alignment */
u_int CONVT(char *s)
{
  u_long kk;

  memcpy(&kk, s, 4);
  return kk;
}

/* this function is used to avoid 4-byte bus-alignment */
u_int64_t CONVT2(INMD *inmd, int i)
{
  u_int64_t kk;

  if (inmd->key64)
    memcpy(&kk, inmd->tbl64[i].key, sizeof(u_int64_t));
  else {
    kk = 0;
    memcpy(&kk, inmd->tbl[i].key, sizeof(u_int));
  }

  return kk;
}
