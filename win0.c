#include "gcin.h"
#include "pho.h"
#include "win-sym.h"

GtkWidget *gwin0;
Window xwin0;
extern Display *dpy;
GtkWidget *frame;
static GtkWidget *image_pin;

static GtkWidget *hbox_edit, *hbox_row2;
static PangoAttrList* attr_list, *attr_list_blank;

void compact_win0();
void compact_win0_x();
void move_win0(int x, int y);
void get_win0_geom();

static struct {
  GtkWidget *vbox;
  GtkWidget *label;
  GtkWidget *line;
} chars[MAX_PH_BF_EXT];

static GtkWidget *button_pho;
static GtkWidget *labels_pho[4];
static GtkWidget *button_eng_ph;
static int max_yl;


void disp_hide_tsin_status_row()
{
  if (tsin_disp_status_row)
    gtk_widget_show_all(hbox_row2);
  else {
    gtk_widget_hide_all(hbox_row2);
    compact_win0();
  }
}

void set_label_font_size(GtkWidget *label, int size)
{
  if (!label)
    return;

  PangoContext *pango_context = gtk_widget_get_pango_context (label);
  PangoFontDescription* font=pango_context_get_font_description
       (pango_context);
  pango_font_description_set_size(font, PANGO_SCALE * size);
  gtk_widget_modify_font(label, font);
}


void set_label_font_family(GtkWidget *label, char *family)
{
  PangoContext *pango_context = gtk_widget_get_pango_context (label);
  PangoFontDescription* font=pango_context_get_font_description
       (pango_context);
  pango_font_description_set_family(font, family);
  gtk_widget_modify_font(label, font);
}


static void create_char(int index)
{
  if (chars[index].vbox)
    return;

  GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox_edit), vbox, FALSE, FALSE, 0);
  chars[index].vbox = vbox;

  GtkWidget *label = gtk_label_new(NULL);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  set_label_font_size(label, gcin_font_size);

  chars[index].label = label;

  GtkWidget *separator = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);
  chars[index].line = separator;
}


gboolean b_use_full_space = TRUE;

// the width of ascii space in firefly song
void set_label_space(GtkWidget *label)
{
#if 1
  gtk_label_set_text(GTK_LABEL(label), "　");
  return;
#else
  if (b_use_full_space) {
    gtk_label_set_text(GTK_LABEL(label), "　");
    return;
  }

  GtkRequisition sz;
  // the value of gtk_window_get_size is old
  gtk_label_set_text(GTK_LABEL(label), "測");

  gtk_widget_size_request(GTK_WIDGET(label), &sz);
  int width_chinese = sz.width;
//  dbg("height chinese %d\n", sz.height);

  gtk_label_set_text(GTK_LABEL(label), " ");
  gtk_widget_size_request(GTK_WIDGET(label), &sz);
//  dbg("height space %d\n", sz.height);
  int N = width_chinese / sz.width;

//  dbg("sz %d\n", sz.width);
  char tt[32];

  memset(tt, ' ', sizeof(tt));
  tt[N] = 0;

  gtk_label_set_text(GTK_LABEL(label), tt);
#endif
}


void disp_char(int index, u_char *ch)
{
  char tt[CH_SZ+1];

//  dbg("disp_char %d %c%c\n", index, ch[0], ch[1]);
  create_char(index);
  GtkWidget *label = chars[index].label;

  int u8len = u8cpy(tt, ch);

  if (ch[0]==' ' && ch[1]==' ')
      set_label_space(label);
  else {
    tt[u8len] = 0;
    gtk_label_set_text(GTK_LABEL(label), tt);
  }

  gtk_widget_show(label);
  gtk_widget_show(chars[index].vbox);

  get_win0_geom();

  if (win_x + win_xl >= dpy_xl)
    move_win0(dpy_xl - win_xl, win_y);
}

void hide_char(int index)
{
  if (!chars[index].vbox)
    return;
  gtk_label_set_text(GTK_LABEL(chars[index].label), "");
  gtk_widget_hide_all(chars[index].vbox);
}


void clear_chars_all()
{
  int i;

  for(i=0; i < MAX_PH_BF_EXT; i++) {
    hide_char(i);
  }

  compact_win0();
}


void draw_underline(int index)
{
  create_char(index);

  gtk_widget_show(chars[index].line);
}

void clear_underline(int index)
{
  gtk_widget_hide(chars[index].line);
}

void set_cursor_tsin(int index)
{
  GtkWidget *label = chars[index].label;

  if (!label)
    return;

  gtk_label_set_attributes(GTK_LABEL(label), attr_list);
}

void clr_tsin_cursor(int index)
{
  GtkWidget *label = chars[index].label;

  if (!label)
    return;

  gtk_label_set_attributes(GTK_LABEL(label), attr_list_blank);
}


void disp_tsin_pho(int index, char *pho)
{
  if (index>=3)
    return;

  char s[CH_SZ+1];

  if (pho[0]==' ')
    set_label_space(labels_pho[index]);
  else {
    utf8cpy(s, pho);
    gtk_label_set_text(GTK_LABEL(labels_pho[index]), s);
  }
}


void clr_in_area_pho_tsin()
{
  int i;

  for(i=0; i < 4; i++)
   disp_tsin_pho(i, " ");
}

void hide_pho(int index)
{
  gtk_widget_hide(labels_pho[index]);
}


void get_char_index_xy(int index, int *rx, int *ry)
{
  int wx, wy;
  GtkWidget *widget =chars[index].line;
  int ofs=0;

  if (!GTK_WIDGET_VISIBLE(widget)) {
    widget = chars[index].vbox;
    ofs=2;
  }

  gtk_widget_show(widget);

  GtkRequisition sz;
  gtk_widget_size_request(widget, &sz);

  gtk_widget_translate_coordinates(widget, gwin0,
         0, sz.height, &wx, &wy);

//  dbg("wy: %d %d   wx:%d\n", wy, sz.height, wx);

  gtk_window_get_position(GTK_WINDOW(gwin0), &win_x, &win_y);

  *rx = win_x + wx;
  *ry = win_y + wy + ofs;
}

void move_win_char_index(GtkWidget *win1, int index)
{
  int x,y;

  get_char_index_xy(index, &x, &y);

  GtkRequisition sz;

  gtk_widget_size_request(win1, &sz);
  int win1_xl = sz.width;  int win1_yl = sz.height;

  if (x + win1_xl > dpy_xl)

    x = dpy_xl - win1_xl;
  if (y + win1_yl > dpy_yl)
    y = win_y - win1_yl;

  gtk_window_move(GTK_WINDOW(win1), x, y);
}



void compact_win0_x()
{
  int win_xl, win_yl;

  gtk_window_get_size(GTK_WINDOW(gwin0), &win_xl, &win_yl);
  if (max_yl < win_yl)
    max_yl = win_yl;

  gtk_window_resize(GTK_WINDOW(gwin0), 60, max_yl);
}

void compact_win0()
{
  max_yl = 0;
  gtk_window_resize(GTK_WINDOW(gwin0), 60, 16);
}


void move_win0(int x, int y)
{
  if (current_CS && current_CS->fixed_pos) {
    x = current_CS->fixed_x;
    y = current_CS->fixed_y;
  }

  gtk_window_get_size(GTK_WINDOW(gwin0), &win_xl, &win_yl);

  if (x + win_xl > dpy_xl)
    x = dpy_xl - win_xl;
  if (x < 0)
    x = 0;

  if (y + win_yl > dpy_yl)
    y = dpy_yl - win_yl;
  if (y < 0)
    y = 0;

  win_x = x;  win_y = y;
  gtk_window_move(GTK_WINDOW(gwin0), x, y);

  show_win_sym();
}

GtkWidget *gwin1;

void disp_tsin_eng_pho(int eng_pho)
{
  static char *eng_pho_strs[]={"英", "注"};

  gtk_button_set_label(GTK_BUTTON(button_eng_ph), eng_pho_strs[eng_pho]);
}

void clear_tsin_line()
{
  int i;

  for(i=0; i < MAX_PH_BF_EXT; i++) {
    GtkWidget *line = chars[i].line;
    if (!line)
      continue;
    gtk_widget_hide(line);
  }
}

void exec_gcin_setup()
{
#if DEBUG
      dbg("exec gcin\n");
#endif
      system(GCIN_BIN_DIR"/gcin-setup &");
}


static void mouse_button_callback( GtkWidget *widget,GdkEventButton *event, gpointer data)
{
//  dbg("mouse_button_callback %d\n", event->button);
  switch (event->button) {
    case 1:
      create_win_sym();
      break;
    case 2:
      inmd_switch_popup_handler(widget, event);
      break;
    case 3:
      exec_gcin_setup();
      break;
  }

}


char file_pin_float[] = GCIN_ICON_DIR"/pin-float16.png";

static void set_currenet_IC_pin_image_pin()
{
  if (current_CS->fixed_pos)
    gtk_image_set_from_file(GTK_IMAGE(image_pin),GCIN_ICON_DIR"/pin-fixed24.png");
  else
    gtk_image_set_from_file(GTK_IMAGE(image_pin), file_pin_float);
}

static void cb_clicked_fixed_pos()
{

  if (!current_CS)
    return;

  current_CS->fixed_pos^=1;

//  dbg("cb_clicked_fixed_pos %d\n", current_CS->fixed_pos);

  if (current_CS->fixed_pos) {
    get_win0_geom();
    current_CS->fixed_x = win_x;  current_CS->fixed_y = win_y;
  }

  set_currenet_IC_pin_image_pin();
}

void tsin_toggle_eng_ch();

static void cb_clicked_eng_ph()
{
  tsin_toggle_eng_ch();
}


void create_win0()
{
  if (gwin0)
    return;

  gwin0 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width (GTK_CONTAINER (gwin0), 0);
  gtk_widget_realize (gwin0);
  GdkWindow *gdkwin0 = gwin0->window;
  xwin0 = GDK_WINDOW_XWINDOW(gdkwin0);
  gdk_window_set_override_redirect(gdkwin0, TRUE);

  g_signal_connect(G_OBJECT(gwin0),"button-press-event",
                   G_CALLBACK(mouse_button_callback), NULL);
}


#define PHO_IN_AREA_FONT_SIZE_DELTA 6

void create_win0_gui()
{
  if (frame)
    return;

  bzero(chars, sizeof(chars));

  frame = gtk_frame_new(NULL);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
  gtk_container_add (GTK_CONTAINER(gwin0), frame);

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 0);
  /* This packs the button into the gwin0 (a gtk container). */
  gtk_container_add (GTK_CONTAINER (frame), vbox_top);

  GtkWidget *hbox_row1 = gtk_hbox_new (FALSE, 0);
  /* This packs the button into the gwin0 (a gtk container). */
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_row1, FALSE, FALSE, 0);

  hbox_edit = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox_edit), 0);
  /* This packs the button into the gwin0 (a gtk container). */
  gtk_box_pack_start (GTK_BOX (hbox_row1), hbox_edit, FALSE, FALSE, 0);

  GdkColor color_bg, color_fg;
  gdk_color_parse("blue", &color_bg);
  gdk_color_parse("white", &color_fg);

  attr_list = pango_attr_list_new ();
  attr_list_blank = pango_attr_list_new ();

  PangoAttribute *blue_bg = pango_attr_background_new(
    color_bg.red, color_bg.green, color_bg.blue);
  blue_bg->start_index = 0;
  blue_bg->end_index = 128;
  pango_attr_list_insert (attr_list, blue_bg);

  PangoAttribute *white_fg = pango_attr_foreground_new(
    color_fg.red, color_fg.green, color_fg.blue);
  white_fg->start_index = 0;
  white_fg->end_index = 128;
  pango_attr_list_insert (attr_list, white_fg);


  button_pho = gtk_button_new();
  gtk_container_set_border_width (GTK_CONTAINER (button_pho), 0);
  gtk_box_pack_start (GTK_BOX (hbox_row1), button_pho, FALSE, FALSE, 0);
  GtkWidget *hbox_pho = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (button_pho), hbox_pho);

  g_signal_connect(G_OBJECT(button_pho),"button-press-event",
                   G_CALLBACK(mouse_button_callback), NULL);


  if (left_right_button_tips) {
    GtkTooltips *button_pho_tips = gtk_tooltips_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (button_pho_tips), button_pho, "左鍵符號，右鍵設定",NULL);
  }


  int i;
  for(i=0; i < 3;i ++) {
    GtkWidget *label = gtk_label_new("");
    labels_pho[i] = label;
    gtk_box_pack_start (GTK_BOX (hbox_pho), label, FALSE, FALSE, 0);
    set_label_font_size(label, gcin_font_size_tsin_pho_in);
  }

  hbox_row2 = gtk_hbox_new (FALSE, 0);
//  gtk_container_set_border_width (GTK_CONTAINER (hbox_row2), 0);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_row2, FALSE, FALSE, 0);
  GtkWidget *button_tsin = gtk_button_new_with_label("詞音");
  g_signal_connect_swapped (GTK_OBJECT (button_tsin), "button_press_event",
        G_CALLBACK (inmd_switch_popup_handler), NULL);
  gtk_box_pack_start (GTK_BOX (hbox_row2), button_tsin, FALSE, FALSE, 0);


  button_eng_ph = gtk_button_new_with_label("注");
  gtk_box_pack_start (GTK_BOX (hbox_row2), button_eng_ph, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_eng_ph), "clicked",
      G_CALLBACK (cb_clicked_eng_ph), (gpointer) NULL);

  image_pin = gtk_image_new_from_file(file_pin_float);
  GtkWidget *event_box_pin = gtk_event_box_new();
  gtk_container_add (GTK_CONTAINER (event_box_pin), image_pin);
  gtk_box_pack_start (GTK_BOX (hbox_row2), event_box_pin, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (event_box_pin), "button_press_event",
      G_CALLBACK (cb_clicked_fixed_pos), (gpointer) NULL);

  clr_in_area_pho_tsin();

  gtk_widget_show_all (gwin0);

  disp_hide_tsin_status_row();

  gdk_flush();

  create_win1();
  create_win1_gui();
}


void destory_win0()
{
  if (!gwin0)
    return;

  gtk_widget_destroy(gwin0);
}

void get_win0_geom()
{
  gtk_window_get_position(GTK_WINDOW(gwin0), &win_x, &win_y);

  GtkRequisition sz;
  // the value of gtk_window_get_size is old
  gtk_widget_size_request(gwin0, &sz);
  win_xl = sz.width;  win_yl = sz.height;
}

void show_win0()
{
  create_win0();
  create_win0_gui();
  set_currenet_IC_pin_image_pin();

  if (current_CS) {
    if (current_CS->fixed_pos)
      move_win0(0,0);
  }
#if 0
  else
    dbg("show_win0 current_CS is null");
#endif
  gtk_widget_show(gwin0);
  show_win_sym();
}

void hide_win0()
{
  gtk_widget_hide(gwin0);
  hide_selections_win();
  hide_win_sym();
}

void bell()
{
  XBell(dpy, -97);
//  abort();
}

void change_win1_font();

void change_tsin_font_size()
{
  int i;

  if (!frame)
    return;

  for(i=0; i < 3;i ++) {
    set_label_font_size(labels_pho[i], gcin_font_size_tsin_pho_in);
  }

  for(i=0; i < MAX_PH_BF_EXT; i++) {
    GtkWidget *label = chars[i].label;

    set_label_font_size(label, gcin_font_size);
  }

  compact_win0();

  change_win1_font();
}

