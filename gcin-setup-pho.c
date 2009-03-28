#include "gcin.h"
#include "pho.h"


static struct {
  char *name;
  char *kbm;
}  kbm_sel[]= {
 {N_("標準 standard"), "zo"},
 {N_("標準 standard 使用 asdf 選擇"), "zo-asdf"},
 {N_("倚天 Eten"), "et"},
 {N_("倚天 Eten 使用 asdf 選擇"), "et-asdf"},
 {N_("倚天 26 鍵"), "et26"},
 {N_("倚天 26 鍵,使用 asdf 選擇"), "et26-asdf"},
 {N_("許氏(國音,自然)"), "hsu"},
 {N_("聲調拼音"), "pinyin"},
 {N_("聲調拼音, 使用 asdf 選擇"), "pinyin-asdf"},
 {N_("Dvorak"), "dvorak"},
 {N_("IBM"), "ibm"},
 {N_("神通"), "mitac"},
 {NULL, NULL}
};

static GtkWidget *check_button_tsin_phrase_pre_select,
                 *check_button_phonetic_char_dynamic_sequence,
                 *check_button_pho_hide_row2,
                 *check_button_pho_in_row1,
                 *check_button_phonetic_huge_tab,
                 *check_button_tsin_tone_char_input,
                 *check_button_tsin_tab_phrase_end,
                 *check_button_tsin_tail_select_key,
		 *check_button_tsin_buffer_editing_mode,
                 *check_button_gcin_capslock_lower,
                 *spinner_tsin_buffer_size;

static GtkWidget *opt_kbm_opts, *opt_eng_ch_opts, *opt_speaker_opts;


static struct {
  char *name;
  int key;
} tsin_eng_ch_sw[]={
  {N_("CapsLock"), TSIN_CHINESE_ENGLISH_TOGGLE_KEY_CapsLock},
  {N_("Tab"), TSIN_CHINESE_ENGLISH_TOGGLE_KEY_Tab},
  {N_("Shift(限非 XIM)"), TSIN_CHINESE_ENGLISH_TOGGLE_KEY_Shift},
  {N_("ShiftL(限非 XIM)"), TSIN_CHINESE_ENGLISH_TOGGLE_KEY_ShiftL},
  {N_("ShiftR(限非 XIM)"), TSIN_CHINESE_ENGLISH_TOGGLE_KEY_ShiftR},
};
int tsin_eng_ch_swN = sizeof(tsin_eng_ch_sw) / sizeof(tsin_eng_ch_sw[0]);


static struct {
  char *name;
  int key;
} tsin_space_options[]={
  {N_("選擇同音字"), TSIN_SPACE_OPT_SELECT_CHAR},
  {N_("輸入空白"), TSIN_SPACE_OPT_INPUT}
};
int tsin_space_optionsN = sizeof(tsin_space_options) / sizeof(tsin_space_options[0]);

char *pho_speaker[16];
int pho_speakerN;

int get_current_speaker_idx()
{
  int i;

  for(i=0; i < pho_speakerN; i++)
    if (!strcmp(pho_speaker[i], phonetic_speak_sel))
      return i;

  return 0;
}


static GtkWidget *gcin_kbm_window = NULL;

static int new_select_idx, new_select_idx_tsin_sw, new_select_idx_tsin_space_opt;
static GdkColor tsin_phrase_line_gcolor, tsin_cursor_gcolor;


static gboolean cb_ok( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  int idx = gtk_option_menu_get_history (GTK_OPTION_MENU (opt_kbm_opts));
  save_gcin_conf_str(PHONETIC_KEYBOARD, kbm_sel[idx].kbm);

  idx = gtk_option_menu_get_history (GTK_OPTION_MENU (opt_eng_ch_opts));
  save_gcin_conf_int(TSIN_CHINESE_ENGLISH_TOGGLE_KEY,
                     tsin_eng_ch_sw[idx].key);

  save_gcin_conf_int(TSIN_SPACE_OPT,
                     tsin_space_options[new_select_idx_tsin_space_opt].key);

  save_gcin_conf_int(TSIN_PHRASE_PRE_SELECT,
       gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_tsin_phrase_pre_select)));

  save_gcin_conf_int(PHONETIC_CHAR_DYNAMIC_SEQUENCE,
       gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_phonetic_char_dynamic_sequence)));
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


  save_gcin_conf_int(TSIN_TAIL_SELECT_KEY,
       gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_tsin_tail_select_key)));

  save_gcin_conf_int(TSIN_BUFFER_EDITING_MODE,
       gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_tsin_buffer_editing_mode)));

  save_gcin_conf_int(GCIN_CAPSLOCK_LOWER,
       gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_capslock_lower)));

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
  get_gcin_conf_fstr(PHONETIC_KEYBOARD, kbm_str, "zo-asdf");

  int i, rval;
  for(i=0; kbm_sel[i].kbm; i++)
    if (!strcmp(kbm_sel[i].kbm, kbm_str)) {
      return i;
    }

  p_err("phonetic-keyboard->%s is not valid", kbm_str);
  return 0;
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

static GtkWidget *da_phrase_line, *da_cursor, *da_sel_key;

static void cb_save_tsin_phrase_line_color(GtkWidget *widget, gpointer user_data)
{
  GtkColorSelectionDialog *color_selector = (GtkColorSelectionDialog *)user_data;
  gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(color_selector->colorsel), &tsin_phrase_line_gcolor);

  gtk_widget_modify_bg(da_phrase_line, GTK_STATE_NORMAL, &tsin_phrase_line_gcolor);
}


static gboolean cb_tsin_phrase_line_color( GtkWidget *widget,
                                   gpointer   data )
{
   GtkColorSelectionDialog *color_selector = (GtkColorSelectionDialog *)gtk_color_selection_dialog_new (_("詞音標示詞的底線顏色"));

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

  gtk_widget_modify_bg(da_cursor, GTK_STATE_NORMAL, &tsin_cursor_gcolor);
}


static gboolean cb_tsin_cursor_color( GtkWidget *widget,
                                   gpointer   data )
{
   GtkColorSelectionDialog *color_selector = (GtkColorSelectionDialog *)gtk_color_selection_dialog_new (_("詞音游標的顏色"));

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
    GtkWidget *item = gtk_menu_item_new_with_label (_(kbm_sel[i].name));

    gtk_menu_shell_append (GTK_MENU_SHELL (menu_kbm_opts), item);
  }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (opt_kbm_opts), menu_kbm_opts);
  gtk_option_menu_set_history (GTK_OPTION_MENU (opt_kbm_opts), current_idx);

  return hbox;
}



static GtkWidget *create_eng_ch_opts()
{

  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);

  opt_eng_ch_opts = gtk_option_menu_new ();
  gtk_box_pack_start (GTK_BOX (hbox), opt_eng_ch_opts, FALSE, FALSE, 0);
  GtkWidget *menu_eng_ch_opts = gtk_menu_new ();

  int i;
  int current_idx = get_currnet_eng_ch_sw_idx();

  for(i=0; i < tsin_eng_ch_swN; i++) {
    GtkWidget *item = gtk_menu_item_new_with_label (_(tsin_eng_ch_sw[i].name));

    gtk_menu_shell_append (GTK_MENU_SHELL (menu_eng_ch_opts), item);
  }

  dbg("current_idx:%d\n", current_idx);

  gtk_option_menu_set_menu (GTK_OPTION_MENU (opt_eng_ch_opts), menu_eng_ch_opts);
  gtk_option_menu_set_history (GTK_OPTION_MENU (opt_eng_ch_opts), current_idx);

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

  gtk_window_set_title (GTK_WINDOW (gcin_kbm_window), _("gcin 注音/詞音設定"));
  gtk_container_set_border_width (GTK_CONTAINER (gcin_kbm_window), 1);

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 3);
  gtk_container_add (GTK_CONTAINER (gcin_kbm_window), vbox_top);


  GtkWidget *hbox_lr = gtk_hbox_new (FALSE, 3);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_lr, TRUE, TRUE, 0);


  GtkWidget *vbox_l = gtk_vbox_new (FALSE, 3);
  gtk_box_pack_start (GTK_BOX (hbox_lr), vbox_l, TRUE, TRUE, 10);

  GtkWidget *vbox_r = gtk_vbox_new (FALSE, 3);
  gtk_box_pack_start (GTK_BOX (hbox_lr), vbox_r, TRUE, TRUE, 10);


  GtkWidget *frame_kbm = gtk_frame_new(_("鍵盤排列方式"));
  gtk_box_pack_start (GTK_BOX (vbox_l), frame_kbm, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_kbm), 1);
  gtk_container_add (GTK_CONTAINER (frame_kbm), create_kbm_opts());


  GtkWidget *frame_tsin_sw = gtk_frame_new(_("詞音輸入[中/英]切換"));
  gtk_box_pack_start (GTK_BOX (vbox_l), frame_tsin_sw, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_tsin_sw), 1);
  gtk_container_add (GTK_CONTAINER (frame_tsin_sw), create_eng_ch_opts());


  GtkWidget *frame_tsin_space_opt = gtk_frame_new(_("詞音輸入空白鍵選項"));
  gtk_box_pack_start (GTK_BOX (vbox_l), frame_tsin_space_opt, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_tsin_space_opt), 1);

  GtkWidget *box_tsin_space_opt = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame_tsin_space_opt), box_tsin_space_opt);
  gtk_container_set_border_width (GTK_CONTAINER (box_tsin_space_opt), 1);

  GSList *group_tsin_space_opt = NULL;
  int current_idx = get_currnet_tsin_space_option_idx();
  new_select_idx_tsin_space_opt = current_idx;

  int i;
  for(i=0; i< tsin_space_optionsN; i++) {
    GtkWidget *button = gtk_radio_button_new_with_label (group_tsin_space_opt, _(tsin_space_options[i].name));
    gtk_box_pack_start (GTK_BOX (box_tsin_space_opt), button, TRUE, TRUE, 0);

    group_tsin_space_opt = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));

    g_signal_connect (G_OBJECT (button), "clicked",
       G_CALLBACK (callback_button_clicked_tsin_space_opt), (gpointer) i);

    if (i==current_idx)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
  }


  GtkWidget *hbox_tsin_phrase_pre_select = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_l), hbox_tsin_phrase_pre_select , TRUE, TRUE, 1);
  GtkWidget *label_tsin_phrase_pre_select = gtk_label_new(_("詞音輸入預選詞視窗"));
  gtk_box_pack_start (GTK_BOX (hbox_tsin_phrase_pre_select), label_tsin_phrase_pre_select , TRUE, TRUE, 0);
  check_button_tsin_phrase_pre_select = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_tsin_phrase_pre_select), check_button_tsin_phrase_pre_select, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_tsin_phrase_pre_select), tsin_phrase_pre_select);


  GtkWidget *hbox_phonetic_char_dynamic_sequence = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_l), hbox_phonetic_char_dynamic_sequence , TRUE, TRUE, 1);
  GtkWidget *label_phonetic_char_dynamic_sequence = gtk_label_new(_("依使用頻率調整字的順序"));
  gtk_box_pack_start (GTK_BOX (hbox_phonetic_char_dynamic_sequence), label_phonetic_char_dynamic_sequence , TRUE, TRUE, 0);
  check_button_phonetic_char_dynamic_sequence = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_phonetic_char_dynamic_sequence), check_button_phonetic_char_dynamic_sequence, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_phonetic_char_dynamic_sequence), phonetic_char_dynamic_sequence);


  GtkWidget *hbox_pho_hide_row2 = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_l), hbox_pho_hide_row2 , TRUE, TRUE, 1);
  GtkWidget *label_pho_hide_row2 = gtk_label_new(_("注音隱藏第二列(按鍵顯示)"));
  gtk_box_pack_start (GTK_BOX (hbox_pho_hide_row2), label_pho_hide_row2 , TRUE, TRUE, 0);
  check_button_pho_hide_row2 = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_pho_hide_row2), check_button_pho_hide_row2, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_pho_hide_row2), pho_hide_row2);


  GtkWidget *hbox_pho_in_row1 = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_l), hbox_pho_in_row1 , TRUE, TRUE, 1);
  GtkWidget *label_pho_in_row1 = gtk_label_new(_("注音輸入顯示移至第一列"));
  gtk_box_pack_start (GTK_BOX (hbox_pho_in_row1), label_pho_in_row1 , TRUE, TRUE, 0);
  check_button_pho_in_row1 = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_pho_in_row1), check_button_pho_in_row1, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_pho_in_row1), pho_in_row1);


  GtkWidget *hbox_phonetic_huge_tab = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_r), hbox_phonetic_huge_tab , TRUE, TRUE, 1);
  GtkWidget *label_phonetic_huge_tab = gtk_label_new(_("使用巨大 UTF-8 字集"));
  gtk_box_pack_start (GTK_BOX (hbox_phonetic_huge_tab), label_phonetic_huge_tab , TRUE, TRUE, 0);
  check_button_phonetic_huge_tab = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_phonetic_huge_tab), check_button_phonetic_huge_tab, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_phonetic_huge_tab), phonetic_huge_tab);


  GtkWidget *hbox_tsin_tone_char_input = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_r), hbox_tsin_tone_char_input , TRUE, TRUE, 1);
  GtkWidget *label_tsin_tone_char_input = gtk_label_new(_("詞音輸入注音聲調符號"));
  gtk_box_pack_start (GTK_BOX (hbox_tsin_tone_char_input), label_tsin_tone_char_input , TRUE, TRUE, 0);
  check_button_tsin_tone_char_input = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_tsin_tone_char_input), check_button_tsin_tone_char_input, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_tsin_tone_char_input), tsin_tone_char_input);


  GtkWidget *hbox_tsin_tab_phrase_end = gtk_hbox_new(FALSE, 1);
  gtk_box_pack_start (GTK_BOX (vbox_r), hbox_tsin_tab_phrase_end , TRUE, TRUE, 1);
  GtkWidget *label_tsin_tab_phrase_end = gtk_label_new(_("詞音/Escape Tab 斷詞"));
  gtk_box_pack_start (GTK_BOX (hbox_tsin_tab_phrase_end), label_tsin_tab_phrase_end , TRUE, TRUE, 0);
  check_button_tsin_tab_phrase_end = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_tsin_tab_phrase_end), check_button_tsin_tab_phrase_end, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_tsin_tab_phrase_end), tsin_tab_phrase_end);

  GtkWidget *hbox_tsin_tail_select_key = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_r), hbox_tsin_tail_select_key , TRUE, TRUE, 1);
  GtkWidget *label_tsin_tail_select_key = gtk_label_new(_("同音字詞選擇按鍵在後"));
  gtk_box_pack_start (GTK_BOX (hbox_tsin_tail_select_key), label_tsin_tail_select_key , TRUE, TRUE, 0);
  check_button_tsin_tail_select_key = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_tsin_tail_select_key), check_button_tsin_tail_select_key, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_tsin_tail_select_key), tsin_tail_select_key);

  GtkWidget *hbox_tsin_buffer_editing_mode = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_r), hbox_tsin_buffer_editing_mode , TRUE, TRUE, 1);
  GtkWidget *label_tsin_buffer_editing_mode = gtk_label_new(_("啟用詞音緩衝區編輯模式"));
  gtk_box_pack_start (GTK_BOX (hbox_tsin_buffer_editing_mode), label_tsin_buffer_editing_mode, TRUE, TRUE, 0);
  check_button_tsin_buffer_editing_mode = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_tsin_buffer_editing_mode), check_button_tsin_buffer_editing_mode, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_tsin_buffer_editing_mode), tsin_buffer_editing_mode);

  GtkWidget *hbox_gcin_capslock_lower = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_r), hbox_gcin_capslock_lower , TRUE, TRUE, 1);
  GtkWidget *label_gcin_capslock_lower = gtk_label_new(_("Capslock英數用小寫"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_capslock_lower), label_gcin_capslock_lower , TRUE, TRUE, 0);
  check_button_gcin_capslock_lower = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcin_capslock_lower), check_button_gcin_capslock_lower, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_gcin_capslock_lower), gcin_capslock_lower);

  GtkWidget *frame_tsin_buffer_size = gtk_frame_new(_("詞音編輯緩衝區大小"));
  gtk_box_pack_start (GTK_BOX (vbox_r), frame_tsin_buffer_size, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_tsin_buffer_size), 1);
  GtkAdjustment *adj_gtab_in =
   (GtkAdjustment *) gtk_adjustment_new (tsin_buffer_size, 10.0, MAX_PH_BF, 1.0, 1.0, 0.0);
  spinner_tsin_buffer_size = gtk_spin_button_new (adj_gtab_in, 0, 0);
  gtk_container_add (GTK_CONTAINER (frame_tsin_buffer_size), spinner_tsin_buffer_size);


  GtkWidget *frame_tsin_phrase_line_color = gtk_frame_new(_("詞音標示詞的底線顏色"));
  gtk_box_pack_start (GTK_BOX (vbox_r), frame_tsin_phrase_line_color, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_tsin_phrase_line_color), 1);
  GtkWidget *button_tsin_phrase_line_color = gtk_button_new();
  g_signal_connect (G_OBJECT (button_tsin_phrase_line_color), "clicked",
                    G_CALLBACK (cb_tsin_phrase_line_color), G_OBJECT (gcin_kbm_window));
  da_phrase_line =  gtk_drawing_area_new();
  gtk_container_add (GTK_CONTAINER (button_tsin_phrase_line_color), da_phrase_line);
  gdk_color_parse(tsin_phrase_line_color, &tsin_phrase_line_gcolor);
  gtk_widget_modify_bg(da_phrase_line, GTK_STATE_NORMAL, &tsin_phrase_line_gcolor);
  gtk_widget_set_size_request(da_phrase_line, 16, 2);
  gtk_container_add (GTK_CONTAINER (frame_tsin_phrase_line_color), button_tsin_phrase_line_color);

  GtkWidget *frame_tsin_cursor_color = gtk_frame_new(_("詞音游標的顏色"));
  gtk_box_pack_start (GTK_BOX (vbox_r), frame_tsin_cursor_color, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_tsin_cursor_color), 1);
  GtkWidget *button_tsin_cursor_color = gtk_button_new();
  g_signal_connect (G_OBJECT (button_tsin_cursor_color), "clicked",
                    G_CALLBACK (cb_tsin_cursor_color), G_OBJECT (gcin_kbm_window));
  da_cursor =  gtk_drawing_area_new();
  gtk_container_add (GTK_CONTAINER (button_tsin_cursor_color), da_cursor);
  gdk_color_parse(tsin_cursor_color, &tsin_cursor_gcolor);
  gtk_widget_modify_bg(da_cursor, GTK_STATE_NORMAL, &tsin_cursor_gcolor);
  gtk_widget_set_size_request(da_cursor, 16, 2);
  gtk_container_add (GTK_CONTAINER (frame_tsin_cursor_color), button_tsin_cursor_color);

  GtkWidget *hbox_cancel_ok = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_cancel_ok , FALSE, FALSE, 5);
  GtkWidget *button_cancel = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_cancel, TRUE, TRUE, 0);
  GtkWidget *button_ok = gtk_button_new_from_stock (GTK_STOCK_OK);
  gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_ok, TRUE, TRUE, 5);

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
