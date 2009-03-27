#include "gcin.h"
#include "gtab.h"

/* this function is used to avoid 4-byte bus-alignment */
u_long CONVT(char *s)
{
  u_long kk;

  memcpy(&kk, s, 4);
  return kk;
}
