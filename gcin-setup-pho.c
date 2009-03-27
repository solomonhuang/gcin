#include "gcin.h"
#include "gtab.h"


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
};

static GtkWidget *check_button_phrase_pre_select,
                 *check_button_phrase_pre_select,
                 *check_button_phonetic_char_dynamic_sequence,
                 *check_button_tsin_disp_status_row,
                 *check_button_phonetic_huge_tab;

static struct {
  char *name;
  int key;
} tsin_eng_ch_sw[]={
  {"CapsLock", TSIN_CHINESE_ENGLISH_TOGGLE_KEY_CapsLock},
  {"Tab", TSIN_CHINESE_ENGLISH_TOGGLE_KEY_Tab},
  {"Shift(限 GTK_IM_MODULE=gcin)", TSIN_CHINESE_ENGLISH_TOGGLE_KEY_Shift},
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


static int kbm_selN = sizeof(kbm_sel) / sizeof(kbm_sel[0]);

static GtkWidget *gcin_kbm_window = NULL;

static int new_select_idx, new_select_idx_tsin_sw, new_select_idx_tsin_space_opt;


static gboolean cb_ok( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  save_gcin_conf_str(PHONETIC_KEYBOARD, kbm_sel[new_select_idx].kbm);
  save_gcin_conf_int(TSIN_CHINESE_ENGLISH_TOGGLE_KEY,
                     tsin_eng_ch_sw[new_select_idx_tsin_sw].key);

  save_gcin_conf_int(TSIN_SPACE_OPT,
                     tsin_space_options[new_select_idx_tsin_space_opt].key);

  save_gcin_conf_int(TSIN_PHRASE_PRE_SELECT,
       gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_phrase_pre_select)));

  save_gcin_conf_int(PHONETIC_CHAR_DYNAMIC_SEQUENCE,
       gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_phonetic_char_dynamic_sequence)));

  save_gcin_conf_int(TSIN_DISP_STATUS_ROW,
       gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_tsin_disp_status_row)));

  save_gcin_conf_int(PHONETIC_HUGE_TAB,
       gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_phonetic_huge_tab)));

  send_gcin_message(GDK_DISPLAY(), "reload kbm");

  gtk_widget_destroy(gcin_kbm_window); gcin_kbm_window = NULL;

  return TRUE;
}


static void callback_button_clicked( GtkWidget *widget, gpointer data)
{
  new_select_idx = (int) data;
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
  for(i=0; i < kbm_selN; i++)
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
    gtk_container_set_border_width (GTK_CONTAINER (gcin_kbm_window), 3);

    GtkWidget *vbox_top = gtk_vbox_new (FALSE, 10);
    gtk_container_add (GTK_CONTAINER (gcin_kbm_window), vbox_top);

    GtkWidget *frame_kbm = gtk_frame_new("鍵盤排列方式");
    gtk_box_pack_start (GTK_BOX (vbox_top), frame_kbm, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame_kbm), 3);

    GtkWidget *box_kbm = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (frame_kbm), box_kbm);
    gtk_container_set_border_width (GTK_CONTAINER (box_kbm), 3);

    int i;
    GSList *group_kbm = NULL;
    int current_idx = get_current_kbm_idx();
    new_select_idx = current_idx;

    for(i=0; i<kbm_selN; i++) {
      GtkWidget *button = gtk_radio_button_new_with_label (group_kbm, kbm_sel[i].name);
      gtk_box_pack_start (GTK_BOX (box_kbm), button, TRUE, TRUE, 0);

      group_kbm = gtk_radio_button_get_group (GTK_RADIO_BUTTON (button));

      g_signal_connect (G_OBJECT (button), "clicked",
         G_CALLBACK (callback_button_clicked), (gpointer) i);

      if (i==current_idx)
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
    }

    GtkWidget *frame_tsin_sw = gtk_frame_new("詞音輸入[中/英]切換");
    gtk_box_pack_start (GTK_BOX (vbox_top), frame_tsin_sw, TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame_tsin_sw), 3);

    GtkWidget *box_tsin_sw = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (frame_tsin_sw), box_tsin_sw);
    gtk_container_set_border_width (GTK_CONTAINER (box_tsin_sw), 3);

    GSList *group_tsin_sw = NULL;
    current_idx = get_currnet_eng_ch_sw_idx();
    new_select_idx_tsin_sw = current_idx;

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
    gtk_container_set_border_width (GTK_CONTAINER (frame_tsin_space_opt), 3);

    GtkWidget *box_tsin_space_opt = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (frame_tsin_space_opt), box_tsin_space_opt);
    gtk_container_set_border_width (GTK_CONTAINER (box_tsin_space_opt), 3);

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


    GtkWidget *frame_tsin_phrase_pre_select = gtk_frame_new("詞音輸入預選詞視窗");
    gtk_box_pack_start (GTK_BOX (vbox_top), frame_tsin_phrase_pre_select , TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame_tsin_phrase_pre_select), 3);
    check_button_phrase_pre_select = gtk_check_button_new ();
    gtk_container_add (GTK_CONTAINER (frame_tsin_phrase_pre_select),
        check_button_phrase_pre_select);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_phrase_pre_select),
       tsin_phrase_pre_select);


    GtkWidget *frame_phonetic_char_dynamic_sequence = gtk_frame_new("注音依使用頻率調整順序");
    gtk_box_pack_start (GTK_BOX (vbox_top), frame_phonetic_char_dynamic_sequence , TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame_phonetic_char_dynamic_sequence), 3);
    check_button_phonetic_char_dynamic_sequence = gtk_check_button_new ();
    gtk_container_add (GTK_CONTAINER (frame_phonetic_char_dynamic_sequence),
        check_button_phonetic_char_dynamic_sequence);
    gtk_toggle_button_set_active(
       GTK_TOGGLE_BUTTON(check_button_phonetic_char_dynamic_sequence),
       phonetic_char_dynamic_sequence);

    GtkWidget *frame_tsin_disp_status_row = gtk_frame_new("詞音顯示狀態列(較小視窗)");
    gtk_box_pack_start (GTK_BOX (vbox_top), frame_tsin_disp_status_row , TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame_tsin_disp_status_row), 3);
    check_button_tsin_disp_status_row = gtk_check_button_new ();
    gtk_container_add (GTK_CONTAINER (frame_tsin_disp_status_row),
        check_button_tsin_disp_status_row);
    gtk_toggle_button_set_active(
       GTK_TOGGLE_BUTTON(check_button_tsin_disp_status_row),
       tsin_disp_status_row);

    GtkWidget *frame_phonetic_huge_tab = gtk_frame_new("使用巨大 UTF-8 字集");
    gtk_box_pack_start (GTK_BOX (vbox_top), frame_phonetic_huge_tab , TRUE, TRUE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (frame_phonetic_huge_tab), 3);
    check_button_phonetic_huge_tab = gtk_check_button_new ();
    gtk_container_add (GTK_CONTAINER (frame_phonetic_huge_tab),
        check_button_phonetic_huge_tab);
    gtk_toggle_button_set_active(
       GTK_TOGGLE_BUTTON(check_button_phonetic_huge_tab),
       phonetic_huge_tab);


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
