#include "gcin.h"
#include "pho.h"
#include "gst.h"

extern void key_typ_pho(phokey_t phokey, u_char rtyp_pho[]);

gboolean pin2juyin()
{
  int i;

  char pin[7];
  pin[7]=0;
#if 0
  for(i=0; i < pin_juyinN; i++) {
    memcpy(pin,  pin_juyin[i].pinyin, sizeof(pin_juyin[0].pinyin));

    if (!strcmp(pin, poo.inph))
      goto match;
  }
#endif
  int inphN = strlen(poo.inph);
  for(i=0; i < pin_juyinN; i++) {
    memcpy(pin,  pin_juyin[i].pinyin, sizeof(pin_juyin[0].pinyin));

    int pinN = strlen(pin);

    if (pinN < inphN)
      continue;

    if (!memcmp(pin, poo.inph, inphN))
      break;
  }

  if (i==pin_juyinN)
    return FALSE;

  bzero(poo.typ_pho, sizeof(poo.typ_pho));
  key_typ_pho(pin_juyin[i].key, (u_char *)poo.typ_pho);

  return TRUE;
}

gboolean inph_typ_pho_pinyin(int newkey)
{
  char num = phkbm.phokbm[(int)newkey][0].num;
  int typ = phkbm.phokbm[(int)newkey][0].typ;

  int i;
  for(i=0; i < 7; i++)
    if (!poo.inph[i])
      break;
  if (i==7)
    return FALSE;

  poo.inph[i] = newkey;

  if (typ==3) {
    poo.typ_pho[typ] = num;
    return TRUE;
  }

  if (!pin2juyin()) {
    if (newkey != ' ')
      bell();

    poo.inph[i]=0;
    return FALSE;
  }

  return TRUE;
}

extern int text_pho_N;

void load_pin_juyin()
{
  text_pho_N = 6;
  char pinfname[128];

  get_sys_table_file_name("pin-juyin.xlt", pinfname);
  dbg("pinyin kbm %s\n", pinfname);

  FILE *fr;
  if ((fr=fopen(pinfname,"rb"))==NULL)
     p_err("Cannot open %s", pinfname);

  fread(&pin_juyinN, sizeof(short), 1, fr);
  pin_juyin = tmalloc(PIN_JUYIN, pin_juyinN);
  fread(pin_juyin, sizeof(PIN_JUYIN), pin_juyinN, fr);
  fclose(fr);
}
