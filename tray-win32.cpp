#include "gcin.h"
#include "pho.h"
#include "gtab.h"
#include "win-sym.h"
#include "eggtrayicon.h"
#if UNIX
#include <signal.h>
#else
#include <process.h>
#endif

gboolean tsin_pho_mode();
extern int tsin_half_full, gb_output;
extern int win32_tray_disabled;
GtkStatusIcon *icon_main, *icon_state;

void get_icon_path(char *iconame, char fname[]);

void cb_trad_sim_toggle();

#if WIN32
void cb_sim2trad(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
  win32exec("sim2trad");
}
void cb_trad2sim(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
  win32exec("trad2sim");
}
#else
void cb_sim2trad(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
  system(GCIN_BIN_DIR"/sim2trad &");
}

void cb_trad2sim(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
  system(GCIN_BIN_DIR"/trad2sim &");
}
#endif


void cb_tog_phospeak(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
  phonetic_speak= gtk_check_menu_item_get_active(checkmenuitem);
}

void close_all_clients();
void restart_gcin(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
#if WIN32
#if 0
  char execbin[256];
  strcpy(execbin, gcin_program_files_path);
  strcat(execbin, "\\gcin");
  close_all_clients();
  _execl(execbin, "gcin", NULL);
#else
  exit(0);
#endif
#else
  static char execbin[]=GCIN_BIN_DIR"/gcin";

  signal(SIGCHLD, SIG_IGN);

  int pid = fork();
  if (!pid) {
    close_all_clients();
    sleep(1);
    execl(execbin, "gcin", NULL);
  } else
    exit(0);
#endif
}


void gcb_main();
void cb_tog_gcb(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
#if USE_GCB
  gcb_enabled = gtk_check_menu_item_get_active(checkmenuitem);
//  dbg("gcb_enabled %d\n", gcb_enabled);
  gcb_main();
#endif
}


void kbm_toggle(), exec_gcin_setup(), restart_gcin(), cb_trad2sim(), cb_sim2trad();

void cb_trad2sim(GtkCheckMenuItem *checkmenuitem, gpointer dat);

void cb_trad_sim_toggle_(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
  cb_trad_sim_toggle();
  dbg("checkmenuitem %x\n", checkmenuitem);
}

void exec_gcin_setup_(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
  exec_gcin_setup();
}

void kbm_toggle_(GtkCheckMenuItem *checkmenuitem, gpointer dat)
{
  kbm_toggle();
}

gint inmd_switch_popup_handler (GtkWidget *widget, GdkEvent *event);
extern gboolean win_kbm_inited;

#include "mitem.h"
extern int win_kbm_on;

static MITEM mitems_main[] = {
  {N_(_L("設定")), GTK_STOCK_PREFERENCES, exec_gcin_setup_},
  {N_(_L("重新執行gcin")), NULL, restart_gcin},
  {N_(_L("念出發音")), NULL, cb_tog_phospeak, &phonetic_speak},
  {N_(_L("小鍵盤")), NULL, kbm_toggle_, &win_kbm_on},
#if USE_GCB
  {N_(_L("gcb(剪貼區暫存)")), NULL, cb_tog_gcb, &gcb_enabled},
#endif
  {NULL}
};


static MITEM mitems_state[] = {
  {N_(_L("正->簡體")), NULL, cb_trad2sim},
  {N_(_L("簡->正體")), NULL, cb_sim2trad},
  {N_(_L("简体输出")), NULL, cb_trad_sim_toggle_, &gb_output},
  {NULL}
};


static GtkWidget *tray_menu, *tray_menu_state;


gint inmd_switch_popup_handler (GtkWidget *widget, GdkEvent *event);
extern gboolean win_kbm_inited;


void toggle_im_enabled();
GtkWidget *create_tray_menu(MITEM *mitems)
{
  GtkWidget *menu = gtk_menu_new ();

  int i;
  for(i=0; mitems[i].name; i++) {
    GtkWidget *item;

    if (mitems[i].stock_id) {
      item = gtk_image_menu_item_new_with_label (_(mitems[i].name));
      gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), gtk_image_new_from_stock(mitems[i].stock_id, GTK_ICON_SIZE_MENU));
    }
    else
    if (mitems[i].check_dat) {
      item = gtk_check_menu_item_new_with_label (_(mitems[i].name));
      gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), *mitems[i].check_dat);
    } else
      item = gtk_menu_item_new_with_label (_(mitems[i].name));

    g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (mitems[i].cb), NULL);

    gtk_widget_show(item);

    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  }

  return menu;
}

void inmd_popup_tray();

static void cb_activate(GtkStatusIcon *status_icon, gpointer user_data)
{
#if UNIX
  dbg("cb_activate\n");
  toggle_im_enabled();

  GdkRectangle rect;
  bzero(&rect, sizeof(rect));
  GtkOrientation ori;
  gtk_status_icon_get_geometry(status_icon, NULL, &rect, &ori);
#else
  inmd_popup_tray();
#endif
}

static void cb_popup(GtkStatusIcon *status_icon, guint button, guint activate_time, gpointer user_data)
{
  dbg("cb_popup\n");
  switch (button) {
#if UNIX
    case 1:
      toggle_im_enabled();

      break;
    case 2:
      kbm_toggle();
      break;
#endif
    case 3:
      if (!tray_menu)
        tray_menu = create_tray_menu(mitems_main);
#if 0
      gtk_menu_popup(GTK_MENU(tray_menu), NULL, NULL, gtk_status_icon_position_menu, status_icon, button, activate_time);
#else
	  gtk_menu_popup(GTK_MENU(tray_menu), NULL, NULL, NULL, NULL, button, activate_time);
#endif
      break;
  }
}

void toggle_half_full_char();
static void cb_activate_state(GtkStatusIcon *status_icon, gpointer user_data)
{
  dbg("cb_activate\n");
  toggle_half_full_char();
}


static void cb_popup_state(GtkStatusIcon *status_icon, guint button, guint activate_time, gpointer user_data)
{
  dbg("cb_popup_state\n");

  switch (button) {

    case 1:
#if UNIX
      toggle_im_enabled();
      break;
    case 2:
#endif
      kbm_toggle();
      break;
    case 3:
      if (!tray_menu_state)
        tray_menu_state = create_tray_menu(mitems_state);

      gtk_menu_popup(GTK_MENU(tray_menu_state), NULL, NULL, NULL, NULL, button, activate_time);
      break;
  }
}


#define GCIN_TRAY_PNG "gcin-tray.png"


void load_tray_icon_win32()
{
#if UNIX
  if (!gcin_win32_icon)
    return;
#endif

#if WIN32
  // when login, creating icon too early may cause block in gtk_status_icon_new_from_file
  if (win32_tray_disabled)
	  return;
#endif

  dbg("load_tray_icon_win32\n");

  char *iconame;
  if (!current_CS || current_CS->im_state == GCIN_STATE_DISABLED||current_CS->im_state == GCIN_STATE_ENG_FULL) {
    iconame=GCIN_TRAY_PNG;
  } else {
    iconame=inmd[current_CS->in_method].icon;
  }

  char tt[32];
  if (current_CS && (current_CS->in_method==6 || current_CS->in_method==12) &&current_CS->im_state == GCIN_STATE_CHINESE && !tsin_pho_mode()) {
    strcpy(tt, "en-");
    strcat(tt, iconame);
    iconame = tt;
  }

  dbg("iconame %s\n", iconame);
  char fname[128];
  fname[0]=0;
  if (iconame)
    get_icon_path(iconame, fname);


  char *icon_st=NULL;
  char fname_state[128];
  if (current_CS && (current_CS->im_state == GCIN_STATE_ENG_FULL || current_CS->b_half_full_char ||
      current_CS->in_method==6 && tsin_half_full)) {
      if (gb_output)
        icon_st="full-simp.png";
      else
        icon_st="full-trad.png";
  } else {
      if (gb_output)
        icon_st="half-simp.png";
      else
        icon_st="half-trad.png";
  }
  get_icon_path(icon_st, fname_state);
  dbg("wwwwwwww %s\n", fname_state);


  if (icon_main) {
    dbg("set %s %s\n", fname, fname_state);
    gtk_status_icon_set_from_file(icon_main, fname);
    gtk_status_icon_set_from_file(icon_state, fname_state);
  }
  else {
    dbg("gtk_status_icon_new_from_file a\n");
    icon_main = gtk_status_icon_new_from_file(fname);
    g_signal_connect(G_OBJECT(icon_main),"activate", G_CALLBACK (cb_activate), NULL);
    g_signal_connect(G_OBJECT(icon_main),"popup-menu", G_CALLBACK (cb_popup), NULL);

	dbg("gtk_status_icon_new_from_file b\n");
    icon_state = gtk_status_icon_new_from_file(fname_state);
    g_signal_connect(G_OBJECT(icon_state),"activate", G_CALLBACK (cb_activate_state), NULL);
    g_signal_connect(G_OBJECT(icon_state),"popup-menu", G_CALLBACK (cb_popup_state), NULL);

	dbg("icon %s %s\n", fname, fname_state);
  }

  return;
}



void init_tray_win32()
{
#if UNIX
  if (!gcin_win32_icon)
    return;
  load_tray_icon_win32();
#else
  load_tray_icon_win32();
#endif
}

#if WIN32
void update_tray_icon() {}
#endif
