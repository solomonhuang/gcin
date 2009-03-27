#include "gcin.h"
#include <X11/extensions/XTest.h>


static GtkWidget *gwin_gtab;
static GtkWidget *label_gtab_sele;
static GtkWidget *button_gtab;
static GtkWidget *labels_gtab[MAX_TAB_KEY_NUM];
static GtkWidget *button_input_method_name;
static GtkWidget *frame;
static GtkWidget *label_key_codes;
static int max_tab_keyN=5;
static GtkWidget *image_pin;

Window xwin_gtab;

void disp_gtab(int index, char *gtabchar)
{
  GError *err = NULL;
  int rn, wn;
  char *utf8 = g_locale_to_utf8 (gtabchar, 2, &rn, &wn, &err);

  if (err) {
    printf("utf8 conver error");
    return;
  }

  gtk_label_set_text(GTK_LABEL(labels_gtab[index]), utf8);
  g_free(utf8);
}

void disp_gtab_sel(char *s)
{
  GError *err = NULL;
  int rn, wn;
  char *utf8 = g_locale_to_utf8 (s, strlen(s), &rn, &wn, &err);

  if (err) {
    printf("utf8 conver error");
    return;
  }

  gtk_label_set_text(GTK_LABEL(label_gtab_sele), utf8);
}


void set_key_codes_label(char *s)
{
  if (!label_key_codes)
    return;

  GError *err = NULL;
  int rn, wn;
  char *utf8 = g_locale_to_utf8 (s, strlen(s), &rn, &wn, &err);

  if (err) {
    printf("utf8 conver error");
    return;
  }

  gtk_label_set_text(GTK_LABEL(label_key_codes), utf8);
}


void move_win_gtab(int x, int y)
{
  if (current_IC && current_IC->fixed_pos) {
    x = current_IC->fixed_x;
    y = current_IC->fixed_y;
  }

  gtk_window_get_size(GTK_WINDOW(gwin_gtab), &win_xl, &win_yl);

  if (x + win_xl > dpy_xl)
    x = dpy_xl - win_xl;
  if (x < 0)
    x = 0;

  if (y + win_yl > dpy_yl)
    y = dpy_yl - win_yl;
  if (y < 0)
    y = 0;

  win_x = x;  win_y = y;
  gtk_window_move(GTK_WINDOW(gwin_gtab), x, y);
}

void set_gtab_input_method_name(char *s)
{
//  dbg("set_gtab_input_method_name '%s'\n", s);

  GError *err = NULL;
  int rn, wn;
  char *utf8 = g_locale_to_utf8 (s, strlen(s), &rn, &wn, &err);

  if (err) {
    printf("utf8 conver error");
    return;
  }

  gtk_button_set_label(GTK_BUTTON(button_input_method_name), utf8);
}


void create_win_gtab()
{
  if (gwin_gtab)
    return;

  gwin_gtab = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width (GTK_CONTAINER (gwin_gtab), 0);
  gtk_widget_realize (gwin_gtab);
  GdkWindow *gdkwin = gwin_gtab->window;

  gdk_window_set_override_redirect(gdkwin, TRUE);
  xwin_gtab = GDK_WINDOW_XWINDOW(gdkwin);
}



static void mouse_button_callback( GtkWidget *widget,GdkEventButton *event, gpointer data)
{
  int x=event->x, y=event->y;

//  dbg("mouse_button_callback %d\n", event->button);
  switch (event->button) {
    case 1:
      create_win_sym();
//      send_fake_key_eve();
      break;
    case 2:
      break;
    case 3:
#if DEBUG
      dbg("exec gcin\n");
#endif
      system("gcin-setup &");
      break;
  }

}

extern char file_pin_float[];

static void set_currenet_IC_pin_image_pin()
{
  if (current_IC->fixed_pos)
    gtk_image_set_from_file(GTK_IMAGE(image_pin), GCIN_ICON_DIR"/pin-fixed24.png");
  else
    gtk_image_set_from_file(GTK_IMAGE(image_pin), file_pin_float);
}

void get_win_gtab_geom();

static void cb_clicked_fixed_pos()
{

  if (!current_IC)
    return;

  current_IC->fixed_pos^=1;

//  dbg("cb_clicked_fixed_pos %d\n", current_IC->fixed_pos);

  if (current_IC->fixed_pos) {
    get_win_gtab_geom();
    current_IC->fixed_x = win_x;  current_IC->fixed_y = win_y;
  }

  set_currenet_IC_pin_image_pin();
}



void create_win_gtab_gui()
{
//  dbg("create_win_gtab_gui .....\n");
  if (frame)
    return;

  frame = gtk_frame_new(NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
  gtk_container_add (GTK_CONTAINER(gwin_gtab), frame);

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox_top);

  GtkWidget *align = gtk_alignment_new (0, 0, 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox_top), align, FALSE, FALSE, 0);

  label_gtab_sele = gtk_label_new(NULL);
  gtk_container_add (GTK_CONTAINER (align), label_gtab_sele);
  set_label_font_size(label_gtab_sele, gcin_font_size);


  GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
  /* This packs the button into the gwin_gtab (a gtk container). */
  gtk_container_add (GTK_CONTAINER (vbox_top), hbox);

  button_input_method_name = gtk_button_new_with_label(" ");
  gtk_box_pack_start (GTK_BOX (hbox), button_input_method_name, FALSE, FALSE, 0);

  button_gtab = gtk_button_new();
  gtk_container_set_border_width (GTK_CONTAINER (button_gtab), 0);
  gtk_box_pack_start (GTK_BOX (hbox), button_gtab, FALSE, FALSE, 0);

  g_signal_connect(G_OBJECT(button_gtab),"button-press-event",
                   G_CALLBACK(mouse_button_callback), NULL);

  if (left_right_button_tips) {
    GtkTooltips *button_gtab_tips = gtk_tooltips_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (button_gtab_tips), button_gtab, "左鍵符號，右鍵設定",NULL);
  }

  GtkWidget *hbox_gtab = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (button_gtab), hbox_gtab);

  int i;
  for(i=0; i < MAX_TAB_KEY_NUM; i++) {
    GtkWidget *label = gtk_label_new(NULL);
    labels_gtab[i] = label;
    gtk_box_pack_start (GTK_BOX (hbox_gtab), label, FALSE, FALSE, 0);
    set_label_font_size(label, gcin_font_size);
  }

  image_pin = gtk_image_new_from_file(file_pin_float);
  GtkWidget *event_box_pin = gtk_event_box_new();
  gtk_container_add (GTK_CONTAINER (event_box_pin), image_pin);
  gtk_box_pack_start (GTK_BOX (hbox), event_box_pin, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (event_box_pin), "button_press_event",
      G_CALLBACK (cb_clicked_fixed_pos), (gpointer) NULL);

  label_key_codes  = gtk_label_new(NULL);
  gtk_box_pack_start (GTK_BOX (hbox), label_key_codes, FALSE, FALSE, 2);

  gtk_widget_show_all (gwin_gtab);
}


void show_win_gtab()
{
//  dbg("show_win_gtab %d ..\n", current_IC->in_method);
  create_win_gtab();
  create_win_gtab_gui();
  set_currenet_IC_pin_image_pin();

  if (current_IC) {
    if (current_IC->fixed_pos)
      move_win_gtab(0,0);
  }


  init_gtab(current_IC->in_method, True);
  gtk_widget_show(gwin_gtab);
  show_win_sym();
}


void hide_win_gtab()
{
//  dbg("hide_win_gtab gwin_gtab\n", gwin_gtab);
  if (!gwin_gtab)
    return;

  gtk_widget_hide(gwin_gtab);
  hide_win_sym();
}

void minimize_win_gtab()
{
  gtk_window_resize(GTK_WINDOW(gwin_gtab), 10, 10);
}


void destroy_win_gtab()
{
  if (!gwin_gtab)
    return;

  gtk_widget_destroy(gwin_gtab);
}


void get_win_gtab_geom()
{
  if (!gwin_gtab)
    return;

  gtk_window_get_position(GTK_WINDOW(gwin_gtab), &win_x, &win_y);

  GtkRequisition sz;
  // the value of gtk_window_get_size is old
  gtk_widget_size_request(gwin_gtab, &sz);
  win_xl = sz.width;  win_yl = sz.height;
}
