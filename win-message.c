#include "gcin.h"

static gboolean timeout_destroy_window(GtkWidget *win)
{
  gtk_widget_destroy(win);
  return FALSE;
}

#if !TRAY_ENABLED
GdkWindow *tray_da_win;
#endif
extern GdkWindow *tray_da_win;

static void create_win_message(char *icon, char *text, int duration)
{
  GtkWidget *gwin_message = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width (GTK_CONTAINER (gwin_message), 0);
  gtk_widget_realize (gwin_message);
  GdkWindow *gdkwin = gwin_message->window;
  set_no_focus(gwin_message);

  GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (gwin_message), hbox);

  if (icon[0] != '-') {
    GtkWidget *image = gtk_image_new_from_file(icon);
    gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
  }

  if (text[0] != '-') {
    GtkWidget *label = gtk_label_new(text);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  }

  gtk_widget_show_all(gwin_message);

  int ox, oy, szx, szy;
  gdk_window_get_origin  (tray_da_win, &ox, &oy);
  gdk_window_get_size(tray_da_win, &szx, &szy);

  int width, height;
  get_win_size(gwin_message, &width, &height);

  if (oy<height) {
    oy = szy;
  } else {
    oy -= height;
    if (oy + height > dpy_yl)
      oy = dpy_yl - height;
    if (oy < 0)
      oy = 0;
  }

  if (ox + width > dpy_xl)
    ox = dpy_xl - width;
  if (ox < 0)
    ox = 0;

  gtk_window_move(GTK_WINDOW(gwin_message), ox, oy);

  g_timeout_add(duration, timeout_destroy_window, gwin_message);
}

void execute_message(char *message)
{
  char head[32];
  char icon[128];
  char text[128];
  int duration = 3000;

  icon[0] = text[0] = 0;

  sscanf(message, "%s %s %s %d", head, icon, text, &duration);

  create_win_message(icon, text, duration);
}
