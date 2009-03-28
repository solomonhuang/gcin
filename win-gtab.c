#include "gcin.h"
#include "gtab.h"
#include "win-sym.h"

static int current_gtab_simple_win;
static int current_gcin_inner_frame;
static int current_gtab_in_row1;
static int current_gtab_vertical_select;

GtkWidget *gwin_gtab;
static GtkWidget *top_bin;
static GtkWidget *label_full, *label_gtab_sele;
static GtkWidget *labels_gtab[MAX_TAB_KEY_NUM64_6];
static GtkWidget *label_input_method_name;
static GtkWidget *label_key_codes;
static GtkWidget *image_pin;
static GtkWidget *box_gtab_im_name;
static GtkWidget *hbox_row2;
static GtkWidget *label_page;
static GtkWidget *label_edit;
static GdkColor better_color;

Window xwin_gtab;
void set_label_space(GtkWidget *label);
void minimize_win_gtab();
gboolean win_size_exceed(GtkWidget *win);
void move_win_gtab(int x, int y);

static void adj_gtab_win_pos()
{
  if (win_size_exceed(gwin_gtab))
    move_win_gtab(current_in_win_x, current_in_win_y);
}

void disp_gtab(int index, char *gtabchar)
{
  char utf8[512];

   if (!labels_gtab[index])
     return;

  if (gtabchar[0]==' ') {
    if (gcin_pop_up_win) {
      gtk_widget_hide(labels_gtab[index]);
      minimize_win_gtab();
    } else
      set_label_space(labels_gtab[index]);
  }
  else {
    if (gtabchar[0] & 128)
      utf8cpy(utf8, gtabchar);
    else {
      memcpy(utf8, gtabchar, 2);
      utf8[2]=0;
    }

//    printf("index:%d jjjj '%s'\n", index, utf8);
    gtk_label_set_text(GTK_LABEL(labels_gtab[index]), utf8);
    gtk_widget_show(labels_gtab[index]);
    adj_gtab_win_pos();
  }
}


void set_gtab_input_color(GdkColor *color)
{
  int i;

  for(i=0; i < MAX_TAB_KEY_NUM64_6; i++) {
    GtkWidget *label = labels_gtab[i];

    if (!label)
      continue;
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, color);
  }
}


void set_gtab_input_error_color()
{
  GdkColor red;
  gdk_color_parse("red", &red);
  set_gtab_input_color(&red);
}

void clear_gtab_input_error_color()
{
  set_gtab_input_color(NULL);
}

void clear_gtab_in_area()
{
  int i;
  int maxpress = cur_inmd ? cur_inmd->MaxPress : 1;

  if (maxpress > 5)
    maxpress = 5;
  if (!maxpress)
    maxpress = 1;


  for(i=0; i < maxpress; i++) {
    disp_gtab(i, " ");
  }

  for(i=maxpress; i < 10; i++) {
    gtk_widget_hide(labels_gtab[i]);
  }
}


static void set_disp_im_name();

void change_win_bg(GtkWidget *win)
{
  if (!gcin_win_color_use) {
    gtk_widget_modify_bg(win, GTK_STATE_NORMAL, NULL);
    return;
  }

  GdkColor col;
  gdk_color_parse(gcin_win_color_bg, &col);
  gtk_widget_modify_bg(win, GTK_STATE_NORMAL, &col);
}

void change_win_fg_bg(GtkWidget *win, GtkWidget *label)
{
  if (!gcin_win_color_use) {
    if (label)
      gtk_widget_modify_fg(label, GTK_STATE_NORMAL, NULL);
    return;
  }

  GdkColor col;
  gdk_color_parse(gcin_win_color_fg, &col);
  if (label)
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &col);
  if (label_edit)
    gtk_widget_modify_fg(label_edit, GTK_STATE_NORMAL, &col);

  change_win_bg(win);
}



void change_gtab_font_size()
{
  if (!label_gtab_sele)
    return;

  set_label_font_size(label_gtab_sele, gcin_font_size);
  set_label_font_size(label_edit, gcin_font_size);

  int i;
  for(i=0; i < MAX_TAB_KEY_NUM64_6; i++) {
    set_label_font_size(labels_gtab[i], gcin_font_size_gtab_in);
  }

  set_disp_im_name();

  change_win_fg_bg(gwin_gtab, label_gtab_sele);
}


void disp_gtab_sel(char *s)
{
  if (!label_gtab_sele)
    return;
  gtk_label_set_markup(GTK_LABEL(label_gtab_sele), s);
  minimize_win_gtab();
  adj_gtab_win_pos();
}


void set_key_codes_label(char *s, int better)
{
  if (!label_key_codes)
    return;

  if (s && strlen(s))
    gtk_widget_show(hbox_row2);
  else {
    if (gtab_hide_row2 && hbox_row2) {
      gtk_widget_hide(hbox_row2);
    }
  }

  if (better)
    gtk_widget_modify_fg(label_key_codes, GTK_STATE_NORMAL, &better_color);
  else
    gtk_widget_modify_fg(label_key_codes, GTK_STATE_NORMAL, NULL);

  gtk_label_set_text(GTK_LABEL(label_key_codes), s);
}


void set_page_label(char *s)
{
  if (!label_page)
    return;
  gtk_label_set_text(GTK_LABEL(label_page), s);
}

void show_win_sym();

void move_win_gtab(int x, int y)
{
//  dbg("move_win_gtab %d %d\n", x, y);
  get_win_size(gwin_gtab, &win_xl, &win_yl);

  if (x + win_xl > dpy_xl)
    x = dpy_xl - win_xl;
  if (x < 0)
    x = 0;

  if (y + win_yl > dpy_yl)
    y = dpy_yl - win_yl;
  if (y < 0)
    y = 0;

  gtk_window_move(GTK_WINDOW(gwin_gtab), x, y);
  win_x = x;  win_y = y;

  move_win_sym();
}

void set_gtab_input_method_name(char *s)
{
//  dbg("set_gtab_input_method_name '%s'\n", s);
  if (!label_input_method_name)
    return;
  gtk_label_set(GTK_LABEL(label_input_method_name), s);
}


void create_win_gtab()
{
  if (gwin_gtab)
    return;

  gwin_gtab = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width (GTK_CONTAINER (gwin_gtab), 0);
  gtk_widget_realize (gwin_gtab);
  GdkWindow *gdkwin = gwin_gtab->window;
  set_no_focus(gwin_gtab);

  xwin_gtab = GDK_WINDOW_XWINDOW(gdkwin);
}

void create_win_sym();
void exec_gcin_setup();

static void mouse_button_callback( GtkWidget *widget,GdkEventButton *event, gpointer data)
{
//  dbg("mouse_button_callback %d\n", event->button);
  switch (event->button) {
    case 1:
      toggle_win_sym();
      break;
    case 2:
      inmd_switch_popup_handler(widget, (GdkEvent *)event);
      break;
    case 3:
      exec_gcin_setup();
      break;
  }
}

extern char file_pin_float[];

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

//  set_currenet_IC_pin_image_pin();
}


void toggle_half_full_char();

gint inmd_switch_popup_handler (GtkWidget *widget, GdkEvent *event);
char full_char_str[]="全";

void disp_label_edit(char *str)
{
  gtk_label_set_markup(GTK_LABEL(label_edit), str);
}

void create_win_gtab_gui_simple()
{
//  dbg("create_win_gtab_gui .....\n");
  if (top_bin)
    return;

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 0);


  GtkWidget *event_box_gtab = gtk_event_box_new();
  gtk_container_set_border_width (GTK_CONTAINER (event_box_gtab), 0);

  if (gcin_inner_frame) {
    GtkWidget *frame = top_bin = gtk_frame_new(NULL);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
    gtk_container_add (GTK_CONTAINER(gwin_gtab), frame);
    gtk_container_set_border_width (GTK_CONTAINER (gwin_gtab), 0);
    gtk_container_add (GTK_CONTAINER (frame), vbox_top);
  } else {
    gtk_container_add (GTK_CONTAINER(gwin_gtab), vbox_top);
    top_bin = vbox_top;
  }

  if (gtab_auto_select_by_phrase) {
    label_edit = gtk_label_new(NULL);
    GtkWidget *align = gtk_alignment_new (0, 0.0, 0, 0);
    gtk_container_add (GTK_CONTAINER (align), label_edit);
    gtk_container_add (GTK_CONTAINER (vbox_top), align);
  }

  GtkWidget *align = gtk_alignment_new (0, 0.0, 0, 0);

  label_gtab_sele = gtk_label_new(NULL);
  gtk_container_add (GTK_CONTAINER (align), label_gtab_sele);

  if (!gtab_in_row1) {
    if (!gtab_vertical_select)
      gtk_box_pack_start (GTK_BOX (vbox_top), align, FALSE, FALSE, 0);
  } else {
    GtkWidget *hbox_row1 = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox_top), hbox_row1, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox_row1), event_box_gtab, FALSE, FALSE, 0);

    if (!gtab_vertical_select)
      gtk_box_pack_start (GTK_BOX (hbox_row1), align, FALSE, FALSE, 0);
  }


  hbox_row2 = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (vbox_top), hbox_row2);

  label_full = gtk_label_new("全");
  gtk_box_pack_start (GTK_BOX (hbox_row2), label_full, FALSE, FALSE, 0);

  GtkWidget *event_box_input_method_name = gtk_event_box_new();
  gtk_box_pack_start (GTK_BOX (hbox_row2), event_box_input_method_name, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (event_box_input_method_name), 0);

  GtkWidget *frame_input_method_name = gtk_frame_new(NULL);
  gtk_container_add (GTK_CONTAINER (event_box_input_method_name), frame_input_method_name);
  gtk_container_set_border_width (GTK_CONTAINER (frame_input_method_name), 0);

  label_input_method_name = gtk_label_new("");
  gtk_container_add (GTK_CONTAINER (frame_input_method_name), label_input_method_name);
  g_signal_connect_swapped (GTK_OBJECT (event_box_input_method_name), "button-press-event",
        G_CALLBACK (inmd_switch_popup_handler), NULL);

  box_gtab_im_name = event_box_input_method_name;

  if (!gtab_in_row1)
    gtk_box_pack_start (GTK_BOX (hbox_row2), event_box_gtab, FALSE, FALSE, 0);

#if 1
  GtkWidget *frame_gtab = gtk_frame_new(NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame_gtab), 0);
  gtk_container_add (GTK_CONTAINER (event_box_gtab), frame_gtab);
#endif

  g_signal_connect(G_OBJECT(event_box_gtab),"button-press-event",
                   G_CALLBACK(mouse_button_callback), NULL);

  if (left_right_button_tips) {
    GtkTooltips *button_gtab_tips = gtk_tooltips_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (button_gtab_tips), event_box_gtab, _("左鍵符號，右鍵設定"),NULL);
  }

  GtkWidget *hbox_gtab = gtk_hbox_new (FALSE, 0);
#if 1
  gtk_container_add (GTK_CONTAINER (frame_gtab), hbox_gtab);
#else
  gtk_container_add (GTK_CONTAINER (event_box_gtab), hbox_gtab);
#endif

  int i;
  for(i=0; i < MAX_TAB_KEY_NUM64_6; i++) {
    GtkWidget *label = gtk_label_new(NULL);
    labels_gtab[i] = label;
    gtk_box_pack_start (GTK_BOX (hbox_gtab), label, FALSE, FALSE, 0);
//    set_label_font_family(label, "serif");
  }

  label_key_codes  = gtk_label_new(NULL);
  gtk_label_set_selectable(GTK_LABEL(label_key_codes), TRUE);
  gtk_box_pack_start (GTK_BOX (hbox_row2), label_key_codes, FALSE, FALSE, 2);

  label_page  = gtk_label_new(NULL);
  gtk_box_pack_start (GTK_BOX (hbox_row2), label_page, FALSE, FALSE, 2);

  if (gtab_vertical_select) {
    gtk_box_pack_start (GTK_BOX (vbox_top), align, FALSE, FALSE, 0);
  }

  change_gtab_font_size();

  gtk_widget_show_all (gwin_gtab);
  gtk_widget_hide (gwin_gtab);

  set_disp_im_name();
  gtk_widget_hide(label_full);

  if (gtab_hide_row2)
    gtk_widget_hide(hbox_row2);
}


static void create_win_gtab_gui()
{
  create_win_gtab_gui_simple();
  current_gtab_simple_win = gtab_simple_win;
  current_gtab_in_row1 = gtab_in_row1;
  current_gtab_vertical_select = gtab_vertical_select;
  current_gcin_inner_frame = gcin_inner_frame;
  gdk_color_parse("red", &better_color);
}


void change_win_gtab_style()
{
  if (!top_bin || current_gtab_simple_win == gtab_simple_win &&
      current_gcin_inner_frame == gcin_inner_frame &&
      current_gtab_in_row1 == gtab_in_row1 &&
      current_gtab_vertical_select == gtab_vertical_select &&
      (gtab_auto_select_by_phrase && label_edit || !gtab_auto_select_by_phrase && !label_edit)
      )
    return;

  gtk_widget_destroy(top_bin);
  top_bin = NULL;
  label_edit = NULL;

  create_win_gtab_gui();
}


void init_gtab(int inmdno);
gboolean gtab_has_input(), force_show;

void show_win_gtab()
{
  create_win_gtab();
  create_win_gtab_gui();
//  set_currenet_IC_pin_image_pin();

  if (current_CS) {
    if (current_CS->fixed_pos)
      move_win_gtab(0,0);
  }

  init_gtab(current_CS->in_method);

  if (gcin_pop_up_win && !gtab_has_input() && !force_show)
    return;

  gtk_widget_show(gwin_gtab);
  if (current_CS->b_raise_window)
    gtk_window_present(GTK_WINDOW(gwin_gtab));

  show_win_sym();
}


void hide_win_sym();
void close_gtab_pho_win();

void hide_win_gtab()
{
//  dbg("hide_win_gtab gwin_gtab\n", gwin_gtab);
  if (!gwin_gtab)
    return;
#if 0
  // get around the grey window bug in GTK/X11
  gdk_flush();
  XSync(dpy, False);
  usleep(10000);
#endif
  gtk_widget_hide(gwin_gtab);
  close_gtab_pho_win();

  hide_win_sym();
}

void minimize_win_gtab()
{
  gtk_window_resize(GTK_WINDOW(gwin_gtab), 32, 12);
}


void destroy_win_gtab()
{
  if (!gwin_gtab)
    return;

  gtk_widget_destroy(gwin_gtab);
  gwin_gtab = NULL;
  top_bin = NULL;
}


void get_win_gtab_geom()
{
  if (!gwin_gtab)
    return;

  gtk_window_get_position(GTK_WINDOW(gwin_gtab), &win_x, &win_y);

  get_win_size(gwin_gtab, &win_xl, &win_yl);
}

static void set_disp_im_name()
{
  if (!box_gtab_im_name)
    return;

  if (gtab_disp_im_name)
    gtk_widget_show(box_gtab_im_name);
  else
    gtk_widget_hide(box_gtab_im_name);
}

char eng_full_str[]="英/全";

char *get_full_str()
{
  switch (current_CS->im_state) {
    case GCIN_STATE_ENG_FULL:
      return eng_full_str;
    default:
      return "　";
  }
}

void win_gtab_disp_half_full()
{
  if (label_full) {
    if (current_CS->im_state == GCIN_STATE_CHINESE && current_CS->b_half_full_char) {
      gtk_widget_show(label_full);
    } else {
      gtk_widget_hide(label_full);
    }
  }

  if (labels_gtab[0])
    gtk_label_set_text(GTK_LABEL(labels_gtab[0]), get_full_str());

  int i;

  for(i=1; i < MAX_TAB_KEY_NUM64_6; i++) {
    if (!labels_gtab[i])
      continue;
    gtk_widget_hide(labels_gtab[i]);
  }

  minimize_win_gtab();
}


void recreate_win_gtab()
{
//  puts("recreate_win_gtab");
  destroy_win_gtab();
  create_win_gtab();
  create_win_gtab_gui();
}
