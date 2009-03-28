char gcin_switch_keys[]="0123456789-=";
int gcin_switch_keysN=sizeof(gcin_switch_keys) - 1;

int gcin_switch_keys_lookup(int key)
{
  char *p = strchr(gcin_switch_keys, key);
  if (!p)
    return -1;
  return p - gcin_switch_keys;
}
