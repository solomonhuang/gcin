#include "gcin.h"

static GtkWidget *win_status, *label_inmd, *label_half_full;

static void move_to_corner()
{
  gtk_window_parse_geometry(GTK_WINDOW(win_status), "-0-0");
}

static void create_win_status()
{
  if (win_status)
    return;

  win_status = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width (GTK_CONTAINER (win_status), 0);
#if 0
  gtk_widget_realize (win_status);
  GdkWindow *gdkwin_status = win_status->window;
  gdk_window_set_override_redirect(gdkwin_status, TRUE);
#else
  gtk_window_set_type_hint(GTK_WINDOW(win_status), GDK_WINDOW_TYPE_HINT_TOOLBAR);
  gtk_window_set_decorated(GTK_WINDOW(win_status), FALSE);
  gtk_window_stick(GTK_WINDOW(win_status));
#endif
  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_container_add (GTK_CONTAINER(win_status), frame);

  GtkWidget *hbox_top = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER(frame), hbox_top);
  label_inmd = gtk_label_new(NULL);
  gtk_box_pack_start (GTK_BOX (hbox_top), label_inmd, FALSE, FALSE, 0);

  GtkWidget *separator = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (hbox_top), separator, FALSE, FALSE, 0);

  label_half_full = gtk_label_new(NULL);
  gtk_box_pack_start (GTK_BOX (hbox_top), label_half_full, FALSE, FALSE, 0);

  gtk_widget_show_all(win_status);
#if 0
  gdk_window_set_keep_above(win_status->window, TRUE);
#endif
  move_to_corner();
}

void show_win_stautus()
{
  if (!gcin_pop_up_win)
    return;

  create_win_status();
  gtk_widget_show(win_status);
  move_to_corner();
}

void hide_win_status()
{
  if (!win_status || !gcin_pop_up_win)
    return;

  gtk_widget_hide(win_status);
}


void set_win_status_inmd(char *s)
{
  if (!s || !gcin_pop_up_win)
    return;

  if (!win_status)
    create_win_status();

  gtk_label_set_text(GTK_LABEL(label_inmd), s);
  gtk_window_resize(GTK_WINDOW(win_status), 10, 10);
  show_win_stautus();
}


void set_win_status_half_full(char *s)
{
  if (!s || !gcin_pop_up_win)
    return;

  if (!win_status)
    create_win_status();

  gtk_label_set_text(GTK_LABEL(label_half_full), s);
//  dbg("hhh '%s'\n", s);
  gtk_window_resize(GTK_WINDOW(win_status), 10, 10);
  show_win_stautus();
}
