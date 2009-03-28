#include "gcin.h"
#include "pho.h"
#include "gtab.h"
#include "win-sym.h"
#include "eggtrayicon.h"
#include <signal.h>

static GdkPixbuf *pixbuf, *pixbuf_ch;
static PangoLayout* pango;
static GtkWidget *da;
static GdkGC *gc;
GdkWindow *tray_da_win;

#define GCIN_TRAY_PNG "gcin-tray.png"
static char gcin_icon[]=GCIN_ICON_DIR"/"GCIN_TRAY_PNG;
static char *pixbuf_ch_fname;
void exec_gcin_setup();

void toggle_gb_output();
extern gboolean gb_output;

static void get_text_w_h(char *s, int *w, int *h)
{
  pango_layout_set_text(pango, s, strlen(s));
  pango_layout_get_pixel_size(pango, w, h);
}

#if USE_TSIN
extern gboolean eng_ph, tsin_half_full;
#endif

static void draw_icon()
{
#if 0
  return;
#endif
  if (!da)
    return;

  GdkPixbuf *pix =  !current_CS ||
    (current_CS->im_state == GCIN_STATE_DISABLED||current_CS->im_state == GCIN_STATE_ENG_FULL) ?
    pixbuf : pixbuf_ch;

  int dw = da->allocation.width, dh = da->allocation.height;
  int w, h;

  GdkColor color_fg;

  gdk_color_parse("black", &color_fg);
  gdk_gc_set_rgb_fg_color(gc, &color_fg);

  if (pix) {
    int ofs = (dh - gdk_pixbuf_get_height (pix))/2;
    gdk_draw_pixbuf(tray_da_win, NULL, pix, 0, 0, 0, ofs, -1, -1, GDK_RGB_DITHER_NORMAL, 0, 0);
  } else {
    get_text_w_h(inmd[current_CS->in_method].cname, &w, &h);
    gdk_draw_layout(tray_da_win, gc, 0, 0, pango);
  }

  if (current_CS) {
    if (current_CS->b_half_full_char || current_CS->in_method==6 && tsin_half_full && current_CS->im_state == GCIN_STATE_CHINESE) {
      static char full[] = "全";
      get_text_w_h(full,  &w, &h);
      gdk_draw_layout(tray_da_win, gc, dw - w, dh - h, pango);
    }

    if (current_CS->im_state == GCIN_STATE_ENG_FULL) {
      static char efull[] = "A全";
      get_text_w_h(efull,  &w, &h);
      gdk_draw_layout(tray_da_win, gc, 0, 0, pango);
    }
#if USE_TSIN
    if ((current_CS->in_method==6||current_CS->in_method==12) && current_CS->im_state == GCIN_STATE_CHINESE && !eng_ph) {
      static char efull[] = "ABC";
      gdk_color_parse("blue", &color_fg);
      gdk_gc_set_rgb_fg_color(gc, &color_fg);

      get_text_w_h(efull,  &w, &h);
      gdk_draw_layout(tray_da_win, gc, 0, 0, pango);
    }
#endif
  }

  gdk_color_parse("red", &color_fg);
  gdk_gc_set_rgb_fg_color(gc, &color_fg);

  if (gb_output) {
    static char sim[] = "简";
    get_text_w_h(sim,  &w, &h);
    gdk_draw_layout(tray_da_win, gc, 0, dh - h, pango);
  }

}

gboolean create_tray();
void update_tray_icon()
{
  if (!gcin_status_tray)
    return;

  if (!da)
    create_tray();

  gtk_widget_queue_draw(da);
}

void get_icon_path(char *iconame, char fname[]);

void load_tray_icon()
{
  if (!gcin_status_tray)
    return;

  if (!da)
    create_tray();

  char *iconame = inmd[current_CS->in_method].icon;
  char fname[512];

  fname[0]=0;

  if (iconame)
    get_icon_path(iconame, fname);

#if 0
  dbg("fname %x %s\n", fname, fname);
#endif
  if (!fname[0]) {
    if (pixbuf_ch)
      gdk_pixbuf_unref(pixbuf_ch);

    pixbuf_ch = NULL;
    if (pixbuf_ch_fname)
      pixbuf_ch_fname[0] = 0;
  } else
  if (!pixbuf_ch_fname || strcmp(fname, pixbuf_ch_fname)) {
    free(pixbuf_ch_fname);
    pixbuf_ch_fname = strdup(fname);

    if (pixbuf_ch)
      gdk_pixbuf_unref(pixbuf_ch);

    GError *err = NULL;
    pixbuf_ch = gdk_pixbuf_new_from_file(fname, &err);
  }

  update_tray_icon();
}


void cb_trad_sim_toggle();

static void cb_sim2trad()
{
  system(GCIN_BIN_DIR"/sim2trad &");
}


static void cb_trad2sim()
{
  system(GCIN_BIN_DIR"/trad2sim &");
}


static void cb_tog_phospeak(GtkCheckMenuItem *checkmenuitem, gpointer *dat)
{
  phonetic_speak= gtk_check_menu_item_get_active(checkmenuitem);
}


void restart_gcin()
{
  static char execbin[]=GCIN_BIN_DIR"/gcin";

  signal(SIGCHLD, SIG_IGN);

  int pid = fork();
  if (!pid) {
    close_all_clients();
    sleep(1);
    execl(execbin, "gcin", NULL);
  } else
    exit(0);
}


void kbm_toggle();

struct {
  char *name;
  char *stock_id;
  void (*cb)();
  int *check_dat;
} mitems[] = {
  {N_("設定"), GTK_STOCK_PREFERENCES, exec_gcin_setup},
  {N_("重新執行gcin"), NULL, restart_gcin},
  {N_("念出發音"), NULL, cb_tog_phospeak, &phonetic_speak},
  {N_("正->簡體"), NULL, cb_trad2sim},
  {N_("簡->正體"), NULL, cb_sim2trad},
  {N_("小鍵盤"), NULL, kbm_toggle},
};



static GtkWidget *tray_menu;

static void create_menu()
{
  tray_menu = gtk_menu_new ();

  int i;
  for(i=0; i < sizeof(mitems)/ sizeof(mitems[0]); i++) {
    GtkWidget *item;

    if (mitems[i].stock_id) {
      item = gtk_image_menu_item_new_with_label (_(mitems[i].name));
      gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(item), gtk_image_new_from_stock(mitems[i].stock_id, GTK_ICON_SIZE_MENU));
    }
    else
    if (mitems[i].check_dat) {
      item = gtk_check_menu_item_new_with_label (_(mitems[i].name));
      gtk_check_menu_item_set_active(item, *mitems[i].check_dat);
    }
    else
      item = gtk_menu_item_new_with_label (_(mitems[i].name));

    g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (mitems[i].cb), NULL);

    gtk_widget_show(item);

    gtk_menu_shell_append (GTK_MENU_SHELL (tray_menu), item);
  }

  return;
}

gint inmd_switch_popup_handler (GtkWidget *widget, GdkEvent *event);
extern gboolean win_kbm_inited;

gboolean
tray_button_press_event_cb (GtkWidget * button, GdkEventButton * event, gpointer userdata)
{
  switch (event->button) {
    case 1:
      cb_trad_sim_toggle(NULL);
      break;
    case 2:
#if 0
      inmd_switch_popup_handler(NULL, (GdkEvent *)event);
#else
      kbm_toggle();
#endif
      break;
    case 3:
      if (!tray_menu)
        create_menu();

      gtk_menu_popup(GTK_MENU(tray_menu), NULL, NULL, NULL, NULL,
         event->button, event->time);
      break;
  }

  return TRUE;
}

gboolean cb_expose(GtkWidget *da, GdkEventExpose *event, gpointer data)
{
  if (!da)
    create_tray();

  draw_icon();
  return FALSE;
}


gboolean create_tray()
{
  if (da)
    return;

  EggTrayIcon *tray_icon = egg_tray_icon_new ("gcin");

  if (!tray_icon)
    return;

  GtkWidget *event_box = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (tray_icon), event_box);
  GtkTooltips *tips = gtk_tooltips_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), event_box, _("左:正/簡體 中:小鍵盤 右:選項"), NULL);

  g_signal_connect (G_OBJECT (event_box), "button-press-event",
                    G_CALLBACK (tray_button_press_event_cb), NULL);

  GError *err = NULL;

  char icon_fname[128];
  get_icon_path(GCIN_TRAY_PNG, icon_fname);

  if (pixbuf)
    gdk_pixbuf_unref(pixbuf);
  pixbuf = gdk_pixbuf_new_from_file(icon_fname, &err);

  int pwidth = gdk_pixbuf_get_width (pixbuf);
  int pheight = gdk_pixbuf_get_height (pixbuf);

  if (!pixbuf)
    p_err("cannot load file %s", icon_fname);

  da =  gtk_drawing_area_new();
  g_signal_connect (G_OBJECT (event_box), "destroy",
                    G_CALLBACK (gtk_widget_destroyed), &da);
  g_signal_connect(G_OBJECT(da), "expose-event", G_CALLBACK(cb_expose), NULL);

  gtk_container_add (GTK_CONTAINER (event_box), da);
  gtk_widget_set_size_request(GTK_WIDGET(tray_icon), pwidth, pheight);

  gtk_widget_show_all (GTK_WIDGET (tray_icon));
  tray_da_win = da->window;
  // tray window is not ready ??
  if (!tray_da_win || !GTK_WIDGET_DRAWABLE(da)) {
    gtk_widget_destroy(tray_icon);
    da = NULL;
    return;
  }

#if 0
  Window xwin = GDK_WINDOW_XID(tray_da_win);
  XWindowAttributes att;
  XGetWindowAttributes(dpy, xwin, &att);
  dbg("www %d %d\n", att.width, att.height);
#endif

  PangoContext *context=gtk_widget_get_pango_context(da);
  PangoFontDescription* desc=pango_context_get_font_description(context);

//  dbg("zz %s %d\n",  pango_font_description_to_string(desc), PANGO_SCALE);

  pango = gtk_widget_create_pango_layout(da, NULL);
  pango_layout_set_font_description(pango, desc);
#if 1
  // strange bug, why do we need this ?
  desc = pango_layout_get_font_description(pango);
#endif
  pango_font_description_set_size(desc, 9 * PANGO_SCALE);

#if 0
  dbg("aa %s\n",  pango_font_description_to_string(desc));
  dbg("context %x %x\n", pango_layout_get_context(pango), context);
  dbg("font %x %x\n",pango_layout_get_font_description(pango), desc);
#endif
  gc = gdk_gc_new (tray_da_win);
  return FALSE;
}

void init_tray()
{
  g_timeout_add(5000, create_tray, NULL);
}
