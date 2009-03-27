#include "gcin.h"
#include "gtab.h"


static GtkWidget *check_button_gtab_dup_select_bell,
                 *check_button_gtab_auto_select_by_phrase,
                 *check_button_gtab_press_full_auto_send,
                 *check_button_gtab_pre_select,
                 *check_button_gtab_disp_partial_match,
                 *check_button_gtab_simple_win,
                 *check_button_gtab_disp_key_codes,
                 *check_button_gtab_disp_im_name;

static GtkWidget *opt_spc_opts;

static GtkWidget *gcin_kbm_window = NULL, *gcin_appearance_conf_window;


static gboolean close_application( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  exit(0);
}


void create_kbm_window();

static void cb_kbm()
{
  create_kbm_window();
}


static void cb_tslearn()
{
  system("tslearn &");
  exit(0);
}


static void cb_ret(GtkWidget *widget, gpointer user_data)
{
  gtk_widget_destroy(user_data);
}


static void create_result_win(int res)
{
  char *restr = res ? "結果失敗":"結果成功";
  GtkWidget *main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  GtkWidget *button = gtk_button_new_with_label(restr);
  gtk_container_add (GTK_CONTAINER (main_window), button);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (cb_ret), main_window);

  gtk_widget_show_all(main_window);
}



static void cb_file_ts_export(GtkWidget *widget, gpointer user_data)
{
   GtkWidget *file_selector = (GtkWidget *)user_data;
   const gchar *selected_filename;

   selected_filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector));
//   g_print ("Selected filename: %s\n", selected_filename);

   char gcin_dir[512];

   get_gcin_dir(gcin_dir);

   char cmd[256];
   snprintf(cmd, sizeof(cmd), "tsd2a %s/tsin > %s", gcin_dir, selected_filename);
   int res = system(cmd);
   create_result_win(res);
}

static void cb_ts_export()
{
   /* Create the selector */

   GtkWidget *file_selector = gtk_file_selection_new ("請輸入要匯出的檔案名稱");

   g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                     "clicked",
                     G_CALLBACK (cb_file_ts_export),
                     (gpointer) file_selector);

   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) file_selector);

   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) file_selector);

   gtk_widget_show(file_selector);
}

static void cb_file_ts_import(GtkWidget *widget, gpointer user_data)
{
   GtkWidget *file_selector = (GtkWidget *)user_data;
   const gchar *selected_filename;

   selected_filename = gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector));
//   g_print ("Selected filename: %s\n", selected_filename);

   char cmd[256];
   snprintf(cmd, sizeof(cmd),
      "cd %s/.gcin && tsd2a tsin > tmpfile && cat %s >> tmpfile && tsa2d tmpfile",
      getenv("HOME"), selected_filename);
   int res = system(cmd);
   create_result_win(res);
}

static void cb_ts_import()
{
   /* Create the selector */

   GtkWidget *file_selector = gtk_file_selection_new ("請輸入要匯入的檔案名稱");

   g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                     "clicked",
                     G_CALLBACK (cb_file_ts_import),
                     (gpointer) file_selector);

   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->ok_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) file_selector);

   g_signal_connect_swapped (GTK_OBJECT (GTK_FILE_SELECTION (file_selector)->cancel_button),
                             "clicked",
                             G_CALLBACK (gtk_widget_destroy),
                             (gpointer) file_selector);

   gtk_widget_show(file_selector);
}

static char utf8_edit[]=GCIN_SCRIPT_DIR"/utf8-edit";

static void cb_ts_edit()
{
  char tt[512];

  sprintf(tt, "( cd ~/.gcin && tsd2a tsin > tmpfile && %s tmpfile && tsa2d tmpfile ) &", utf8_edit);
  system(tt);
}


static void cb_alt_shift()
{
  char tt[512];

  sprintf(tt, "( cd ~/.gcin && %s phrase.table ) &", utf8_edit);
  system(tt);
}


static void cb_symbol_table()
{
  char tt[512];

  sprintf(tt, "( cd ~/.gcin && %s symbol-table ) &", utf8_edit);
  system(tt);
}


int utf8_editor(char *fname)
{
  char tt[256];

  sprintf(tt, "%s %s", utf8_edit, fname);
  dbg("%s\n", tt);
  return system(tt);
}


static void cb_help()
{
  utf8_editor(DOC_DIR"/README");
}

struct {
  char *str;
  int num;
} spc_opts[] = {
  {"按空白立即送出第一字(嘸蝦米)", GTAB_space_auto_first_any},
  {"按滿按空白送出第一字(內定)", GTAB_space_auto_first_full},
  {"按滿按空白不送出第一字(Windows 行列)", GTAB_space_auto_first_nofull},
  { NULL, 0},
};

static GtkWidget *spinner, *spinner_tsin_presel, *spinner_symbol, *spinner_tsin_pho_in;

static gboolean cb_appearance_conf_ok( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{

  int font_size = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner));
  save_gcin_conf_int(GCIN_FONT_SIZE, font_size);

  int font_size_tsin_presel = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_tsin_presel));
  save_gcin_conf_int(GCIN_FONT_SIZE_TSIN_PRESEL, font_size_tsin_presel);

  int font_size_symbol = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_symbol));
  save_gcin_conf_int(GCIN_FONT_SIZE_SYMBOL, font_size_symbol);

  int font_size_tsin_pho_in = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_tsin_pho_in));
  save_gcin_conf_int(GCIN_FONT_SIZE_TSIN_PHO_IN, font_size_tsin_pho_in);

  send_gcin_message(GDK_DISPLAY(), CHANGE_FONT_SIZE);
  gtk_widget_destroy(gcin_appearance_conf_window); gcin_appearance_conf_window = NULL;

  return TRUE;
}

static gboolean close_appearance_conf_window( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  gtk_widget_destroy(gcin_appearance_conf_window); gcin_appearance_conf_window = NULL;
  return TRUE;
}


void create_appearance_conf_window()
{
  if (gcin_appearance_conf_window) {
    gtk_window_present(GTK_WINDOW(gcin_appearance_conf_window));
    return;
  }

  load_setttings();

  gcin_appearance_conf_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  g_signal_connect (G_OBJECT (gcin_appearance_conf_window), "delete_event",
                    G_CALLBACK (close_appearance_conf_window),
                    NULL);

  gtk_window_set_title (GTK_WINDOW (gcin_appearance_conf_window), "輸入視窗外觀設定");
  gtk_container_set_border_width (GTK_CONTAINER (gcin_appearance_conf_window), 3);

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 10);
  gtk_container_add (GTK_CONTAINER (gcin_appearance_conf_window), vbox_top);

  GtkWidget *frame_font_size = gtk_frame_new("字型大小");
  gtk_box_pack_start (GTK_BOX (vbox_top), frame_font_size, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_font_size), 3);
  GtkAdjustment *adj =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size, 8.0, 24.0, 1.0, 1.0, 0.0);
  spinner = gtk_spin_button_new (adj, 0, 0);
  gtk_container_add (GTK_CONTAINER (frame_font_size), spinner);

  GtkWidget *frame_font_size_symbol = gtk_frame_new("符號選擇視窗字型大小");
  gtk_box_pack_start (GTK_BOX (vbox_top), frame_font_size_symbol, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_font_size_symbol), 3);
  GtkAdjustment *adj_symbol =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size_symbol, 8.0, 24.0, 1.0, 1.0, 0.0);
  spinner_symbol = gtk_spin_button_new (adj_symbol, 0, 0);
  gtk_container_add (GTK_CONTAINER (frame_font_size_symbol), spinner_symbol);


  GtkWidget *frame_font_size_tsin_presel = gtk_frame_new("詞音預選詞視窗字型大小");
  gtk_box_pack_start (GTK_BOX (vbox_top), frame_font_size_tsin_presel, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_font_size_tsin_presel), 3);
  GtkAdjustment *adj_tsin_presel =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size_tsin_presel, 8.0, 24.0, 1.0, 1.0, 0.0);
  spinner_tsin_presel = gtk_spin_button_new (adj_tsin_presel, 0, 0);
  gtk_container_add (GTK_CONTAINER (frame_font_size_tsin_presel), spinner_tsin_presel);

  GtkWidget *frame_font_size_tsin_pho_in = gtk_frame_new("詞音注音輸入區字型大小");
  gtk_box_pack_start (GTK_BOX (vbox_top), frame_font_size_tsin_pho_in, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_font_size_tsin_pho_in), 3);
  GtkAdjustment *adj_tsin_pho_in =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size_tsin_pho_in, 8.0, 24.0, 1.0, 1.0, 0.0);
  spinner_tsin_pho_in = gtk_spin_button_new (adj_tsin_pho_in, 0, 0);
  gtk_container_add (GTK_CONTAINER (frame_font_size_tsin_pho_in), spinner_tsin_pho_in);


  GtkWidget *hbox_cancel_ok = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_cancel_ok, FALSE, FALSE, 0);

  GtkWidget *button_cancel = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_cancel, FALSE, FALSE, 0);

  g_signal_connect (G_OBJECT (button_cancel), "clicked",
                            G_CALLBACK (close_appearance_conf_window),
                            G_OBJECT (gcin_appearance_conf_window));

  GtkWidget *button_close = gtk_button_new_from_stock (GTK_STOCK_OK);
  gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_close, TRUE, TRUE, 0);

  g_signal_connect_swapped (G_OBJECT (button_close), "clicked",
                            G_CALLBACK (cb_appearance_conf_ok),
                            G_OBJECT (gcin_kbm_window));

  GTK_WIDGET_SET_FLAGS (button_close, GTK_CAN_DEFAULT);
  gtk_widget_grab_default (button_close);

  gtk_widget_show_all (gcin_appearance_conf_window);

  return;
}

static GtkWidget *gcin_gtab_conf_window;

static gboolean cb_gtab_conf_ok( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  save_gcin_conf_int(GTAB_DUP_SELECT_BELL,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_dup_select_bell)));

  save_gcin_conf_int(GTAB_AUTO_SELECT_BY_PHRASE,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_auto_select_by_phrase)));

  save_gcin_conf_int(GTAB_PRE_SELECT,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_pre_select)));

  save_gcin_conf_int(GTAB_PRESS_FULL_AUTO_SEND,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_press_full_auto_send)));

  save_gcin_conf_int(GTAB_DISP_PARTIAL_MATCH,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_disp_partial_match)));

  save_gcin_conf_int(GTAB_SIMPLE_WIN,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_simple_win)));

  save_gcin_conf_int(GTAB_DISP_KEY_CODES,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_disp_key_codes)));

  save_gcin_conf_int(GTAB_DISP_IM_NAME,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_disp_im_name)));

  int idx = gtk_option_menu_get_history (GTK_OPTION_MENU (opt_spc_opts));
  save_gcin_conf_int(GTAB_SPACE_AUTO_FIRST, spc_opts[idx].num);

  send_gcin_message(GDK_DISPLAY(), CHANGE_FONT_SIZE);
  gtk_widget_destroy(gcin_gtab_conf_window); gcin_gtab_conf_window = NULL;

  return TRUE;
}


static gboolean close_gtab_conf_window( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  gtk_widget_destroy(gcin_gtab_conf_window); gcin_gtab_conf_window = NULL;
  return TRUE;
}


static GtkWidget *create_spc_opts()
{

  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);
  GtkWidget *label = gtk_label_new("空白鍵選項");
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  opt_spc_opts = gtk_option_menu_new ();
  gtk_box_pack_start (GTK_BOX (hbox), opt_spc_opts, FALSE, FALSE, 0);
  GtkWidget *menu_spc_opts = gtk_menu_new ();

  int i, current_idx=0;

  for(i=0; spc_opts[i].str; i++) {
    GtkWidget *item = gtk_menu_item_new_with_label (spc_opts[i].str);

    if (spc_opts[i].num == gtab_space_auto_first)
      current_idx = i;

    gtk_menu_shell_append (GTK_MENU_SHELL (menu_spc_opts), item);
  }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (opt_spc_opts), menu_spc_opts);
  gtk_option_menu_set_history (GTK_OPTION_MENU (opt_spc_opts), current_idx);

  return hbox;
}


void create_gtab_conf_window()
{
  if (gcin_gtab_conf_window) {
    gtk_window_present(GTK_WINDOW(gcin_gtab_conf_window));
    return;
  }

  load_setttings();

    gcin_gtab_conf_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    g_signal_connect (G_OBJECT (gcin_gtab_conf_window), "delete_event",
                      G_CALLBACK (close_gtab_conf_window),
                      NULL);

    gtk_window_set_title (GTK_WINDOW (gcin_gtab_conf_window), "倉頡/行列/嘸蝦米/大易設定");
    gtk_container_set_border_width (GTK_CONTAINER (gcin_gtab_conf_window), 3);

    GtkWidget *vbox_top = gtk_vbox_new (FALSE, 10);
    gtk_container_add (GTK_CONTAINER (gcin_gtab_conf_window), vbox_top);


    GtkWidget *frame_gtab = gtk_frame_new("倉頡/行列/嘸蝦米/大易");
    gtk_container_set_border_width (GTK_CONTAINER (frame_gtab), 5);
    gtk_box_pack_start (GTK_BOX (vbox_top), frame_gtab, FALSE, FALSE, 0);
    GtkWidget *vbox_gtab = gtk_vbox_new (FALSE, 10);
    gtk_container_add (GTK_CONTAINER (frame_gtab), vbox_gtab);
    gtk_container_set_border_width (GTK_CONTAINER (vbox_gtab), 10);


    GtkWidget *hbox_gtab_dup_select_bell = gtk_hbox_new (FALSE, 10);
    gtk_box_pack_start (GTK_BOX (vbox_gtab), hbox_gtab_dup_select_bell, FALSE, FALSE, 0);
    GtkWidget *label_gtab_sele = gtk_label_new("重複字選擇鈴聲");
    gtk_box_pack_start (GTK_BOX (hbox_gtab_dup_select_bell), label_gtab_sele,  FALSE, FALSE, 0);
    check_button_gtab_dup_select_bell = gtk_check_button_new ();
    gtk_box_pack_start (GTK_BOX (hbox_gtab_dup_select_bell),check_button_gtab_dup_select_bell,  FALSE, FALSE, 0);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_dup_select_bell),
       gtab_dup_select_bell);

    gtk_box_pack_start (GTK_BOX (vbox_gtab), create_spc_opts(), FALSE, FALSE, 0);

    GtkWidget *hbox_gtab_auto_select_by_phrase = gtk_hbox_new (FALSE, 10);
    gtk_box_pack_start (GTK_BOX (vbox_gtab), hbox_gtab_auto_select_by_phrase, FALSE, FALSE, 0);
    GtkWidget *label_gtab_auto_select = gtk_label_new("自動由詞庫自動選擇字");
    gtk_box_pack_start (GTK_BOX (hbox_gtab_auto_select_by_phrase), label_gtab_auto_select,  FALSE, FALSE, 0);
    check_button_gtab_auto_select_by_phrase = gtk_check_button_new ();
    gtk_box_pack_start (GTK_BOX (hbox_gtab_auto_select_by_phrase),check_button_gtab_auto_select_by_phrase,  FALSE, FALSE, 0);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_auto_select_by_phrase),
       gtab_auto_select_by_phrase);


    GtkWidget *hbox_gtab_pre_select = gtk_hbox_new (FALSE, 10);
    gtk_box_pack_start (GTK_BOX (vbox_gtab), hbox_gtab_pre_select, FALSE, FALSE, 0);
    GtkWidget *label_gtab_pre_select = gtk_label_new("預覽/預選 字");
    gtk_box_pack_start (GTK_BOX (hbox_gtab_pre_select), label_gtab_pre_select,  FALSE, FALSE, 0);
    check_button_gtab_pre_select = gtk_check_button_new ();
    gtk_box_pack_start (GTK_BOX (hbox_gtab_pre_select),check_button_gtab_pre_select,  FALSE, FALSE, 0);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_pre_select),
       gtab_pre_select);


    GtkWidget *hbox_gtab_press_full_auto_send = gtk_hbox_new (FALSE, 10);
    gtk_box_pack_start (GTK_BOX (vbox_gtab), hbox_gtab_press_full_auto_send, FALSE, FALSE, 0);
    GtkWidget *label_gtab_gtab_press_full_auto_send = gtk_label_new("按滿自動送字");
    gtk_box_pack_start (GTK_BOX (hbox_gtab_press_full_auto_send), label_gtab_gtab_press_full_auto_send,  FALSE, FALSE, 0);
    check_button_gtab_press_full_auto_send = gtk_check_button_new ();
    gtk_box_pack_start (GTK_BOX (hbox_gtab_press_full_auto_send), check_button_gtab_press_full_auto_send,  FALSE, FALSE, 0);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_press_full_auto_send),
       gtab_press_full_auto_send);


    GtkWidget *hbox_gtab_disp_partial_match = gtk_hbox_new (FALSE, 10);
    gtk_box_pack_start (GTK_BOX (vbox_gtab), hbox_gtab_disp_partial_match, FALSE, FALSE, 0);
    GtkWidget *label_gtab_gtab_disp_partial_match = gtk_label_new("預選列中顯示部份符合的字");
    gtk_box_pack_start (GTK_BOX (hbox_gtab_disp_partial_match), label_gtab_gtab_disp_partial_match,  FALSE, FALSE, 0);
    check_button_gtab_disp_partial_match = gtk_check_button_new ();
    gtk_box_pack_start (GTK_BOX (hbox_gtab_disp_partial_match), check_button_gtab_disp_partial_match,  FALSE, FALSE, 0);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_disp_partial_match),
       gtab_disp_partial_match);


    GtkWidget *hbox_gtab_simple_win = gtk_hbox_new (FALSE, 10);
    gtk_box_pack_start (GTK_BOX (vbox_gtab), hbox_gtab_simple_win, FALSE, FALSE, 0);
    GtkWidget *label_gtab_gtab_simple_win = gtk_label_new("較小視窗");
    gtk_box_pack_start (GTK_BOX (hbox_gtab_simple_win), label_gtab_gtab_simple_win,  FALSE, FALSE, 0);
    check_button_gtab_simple_win = gtk_check_button_new ();
    gtk_box_pack_start (GTK_BOX (hbox_gtab_simple_win), check_button_gtab_simple_win,  FALSE, FALSE, 0);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_simple_win),
       gtab_simple_win);


    GtkWidget *hbox_gtab_disp_key_codes = gtk_hbox_new (FALSE, 10);
    gtk_box_pack_start (GTK_BOX (vbox_gtab), hbox_gtab_disp_key_codes, FALSE, FALSE, 0);
    GtkWidget *label_gtab_gtab_disp_key_codes = gtk_label_new("顯示拆字");
    gtk_box_pack_start (GTK_BOX (hbox_gtab_disp_key_codes), label_gtab_gtab_disp_key_codes,  FALSE, FALSE, 0);
    check_button_gtab_disp_key_codes = gtk_check_button_new ();
    gtk_box_pack_start (GTK_BOX (hbox_gtab_disp_key_codes), check_button_gtab_disp_key_codes,  FALSE, FALSE, 0);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_disp_key_codes),
       gtab_disp_key_codes);


    GtkWidget *hbox_gtab_disp_im_name = gtk_hbox_new (FALSE, 10);
    gtk_box_pack_start (GTK_BOX (vbox_gtab), hbox_gtab_disp_im_name, FALSE, FALSE, 0);
    GtkWidget *label_gtab_gtab_disp_im_name = gtk_label_new("顯示輸入法名稱");
    gtk_box_pack_start (GTK_BOX (hbox_gtab_disp_im_name), label_gtab_gtab_disp_im_name,  FALSE, FALSE, 0);
    check_button_gtab_disp_im_name = gtk_check_button_new ();
    gtk_box_pack_start (GTK_BOX (hbox_gtab_disp_im_name), check_button_gtab_disp_im_name,  FALSE, FALSE, 0);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_disp_im_name),
       gtab_disp_im_name);


    GtkWidget *hbox_cancel_ok = gtk_hbox_new (FALSE, 10);
    gtk_box_pack_start (GTK_BOX (vbox_top), hbox_cancel_ok, FALSE, FALSE, 0);

    GtkWidget *button_cancel = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
    gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_cancel, TRUE, TRUE, 0);

    g_signal_connect (G_OBJECT (button_cancel), "clicked",
                              G_CALLBACK (close_gtab_conf_window),
                              G_OBJECT (gcin_gtab_conf_window));

    GtkWidget *button_ok = gtk_button_new_from_stock (GTK_STOCK_OK);
    gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_ok, TRUE, TRUE, 0);

    g_signal_connect_swapped (G_OBJECT (button_ok), "clicked",
                              G_CALLBACK (cb_gtab_conf_ok),
                              G_OBJECT (gcin_gtab_conf_window));

    GTK_WIDGET_SET_FLAGS (button_ok, GTK_CAN_DEFAULT);
    gtk_widget_grab_default (button_ok);

    gtk_widget_show_all (gcin_gtab_conf_window);

    return;
}



static void cb_appearance_conf()
{
  create_appearance_conf_window();
}


static void cb_gtab_conf()
{
  create_gtab_conf_window();
}


void create_gtablist_window();
static void cb_default_input_method()
{
  create_gtablist_window();
}

GtkWidget *main_window;
void create_about_window();

static void create_main_win()
{
  main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  g_signal_connect (G_OBJECT (main_window), "delete_event",
                     G_CALLBACK (close_application),
                     NULL);

  gtk_window_set_icon_from_file(GTK_WINDOW(main_window), SYS_ICON_DIR"/gcin.png", NULL);


  GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (main_window), vbox);

  GtkWidget *button_kbm = gtk_button_new_with_label("gcin 注音輸入設定");
  gtk_box_pack_start (GTK_BOX (vbox), button_kbm, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_kbm), "clicked",
                    G_CALLBACK (cb_kbm), NULL);

  GtkWidget *button_appearance_conf = gtk_button_new_with_label("外觀設定");
  gtk_box_pack_start (GTK_BOX (vbox), button_appearance_conf, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_appearance_conf), "clicked",
                    G_CALLBACK (cb_appearance_conf), NULL);

  GtkWidget *button_gtab_conf = gtk_button_new_with_label("倉頡/行列/嘸蝦米/大易設定");
  gtk_box_pack_start (GTK_BOX (vbox), button_gtab_conf, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_gtab_conf), "clicked",
                    G_CALLBACK (cb_gtab_conf), NULL);


  GtkWidget *button_default_input_method = gtk_button_new_with_label("內定輸入法 & 開啟/關閉");
  gtk_box_pack_start (GTK_BOX (vbox), button_default_input_method, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_default_input_method), "clicked",
                    G_CALLBACK (cb_default_input_method), NULL);

  GtkWidget *button_tslearn = gtk_button_new_with_label("讓詞音從文章學習詞");
  gtk_box_pack_start (GTK_BOX (vbox), button_tslearn, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_tslearn), "clicked",
                    G_CALLBACK (cb_tslearn), NULL);

  GtkWidget *button_ts_export = gtk_button_new_with_label("詞庫匯出");
  gtk_box_pack_start (GTK_BOX (vbox), button_ts_export, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_ts_export), "clicked",
                    G_CALLBACK (cb_ts_export), NULL);

  GtkWidget *button_ts_import = gtk_button_new_with_label("詞庫匯入");
  gtk_box_pack_start (GTK_BOX (vbox), button_ts_import, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_ts_import), "clicked",
                    G_CALLBACK (cb_ts_import), NULL);

  GtkWidget *button_ts_edit = gtk_button_new_with_label("詞庫編輯");
  gtk_box_pack_start (GTK_BOX (vbox), button_ts_edit, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_ts_edit), "clicked",
                    G_CALLBACK (cb_ts_edit), NULL);


  GtkWidget *button_alt_shift = gtk_button_new_with_label("alt-shift 片語編輯");
  gtk_box_pack_start (GTK_BOX (vbox), button_alt_shift, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_alt_shift), "clicked",
                    G_CALLBACK (cb_alt_shift), NULL);

  GtkWidget *button_symbol_table = gtk_button_new_with_label("符號表編輯");
  gtk_box_pack_start (GTK_BOX (vbox), button_symbol_table, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_symbol_table), "clicked",
                    G_CALLBACK (cb_symbol_table), NULL);


  GtkWidget *button_about = gtk_button_new_with_label("關於 gcin");
  gtk_box_pack_start (GTK_BOX (vbox), button_about, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_about), "clicked",
                    G_CALLBACK (create_about_window),  NULL);


  GtkWidget *button_help = gtk_button_new_from_stock (GTK_STOCK_HELP);
  gtk_box_pack_start (GTK_BOX (vbox), button_help, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_help), "clicked",
                    G_CALLBACK (cb_help), NULL);

  GtkWidget *button_quit = gtk_button_new_from_stock (GTK_STOCK_QUIT);
  gtk_box_pack_start (GTK_BOX (vbox), button_quit, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_quit), "clicked",
                    G_CALLBACK (close_application), NULL);

  gtk_widget_show_all(main_window);
}


void init_TableDir();

int main(int argc, char **argv)
{
  init_TableDir();

  load_setttings();

  gtk_init(&argc, &argv);

  create_main_win();

  // once you invoke gcin-setup, the left-right buton tips is disabled
  save_gcin_conf_int(LEFT_RIGHT_BUTTON_TIPS, 0);

  gtk_main();

  return 0;
}
