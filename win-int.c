#include "gcin.h"

GtkWidget *gwin_int;

static GtkWidget *button_int;
static GtkWidget *labels_int[4];

void disp_int(int index, char *intcode)
{
  char utf8[512];
  big5_utf8_n(intcode, CH_SZ, utf8);

  gtk_label_set_text(GTK_LABEL(labels_int[index]), utf8);
}

void clear_int_code(int index)
{
  gtk_label_set_text(GTK_LABEL(labels_int[index]), "  ");
}

void clear_int_code_all()
{
  int i;

  for(i=0; i < 4; i++)
    clear_int_code(i);
}

void hide_win_int()
{
  if (!gwin_int)
    return;
  gtk_widget_hide(gwin_int);
}

void show_win_int()
{
  if (!gwin_int)
    return;
  gtk_widget_show(gwin_int);
}

void move_win_int(int x, int y)
{
  if (!gwin_int)
    return;

  gtk_window_get_size(GTK_WINDOW(gwin_int), &win_xl, &win_yl);

  if (x + win_xl > dpy_xl)
    x = dpy_xl - win_xl;
  if (x < 0)
    x = 0;

  if (y + win_yl > dpy_yl)
    y = dpy_yl - win_yl;
  if (y < 0)
    y = 0;

  win_x = x;  win_y = y;
  gtk_window_move(GTK_WINDOW(gwin_int), x, y);
}

void create_win_intcode()
{
  if (gwin_int) {
    show_win_int();
    return;
  }

  gwin_int = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width (GTK_CONTAINER (gwin_int), 0);
  gtk_widget_realize (gwin_int);
  GdkWindow *gdkwin = gwin_int->window;

  gdk_window_set_override_redirect(gdkwin, TRUE);

  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
  gtk_container_add (GTK_CONTAINER(gwin_int), frame);


  GtkWidget *hbox_top = gtk_hbox_new (FALSE, 0);
  /* This packs the button into the gwin_int (a gtk container). */
  gtk_container_add (GTK_CONTAINER (frame), hbox_top);

  GtkWidget *button_intcode = gtk_button_new_with_label("內碼");
  g_signal_connect_swapped (GTK_OBJECT (button_intcode), "button_press_event",
        G_CALLBACK (inmd_switch_popup_handler), NULL);
  gtk_box_pack_start (GTK_BOX (hbox_top), button_intcode, FALSE, FALSE, 0);

  int i;

  button_int = gtk_button_new();
  gtk_container_set_border_width (GTK_CONTAINER (button_int), 0);
  gtk_box_pack_start (GTK_BOX (hbox_top), button_int, FALSE, FALSE, 0);
  GtkWidget *hbox_int = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (button_int), hbox_int);

  for(i=0; i < 4;i ++) {
    GtkWidget *label = gtk_label_new("  ");
    labels_int[i] = label;
    gtk_box_pack_start (GTK_BOX (hbox_int), label, FALSE, FALSE, 0);
    set_label_font_size(label, gcin_font_size);
  }


  gtk_widget_show_all (gwin_int);
}
