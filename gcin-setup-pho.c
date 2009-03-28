#include "gcin.h"
#include "pho.h"


static struct {
  char *name;
  char *kbm;
}  kbm_sel[]= {
 {"標準 standard", "zo"},
 {"標準 standard 使用 asdf 選擇", "zo-asdf"},
 {"倚天 Eten", "et"},
 {"倚天 Eten 使用 asdf 選擇", "et-asdf"},
 {"倚天 26 鍵", "et26"},
 {"倚天 26 鍵,使用 asdf 選擇", "et26-asdf"},
 {"許氏(國音,自然)", "hsu"},
 {"IBM", "ibm"},
 {NULL, NULL}
};

static GtkWidget *check_button_tsin_phrase_pre_select,
                 *check_button_phonetic_char_dynamic_sequence,
                 *check_button_pho_simple_win,
                 *check_button_pho_hide_row2,
                 *check_button_pho_in_row1,
                 *check_button_phonetic_huge_tab,
                 *check_button_tsin_tone_char_input,
                 *check_button_tsin_tab_phrase_end,
                 *spinner_tsin_buffer_size;

static GtkWidget *opt_kbm_opts;


static struct {
  char *name;
  int key;
} tsin_eng_ch_sw[]={
  {"CapsLock", TSIN_CHINESE_ENGLISH_TOGGLE_KEY_CapsLock},
  {"Tab", TSIN_CHINESE_ENGLISH_TOGGLE_KEY_Tab},
  {"Shift(限非 XIM)", TSIN_CHINESE_ENGLISH_TOGGLE_KEY_Shift},
};
int tsin_eng_ch_swN = sizeof(tsin_eng_ch_sw) / sizeof(tsin_eng_ch_sw[0]);


static struct {
  char *name;
  int key;
} tsin_space_options[]={
  {"選擇同音字", TSIN_SPACE_OPT_SELECT_CHAR},
  {"送出編輯區的內容", TSIN_SPACE_OPT_FLUSH_BUFFER},
};
int tsin_space_optionsN = sizeof(tsin_space_options) / sizeof(tsin_space_options[0]);



static GtkWidget *gcin_kbm_window = NULL;

static int new_select_idx, new_select_idx_tsin_sw, new_select_idx_tsin_space_opt;
static GdkColor tsin_phrase_line_gcolor, tsin_cursor_gcolor;


static gboolean cb_ok( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  int idx = gtk_option_menu_get_history (GTK_OPTION_MENU (opt_kbm_opts));

  save_gcin_conf_str(PHONETIC_KEYBOARD, kbm_sel[idx].kbm);

  save_gcin_conf_int(TSIN_CHINESE_ENGLISH_TOGGLE_KEY,
                     tsin_eng_ch_sw[new_select_idx_tsin_sw].key);

  save_gcin_conf_int(TSIN_SPACE_OPT,
                     tsin_space_options[new_select_idx_tsin_space_opt].key);

  save_gcin_conf_int(TSIN_PHRASE_PRE_SELECT,
       gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_tsin_phrase_pre_select)));

  save_gcin_conf_int(PHONETIC_CHAR_DYNAMIC_SEQUENCE,
       gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_phonetic_char_dynamic_sequence)));

  save_gcin_conf_int(PHO_SIMPLE_WIN,
       gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_pho_simple_win)));

  save_gcin_conf_int(PHO_HIDE_ROW2,
       gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_pho_hide_row2)));

  save_gcin_conf_int(PHO_IN_ROW1,
       gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_pho_in_row1)));

  save_gcin_conf_int(PHONETIC_HUGE_TAB,
       gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_phonetic_huge_tab)));

  save_gcin_conf_int(TSIN_TONE_CHAR_INPUT,
       gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_tsin_tone_char_input)));


  save_gcin_conf_int(TSIN_TAB_PHRASE_END,
       gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_tsin_tab_phrase_end)));


  tsin_buffer_size = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_tsin_buffer_size));
  save_gcin_conf_int(TSIN_BUFFER_SIZE, tsin_buffer_size);

  gchar *cstr = gtk_color_selection_palette_to_string(&tsin_phrase_line_gcolor, 1);
  dbg("color %s\n", cstr);
  save_gcin_conf_str(TSIN_PHRASE_LINE_COLOR, cstr);
  g_free(cstr);


  cstr = gtk_color_selection_palette_to_string(&tsin_cursor_gcolor, 1);
  dbg("color %s\n", cstr);
  save_gcin_conf_str(TSIN_CURSOR_COLOR, cstr);
  g_free(cstr);

  send_gcin_message(GDK_DISPLAY(), "reload kbm");

  gtk_widget_destroy(gcin_kbm_window); gcin_kbm_window = NULL;

  return TRUE;
}


static void callback_button_clicked_tsin_sw( GtkWidget *widget, gpointer data)
{
  new_select_idx_tsin_sw = (int) data;
}


static void callback_button_clicked_tsin_space_opt( GtkWidget *widget, gpointer data)
{
  new_select_idx_tsin_space_opt = (int) data;
}


static int get_current_kbm_idx()
{
  char kbm_str[32];
  get_gcin_conf_str("phonetic-keyboard", kbm_str, "zo");

  int i;
  for(i=0; kbm_sel[i].kbm; i++)
    if (!strcmp(kbm_sel[i].kbm, kbm_str))
      return i;

  p_err("phonetic-keyboard->%s is not valid", kbm_str);
  return -1;
}

static int get_currnet_eng_ch_sw_idx()
{
  int i;
  for(i=0; i < tsin_eng_ch_swN; i++)
    if (tsin_eng_ch_sw[i].key == tsin_chinese_english_toggle_key)
      return i;

  p_err("tsin-chinese-english-switch->%d is not valid", tsin_chinese_english_toggle_key);
  return -1;
}


static int get_currnet_tsin_space_option_idx()
{
  int i;
  for(i=0; i < tsin_space_optionsN; i++)
    if (tsin_space_options[i].key == tsin_space_opt)
      return i;

  p_err("tsin-space-opt->%d is not valid", tsin_space_opt);
  return -1;
}

static gboolean close_kbm_window( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  gtk_widget_destroy(gcin_kbm_window); gcin_kbm_window = NULL;
  return TRUE;
}

static GtkWidget *da;

static void cb_save_tsin_phrase_line_color(GtkWidget *widget, gpointer user_data)
{
  GtkColorSelectionDialog *color_selector = (GtkColorSelectionDialog *)user_data;
  gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(color_selector->colorsel), &tsin_phrase_line_gcolor);

  gtk_widget_modify_bg(da, GTK_STATE_NORMAL, &tsin_phrase_line_gcolor);
}


static gboolean cb_tsin_phrase_line_color( GtkWidget *widget,
                                   gpointer   data )
{
   GtkColorSelectionDialog *color_selector = (GtkColorSelectionDialog *)gtk_color_selection_dialog_new ("詞音標示詞的底線顏色");

   gtk_color_selection_set_current_color(
           GTK_COLOR_SELECTION(color_selector->colorsel),
           &tsin_phrase_line_gcolor);


   g_signal_connect (GTK_OBJECT (color_selector->ok_button),
                     "clicked",
                     G_CALLBACK (cb_save_tsin_phrase_line_color),
                     (gpointer) color_selector);
#if 1
   g_signal_connect_swapped (GTK_OBJECT (color_selector->ok_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) color_selector);
#endif
   g_signal_connect_swapped (GTK_OBJECT (color_selector->cancel_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) color_selector);

   gtk_widget_show((GtkWidget*)color_selector);
   return TRUE;
}


static void cb_save_tsin_cursor_color(GtkWidget *widget, gpointer user_data)
{
  GtkColorSelectionDialog *color_selector = (GtkColorSelectionDialog *)user_data;
  gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(color_selector->colorsel), &tsin_cursor_gcolor);

  gtk_widget_modify_bg(da, GTK_STATE_NORMAL, &tsin_cursor_gcolor);
}


static gboolean cb_tsin_cursor_color( GtkWidget *widget,
                                   gpointer   data )
{
   GtkColorSelectionDialog *color_selector = (GtkColorSelectionDialog *)gtk_color_selection_dialog_new ("詞音標示詞的底線顏色");

   gtk_color_selection_set_current_color(
           GTK_COLOR_SELECTION(color_selector->colorsel),
           &tsin_cursor_gcolor);


   g_signal_connect (GTK_OBJECT (color_selector->ok_button),
                     "clicked",
                     G_CALLBACK (cb_save_tsin_cursor_color),
                     (gpointer) color_selector);
#if 1
   g_signal_connect_swapped (GTK_OBJECT (color_selector->ok_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) color_selector);
#endif
   g_signal_connect_swapped (GTK_OBJECT (color_selector->cancel_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) color_selector);

   gtk_widget_show((GtkWidget*)color_selector);
   return TRUE;
}



static GtkWidget *create_kbm_opts()
{

  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);

  opt_kbm_opts = gtk_option_menu_new ();
  gtk_box_pack_start (GTK_BOX (hbox), opt_kbm_opts, FALSE, FALSE, 0);
  GtkWidget *menu_kbm_opts = gtk_menu_new ();

  int i;
  int current_idx = get_current_kbm_idx();

  for(i=0; kbm_sel[i].name; i++) {
    GtkWidget *item = gtk_menu_item_new_with_label (kbm_sel[i].name);

    gtk_menu_shell_append (GTK_MENU_SHELL (menu_kbm_opts), item);
  }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (opt_kbm_opts), menu_kbm_opts);
  gtk_option_menu_set_history (GTK_OPTION_MENU (opt_kbm_opts), current_idx);

  return hbox;
}


void load_setttings();

void create_kbm_window()
{
  if (gcin_kbm_window) {
    gtk_window_present(GTK_WINDOW(gcin_kbm_window));
    return;
  }

  load_setttings();

  gcin_kbm_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  g_signal_connect (G_OBJECT (gcin_kbm_window), "delete_event",
                    G_CALLBACK (close_kbm_window),
                    NULL);

  gtk_window_set_title (GTK_WINDOW (gcin_kbm_window), "gcin 注音/詞音設定");
  gtk_container_set_border_width (GTK_CONTAINER (gcin_kbm_window), 1);

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 3);
  gtk_container_add (GTK_CONTAINER (gcin_kbm_window), vbox_top);

  GtkWidget *frame_kbm = gtk_frame_new("鍵盤排列方式");
  gtk_box_pack_start (GTK_BOX (vbox_top), frame_kbm, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_kbm), 1);

  gtk_container_add (GTK_CONTAINER (frame_kbm), create_kbm_opts());


  GtkWidget *frame_tsin_sw = gtk_frame_new("詞音輸入[中/英]切換");
  gtk_box_pack_start (GTK_BOX (vbox_top), frame_tsin_sw, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_tsin_sw), 1);

  GtkWidget *box_tsin_sw = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame_tsin_sw), box_tsin_sw);
  gtk_container_set_border_width (GTK_CONTAINER (box_tsin_sw), 1);

  GSList *group_tsin_sw = NULL;
  int current_idx = get_currnet_eng_ch_sw_idx();
  new_select_idx_tsin_sw = current_idx;

  int i;
  for(i=0; i< tsin_eng_ch_swN; i++) {
    GtkWidget *button = gtk_radio_button_new_with_label (group_tsin_sw, tsin_eng_ch_sw[i].name);
    gtk_box_pack_start (GTK_BOX (box_tsin_sw), button, TRUE, TRUE, 0);

    group_tsin_sw = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));

    g_signal_connect (G_OBJECT (button), "clicked",
       G_CALLBACK (callback_button_clicked_tsin_sw), (gpointer) i);

    if (i==current_idx)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
  }


  GtkWidget *frame_tsin_space_opt = gtk_frame_new("詞音輸入空白鍵選項");
  gtk_box_pack_start (GTK_BOX (vbox_top), frame_tsin_space_opt, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_tsin_space_opt), 1);

  GtkWidget *box_tsin_space_opt = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame_tsin_space_opt), box_tsin_space_opt);
  gtk_container_set_border_width (GTK_CONTAINER (box_tsin_space_opt), 1);

  GSList *group_tsin_space_opt = NULL;
  current_idx = get_currnet_tsin_space_option_idx();
  new_select_idx_tsin_space_opt = current_idx;

  for(i=0; i< tsin_space_optionsN; i++) {
    GtkWidget *button = gtk_radio_button_new_with_label (group_tsin_space_opt, tsin_space_options[i].name);
    gtk_box_pack_start (GTK_BOX (box_tsin_space_opt), button, TRUE, TRUE, 0);

    group_tsin_space_opt = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));

    g_signal_connect (G_OBJECT (button), "clicked",
       G_CALLBACK (callback_button_clicked_tsin_space_opt), (gpointer) i);

    if (i==current_idx)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
  }


  GtkWidget *hbox_tsin_phrase_pre_select = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_tsin_phrase_pre_select , TRUE, TRUE, 1);
  GtkWidget *label_tsin_phrase_pre_select = gtk_label_new("詞音輸入預選詞視窗");
  gtk_box_pack_start (GTK_BOX (hbox_tsin_phrase_pre_select), label_tsin_phrase_pre_select , TRUE, TRUE, 0);
  check_button_tsin_phrase_pre_select = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_tsin_phrase_pre_select), check_button_tsin_phrase_pre_select, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_tsin_phrase_pre_select), tsin_phrase_pre_select);


  GtkWidget *hbox_phonetic_char_dynamic_sequence = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_phonetic_char_dynamic_sequence , TRUE, TRUE, 1);
  GtkWidget *label_phonetic_char_dynamic_sequence = gtk_label_new("依使用頻率調整字的順序");
  gtk_box_pack_start (GTK_BOX (hbox_phonetic_char_dynamic_sequence), label_phonetic_char_dynamic_sequence , TRUE, TRUE, 0);
  check_button_phonetic_char_dynamic_sequence = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_phonetic_char_dynamic_sequence), check_button_phonetic_char_dynamic_sequence, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_phonetic_char_dynamic_sequence), phonetic_char_dynamic_sequence);


  GtkWidget *hbox_pho_simple_win = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_pho_simple_win , TRUE, TRUE, 1);
  GtkWidget *label_pho_simple_win = gtk_label_new("詞音/注音精簡視窗");
  gtk_box_pack_start (GTK_BOX (hbox_pho_simple_win), label_pho_simple_win , TRUE, TRUE, 0);
  check_button_pho_simple_win = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_pho_simple_win), check_button_pho_simple_win, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_pho_simple_win), pho_simple_win);


  GtkWidget *hbox_pho_hide_row2 = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_pho_hide_row2 , TRUE, TRUE, 1);
  GtkWidget *label_pho_hide_row2 = gtk_label_new("注音隱藏第二列(按鍵顯示)");
  gtk_box_pack_start (GTK_BOX (hbox_pho_hide_row2), label_pho_hide_row2 , TRUE, TRUE, 0);
  check_button_pho_hide_row2 = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_pho_hide_row2), check_button_pho_hide_row2, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_pho_hide_row2), pho_hide_row2);


  GtkWidget *hbox_pho_in_row1 = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_pho_in_row1 , TRUE, TRUE, 1);
  GtkWidget *label_pho_in_row1 = gtk_label_new("注音輸入顯示移至第一列");
  gtk_box_pack_start (GTK_BOX (hbox_pho_in_row1), label_pho_in_row1 , TRUE, TRUE, 0);
  check_button_pho_in_row1 = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_pho_in_row1), check_button_pho_in_row1, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_pho_in_row1), pho_in_row1);


  GtkWidget *hbox_phonetic_huge_tab = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_phonetic_huge_tab , TRUE, TRUE, 1);
  GtkWidget *label_phonetic_huge_tab = gtk_label_new("使用巨大 UTF-8 字集");
  gtk_box_pack_start (GTK_BOX (hbox_phonetic_huge_tab), label_phonetic_huge_tab , TRUE, TRUE, 0);
  check_button_phonetic_huge_tab = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_phonetic_huge_tab), check_button_phonetic_huge_tab, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_phonetic_huge_tab), phonetic_huge_tab);


  GtkWidget *hbox_tsin_tone_char_input = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_tsin_tone_char_input , TRUE, TRUE, 1);
  GtkWidget *label_tsin_tone_char_input = gtk_label_new("詞音輸入注音聲調符號");
  gtk_box_pack_start (GTK_BOX (hbox_tsin_tone_char_input), label_tsin_tone_char_input , TRUE, TRUE, 0);
  check_button_tsin_tone_char_input = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_tsin_tone_char_input), check_button_tsin_tone_char_input, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_tsin_tone_char_input), tsin_tone_char_input);


  GtkWidget *hbox_tsin_tab_phrase_end = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_tsin_tab_phrase_end , TRUE, TRUE, 1);
  GtkWidget *label_tsin_tab_phrase_end = gtk_label_new("詞音 Tab 斷詞");
  gtk_box_pack_start (GTK_BOX (hbox_tsin_tab_phrase_end), label_tsin_tab_phrase_end , TRUE, TRUE, 0);
  check_button_tsin_tab_phrase_end = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_tsin_tab_phrase_end), check_button_tsin_tab_phrase_end, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_tsin_tab_phrase_end), tsin_tab_phrase_end);


  GtkWidget *frame_tsin_buffer_size = gtk_frame_new("詞音編輯緩衝區大小");
  gtk_box_pack_start (GTK_BOX (vbox_top), frame_tsin_buffer_size, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_tsin_buffer_size), 1);
  GtkAdjustment *adj_gtab_in =
   (GtkAdjustment *) gtk_adjustment_new (tsin_buffer_size, 10.0, MAX_PH_BF, 1.0, 1.0, 0.0);
  spinner_tsin_buffer_size = gtk_spin_button_new (adj_gtab_in, 0, 0);
  gtk_container_add (GTK_CONTAINER (frame_tsin_buffer_size), spinner_tsin_buffer_size);


  GtkWidget *frame_tsin_phrase_line_color = gtk_frame_new("詞音標示詞的底線顏色");
  gtk_box_pack_start (GTK_BOX (vbox_top), frame_tsin_phrase_line_color, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_tsin_phrase_line_color), 1);
  GtkWidget *button_tsin_phrase_line_color = gtk_button_new();
  g_signal_connect (G_OBJECT (button_tsin_phrase_line_color), "clicked",
                    G_CALLBACK (cb_tsin_phrase_line_color), G_OBJECT (gcin_kbm_window));
  da =  gtk_drawing_area_new();
  gtk_container_add (GTK_CONTAINER (button_tsin_phrase_line_color), da);
  gdk_color_parse(tsin_phrase_line_color, &tsin_phrase_line_gcolor);
  gtk_widget_modify_bg(da, GTK_STATE_NORMAL, &tsin_phrase_line_gcolor);
  gtk_widget_set_size_request(da, 16, 2);
  gtk_container_add (GTK_CONTAINER (frame_tsin_phrase_line_color), button_tsin_phrase_line_color);

  GtkWidget *frame_tsin_cursor_color = gtk_frame_new("詞音游標的顏色");
  gtk_box_pack_start (GTK_BOX (vbox_top), frame_tsin_cursor_color, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_tsin_cursor_color), 1);
  GtkWidget *button_tsin_cursor_color = gtk_button_new();
  g_signal_connect (G_OBJECT (button_tsin_cursor_color), "clicked",
                    G_CALLBACK (cb_tsin_cursor_color), G_OBJECT (gcin_kbm_window));
  da =  gtk_drawing_area_new();
  gtk_container_add (GTK_CONTAINER (button_tsin_cursor_color), da);
  gdk_color_parse(tsin_cursor_color, &tsin_cursor_gcolor);
  gtk_widget_modify_bg(da, GTK_STATE_NORMAL, &tsin_cursor_gcolor);
  gtk_widget_set_size_request(da, 16, 2);
  gtk_container_add (GTK_CONTAINER (frame_tsin_cursor_color), button_tsin_cursor_color);




  GtkWidget *hbox_cancel_ok = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_cancel_ok , FALSE, FALSE, 0);
  GtkWidget *button_cancel = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_cancel, TRUE, TRUE, 0);
  GtkWidget *button_ok = gtk_button_new_from_stock (GTK_STOCK_OK);
  gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_ok, TRUE, TRUE, 0);

  g_signal_connect (G_OBJECT (button_cancel), "clicked",
                            G_CALLBACK (close_kbm_window),
                            G_OBJECT (gcin_kbm_window));

  g_signal_connect_swapped (G_OBJECT (button_ok), "clicked",
                            G_CALLBACK (cb_ok),
                            G_OBJECT (gcin_kbm_window));

  GTK_WIDGET_SET_FLAGS (button_cancel, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (button_cancel);

  gtk_widget_show_all (gcin_kbm_window);

  return;
}
