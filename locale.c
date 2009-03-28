#include "gcin.h"

void big5_utf8_n(char *s, int len, char out[])
{
  out[0]=0;

  GError *err = NULL;
  int rn, wn;
  char *utf8 = g_convert(s, len, "UTF-8", "Big5", &rn, &wn, &err);

  if (err) {
    dbg("big5_utf8  convert error\n");
    out[0]=0;
//    abort();
    return;
  }

  strcpy(out, utf8);
  g_free(utf8);
}


void big5_utf8(char *s, char out[])
{
  big5_utf8_n(s, strlen(s), out);
}


void utf8_big5_n(char *s, int len, char out[])
{
  out[0]=0;

  GError *err = NULL;
  int rn, wn;
  char *big5 = g_locale_from_utf8 (s, len, &rn, &wn, &err);

  if (err || !big5) {
    dbg("utf8_big5 convert error\n");
//    abort();
    return;
  }

  strcpy(out, big5);
  g_free(big5);
}


void utf8_big5(char *s, char out[])
{
  utf8_big5_n(s, strlen(s), out);
}



int utf8_sz(char *s)
{
  if (!(*s & 0x80))
    return 1;

  if ((*s & 0xe0) == 0xc0)
    return 2;

  if ((*s & 0xf0) == 0xe0)
    return 3;

  if ((*s & 0xf8) == 0xf0)
    return 4;

  p_err("bad utf8 char %x %c%c%c", *s, *s, *(s+1), *(s+2));
  return -1;
}


void utf8cpy(char *t, char *s)
{
  int utf8sz = utf8_sz(s);

  memcpy(t, s, utf8sz);
  t[utf8sz] = 0;
}


void utf8cpyN(char *t, char *s, int N)
{
  int len = utf8_tlen(s, N);

  memcpy(t, s, len);

  t[len] = 0;
}


int u8cpy(char *t, char *s)
{
  int utf8sz = utf8_sz(s);

  memcpy(t, s, utf8sz);
  return utf8sz;
}


int utf8_tlen(char *s, int N)
{
  int i;
  char *p = s;

  for(i=0; i < N; i++) {
    int len = utf8_sz(p);
    p+=len;
  }

  return p - s;
}

void utf8_putchar(char *s)
{
  int i;
  int len = utf8_sz(s);

  for(i=0;i<len;i++)
    putchar(s[i]);
}

void utf8_putcharn(char *s, int n)
{
  int i, ofs;

  for(ofs=i=0; i < n; i++) {
    utf8_putchar(&s[ofs]);
    ofs+= utf8_sz(&s[ofs]);
  }
}

gboolean utf8_eq(char *a, char *b)
{
  int ta = utf8_sz(a);
  int tb = utf8_sz(b);

  if (ta != tb)
    return FALSE;

  return !memcmp(a,b, ta);
}

gboolean utf8_str_eq(char *a, char *b, int len)
{
  int ta = utf8_tlen(a, len);
  int tb = utf8_tlen(b, len);

  if (ta != tb)
    return FALSE;

  return !memcmp(a, b, ta);
}

int utf8_str_N(char *str)
{
  int N=0;

  while (*str) {
    str+= utf8_sz(str);
    N++;
  }

  return N;
}
