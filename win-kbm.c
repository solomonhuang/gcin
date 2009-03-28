#include <sys/stat.h>
#include "gcin.h"
#include "gtab.h"
extern INMD *cur_inmd;

static GtkWidget *gwin_kbm;
static GdkColor red;

enum {
  K_FILL=1,
  K_HOLD=2,
  K_PRESS=4,
  K_AREA_R=8
};

typedef struct {
  KeySym keysym;
  char *enkey;
  char flag;
  char *imstr;
  GtkWidget *lab, *but, *laben;
} KEY;

#define COLN 19
static KEY keys[][COLN]={
{{XK_Escape,"Esc"},{XK_F1,"F1"},{XK_F2,"F2"},{XK_F3,"F3"},{XK_F4,"F4"},{XK_F5,"F5"},{XK_F6,"F6"},{XK_F7,"F7"},{XK_F8,"F8"},{XK_F9,"F9"},{XK_F10,"F10"},{XK_F11,"11"},{XK_F12,"12"},{XK_Print,"Pr",8},{XK_Scroll_Lock,"Slk",8},{XK_Pause,"Pau",8}},
{{'`'," ` "},{'1'," 1 "},{'2'," 2 "},{'3'," 3 "},{'4'," 4 "},{'5'," 5 "},{'6'," 6 "},{'7'," 7 "},{'8'," 8 "},{'9'," 9 "},{'0'," 0 "},{'-'," - "},{'='," = "},{XK_BackSpace,"←",1},{XK_Insert,"Ins",8},{XK_Home,"Ho",8},{XK_Prior,"P↑",8}},
{{XK_Tab, "Tab"}, {'q'," q "},{'w'," w "},{'e'," e "},{'r'," r "},{'t'," t "}, {'y'," y "},{'u'," u "},{'i'," i "},{'o'," o "}, {'p'," p "},{'['," [ "},{']'," ] "},{'\\'," \\ "},{XK_Delete,"Del",8},{XK_End,"En",8},{XK_Next,"P↓",8}},
{{XK_Caps_Lock, "Caps"},{'a'," a "},{'s'," s "},{'d'," d "},{'f', " f "},{'g'," g "},{'h'," h "},{'j'," j "},{'k'," k "},{'l'," l "},{';'," ; "},{'\''," ' "},{XK_Return," Enter ",1},{XK_Num_Lock,"Num",8},{XK_KP_Add," + ",8}},
{{XK_Shift_L,"  Shift  ",K_HOLD},{'z'," z "},{'x'," x "},{'c'," c "},{'v'," v "},{'b'," b "},{'n'," n "},{'m'," m "},{','," , "},{'.'," . "},{'/'," / "},{XK_Shift_R," Shift",K_HOLD|K_FILL},{XK_KP_Multiply," * ",8},{XK_Up,"↑",8}},
{{XK_Control_L,"Ctrl",K_HOLD},{XK_Alt_R,"Alt",K_HOLD},{' ',"Space", 1},{XK_Left, "←",8},{XK_Down, "↓",8},{XK_Right, "→",8}}
};

static int keysN=sizeof(keys)/sizeof(keys[0]);

void update_win_kbm();

void mod_fg_all(GtkWidget *lab, GdkColor *col)
{
  gtk_widget_modify_fg(lab, GTK_STATE_NORMAL, col);
  gtk_widget_modify_fg(lab, GTK_STATE_ACTIVE, col);
  gtk_widget_modify_fg(lab, GTK_STATE_SELECTED, col);
  gtk_widget_modify_fg(lab, GTK_STATE_PRELIGHT, col);
}

void send_fake_key_eve(KeySym key);
static void cb_button_click(GtkWidget *wid, KEY *k)
{
  KeySym keysym=k->keysym;
  KeyCode kc = XKeysymToKeycode(dpy, keysym);
  GtkWidget *laben = k->laben;

  if (k->flag & K_HOLD) {
    if (k->flag & K_PRESS) {
      k->flag &= ~K_PRESS;
      mod_fg_all(laben, NULL);
      XTestFakeKeyEvent(dpy, kc, False, CurrentTime);
    }
    else {
      XTestFakeKeyEvent(dpy, kc, True, CurrentTime);
      k->flag |= K_PRESS;
      mod_fg_all(laben, &red);
    }
  } else {
    send_fake_key_eve(keysym);
    int i;
    for(i=0;i<keysN;i++) {
      int j;
      for(j=0; keys[i][j].enkey; j++) {
        if (!(keys[i][j].flag & K_PRESS))
          continue;
        keys[i][j].flag &= ~K_PRESS;
        KeyCode kcj = XKeysymToKeycode(dpy, keys[i][j].keysym);
        XTestFakeKeyEvent(dpy, kcj, False, CurrentTime);
        mod_fg_all(keys[i][j].laben, NULL);
      }
    }

  }
}


static void create_win_kbm()
{
  gdk_color_parse("red", &red);

  gwin_kbm = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width (GTK_CONTAINER (gwin_kbm), 0);
  GtkWidget *hbox_top = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (gwin_kbm), hbox_top);


  GtkWidget *vbox_l = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox_top), vbox_l, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox_l), 0);
  GtkWidget *vbox_r = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox_top), vbox_r, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox_r), 0);

  int i;
  for(i=0;i<keysN;i++) {
    GtkWidget *hboxl = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hboxl), 0);
    gtk_box_pack_start (GTK_BOX (vbox_l), hboxl, FALSE, FALSE, 0);
    GtkWidget *hboxr = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hboxr), 0);
    gtk_box_pack_start (GTK_BOX (vbox_r), hboxr, FALSE, FALSE, 0);
    KEY *pk = keys[i];

    int j;
    for(j=0; pk[j].enkey; j++) {
      KEY *ppk=&pk[j];
      char flag=ppk->flag;
      if (!ppk->keysym)
        continue;
      GtkWidget *but=pk[j].but=gtk_button_new();
      g_signal_connect (G_OBJECT (but), "clicked", G_CALLBACK (cb_button_click), ppk);
      GtkWidget *hbox = (flag&K_AREA_R)?hboxr:hboxl;

      gtk_container_set_border_width (GTK_CONTAINER (but), 0);

      if (flag & K_FILL)
        gtk_box_pack_start (GTK_BOX (hbox), but, TRUE, TRUE, 0);
      else
        gtk_box_pack_start (GTK_BOX (hbox), but, FALSE, FALSE, 0);

      GtkWidget *v = gtk_vbox_new (FALSE, 0);
      gtk_container_set_border_width (GTK_CONTAINER (v), 0);
      gtk_container_add (GTK_CONTAINER (but), v);
      GtkWidget *laben = ppk->laben=gtk_label_new(ppk->enkey);
      set_label_font_size(laben, 8);
      gtk_box_pack_start (GTK_BOX (v), laben, FALSE, FALSE, 0);

      if (i>0&&i<5) {
        GtkWidget *lab = ppk->lab = gtk_label_new("  ");
        gtk_box_pack_start (GTK_BOX (v), lab, FALSE, FALSE, 0);
      }
    }
  }

  gtk_widget_realize (gwin_kbm);
  GdkWindow *gdkwin_kbm = gwin_kbm->window;
  set_no_focus(gwin_kbm);
}

extern GdkWindow *tray_da_win;

static void move_win_kbm()
{
  int width, height;
  get_win_size(gwin_kbm, &width, &height);

  int ox, oy, szx, szy;
  if (tray_da_win) {
    gdk_window_get_origin(tray_da_win, &ox, &oy);
    gdk_window_get_size(tray_da_win, &szx, &szy);
    oy -= height;
    if (oy + height > dpy_yl)
      oy = dpy_yl - height;
    if (oy < 0)
      oy = szy;

    if (ox + width > dpy_xl)
      ox = dpy_xl - width;
    if (ox < 0)
      ox = 0;
  } else {
    ox = dpy_xl - width;
    oy = dpy_yl - height;
  }

  gtk_window_move(GTK_WINDOW(gwin_kbm), ox, oy);
}

void show_win_kbm()
{
  if (!current_CS)
    return;

  if (!gwin_kbm) {
    create_win_kbm();
    update_win_kbm();
  }

  gtk_widget_show_all(gwin_kbm);
  move_win_kbm();
}

static char   shift_chars[]="~!@#$%^&*()_+{}|:\"<>?";
static char shift_chars_o[]="`1234567890-=[]\\;',./";


#include "pho.h"

static void set_kbm_key(KeySym keysym, char *str)
{
  int i;
  for(i=0;i<keysN;i++) {
    int j;
    for(j=0;j<COLN;j++) {
      char *p;
      if (keysym >='A' && keysym<='Z')
        keysym += 0x20;
      else
      if (p=strchr(shift_chars, keysym)) {
        keysym = shift_chars_o[p - shift_chars];
      }

      if (keys[i][j].keysym!=keysym)
        continue;

      GtkWidget *lab = keys[i][j].lab;
      char *t = gtk_label_get_text(lab);
      char tt[64];

      if (t && strcmp(t, str)) {
        strcat(strcpy(tt, t), str);
        str = tt;
      }

      gtk_label_set_text(GTK_LABEL(lab), str);
    }
  }
}

static void clear_kbm()
{
  int i;
  for(i=0;i<keysN;i++) {
    int j;
    for(j=0;j<COLN;j++) {
      GtkWidget *lab = keys[i][j].lab;
      if (lab)
        gtk_label_set_text(GTK_LABEL(lab), NULL);
    }
  }
}

void update_win_kbm()
{
  if (!current_CS)
    return;

  clear_kbm();

  int i;
  switch (current_CS->in_method) {
    case 3:
    case 6:
      for(i=0; i < 128; i++) {
        int j;
        char tt[64];
        int ttN=0;

        for(j=0;j<3; j++) {
          int num = phkbm.phokbm[i][j].num;
          int typ = phkbm.phokbm[i][j].typ;
          if (!num)
            continue;
          ttN+= utf8cpy(&tt[ttN], &pho_chars[typ][num * 3]);
        }

        if (!ttN)
          continue;

        set_kbm_key(i, tt);
      }
      break;
    case 10:
      break;
    default:
      if (!cur_inmd || !cur_inmd->DefChars)
        return;

      char *keyname = &cur_inmd->keyname[1 * CH_SZ];
      for(i=127; i > 0; i--) {
        char k=cur_inmd->keymap[i];
        if (!k)
          continue;

        char *keyname = &cur_inmd->keyname[k * CH_SZ];
        if (!keyname)
          continue;

        char tt[64];

        if (keyname[0] & 128)
          utf8cpy(tt, keyname);
        else {
          tt[1]=0;
          memcpy(tt, keyname, 2);
          tt[2]=0;
        }

        set_kbm_key(i, tt);
      }

      break;
  }

  gtk_window_resize(GTK_WINDOW(gwin_kbm), 10, 10);
  move_win_kbm();
}


void hide_win_kbm()
{
  if (!gwin_kbm)
    return;
  gtk_widget_hide(gwin_kbm);
}
