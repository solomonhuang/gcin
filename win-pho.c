#include "gcin.h"

static GtkWidget *gwin_pho;
static GtkWidget *label_pho_sele;
static GtkWidget *button_pho;
static GtkWidget *labels_pho[4];


void disp_pho(int index, char *phochar)
{
  GError *err = NULL;
  int rn, wn;
  char *utf8 = g_locale_to_utf8 (phochar, 2, &rn, &wn, &err);

  if (err) {
    printf("utf8 conver error");
    return;
  }

  gtk_label_set_text(GTK_LABEL(labels_pho[index]), utf8);
  g_free(utf8);
}

void disp_pho_sel(char *s)
{
  GError *err = NULL;
  int rn, wn;
  char *utf8 = g_locale_to_utf8 (s, strlen(s), &rn, &wn, &err);

  if (err) {
    printf("utf8 conver error");
    return;
  }

  gtk_label_set_text(GTK_LABEL(label_pho_sele), utf8);
}


void show_win_pho()
{
  gtk_widget_show(gwin_pho);
}


void hide_win_pho()
{
  gtk_widget_hide(gwin_pho);
}


void move_win_pho(int x, int y)
{
  gtk_window_get_size(GTK_WINDOW(gwin_pho), &win_xl, &win_yl);

  if (x + win_xl > dpy_xl)
    x = dpy_xl - win_xl;
  if (x < 0)
    x = 0;

  if (y + win_yl > dpy_yl)
    y = dpy_yl - win_yl;
  if (y < 0)
    y = 0;

  win_x = x;  win_y = y;
  gtk_window_move(GTK_WINDOW(gwin_pho), x, y);
}

void create_win_pho()
{
  dbg("create_win_pho .....\n");

  if (gwin_pho) {
    show_win_pho();
    return;
  }

  gwin_pho = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width (GTK_CONTAINER (gwin_pho), 0);
  gtk_widget_realize (gwin_pho);
  GdkWindow *gdkwin = gwin_pho->window;

  gdk_window_set_override_redirect(gdkwin, TRUE);

  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
  gtk_container_add (GTK_CONTAINER(gwin_pho), frame);

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox_top);

  GtkWidget *align = gtk_alignment_new (0, 0, 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox_top), align, FALSE, FALSE, 0);

  label_pho_sele = gtk_label_new(NULL);
  gtk_container_add (GTK_CONTAINER (align), label_pho_sele);
  set_label_font_size(label_pho_sele, gcin_font_size);


  GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
  /* This packs the button into the gwin_pho (a gtk container). */
  gtk_container_add (GTK_CONTAINER (vbox_top), hbox);

  GtkWidget *button_pho = gtk_button_new_with_label("注音");
  gtk_box_pack_start (GTK_BOX (hbox), button_pho, FALSE, FALSE, 0);

  int i;

  button_pho = gtk_button_new();
  gtk_container_set_border_width (GTK_CONTAINER (button_pho), 0);
  gtk_box_pack_start (GTK_BOX (hbox), button_pho, FALSE, FALSE, 0);
  GtkWidget *hbox_pho = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (button_pho), hbox_pho);

  for(i=0; i < 4;i ++) {
    GtkWidget *label = gtk_label_new(NULL);
    labels_pho[i] = label;
    gtk_box_pack_start (GTK_BOX (hbox_pho), label, FALSE, FALSE, 0);
    set_label_font_size(label, gcin_font_size);
  }


  gtk_widget_show_all (gwin_pho);
}

