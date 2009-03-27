#include "gcin.h"

static GtkWidget *gwin1;

#define SELEN (9)

static GtkWidget *labels_sele[SELEN];
static GtkWidget *arrow_up, *arrow_down;


void create_win1()
{
  gwin1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_widget_realize (gwin1);
  GdkWindow *gdkwin1 = gwin1->window;
  gdk_window_set_override_redirect(gdkwin1, TRUE);

  GtkWidget *frame = gtk_frame_new(NULL);
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
    set_label_font_size(labels_sele[i], gcin_font_size);

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

#if 0
  memcpy(tt, text, len);
  tt[len] = 0;
  dbg("set_sele_text: %s\n", text);
#endif
  int rn, wn;
  GError *err = NULL;
  char *utf8 = g_locale_to_utf8 (text, len, &rn, &wn, &err);

  if (err) {
    dbg("conver err %s %s\n", text, err->message);
    g_error_free(err);
  }

  snprintf(tt, sizeof(tt), "%d %s", i+1, utf8);

  gtk_label_set_text(GTK_LABEL(labels_sele[i]), tt);
  g_free(utf8);
  gtk_widget_show(labels_sele[i]);
}


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
  gtk_widget_destroy(gwin1);
}

void change_win1_font()
{
  int i;

  for(i=0; i < SELEN; i++)
    set_label_font_size(labels_sele[i], gcin_font_size);
}
