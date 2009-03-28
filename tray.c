#include "gcin.h"
#include "pho.h"
#include "gtab.h"
#include "win-sym.h"
#include "eggtrayicon.h"

static GdkPixbuf *pixbuf, *pixbuf_ch;
static PangoLayout* pango;
static GtkWidget *da;
static GdkGC *gc;
GdkWindow *tray_da_win;

#define GCIN_TRAY_PNG "gcin-tray.png"
static char gcin_icon[]=GCIN_ICON_DIR"/"GCIN_TRAY_PNG;
static char pixbuf_ch_fname[128];
void exec_gcin_setup();

void toggle_gb_output();
extern gboolean gb_output;

static void get_text_w_h(char *s, int *w, int *h)
{
  pango_layout_set_text(pango, s, strlen(s));
  pango_layout_get_pixel_size(pango, w, h);
}


static void draw_icon()
{

  GdkPixbuf *pix =  !current_CS ||
    (current_CS->im_state == GCIN_STATE_DISABLED||current_CS->im_state == GCIN_STATE_ENG_FULL) ?
    pixbuf : pixbuf_ch;

  int dw = da->allocation.width, dh = da->allocation.height;
  int w, h;

  GdkColor color_fg;

  gdk_color_parse("black", &color_fg);
  gdk_gc_set_rgb_fg_color(gc, &color_fg);

  if (pix)
    gdk_draw_pixbuf(tray_da_win, NULL, pix, 0, 0, 0, 0, -1, -1, GDK_RGB_DITHER_NORMAL, 0, 0);
  else {
    get_text_w_h(inmd[current_CS->in_method].cname, &w, &h);
    gdk_draw_layout(tray_da_win, gc, 0, 0, pango);
  }

  if (current_CS) {
    if (current_CS->b_half_full_char) {
      static char full[] = "全";

      get_text_w_h(full,  &w, &h);
      gdk_draw_layout(tray_da_win, gc, dw - w, dh - h, pango);
    }

    if (current_CS->im_state == GCIN_STATE_ENG_FULL) {
      static char efull[] = "英全";

      get_text_w_h(efull,  &w, &h);
      gdk_draw_layout(tray_da_win, gc, 0, 0, pango);
    }
  }

  gdk_color_parse("red", &color_fg);
  gdk_gc_set_rgb_fg_color(gc, &color_fg);

  if (gb_output) {
    static char sim[] = "简";
    get_text_w_h(sim,  &w, &h);
    gdk_draw_layout(tray_da_win, gc, 0, dh - h, pango);
  }
}

void update_tray_icon()
{
  gtk_widget_queue_draw(da);
}


void load_tray_icon()
{
  char *iconame = inmd[current_CS->in_method].icon;
  char *fname = NULL;
  char tt[128];

  if (iconame) {
    get_gcin_user_fname(iconame, tt);

    fname = tt;
    if (access(tt, R_OK)) {
      sprintf(tt, GCIN_ICON_DIR"/%s", iconame);
      if (access(tt, R_OK))
        fname = NULL;
    }
  }

#if 0
  dbg("fname %x %s\n", fname, fname);
#endif
  if (!fname) {
    if (pixbuf_ch)
      gdk_pixbuf_unref(pixbuf_ch);

    pixbuf_ch = NULL;
    pixbuf_ch_fname[0] = 0;
  } else
  if (strcmp(fname, pixbuf_ch_fname)) {
    strcpy(pixbuf_ch_fname, fname);

    if (pixbuf_ch)
      gdk_pixbuf_unref(pixbuf_ch);

    GError *err = NULL;
    pixbuf_ch = gdk_pixbuf_new_from_file(fname, &err);
  }

  update_tray_icon();
}


void cb_trad_sim_toggle()
{
  toggle_gb_output();
  update_tray_icon();
}

static void cb_sim2trad()
{
  system(GCIN_BIN_DIR"/sim2trad &");
}


static void cb_trad2sim()
{
  system(GCIN_BIN_DIR"/trad2sim &");
}

struct {
  char *name;
  char *stock_id;
  void (*cb)();
} mitems[] = {
  {"設定", GTK_STOCK_PREFERENCES, exec_gcin_setup},
  {"正->簡體", NULL, cb_trad2sim},
  {"簡->正體", NULL, cb_sim2trad},
};


GtkWidget *menu;

static void create_menu()
{
  menu = gtk_menu_new ();

  int i;
  for(i=0; i < sizeof(mitems)/ sizeof(mitems[0]); i++) {
    GtkWidget *item;

    if (mitems[i].stock_id) {
      item = gtk_image_menu_item_new_with_label (mitems[i].name);
      gtk_image_menu_item_set_image(item, gtk_image_new_from_stock(mitems[i].stock_id, GTK_ICON_SIZE_MENU));
    }
    else
      item = gtk_menu_item_new_with_label (mitems[i].name);

    g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (mitems[i].cb), NULL);

    gtk_widget_show(item);

    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  }

  return;
}

gint inmd_switch_popup_handler (GtkWidget *widget, GdkEvent *event);

gboolean
tray_button_press_event_cb (GtkWidget * button, GdkEventButton * event, gpointer userdata)
{
  switch (event->button) {
    case 1:
      cb_trad_sim_toggle(NULL);
      break;
    case 2:
      inmd_switch_popup_handler(NULL, (GdkEvent *)event);
      break;
    case 3:
      if (!menu)
        create_menu();

      gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
         event->button, event->time);
      break;
  }

  return TRUE;
}

gboolean cb_expose(GtkWidget *da, GdkEventExpose *event, gpointer data)
{
  draw_icon();
}


void create_tray()
{
  EggTrayIcon *tray_icon = egg_tray_icon_new ("gcin");
  GtkWidget *event_box = gtk_event_box_new ();

  GtkTooltips *tips = gtk_tooltips_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), event_box, "左:正/簡體 中:輸入法 右:選項", NULL);

  gtk_container_add (GTK_CONTAINER (tray_icon), event_box);

  g_signal_connect (G_OBJECT (event_box), "button-press-event",
                    G_CALLBACK (tray_button_press_event_cb), NULL);

  GError *err = NULL;


  char tt[128];
  char *icon_fname = gcin_icon;
  get_gcin_user_fname(GCIN_TRAY_PNG, tt);

  if (access(tt, R_OK)==0)
   icon_fname = tt;

  pixbuf = gdk_pixbuf_new_from_file(icon_fname, &err);
  int pwidth = gdk_pixbuf_get_width (pixbuf);
  int pheight = gdk_pixbuf_get_height (pixbuf);

  if (!pixbuf)
    p_err("cannot load file");

  da =  gtk_drawing_area_new();

  g_signal_connect(G_OBJECT(da), "expose-event", G_CALLBACK(cb_expose), NULL);

  gtk_container_add (GTK_CONTAINER (event_box), da);

  gtk_widget_set_size_request(tray_icon, pwidth, pheight);

  pango = gtk_widget_create_pango_layout(da, " ");

  PangoContext *context=gtk_widget_get_pango_context(da);
  PangoFontDescription* desc=pango_context_get_font_description(context);
#if GTK_MAJOR_VERSION >=2 && GTK_MINOR_VERSION >= 10
  pango_font_description_set_size(desc, 12);
#endif
  pango_layout_set_font_description(pango, desc);
  // older pango need this
#if GTK_MAJOR_VERSION >=2 && GTK_MINOR_VERSION < 10
  pango_font_description_set_size(desc, 12);
#endif

  gtk_widget_show_all (GTK_WIDGET (tray_icon));

  tray_da_win = da->window;

  gc = gdk_gc_new (tray_da_win);
}
