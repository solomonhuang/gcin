/*
	Copyright (C) 1994-2008	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/

#define InAreaX (0)

#include "gcin.h"
#include "intcode.h"

int current_intcode = INTCODE_UTF32;
extern gboolean test_mode;

static char inch[MAX_INTCODE];
int intcode_cin;

void create_win_intcode();
void show_win_int();

void init_inter_code()
{
  intcode_cin=0;
  create_win_intcode();
}

static int h2i(int x)
{
  return (x<='9'?x-'0':x-'A'+10);
}

static void utf32to8(char *t, char *s)
{
  gsize rn,wn=0;
  GError *err = NULL;
  char *utf8 = g_convert(s, 4, "UTF-8", "UTF-32", &rn, &wn, &err);

  if (utf8) {
    memcpy(t, utf8, wn);
    g_free(utf8);
  }

  t[wn]=0;
}


static unich_t *dstr[]={
_L("０"),_L("１"),_L("２"),_L("３"),_L("４"),_L("５"),_L("６"),_L("７"),_L("８"),_L("９"),_L("Ａ"),_L("Ｂ"),_L("Ｃ"),_L("Ｄ"),_L("Ｅ"),_L("Ｆ")
};

void disp_int(int index, char *intcode);
void clear_int_code_all();

int feedkey_intcode(KeySym key)
{
  int i;
#if 0
  if (key <= XK_KP_9 && key >= XK_KP_0)
    key -= XK_KP_0 - '0';
#endif
  key=toupper(key);
  if (key==XK_BackSpace||key==XK_Delete) {
    if (test_mode)
      return intcode_cin>0;

    if (intcode_cin)
      intcode_cin--;
    else
      return 0;

    goto dispIn;
  }
  else
  if ((key<'0'||key>'F'||(key>'9' && key<'A')) && (key!=' ')){
    return 0;
  }

  if (current_intcode==INTCODE_BIG5) {
    if (intcode_cin==0 && key<'8')
      return 1;
    if (intcode_cin==1 && inch[0]=='F' && key=='F')
      return 1;
    if (intcode_cin==2 && (key<'4' || (key>'7' && key<'A')))
      return 1;
    if (intcode_cin==3 && (inch[2]=='7'||inch[2]=='F') && key=='F')
      return 1;
  }

  if (!intcode_cin && key==' ')
    return 0;
#if WIN32
  if (test_mode)
    return 1;
#endif
  if ((intcode_cin<MAX_INTCODE-1 || (current_intcode!=INTCODE_BIG5 && intcode_cin < MAX_INTCODE)) && key!=' ')
    inch[intcode_cin++]=key;

dispIn:
  clear_int_code_all();
#if 0
  if (intcode_cin)
	  show_win_int();
#endif
  for(i=0;i<intcode_cin;i++) {
    disp_int(i, _(dstr[h2i(inch[i])]));
  }

  if ((current_intcode==INTCODE_BIG5 && intcode_cin==4 ||
       current_intcode==INTCODE_UTF32 && intcode_cin==6) &&
      gtab_press_full_auto_send || key==' ') {
    u_char utf8[CH_SZ+1];

    if (current_intcode==INTCODE_BIG5) {
      u_char ttt[3];
      ttt[2]=ttt[3]=0;
      ttt[0]=(h2i(inch[0])<<4)+h2i(inch[1]);
      ttt[1]=(h2i(inch[2])<<4)+h2i(inch[3]);
      big5_utf8((char *)ttt, (char *)utf8);
    } else {
      int i;
      u_int v = 0;

      for(i=0; i < intcode_cin; i++) {
        v <<= 4;
        v |= h2i(inch[i]);
      }

      utf32to8((char *)utf8, (char *)&v);
    }

    sendkey_b5((char *)utf8);
    intcode_cin=0;

    clear_int_code_all();
  }

  return 1;
}

#include "im-client/gcin-im-client-attr.h"
extern GtkWidget *gwin_int;

int int_get_preedit(char *str, GCIN_PREEDIT_ATTR attr[], int *cursor, int *sub_comp_len)
{
#if WIN32
  *sub_comp_len = intcode_cin>0;
#if 0
  if (gwin_int && GTK_WIDGET_VISIBLE(gwin_int))
    *sub_comp_len|=2;
#endif
  dbg("comp_len %x\n", *sub_comp_len);
#endif
  return 0;
}
