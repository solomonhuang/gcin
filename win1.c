#include "gcin.h"
#include "pho.h"

static GtkWidget *gwin1, *frame;
Window xwin1;

#define SELEN (12)

static GtkWidget *labels_sele[SELEN];
static GtkWidget *arrow_up, *arrow_down;

void create_win1()
{
  if (gwin1)
    return;

  gwin1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_realize (gwin1);
  GdkWindow *gdkwin1 = gwin1->window;
  xwin1 = GDK_WINDOW_XWINDOW(gdkwin1);
  gdk_window_set_override_redirect(gdkwin1, TRUE);
}


void create_win1_gui()
{
  if (frame)
    return;

  frame = gtk_frame_new(NULL);
  gtk_container_add (GTK_CONTAINER(gwin1), frame);

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER(frame), vbox_top);


  arrow_up = gtk_arrow_new (GTK_ARROW_UP, GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (vbox_top), arrow_up, FALSE, FALSE, 0);


  GtkWidget *vbox_labels = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_top), vbox_labels, FALSE, FALSE, 0);
  int i;
  for(i=0; i < SELEN; i++) {
    GtkWidget *align = gtk_alignment_new(0,0,0,0);
    gtk_box_pack_start (GTK_BOX (vbox_labels), align, FALSE, FALSE, 0);

    labels_sele[i] = gtk_label_new(NULL);
    gtk_label_set_justify(GTK_LABEL(labels_sele[i]),GTK_JUSTIFY_LEFT);
    set_label_font_size(labels_sele[i], gcin_font_size_tsin_presel);

    gtk_container_add (GTK_CONTAINER(align), labels_sele[i]);
  }

  arrow_down = gtk_arrow_new (GTK_ARROW_DOWN, GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (vbox_top), arrow_down, FALSE, FALSE, 0);

  gtk_widget_show_all(gwin1);
  gtk_widget_hide(gwin1);
}

void clear_sele()
{
  int i;

  if (!gwin1)
    create_win1();

  for(i=0; i < SELEN; i++)
    gtk_widget_hide(labels_sele[i]);

  gtk_widget_hide(arrow_up);
  gtk_widget_hide(arrow_down);
  gtk_window_resize(GTK_WINDOW(gwin1), 10, 20);
}



void set_sele_text(int i, char *text, int len)
{
  char tt[128];
  char utf8[128];

  memcpy(utf8, text, len);
  utf8[len]=0;

  snprintf(tt, sizeof(tt), "%c %s", phkbm.selkey[i], utf8);

  gtk_label_set_text(GTK_LABEL(labels_sele[i]), tt);
  gtk_widget_show(labels_sele[i]);
}


void move_win_char_index(GtkWidget *win1, int index);

void disp_selections(int idx)
{
  move_win_char_index(gwin1, idx);
  gtk_widget_show(gwin1);
}


void hide_selections_win()
{
  if (!gwin1)
    return;

  gtk_widget_hide(gwin1);
}

void disp_arrow_up()
{
  gtk_widget_show(arrow_up);
}

void disp_arrow_down()
{
  gtk_widget_show(arrow_down);
}

void destory_win1()
{
  if (!gwin1)
    return;
  gtk_widget_destroy(gwin1);
}

void change_win1_font()
{
  int i;

  for(i=0; i < SELEN; i++)
    set_label_font_size(labels_sele[i], gcin_font_size_tsin_presel);
}
