#include "gcin.h"
#include "intcode.h"

GtkWidget *gwin_int;
extern int current_intcode;

static GtkWidget *button_int;
static GtkWidget *labels_int[MAX_INTCODE];

static struct {
  char *name;
} int_sel[]={
  {"Big5"},
  {"UTF-32(U+)"},
};
static int int_selN = sizeof(int_sel)/sizeof(int_sel[0]);

static GtkWidget *opt_int_opts;

static void minimize_win()
{
  gtk_window_resize(GTK_WINDOW(gwin_int), 10, 10);
}

void adj_intcode_buttons()
{
  int i;

  if (current_intcode==INTCODE_UTF32) {
    for(i=4;i < MAX_INTCODE; i++)
      gtk_widget_show(labels_int[i]);
  } else {
    for(i=4;i < MAX_INTCODE; i++)
      gtk_widget_hide(labels_int[i]);
  }

  minimize_win();
}


static void cb_select( GtkWidget *widget, gpointer data)
{
  current_intcode = gtk_option_menu_get_history (GTK_OPTION_MENU (widget));
  adj_intcode_buttons();
}

static GtkWidget *create_int_opts()
{

  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);

  opt_int_opts = gtk_option_menu_new ();
  gtk_box_pack_start (GTK_BOX (hbox), opt_int_opts, FALSE, FALSE, 0);
  GtkWidget *menu_int_opts = gtk_menu_new ();

  int i;
  for(i=0; i < int_selN; i++) {
    GtkWidget *item = gtk_menu_item_new_with_label (int_sel[i].name);


    gtk_menu_shell_append (GTK_MENU_SHELL (menu_int_opts), item);
  }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (opt_int_opts), menu_int_opts);
  gtk_option_menu_set_history (GTK_OPTION_MENU (opt_int_opts), current_intcode);
  g_signal_connect (G_OBJECT (opt_int_opts), "changed", G_CALLBACK (cb_select), NULL);

  return hbox;
}


void disp_int(int index, char *intcode)
{
  gtk_label_set_text(GTK_LABEL(labels_int[index]), intcode);
}

static char full_space[]="　";

void clear_int_code(int index)
{
  gtk_label_set_text(GTK_LABEL(labels_int[index]), full_space);
}






void switch_intcode()
{
  current_intcode ^= 1;
  adj_intcode_buttons();
}


void clear_int_code_all()
{
  int i;

  for(i=0; i < MAX_INTCODE; i++)
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
//  gtk_window_set_default_size(GTK_WINDOW (gwin_int), 1, 1);
  gtk_container_set_border_width (GTK_CONTAINER (gwin_int), 0);
  gtk_widget_realize (gwin_int);
  GdkWindow *gdkwin = gwin_int->window;

  gdk_window_set_override_redirect(gdkwin, TRUE);

  GtkWidget *frame = gtk_frame_new(NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
  gtk_container_add (GTK_CONTAINER(gwin_int), frame);


  GtkWidget *hbox_top = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), hbox_top);

  GtkWidget *button_intcode = gtk_button_new_with_label("內碼");
  g_signal_connect_swapped (GTK_OBJECT (button_intcode), "button_press_event",
        G_CALLBACK (inmd_switch_popup_handler), NULL);
  gtk_box_pack_start (GTK_BOX (hbox_top), button_intcode, FALSE, FALSE, 0);

  button_int = gtk_button_new();
  gtk_container_set_border_width (GTK_CONTAINER (button_int), 0);
  gtk_box_pack_start (GTK_BOX (hbox_top), button_int, FALSE, FALSE, 0);
  GtkWidget *hbox_int = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (button_int), hbox_int);

  int i;
  for(i=0; i < MAX_INTCODE;i ++) {
    GtkWidget *label = gtk_label_new(full_space);
    labels_int[i] = label;
    gtk_box_pack_start (GTK_BOX (hbox_int), label, FALSE, FALSE, 0);
    set_label_font_size(label, gcin_font_size);
  }

  GtkWidget *intsel = create_int_opts();
  gtk_box_pack_start (GTK_BOX (hbox_top), intsel, FALSE, FALSE, 0);

  gtk_widget_show_all (gwin_int);

  adj_intcode_buttons();
  minimize_win();
}
