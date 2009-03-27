#include "gcin.h"

static GtkWidget *gwin_pho, *frame;
Window xwin_pho;
static GtkWidget *label_pho_sele;
static GtkWidget *button_pho;
static GtkWidget *labels_pho[4];
static GtkWidget *label_key_codes;

void disp_pho(int index, char *phochar)
{
  char tt[CH_SZ+1];

  if (phochar[0]==' ')
    strcpy(tt, "  ");
  else
    utf8cpy(tt, phochar);

  gtk_label_set_text(GTK_LABEL(labels_pho[index]), tt);
}

void disp_pho_sel(char *s)
{
  gtk_label_set_text(GTK_LABEL(label_pho_sele), s);
}


void set_key_codes_label_pho(char *s)
{
  if (!label_key_codes)
    return;

  gtk_label_set_text(GTK_LABEL(label_key_codes), s);
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

void create_win_pho_gui()
{
//  dbg("create_win_pho .....\n");

  if (frame)
    return;

  frame = gtk_frame_new(NULL);
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
  g_signal_connect_swapped (GTK_OBJECT (button_pho), "button_press_event",
        G_CALLBACK (inmd_switch_popup_handler), NULL);
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

  label_key_codes  = gtk_label_new(NULL);
  gtk_box_pack_start (GTK_BOX (hbox), label_key_codes, FALSE, FALSE, 2);

  gtk_widget_show_all (gwin_pho);
}


void show_win_pho()
{
//  dbg("show_win_pho\n");
  create_win_pho();
  create_win_pho_gui();
  gtk_widget_show_all(gwin_pho);
}


void hide_win_pho()
{
//  dbg("hide_win_pho\n");
  gtk_widget_hide(gwin_pho);
}
