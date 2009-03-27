/*
	Copyright (C) 1994	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/

#define InAreaX (0)

#include "gcin.h"

static char inch[4];
static int cin;

void create_win_intcode();
void init_inter_code(int usenow)
{
  cin=0;
  create_win_intcode();
}

static int h2i(int x)
{
  return (x<='9'?x-'0':x-'A'+10);
}

static char dstr[]="¢¯¢°¢±¢²¢³¢´¢µ¢¶¢·¢¸¢Ï¢Ð¢Ñ¢Ò¢Ó¢Ô";

void disp_int(int index, char *intcode);

int feedkey_intcode(KeySym key)
{
  int i;
#if 0
  if (key <= XK_KP_9 && key >= XK_KP_0)
    key -= XK_KP_0 - '0';
#endif
  key=toupper(key);
  if (key==XK_BackSpace||key==XK_Delete) {
    if (cin)
      cin--;
    else
      return 0;

    goto dispIn;
  }
  else
  if (key<'0'||key>'F'||(key>'9' && key<'A'))
    return 0;
  if (cin==0 && key<'8' )
    return 1;
  if (cin==1 && inch[0]=='F' && key=='F')
    return 1;
  if (cin==2 && (key<'4' || (key>'7' && key<'A')))
    return 1;
  if (cin==3 && (inch[2]=='7'||inch[2]=='F') && key=='F')
    return 1;

  if (cin<4)
    inch[cin++]=key;

dispIn:
  clear_int_code_all();

  for(i=0;i<cin;i++) {
    disp_int(i, &dstr[h2i(inch[i])<<1]);
  }

  if (cin==4) {
    u_char ttt[3], utf8[CH_SZ+1];

    ttt[3]=0;
    ttt[0]=(h2i(inch[0])<<4)+h2i(inch[1]);
    ttt[1]=(h2i(inch[2])<<4)+h2i(inch[3]);
    big5_utf8(ttt, utf8);

    sendkey_b5(utf8);
    cin=0;

    clear_int_code_all();
  }

  return 1;
}
