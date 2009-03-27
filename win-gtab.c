#include "gcin.h"
#include <X11/extensions/XTest.h>


static GtkWidget *gwin_gtab, *frame;
static GtkWidget *label_gtab_sele;
static GtkWidget *label_half_full;
static GtkWidget *button_gtab;
static GtkWidget *labels_gtab[MAX_TAB_KEY_NUM];
static GtkWidget *button_input_method_name;
static GtkWidget *label_key_codes;
static GtkWidget *image_pin;

Window xwin_gtab;
void set_label_space(GtkWidget *label);

void disp_gtab(int index, char *gtabchar)
{
  char utf8[512];

  if (gtabchar[0]==' ') {
    set_label_space(labels_gtab[index]);
  }
  else {
    if (gtabchar[0] & 128)
      utf8cpy(utf8, gtabchar);
    else {
      memcpy(utf8, gtabchar, 2);
      utf8[2]=0;
    }

    gtk_label_set_text(GTK_LABEL(labels_gtab[index]), utf8);
  }

}

char *str_half_full[]={"半","全"};

void disp_gtab_half_full(gboolean hf)
{
  if (!label_half_full)
    return;

  gtk_label_set_text(GTK_LABEL(label_half_full), str_half_full[hf]);
}

void change_gtab_font_size()
{
  if (!label_gtab_sele)
    return;

  set_label_font_size(label_gtab_sele, gcin_font_size);

  int i;
  for(i=0; i < MAX_TAB_KEY_NUM; i++) {
    set_label_font_size(labels_gtab[i], gcin_font_size);
  }
}


void disp_gtab_sel(char *s)
{
  if (!label_gtab_sele)
    return;
#if 0
  gtk_label_set_text(GTK_LABEL(label_gtab_sele), s);
#else
//  dbg("jjj %s\n", s);
  gtk_label_set_markup(GTK_LABEL(label_gtab_sele), s);
#endif
}


void set_key_codes_label(char *s)
{
  if (!label_key_codes)
    return;

  gtk_label_set_text(GTK_LABEL(label_key_codes), s);
}

void show_win_sym();

void move_win_gtab(int x, int y)
{
  if (current_CS && current_CS->fixed_pos) {
    x = current_CS->fixed_x;
    y = current_CS->fixed_y;
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

  show_win_sym();
}

void set_gtab_input_method_name(char *s)
{
//  dbg("set_gtab_input_method_name '%s'\n", s);
  gtk_button_set_label(GTK_BUTTON(button_input_method_name), s);
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

void create_win_sym();

static void mouse_button_callback( GtkWidget *widget,GdkEventButton *event, gpointer data)
{
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
      system(GCIN_BIN_DIR"/gcin-setup &");
      break;
  }

}

extern char file_pin_float[];

static void set_currenet_IC_pin_image_pin()
{
  if (current_CS->fixed_pos)
    gtk_image_set_from_file(GTK_IMAGE(image_pin), GCIN_ICON_DIR"/pin-fixed24.png");
  else
    gtk_image_set_from_file(GTK_IMAGE(image_pin), file_pin_float);
}

void get_win_gtab_geom();

static void cb_clicked_fixed_pos()
{

  if (!current_CS)
    return;

  current_CS->fixed_pos^=1;

//  dbg("cb_clicked_fixed_pos %d\n", current_CS->fixed_pos);

  if (current_CS->fixed_pos) {
    get_win_gtab_geom();
    current_CS->fixed_x = win_x;  current_CS->fixed_y = win_y;
  }

  set_currenet_IC_pin_image_pin();
}


void toggle_half_full_char();

static void cb_half_bull()
{
  toggle_half_full_char();
}

gint inmd_switch_popup_handler (GtkWidget *widget, GdkEvent *event);

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

  GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
  /* This packs the button into the gwin_gtab (a gtk container). */
  gtk_container_add (GTK_CONTAINER (vbox_top), hbox);

  button_input_method_name = gtk_button_new_with_label("");
  g_signal_connect_swapped (GTK_OBJECT (button_input_method_name), "button_press_event",
        G_CALLBACK (inmd_switch_popup_handler), NULL);
  gtk_container_set_border_width (GTK_CONTAINER (button_input_method_name), 0);
  gtk_box_pack_start (GTK_BOX (hbox), button_input_method_name, FALSE, FALSE, 0);

  GtkWidget *button_half_full = gtk_button_new();
  gtk_container_set_border_width (GTK_CONTAINER (button_half_full), 0);
  label_half_full = gtk_label_new(NULL);
  gtk_container_add (GTK_CONTAINER (button_half_full), label_half_full);
  gtk_box_pack_start (GTK_BOX (hbox), button_half_full, FALSE, FALSE, 0);
  disp_gtab_half_full(current_CS->b_half_full_char);
  g_signal_connect (G_OBJECT (button_half_full), "clicked",
      G_CALLBACK (cb_half_bull), (gpointer) NULL);


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
  }

  image_pin = gtk_image_new_from_file(file_pin_float);
  GtkWidget *event_box_pin = gtk_event_box_new();
  gtk_container_add (GTK_CONTAINER (event_box_pin), image_pin);
  gtk_box_pack_start (GTK_BOX (hbox), event_box_pin, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (event_box_pin), "button_press_event",
      G_CALLBACK (cb_clicked_fixed_pos), (gpointer) NULL);

  label_key_codes  = gtk_label_new(NULL);
  gtk_box_pack_start (GTK_BOX (hbox), label_key_codes, FALSE, FALSE, 2);

  change_gtab_font_size();

  gtk_widget_show_all (gwin_gtab);
}


gboolean init_gtab(int inmdno, int usenow);

void show_win_gtab()
{
//  dbg("show_win_gtab %d ..\n", current_CS->in_method);
  create_win_gtab();
  create_win_gtab_gui();
  set_currenet_IC_pin_image_pin();

  if (current_CS) {
    if (current_CS->fixed_pos)
      move_win_gtab(0,0);
  }

  init_gtab(current_CS->in_method, True);
  gtk_widget_show(gwin_gtab);
  show_win_sym();
}


void hide_win_sym();

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
