#include "gcin.h"
#include "gtab.h"

// space + \0'
char gcin_switch_keys[MAX_GTAB_NUM_KEY+2]=" 1234567890-=`[]\\";

int gcin_switch_keys_lookup(int key)
{
  char *p = strchr(gcin_switch_keys, key);
  if (!p)
    return -1;

  return p - gcin_switch_keys;
}
