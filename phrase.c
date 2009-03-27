/*
	Copyright (C) 1994	Edward Der-Hua Liu, Hsin-Chu, Taiwan
*/

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include "gcin.h"

struct keystruc {
  char *kname;
  KeySym ksym;
  char *str;
} tran[]={
  {"0", ')'}, {"1", '!'}, {"2", '@'}, {"3", '#'}, {"4", '$'}, {"5", '%'},
  {"6", '^'}, {"7", '&'}, {"8", '*'}, {"9", '('},
  {"a", 'a'}, {"b", 'b'}, {"c", 'c'}, {"d", 'd'}, {"e", 'e'}, {"f", 'f'},
  {"g", 'g'}, {"h", 'h'}, {"i", 'i'}, {"j", 'j'}, {"k", 'k'}, {"l", 'l'},
  {"m", 'm'}, {"n", 'n'}, {"o", 'o'}, {"p", 'p'}, {"q", 'q'}, {"r", 'r'},
  {"s", 's'}, {"t", 't'}, {"u", 'u'}, {"v", 'v'}, {"w", 'w'}, {"x", 'x'},
  {"y", 'y'}, {"z", 'z'},
  {",", '<'}, {".", '>'}, {";", ':'}, {"/", '?'},
  {"[", '{'}, {"]", '}'},
  {"f1",XK_F1},{"f2",XK_F2},{"f3",XK_F3},{"f4",XK_F4},{"f5",XK_F5},{"f6",XK_F6},
  {"f7",XK_F7},{"f8",XK_F8},{"f9",XK_F9},{"f10",XK_F10},{"f11",XK_F11},
  {"f12",XK_F12},
  {"k_ins", XK_KP_Insert}, {"k_del", XK_KP_Delete},  {"k_end", XK_KP_End},
  {"k_down",XK_KP_Down}, {"k_pgup",XK_KP_Prior},
  {"k_up",XK_KP_Up},
  {"k_pgdn",XK_KP_Next},    {"k_left",XK_KP_Left},
  {"k_5",   XK_KP_Begin},  {"k_right", XK_KP_Right}, {"k_home",XK_KP_Home},
  {"k_up",XK_Up}, {"k_pgup",XK_Prior},
  {"kp0", XK_KP_0}, {"kp.", XK_KP_Decimal},
  {"kp1", XK_KP_1}, {"kp2", XK_KP_2}, {"kp3", XK_KP_3},
  {"kp4", XK_KP_4}, {"kp5", XK_KP_5}, {"kp6", XK_KP_6},
  {"kp7", XK_KP_7}, {"kp8", XK_KP_8}, {"kp9", XK_KP_9},
  {"kp/",XK_KP_Divide}, {"kp*", XK_KP_Multiply}, {"kp-", XK_KP_Subtract},
  {"kp+",XK_KP_Add}, {"kpenter",XK_KP_Enter}
};

int tranN=sizeof(tran)/sizeof(tran[0]);
extern char *TableDir;

void load_phrase()
{
  FILE *fp;
  char kname[32];
  char ttt[512];

#if 1
  strcat(strcpy(ttt,TableDir),"/phrase.table");
#else
  strcpy(ttt, "phrase.table");
#endif

  if ((fp=fopen(ttt, "r"))==NULL)
    p_err("cannot fopen %s\n", ttt);

  while (!feof(fp)) {
    int i,j;
    char str[256];

    kname[0]=str[0]=0;
    fgets(ttt, sizeof(ttt), fp);
    if (ttt[0]=='#')
      continue;
    for(i=0; ttt[i]!=' ' && ttt[i]!=9 && ttt[i]; i++)
      kname[i]=ttt[i];

    kname[i]=0;
    while((ttt[i]==' ' || ttt[i]==9) && ttt[i])
      i++;

    for(j=0; ttt[i]!='\n' && ttt[i]; i++,j++)
      str[j]=ttt[i];

    if (!str[0] || !kname[0])
      continue;

    str[j]=0;

    for(i=0; i < tranN; i++)
      if (!strcmp(kname, tran[i].kname))
            break;

    if (i==tranN)
      p_err("unknown key: %s\n", kname);

    tran[i].str = strdup(str);
  }
}


gboolean feed_phrase(KeySym ksym)
{
  int i;

//  dbg("ksym:%x\n", ksym);

  if (ksym>='A' && ksym <= 'Z')
    ksym += 0x20;

  for(i=0; i < tranN; i++) {
    if (tran[i].ksym!= ksym)
      continue;

    if (tran[i].str) {
      send_text(tran[i].str);
      return TRUE;
    } else
      return FALSE;
  }

  return FALSE;
}
