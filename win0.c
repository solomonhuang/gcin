#include "gcin.h"
#include "pho.h"
#include "win-sym.h"

GtkWidget *gwin0;
Window xwin0;
extern Display *dpy;
static GtkWidget *top_bin;
static GtkWidget *image_pin;
int current_pho_simple_win;
int current_gcin_inner_frame;

static GtkWidget *hbox_edit;
static PangoAttrList* attr_list, *attr_list_blank;

void compact_win0();
void move_win0(int x, int y);
void get_win0_geom();

static struct {
  GtkWidget *vbox;
  GtkWidget *label;
  GtkWidget *line;
} chars[MAX_PH_BF_EXT];


static GtkWidget *button_pho;
static GtkWidget *label_pho;
static char text_pho[6][CH_SZ];
extern int text_pho_N;
static GtkWidget *button_eng_ph;
static int max_yl;

static void create_win0_gui();

static void recreate_win0()
{
  bzero(chars, sizeof(chars));
  label_pho = NULL;

  create_win0_gui();
}


void change_win0_style()
{
  if (!top_bin || (current_pho_simple_win == pho_simple_win &&
      current_gcin_inner_frame == gcin_inner_frame))
    return;

  gtk_widget_destroy(top_bin);
  top_bin = NULL;

  current_pho_simple_win = pho_simple_win;
  current_gcin_inner_frame = gcin_inner_frame;
  recreate_win0();
}

void set_label_font_size(GtkWidget *label, int size)
{
  if (!label)
    return;

  PangoContext *pango_context = gtk_widget_get_pango_context (label);
  PangoFontDescription* font=pango_context_get_font_description
       (pango_context);
  pango_font_description_set_family(font, gcin_font_name);
  pango_font_description_set_size(font, PANGO_SCALE * size);
  gtk_widget_modify_font(label, font);
}

/* there is a bug in gtk, if the widget is created and hasn't been processed by
   gtk_main(), the coodinate of the widget is sometimes invalid.
   We use pre-create to overcome this bug.
*/
static void create_char(int index)
{
  int i;

  GdkColor fg;
  gdk_color_parse(gcin_win_color_fg, &fg);

  for(i=index; i<=index+1 && i < MAX_PH_BF_EXT; i++) {
    if (chars[i].vbox)
      continue;

    GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (hbox_edit), vbox, FALSE, FALSE, 0);
    chars[i].vbox = vbox;

    GtkWidget *label = gtk_label_new(NULL);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

    set_label_font_size(label, gcin_font_size);

    chars[i].label = label;
    GtkWidget *separator =  gtk_drawing_area_new();
    GdkColor color_bg;
    gdk_color_parse(tsin_phrase_line_color, &color_bg);
    gtk_widget_modify_bg(separator, GTK_STATE_NORMAL, &color_bg);
    gtk_widget_set_size_request(separator, 8, 2);
    gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);
    chars[i].line = separator;

    if (gcin_win_color_use)
      gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &fg);

    gtk_widget_show(vbox);
    gtk_widget_show(label);
  }
}

static void change_tsin_line_color()
{
  int i;

  GdkColor fg;
  gdk_color_parse(gcin_win_color_fg, &fg);

  for(i=0; i < MAX_PH_BF_EXT; i++) {
    if (!chars[i].line)
      continue;
    GdkColor color_bg;
    gdk_color_parse(tsin_phrase_line_color, &color_bg);
    gtk_widget_modify_bg(chars[i].line, GTK_STATE_NORMAL, &color_bg);
    gtk_widget_modify_fg(chars[i].label, GTK_STATE_NORMAL, gcin_win_color_use ? &fg:NULL);
  }
}



gboolean b_use_full_space = TRUE;

// the width of ascii space in firefly song
void set_label_space(GtkWidget *label)
{
  gtk_label_set_text(GTK_LABEL(label), "　");
  return;
}


void disp_char(int index, char *ch)
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

  get_win0_geom();
  if (win_x + win_xl >= dpy_xl)
    move_win0(dpy_xl - win_xl, win_y);

  gtk_widget_show(chars[index].vbox);
  gtk_widget_show(label);
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





void disp_pho_sub(GtkWidget *label, int index, char *pho)
{
  if (index>=text_pho_N)
    return;


  if (pho[0]==' ' && !pin_juyin) {
    u8cpy(text_pho[index], "　");
  }
  else {
    u8cpy(text_pho[index], pho);
  }

  char s[text_pho_N * CH_SZ+1];


  int tn = 0;
  int i;
  for(i=0; i < text_pho_N; i++) {
    int n = utf8cpy(s + tn, text_pho[i]);
    tn += n;
  }

  gtk_label_set_text(label, s);
}


void disp_tsin_pho(int index, char *pho)
{
  disp_pho_sub(label_pho, index, pho);
}

void clr_in_area_pho_tsin()
{
  int i;

  for(i=0; i < text_pho_N; i++)
   disp_tsin_pho(i, " ");
}


void get_char_index_xy(int index, int *rx, int *ry)
{
  int wx, wy;
  int ofs=0;

#if 0
  GtkWidget *widget =chars[index].line;

  if (!GTK_WIDGET_VISIBLE(widget)) {
    ofs=2;
    widget = chars[index].vbox;
  }
#endif
#if 0
  GtkWidget *widget = chars[index].label;
#endif
#if 1
  GtkWidget *widget = chars[index].vbox;
#endif

  gtk_widget_show(widget);

  GtkRequisition sz;
  gtk_widget_size_request(widget, &sz);

  gtk_widget_translate_coordinates(widget, gwin0,
         0, sz.height, &wx, &wy);

//  dbg("%d wx:%d\n", index,  wx);

  int win_x, win_y;

  gtk_window_get_position(GTK_WINDOW(gwin0), &win_x, &win_y);

  *rx = win_x + wx;
  *ry = win_y + wy + ofs;
}

void move_win_char_index(GtkWidget *win1, int index)
{
  int x,y;

  get_char_index_xy(index, &x, &y);

  int win1_xl, win1_yl;
  get_win_size(win1, &win1_xl, &win1_yl);

  if (x + win1_xl > dpy_xl)
    x = dpy_xl - win1_xl;
  if (y + win1_yl > dpy_yl)
    y = win_y - win1_yl;

//  dbg("move_win_char_index:%d %d\n", index, x);
  gtk_window_move(GTK_WINDOW(win1), x, y);
}

#define MIN_X_SIZE 16

static int best_win_x, best_win_y;

static void raw_move(int x, int y)
{
  int xl, yl;

  get_win_size(gwin0, &xl, &yl);

  if (x + xl > dpy_xl)
    x = dpy_xl - xl;
  if (y + yl > dpy_yl)
    y = dpy_yl - yl;

  gtk_window_move(GTK_WINDOW(gwin0), x, y);
}

void compact_win0_x()
{
  int win_xl, win_yl;

  gtk_window_get_size(GTK_WINDOW(gwin0), &win_xl, &win_yl);
  if (max_yl < win_yl)
    max_yl = win_yl;

  gtk_window_resize(GTK_WINDOW(gwin0), MIN_X_SIZE, max_yl);
  raw_move(best_win_x, best_win_y);
}

void compact_win0()
{
  max_yl = 0;
  gtk_window_resize(GTK_WINDOW(gwin0), MIN_X_SIZE, 16);
  raw_move(best_win_x, best_win_y);
}

gboolean tsin_has_input();

void move_win0(int x, int y)
{
  best_win_x = x;
  best_win_y = y;

  gtk_window_get_size(GTK_WINDOW(gwin0), &win_xl, &win_yl);

  if (x + win_xl > dpy_xl)
    x = dpy_xl - win_xl;
  if (x < 0)
    x = 0;

  if (y + win_yl > dpy_yl)
    y = dpy_yl - win_yl;
  if (y < 0)
    y = 0;


  gtk_window_move(GTK_WINDOW(gwin0), x, y);
  win_x = x;
  win_y = y;

  if (gcin_pop_up_win && !tsin_has_input())
    return;

  show_win_sym();
}

GtkWidget *gwin1;

void disp_tsin_eng_pho(int eng_pho)
{
  static char *eng_pho_strs[]={"英", "注"};

  if (!button_eng_ph)
    return;

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

  char pidstr[32];
  sprintf(pidstr, "GCIN_PID=%d",getpid());
  putenv(pidstr);
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
      inmd_switch_popup_handler(widget, (GdkEvent *)event);
      break;
    case 3:
      exec_gcin_setup();
      break;
  }

}


char file_pin_float[] = GCIN_ICON_DIR"/pin-float16.png";

static void set_currenet_IC_pin_image_pin()
{
  if (!image_pin)
    return;

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

void set_no_focus(GtkWidget *win)
{
  gdk_window_set_override_redirect(win->window, TRUE);
#if GTK_MAJOR_VERSION >=2 && GTK_MINOR_VERSION >= 6
  gtk_window_set_accept_focus(win, FALSE);
#endif
#if GTK_MAJOR_VERSION >=2 && GTK_MINOR_VERSION >= 6
  gtk_window_set_focus_on_map (win, FALSE);
#endif
}


void create_win0()
{
  if (gwin0)
    return;

  gwin0 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width (GTK_CONTAINER (gwin0), 0);
  gtk_widget_realize (gwin0);
  GdkWindow *gdkwin0 = gwin0->window;
#if 0
  GError *err = NULL;
  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(
    "/usr/share/icons/gcin/pin-float16.png", &err);
  if (err)
    p_err("not load");

  GdkPixmap *pixmap;
  GdkBitmap *bitmap;

  gdk_pixbuf_render_pixmap_and_mask(pixbuf, &pixmap, &bitmap, 100);
  gdk_window_set_back_pixmap(gdkwin0, pixmap, FALSE);

  dbg("gdkwin0:%x\n", gdkwin0);
#endif
  xwin0 = GDK_WINDOW_XWINDOW(gdkwin0);

  set_no_focus(gwin0);
}


void create_win1();

static void create_cursor_attr()
{
  if (attr_list)
    pango_attr_list_unref(attr_list);

  GdkColor color_bg, color_fg;
  gdk_color_parse(tsin_cursor_color, &color_bg);
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
}

void create_win1_gui();
static void set_win0_bg()
{
#if 1
  change_win_bg(gwin0);
#endif
}

void change_win1_font();

static void create_win0_gui()
{
  if (top_bin)
    return;

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (gwin0), 0);

  if (gcin_inner_frame) {
    GtkWidget *frame;
    top_bin = frame = gtk_frame_new(NULL);
    gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
    gtk_container_add (GTK_CONTAINER(gwin0), frame);
    gtk_container_add (GTK_CONTAINER (frame), vbox_top);
  } else {
    top_bin = vbox_top;
    gtk_container_add (GTK_CONTAINER (gwin0), vbox_top);
  }

  bzero(chars, sizeof(chars));

  GtkWidget *hbox_row1 = gtk_hbox_new (FALSE, 0);
  /* This packs the button into the gwin0 (a gtk container). */
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_row1, FALSE, FALSE, 0);

  hbox_edit = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox_edit), 0);
  /* This packs the button into the gwin0 (a gtk container). */
  gtk_box_pack_start (GTK_BOX (hbox_row1), hbox_edit, FALSE, FALSE, 0);

  create_cursor_attr();

  button_pho = gtk_button_new();
  gtk_container_set_border_width (GTK_CONTAINER (button_pho), 0);
  gtk_box_pack_start (GTK_BOX (hbox_row1), button_pho, FALSE, FALSE, 0);

  g_signal_connect(G_OBJECT(button_pho),"button-press-event",
                   G_CALLBACK(mouse_button_callback), NULL);

  if (left_right_button_tips) {
    GtkTooltips *button_pho_tips = gtk_tooltips_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (button_pho_tips), button_pho, "左鍵符號，右鍵設定",NULL);
  }


  label_pho = gtk_label_new("");
  set_label_font_size(label_pho, gcin_font_size_tsin_pho_in);
  gtk_container_add (GTK_CONTAINER (button_pho), label_pho);


  if (!pho_simple_win) {
    GtkWidget *hbox_row2 = gtk_hbox_new (FALSE, 0);
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
  }

  clr_in_area_pho_tsin();

  gtk_widget_show_all (gwin0);
  gdk_flush();
  gtk_widget_hide (gwin0);

  create_win1();
  create_win1_gui();

  set_win0_bg();

  change_win1_font();
}


void destroy_win0()
{
  if (!gwin0)
    return;

  gtk_widget_destroy(gwin0);
}

void get_win0_geom()
{
  gtk_window_get_position(GTK_WINDOW(gwin0), &win_x, &win_y);

  get_win_size(gwin0, &win_xl, &win_yl);
}

gboolean tsin_has_input();

void show_win0()
{
//  dbg("show_win0\n");
  create_win0();
  create_win0_gui();
  set_currenet_IC_pin_image_pin();

  if (gcin_pop_up_win && !tsin_has_input())
    return;

  gtk_widget_show(gwin0);
//  dbg("show_win0 b\n");
  show_win_sym();
#if 1
  if (current_CS->b_raise_window) {
    gtk_window_present(gwin0);
    raise_tsin_selection_win();
  }
#endif
}

void hide_selections_win();
void hide_win0()
{
//  dbg("hide_win0\n");
  gtk_widget_hide(gwin0);
  hide_selections_win();
  hide_win_sym();
}

void bell()
{
#if 1
  XBell(dpy, -97);
#else
  gdk_beep();
#endif
//  abort();
}


void change_tsin_font_size()
{
  int i;

  if (!top_bin)
    return;

  GdkColor fg;
  gdk_color_parse(gcin_win_color_fg, &fg);

  set_label_font_size(label_pho, gcin_font_size_tsin_pho_in);

  for(i=0; i < MAX_PH_BF_EXT; i++) {
    GtkWidget *label = chars[i].label;

    set_label_font_size(label, gcin_font_size);

    if (gcin_win_color_use)
      gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &fg);
  }

  compact_win0();

  change_win1_font();

  set_win0_bg();
  change_tsin_line_color();
}



void show_button_pho(gboolean bshow)
{
  if (bshow)
    gtk_widget_show(button_pho);
  else {
    gtk_widget_hide(button_pho);
    compact_win0();
  }
}

char *get_full_str();

void win_tsin_disp_half_full()
{
  gtk_label_set_text(GTK_LABEL(label_pho), get_full_str());

  compact_win0();
}


void drawcursor();

void change_tsin_color()
{
  change_tsin_line_color();
  create_cursor_attr();

  drawcursor();
}
