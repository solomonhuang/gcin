#include "gcin.h"

static int current_pho_simple_win;
static int current_gcin_inner_frame;
static int current_pho_in_row1;

static GtkWidget *gwin_pho;
static GtkWidget *top_bin, *hbox_row2;
Window xwin_pho;
static GtkWidget *label_pho_sele;
static GtkWidget *labels_pho[4];
static GtkWidget *label_full;
static GtkWidget *label_key_codes;

void change_pho_font_size();


void disp_pho(int index, char *phochar)
{
  char tt[CH_SZ+1];

  if (phochar[0]==' ')
    strcpy(tt, "　");
  else
    utf8cpy(tt, phochar);

  gtk_label_set_text(GTK_LABEL(labels_pho[index]), tt);
}


void move_win_pho(int x, int y);

void get_win_size(GtkWidget *win, int *width, int *height)
{
  GtkRequisition sz;
  gtk_widget_size_request(GTK_WIDGET(win), &sz);
  *width = sz.width;
  *height = sz.height;
}

gboolean win_size_exceed(GtkWidget *win)
{
  int width, height;

  get_win_size(win, &width, &height);

  return (width + current_in_win_x > dpy_xl);
}


void disp_pho_sel(char *s)
{
  gtk_label_set_text(GTK_LABEL(label_pho_sele), s);

  if (win_size_exceed(gwin_pho)) {
    move_win_pho(current_in_win_x, current_in_win_y);
  }
}


void set_key_codes_label_pho(char *s)
{
  if (!label_key_codes)
    return;

  gtk_label_set_text(GTK_LABEL(label_key_codes), s);
}


void show_win_sym();

void move_win_pho(int x, int y)
{
  int twin_xl, twin_yl;

  win_x = x;  win_y = y;

  get_win_size(gwin_pho, &twin_xl, &twin_yl);

  if (x + twin_xl > dpy_xl)
    x = dpy_xl - twin_xl;
  if (x < 0)
    x = 0;

  if (y + twin_yl > dpy_yl)
    y = dpy_yl - twin_yl;
  if (y < 0)
    y = 0;

  gtk_window_move(GTK_WINDOW(gwin_pho), x, y);
  show_win_sym();
}

void minimize_win_pho()
{
  gtk_window_resize(GTK_WINDOW(gwin_pho), 10, 10);
}


void create_win_pho()
{
  if (gwin_pho)
    return;

  gwin_pho = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width (GTK_CONTAINER (gwin_pho), 0);
  gtk_widget_realize (gwin_pho);
  GdkWindow *gdkwin = gwin_pho->window;
  xwin_pho = GDK_WINDOW_XWINDOW(gdkwin);
  gdk_window_set_override_redirect(gdkwin, TRUE);
}

void create_win_sym(), exec_gcin_setup();

static void mouse_button_callback( GtkWidget *widget,GdkEventButton *event, gpointer data)
{
//  dbg("mouse_button_callback %d\n", event->button);
  switch (event->button) {
    case 1:
      create_win_sym();
//      send_fake_key_eve();
      break;
    case 2:
      inmd_switch_popup_handler(widget, (GdkEvent *)event);
      break;
    case 3:
      exec_gcin_setup();
      break;
  }

}


void create_win_pho_gui_full()
{
//  dbg("create_win_pho .....\n");

  if (top_bin)
    return;

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 0);

  if (gcin_inner_frame) {
    GtkWidget *frame = top_bin = gtk_frame_new(NULL);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
    gtk_container_add (GTK_CONTAINER(gwin_pho), frame);
    gtk_container_add (GTK_CONTAINER (frame), vbox_top);
  } else {
    gtk_container_add (GTK_CONTAINER(gwin_pho), vbox_top);
    top_bin = vbox_top;
  }

  GtkWidget *align = gtk_alignment_new (0, 0, 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox_top), align, FALSE, FALSE, 0);

  label_pho_sele = gtk_label_new(NULL);
  gtk_container_add (GTK_CONTAINER (align), label_pho_sele);
  set_label_font_size(label_pho_sele, gcin_font_size);


  hbox_row2 = gtk_hbox_new (FALSE, 0);
  /* This packs the button into the gwin_pho (a gtk container). */
  gtk_container_add (GTK_CONTAINER (vbox_top), hbox_row2);

  label_full = gtk_label_new("全");
  gtk_container_add (GTK_CONTAINER (hbox_row2), label_full);

  GtkWidget *button_pho = gtk_button_new_with_label("注音");
  g_signal_connect_swapped (GTK_OBJECT (button_pho), "button_press_event",
        G_CALLBACK (inmd_switch_popup_handler), NULL);
  gtk_box_pack_start (GTK_BOX (hbox_row2), button_pho, FALSE, FALSE, 0);

  int i;

  button_pho = gtk_button_new();
  gtk_container_set_border_width (GTK_CONTAINER (button_pho), 0);
  gtk_box_pack_start (GTK_BOX (hbox_row2), button_pho, FALSE, FALSE, 0);
  GtkWidget *hbox_pho = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (button_pho), hbox_pho);

  for(i=0; i < 4;i ++) {
    GtkWidget *label = gtk_label_new(NULL);
    labels_pho[i] = label;
    gtk_box_pack_start (GTK_BOX (hbox_pho), label, FALSE, FALSE, 0);
    set_label_font_size(label, gcin_font_size);
  }

  g_signal_connect(G_OBJECT(button_pho),"button-press-event",
                   G_CALLBACK(mouse_button_callback), NULL);

  if (left_right_button_tips) {
    GtkTooltips *button_gtab_tips = gtk_tooltips_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (button_gtab_tips), button_pho, "左鍵符號，右鍵設定",NULL);
  }

  label_key_codes  = gtk_label_new(NULL);
  gtk_label_set_selectable(GTK_LABEL(label_key_codes), TRUE);
  gtk_box_pack_start (GTK_BOX (hbox_row2), label_key_codes, FALSE, FALSE, 2);

  gtk_widget_show_all (gwin_pho);

  gtk_widget_hide(label_full);
}


void create_win_pho_gui_simple()
{
//  dbg("create_win_pho .....\n");

  if (top_bin)
    return;

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 0);

  GtkWidget *event_box_pho = gtk_event_box_new();
  gtk_container_set_border_width (GTK_CONTAINER (event_box_pho), 0);

  if (gcin_inner_frame) {
    GtkWidget *frame = top_bin = gtk_frame_new(NULL);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
    gtk_container_add (GTK_CONTAINER(gwin_pho), frame);
    gtk_container_add (GTK_CONTAINER (frame), vbox_top);
  } else {
    gtk_container_add (GTK_CONTAINER(gwin_pho), vbox_top);
    top_bin = vbox_top;
  }


  GtkWidget *align = gtk_alignment_new (0, 0, 0, 0);
  label_pho_sele = gtk_label_new(NULL);

  if (!pho_in_row1) {
    gtk_box_pack_start (GTK_BOX (vbox_top), align, FALSE, FALSE, 0);
    gtk_container_add (GTK_CONTAINER (align), label_pho_sele);
  } else {
    GtkWidget *hbox_row1 = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox_top), hbox_row1, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox_row1), event_box_pho, FALSE, FALSE, 0);

    gtk_box_pack_start (GTK_BOX (hbox_row1), align, FALSE, FALSE, 0);
    gtk_container_add (GTK_CONTAINER (align), label_pho_sele);
  }


  hbox_row2 = gtk_hbox_new (FALSE, 0);
  /* This packs the button into the gwin_pho (a gtk container). */
  gtk_container_add (GTK_CONTAINER (vbox_top), hbox_row2);

  label_full = gtk_label_new("全");
  gtk_container_add (GTK_CONTAINER (hbox_row2), label_full);


  if (!pho_in_row1)
    gtk_box_pack_start (GTK_BOX (hbox_row2), event_box_pho, FALSE, FALSE, 0);

  g_signal_connect(G_OBJECT(event_box_pho),"button-press-event",
                   G_CALLBACK(mouse_button_callback), NULL);

  GtkWidget *frame_pho = gtk_frame_new(NULL);
  gtk_container_add (GTK_CONTAINER (event_box_pho), frame_pho);
  gtk_container_set_border_width (GTK_CONTAINER (frame_pho), 0);

  GtkWidget *hbox_pho = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame_pho), hbox_pho);

  int i;
  for(i=0; i < 4;i ++) {
    GtkWidget *label = gtk_label_new(NULL);
    labels_pho[i] = label;
    gtk_box_pack_start (GTK_BOX (hbox_pho), label, FALSE, FALSE, 0);
    set_label_font_size(label, gcin_font_size_tsin_pho_in);
  }

  if (left_right_button_tips) {
    GtkTooltips *button_gtab_tips = gtk_tooltips_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (button_gtab_tips), event_box_pho, "左鍵符號，右鍵設定",NULL);
  }

  label_key_codes  = gtk_label_new(NULL);
  gtk_label_set_selectable(GTK_LABEL(label_key_codes), TRUE);
  gtk_box_pack_start (GTK_BOX (hbox_row2), label_key_codes, FALSE, FALSE, 2);

  change_pho_font_size();

  gtk_widget_show_all (gwin_pho);

  gtk_widget_hide(label_full);
}

void create_win_pho_gui()
{
  if (pho_simple_win)
    create_win_pho_gui_simple();
  else
    create_win_pho_gui_full();

  if (pho_hide_row2) {
    gtk_widget_hide(hbox_row2);
    gtk_window_resize(GTK_WINDOW(gwin_pho), 10, 20);
  }

  current_pho_simple_win = pho_simple_win;
  current_gcin_inner_frame = gcin_inner_frame;
  current_pho_in_row1 = pho_in_row1;
}


void change_win_pho_style()
{
  if (!top_bin || (current_pho_simple_win == pho_simple_win &&
      current_gcin_inner_frame == gcin_inner_frame &&
      current_pho_in_row1 ==  pho_in_row1))
    return;

  gtk_widget_destroy(top_bin);
  top_bin = NULL;

  create_win_pho_gui();
}


gboolean pho_has_input();

void show_win_pho()
{
//  dbg("show_win_pho\n");
  create_win_pho();
  create_win_pho_gui();

  if (gcin_pop_up_win && !pho_has_input())
    return;

  gtk_widget_show(gwin_pho);
  if (current_CS->b_raise_window)
    gtk_window_present(gwin_pho);

  show_win_sym();

  if (pho_hide_row2)
    gtk_widget_hide(hbox_row2);
  else
    gtk_widget_show(hbox_row2);
}


void hide_win_sym();

void hide_win_pho()
{
// dbg("hide_win_pho\n");
  if (!gwin_pho)
    return;
  gtk_widget_hide(gwin_pho);
  hide_win_sym();
}


void init_tab_pho();
void get_win_gtab_geom();

void init_gtab_pho_query_win()
{
  init_tab_pho();
  get_win_gtab_geom();
  move_win_pho(win_x, win_y + win_yl);
}

char *get_full_str();

void win_pho_disp_half_full()
{
  gtk_label_set_text(GTK_LABEL(labels_pho[0]), get_full_str());

  if (current_CS->im_state == GCIN_STATE_CHINESE && current_CS->b_half_full_char) {
    gtk_widget_show(label_full);
  } else
    gtk_widget_hide(label_full);

  minimize_win_pho();
}

void get_win_pho_geom()
{
  if (!gwin_pho)
    return;

  gtk_window_get_position(GTK_WINDOW(gwin_pho), &win_x, &win_y);

  get_win_size(gwin_pho, &win_xl, &win_yl);
}


void change_pho_font_size()
{
  int i;

  if (!top_bin)
    return;

  for(i=0; i < 3;i ++) {
    set_label_font_size(labels_pho[i], gcin_font_size_tsin_pho_in);
  }

  set_label_font_size(label_pho_sele, gcin_font_size);
}
