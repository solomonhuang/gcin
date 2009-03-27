#include "gcin.h"
#include "gtab.h"


/* this function is used to avoid 4-byte bus-alignment */
u_int64_t CONVT2(INMD *inmd, int i)
{
  u_int64_t kk;

  if (inmd->key64)
    memcpy(&kk, inmd->tbl64[i].key, sizeof(u_int64_t));
  else {
    u_int tt;
    memcpy(&tt, inmd->tbl[i].key, sizeof(u_int));
    kk = tt;
  }

  return kk;
}

char gtab64_header[]="## gtab 64-6bit ##";
