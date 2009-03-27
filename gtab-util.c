#include "gcin.h"
#include "gtab.h"


/* this function is used to avoid 4-byte bus-alignment */
u_int64_t CONVT2(INMD *inmd, int i)
{
  u_int64_t kk;

  if (i >= inmd->DefChars || i < 0) {
//    dbg("%d %d\n", i, inmd->DefChars);
    return 0;
  }

  if (inmd->key64) {
    memcpy(&kk, inmd->tbl64[i].key, sizeof(u_int64_t));
  }
  else {
    u_int tt;
    memcpy(&tt, inmd->tbl[i].key, sizeof(u_int));
    kk = tt;
  }

  return kk;
}

char gtab64_header[]="## gtab 64-6bit ##";
char gtab32_ver2_header[]="## gtab 32-6bit ##";
