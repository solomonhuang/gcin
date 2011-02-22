#include "gcin.h"
#include "gtab.h"
#include "win-sym.h"
#include "gst.h"

extern gboolean test_mode;
static int current_gcin_inner_frame;
static int current_gtab_in_row1;
static int current_gtab_vertical_select;

GtkWidget *gwin_gtab;
static GtkWidget *top_bin;
static GtkWidget *label_full, *label_gtab_sele, *label_gtab_pre_sel;
static GtkWidget *label_gtab = NULL;
static GtkWidget *label_input_method_name;
static GtkWidget *label_key_codes;
#if WIN32
char str_key_codes[128];
gboolean better_key_codes;
#endif
static GtkWidget *box_gtab_im_name;
static GtkWidget *hbox_row2;
static GtkWidget *label_page;
static GtkWidget *label_edit;
static GdkColor better_color;
gboolean last_cursor_off;

Window xwin_gtab;
void set_label_space(GtkWidget *label);
void minimize_win_gtab();
gboolean win_size_exceed(GtkWidget *win), gtab_phrase_on(), gcin_edit_display_ap_only();
void move_win_gtab(int x, int y), toggle_win_sym();
int win_gtab_max_key_press;

unich_t eng_full_str[]=_L("英/全");
unich_t cht_full_str[]=_L("全");
unich_t cht_halt_str[]=_L("半");

static void adj_gtab_win_pos()
{
  if (win_size_exceed(gwin_gtab))
    move_win_gtab(current_in_win_x, current_in_win_y);
}

void disp_gtab(char *str)
{
#if WIN32
  if (test_mode)
    return;
#endif
   if (!label_gtab)
     return;
  if (str) {
    gtk_widget_show(label_gtab);
    gtk_label_set_text(GTK_LABEL(label_gtab), str);
  }
  else
    gtk_widget_hide(label_gtab);

  adj_gtab_win_pos();
}


void set_gtab_input_color(GdkColor *color)
{
  if (label_gtab) gtk_widget_modify_fg(label_gtab, GTK_STATE_NORMAL, color);
}

void set_gtab_input_error_color()
{
  GdkColor red;
  gdk_color_parse("red", &red);
  set_gtab_input_color(&red);
}

void clear_gtab_input_error_color()
{
  if (test_mode)
    return;
  set_gtab_input_color(NULL);
}


static gboolean need_label_edit();

void gtab_disp_empty(char *tt, int N)
{
  int i;

  if (!need_label_edit())
    return;

  for (i=0;i < N; i++)
    strcat(tt, _(_L("﹍")));
}

void clear_gtab_in_area()
{
#if 0
  disp_gtab(NULL);
#else
  if (!cur_inmd)
    return;
  char tt[64];
  tt[0]=0;
  gtab_disp_empty(tt, win_gtab_max_key_press);
  disp_gtab(tt);
#endif
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
  if (label_gtab_pre_sel)
    set_label_font_size(label_gtab_pre_sel, gcin_font_size);

  set_label_font_size(label_edit, gcin_font_size);

  set_label_font_size(label_gtab, gcin_font_size_gtab_in);

  set_disp_im_name();

  change_win_fg_bg(gwin_gtab, label_gtab_sele);
}

void show_win_gtab();

void disp_gtab_sel(char *s)
{
  if (!label_gtab_sele)
    return;
#if WIN32
  if (test_mode)
    return;
#endif

  if (!s[0])
    gtk_widget_hide(label_gtab_sele);
  else {
    if (gwin_gtab && !GTK_WIDGET_VISIBLE(gwin_gtab))
       show_win_gtab();
    gtk_widget_show(label_gtab_sele);
  }

//  dbg("disp_gtab_sel '%s'\n", s);
  gtk_label_set_markup(GTK_LABEL(label_gtab_sele), s);
  minimize_win_gtab();
  adj_gtab_win_pos();
}

void set_key_codes_label(char *s, int better)
{
  if (!label_key_codes)
    return;

  if (test_mode)
    return;

  if (s && strlen(s)) {
    if (hbox_row2 && (!gtab_hide_row2 || ggg.wild_mode
#if WIN32
		|| str_key_codes[0]
#endif
		)) {
      gtk_widget_show(hbox_row2);
    }
  } else {
    if (gtab_hide_row2 && hbox_row2) {
      gtk_widget_hide(hbox_row2);
    }
  }

  if (better)
    gtk_widget_modify_fg(label_key_codes, GTK_STATE_NORMAL, &better_color);
  else
    gtk_widget_modify_fg(label_key_codes, GTK_STATE_NORMAL, NULL);

  gtk_label_set_text(GTK_LABEL(label_key_codes), s);
#if WIN32
  better_key_codes = better;
  if (s && s != str_key_codes)
    strcpy(str_key_codes, s);
  else
    str_key_codes[0]=0;
#endif
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
  if (!gtab_disp_im_name)
    return;
  if (!label_input_method_name)
    return;
//  dbg("set_gtab_input_method_name b '%s'\n", s);
  gtk_widget_show(label_input_method_name);
  gtk_label_set_text(GTK_LABEL(label_input_method_name), s);
}


void create_win_gtab()
{
  if (gwin_gtab)
    return;

  gwin_gtab = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_has_resize_grip(GTK_WINDOW(gwin_gtab), FALSE);
#if WIN32
  set_no_focus(gwin_gtab);
#endif
  gtk_container_set_border_width (GTK_CONTAINER (gwin_gtab), 0);
  gtk_widget_realize (gwin_gtab);

#if UNIX
  GdkWindow *gdkwin = gtk_widget_get_window(gwin_gtab);
  set_no_focus(gwin_gtab);
  xwin_gtab = GDK_WINDOW_XWINDOW(gdkwin);
#else
  win32_init_win(gwin_gtab);
#endif
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


void toggle_half_full_char();

gint inmd_switch_popup_handler (GtkWidget *widget, GdkEvent *event);


void show_hide_label_edit()
{
#if WIN32
  if (test_mode)
    return;
#endif
  if (!label_edit)
    return;

  if (gcin_edit_display_ap_only() || !gtab_phrase_on()) {
    gtk_widget_hide(label_edit);
    return;
  } else
    gtk_widget_show(label_edit);
}


void disp_label_edit(char *str)
{
  if (test_mode)
    return;

  if (!label_edit)
    return;

  show_hide_label_edit();

  gtk_label_set_markup(GTK_LABEL(label_edit), str);
}

static gboolean need_label_edit()
{
  return gtab_phrase_on() && !gcin_edit_display_ap_only();
}

static int current_gtab_phrase_pre_select;

static void destroy_if_necessary()
{
  gboolean new_need_label_edit = need_label_edit();
  gboolean new_last_cursor_off = gtab_in_row1 && new_need_label_edit;

//  dbg("zzz %d %d\n", gtab_in_row1, new_need_label_edit);

  if (!top_bin ||
      current_gcin_inner_frame == gcin_inner_frame &&
      current_gtab_in_row1 == gtab_in_row1 &&
          new_last_cursor_off == last_cursor_off &&
      current_gtab_vertical_select == gtab_vertical_select &&
      current_gtab_phrase_pre_select == gtab_phrase_pre_select &&
      (new_need_label_edit && label_edit || !new_need_label_edit && !label_edit)
      )
    return;
#if 0
  dbg("gcin_inner_frame %d,%d,  gtab_in_row1 %d,%d  cursor_off% d,%d   vert:%d,%d  edit:%d,%d\n",
    current_gcin_inner_frame,gcin_inner_frame, current_gtab_in_row1,gtab_in_row1,
    new_last_cursor_off, last_cursor_off, current_gtab_vertical_select, gtab_vertical_select,
    new_need_label_edit, label_edit!=0);
#endif
  current_gtab_phrase_pre_select = gtab_phrase_pre_select;

  gtk_widget_destroy(top_bin);
  top_bin = NULL;
  label_edit = NULL;
  hbox_row2 = NULL;
}


void create_win_gtab_gui_simple()
{
//  dbg("create_win_gtab_gui ..... %d, %d\n", current_CS->use_preedit, gcin_edit_display);

  destroy_if_necessary();

  if (top_bin)
    return;

  dbg("create_win_gtab_gui_simple\n");

  last_cursor_off = FALSE;

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

  GtkWidget *hbox_edit = NULL;

  gboolean b_need_label_edit = need_label_edit();

  if (b_need_label_edit) {
    hbox_edit = gtk_hbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (vbox_top), hbox_edit);
    GtkWidget *align_edit = gtk_alignment_new (0, 0.0, 0, 0);
    gtk_box_pack_start (GTK_BOX (hbox_edit), align_edit, FALSE, FALSE, 0);
    label_edit = gtk_label_new(NULL);
    gtk_container_add (GTK_CONTAINER (align_edit), label_edit);
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

//    dbg("zzz %d zzz %d %d\n", b_need_label_edit, gtab_phrase_on(), gcin_edit_display_ap_only());

    if (b_need_label_edit) {
      last_cursor_off = TRUE;
      gtk_box_pack_start (GTK_BOX (hbox_edit), event_box_gtab, FALSE, FALSE, 0);
    } else
      gtk_box_pack_start (GTK_BOX (hbox_row1), event_box_gtab, FALSE, FALSE, 0);

    if (!gtab_vertical_select)
      gtk_box_pack_start (GTK_BOX (hbox_row1), align, FALSE, FALSE, 0);
  }

  if (gtab_phrase_pre_select) {
    label_gtab_pre_sel = gtk_label_new(NULL);
    gtk_box_pack_start (GTK_BOX (vbox_top), label_gtab_pre_sel, FALSE, FALSE, 0);
  }

  hbox_row2 = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (vbox_top), hbox_row2);

  label_full = gtk_label_new(_(cht_full_str));

  gtk_box_pack_start (GTK_BOX (hbox_row2), label_full, FALSE, FALSE, 0);

  GtkWidget *event_box_input_method_name = gtk_event_box_new();
  gtk_box_pack_start (GTK_BOX (hbox_row2), event_box_input_method_name, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (event_box_input_method_name), 0);

  GtkWidget *frame_input_method_name = gtk_frame_new(NULL);
  gtk_container_add (GTK_CONTAINER (event_box_input_method_name), frame_input_method_name);
  gtk_container_set_border_width (GTK_CONTAINER (frame_input_method_name), 0);

  label_input_method_name = gtk_label_new("");
//  dbg("gtk_label_new label_input_method_name\n");
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
#if GTK_CHECK_VERSION(2,12,0)
    gtk_widget_set_tooltip_text (event_box_gtab, _(_L("左鍵符號，右鍵設定")));
#else
    GtkTooltips *button_gtab_tips = gtk_tooltips_new ();
    gtk_tooltips_set_tip (GTK_TOOLTIPS (button_gtab_tips), event_box_gtab, _("左鍵符號，右鍵設定"),NULL);
#endif
  }

  GtkWidget *hbox_gtab = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame_gtab), hbox_gtab);

  int i;
  label_gtab = gtk_label_new(NULL);
  gtk_box_pack_start (GTK_BOX (hbox_gtab), label_gtab, FALSE, FALSE, 0);


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
  gtk_widget_hide(label_gtab_sele);
  if (label_gtab_pre_sel)
    gtk_widget_hide(label_gtab_pre_sel);

  show_hide_label_edit();

  set_disp_im_name();
  gtk_widget_hide(label_full);

  if (gtab_hide_row2)
    gtk_widget_hide(hbox_row2);
}


static void create_win_gtab_gui()
{
  create_win_gtab_gui_simple();
  current_gtab_in_row1 = gtab_in_row1;
  current_gtab_vertical_select = gtab_vertical_select;
  current_gcin_inner_frame = gcin_inner_frame;
  gdk_color_parse("red", &better_color);
}


void change_win_gtab_style()
{
  destroy_if_necessary();
  create_win_gtab_gui();
}


void init_gtab(int inmdno);
gboolean gtab_has_input();
extern gboolean force_show;

void show_win_gtab()
{
#if WIN32
  if (test_mode)
    return;
#endif

  create_win_gtab();
  create_win_gtab_gui();
#if WIN32
  // window was destroyed
  if (gcin_pop_up_win)
    set_key_codes_label(str_key_codes, better_key_codes);
#endif

  if (current_CS) {
    if (current_CS->fixed_pos)
      move_win_gtab(0,0);
  }

//  init_gtab(current_CS->in_method);

  if (gcin_pop_up_win && !gtab_has_input() && !force_show && poo.same_pho_query_state==SAME_PHO_QUERY_none)
    return;

//  dbg("show_win_gtab()\n");
#if UNIX
  if (!GTK_WIDGET_VISIBLE(gwin_gtab))
#endif
    gtk_widget_show(gwin_gtab);

#if UNIX
  if (current_CS->b_raise_window)
#endif
    gtk_window_present(GTK_WINDOW(gwin_gtab));

#if WIN32
	move_win_gtab(current_in_win_x, current_in_win_y);
#endif

  show_win_sym();
}


void hide_win_sym();
void close_gtab_pho_win();

void destroy_win_gtab()
{
  if (!gwin_gtab)
    return;

  gtk_widget_destroy(gwin_gtab);
  gwin_gtab = NULL;
  top_bin = NULL;
  hbox_row2 = NULL;
  label_full=NULL;
  label_gtab_sele = NULL;
  label_gtab = NULL;
  label_input_method_name = NULL;
  label_key_codes = NULL;
  box_gtab_im_name = NULL;
  label_page = NULL;
  label_edit = NULL;
}

void hide_win_gtab()
{
#if WIN32
  if (test_mode)
    return;
#endif
  win_gtab_max_key_press = 0;

  if (!gwin_gtab)
    return;

//  dbg("hide_win_gtab\n");
  if (gwin_gtab) {
#if UNIX
    gtk_widget_hide(gwin_gtab);
#else
	  destroy_win_gtab();
#endif
  }

  close_gtab_pho_win();
  hide_win_sym();
}

void minimize_win_gtab()
{
#if WIN32
  if (test_mode)
    return;
#endif
//  if (!GTK_WIDGET_VISIBLE(gwin_gtab))
//    return;

//  dbg("minimize_win_gtab\n");
  // bug in GTK, in key disp is incomplete
  gtk_window_resize(GTK_WINDOW(gwin_gtab), 20, 20);
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

char *get_full_str()
{
  switch (current_CS->im_state) {
    case GCIN_STATE_CHINESE:
      if (current_CS->b_half_full_char)
        return _(cht_full_str);
      break;
    case GCIN_STATE_ENG_FULL:
      return _(eng_full_str);
  }
  return _(cht_halt_str);
}

void win_gtab_disp_half_full()
{
  if (test_mode)
    return;

  if (label_full) {
    if (current_CS->im_state == GCIN_STATE_CHINESE && (!current_CS->b_half_full_char))
      gtk_widget_hide(label_full);
    else
      gtk_widget_show(label_full);
  }

  if (label_gtab)
    gtk_label_set_text(GTK_LABEL(label_gtab), get_full_str());

  minimize_win_gtab();
}


void disp_gtab_pre_sel(char *s)
{
//  dbg("disp_gtab_pre_sel %s\n", s);
  gtk_widget_show(label_gtab_pre_sel);
  gtk_label_set_markup(GTK_LABEL(label_gtab_pre_sel), s);
}

void hide_gtab_pre_sel()
{
  tss.pre_selN = 0;
  tss.ctrl_pre_sel = FALSE;
  gtk_widget_hide(label_gtab_pre_sel);

  move_win_gtab(current_in_win_x, current_in_win_y);
}
