#include "config.h"
#include "gcin.h"
#include "gtab.h"
#include "pho.h"
#include <dirent.h>
#include <libintl.h>

char *gcb_pos[] = {
  N_("關閉"), N_("左下"), N_("左上"), N_("右下"), N_("右上")
};

static GdkColor gcin_win_gcolor_fg,
                gcin_win_gcolor_bg,
                gcin_sel_key_gcolor,
                tsin_phrase_line_gcolor,
                tsin_cursor_gcolor;

static GtkClipboard *pclipboard;

static GtkWidget *check_button_root_style_use,
                 *check_button_gcin_pop_up_win,
                 *check_button_gcin_inner_frame,
                 *check_button_gcin_status_tray,
                 *check_button_gcin_win_color_use;

static GtkWidget *check_button_gcin_eng_phrase_enabled,
                 *check_button_gcin_remote_client,
                 *check_button_gcin_shift_space_eng_full,
                 *check_button_phonetic_speak,
                 *event_box_win_color_test,
                 *font_sel,
                 *da_phrase_line,
                 *da_cursor,
                 *da_sel_key,
                 *label_win_color_test,
                 *opt_eng_ch_opts,
                 *opt_gcb_pos,
                 *opt_im_toggle_keys,
                 *opt_kbm_opts,
                 *opt_spc_opts,
                 *opt_speaker_opts,
                 *spinner_gcin_font_size,
                 *spinner_gcin_font_size_gtab_in,
                 *spinner_gcin_font_size_pho_near,
                 *spinner_gcin_font_size_symbol,
                 *spinner_gcin_font_size_tsin_pho_in,
                 *spinner_gcin_font_size_tsin_presel,
                 *spinner_root_style_x,
                 *spinner_root_style_y,
                 *sw,
                 *treeview;

GtkWidget *main_window;

static GtkWidget *check_button_tsin_phrase_pre_select,
                 *check_button_phonetic_char_dynamic_sequence,
                 *check_button_pho_hide_row2,
                 *check_button_pho_in_row1,
                 *check_button_phonetic_huge_tab,
                 *check_button_tsin_tone_char_input,
                 *check_button_tsin_tab_phrase_end,
                 *check_button_tsin_tail_select_key,
                 *check_button_tsin_buffer_editing_mode,
                 *check_button_gtab_que_wild_card,
                 *check_button_gcin_capslock_lower,
                 *spinner_tsin_buffer_size;

static GtkWidget *check_button_gtab_dup_select_bell,
                 *check_button_gtab_auto_select_by_phrase,
                 *check_button_gtab_press_full_auto_send,
                 *check_button_gtab_pre_select,
                 *check_button_gtab_disp_partial_match,
                 *check_button_gtab_disp_key_codes,
                 *check_button_gtab_disp_im_name,
                 *check_button_gtab_invalid_key_in,
                 *check_button_gtab_shift_phrase_key,
                 *check_button_gtab_hide_row2,
                 *check_button_gtab_in_row1,
                 *check_button_gtab_capslock_in_eng,
                 *check_button_gtab_vertical_select,
                 *check_button_gtab_unique_auto_send,
                 *check_button_gcin_init_im_enabled,
                 *check_button_gcin_win_sym_click_close,
                 *spinner_gcb_position_x, *spinner_gcb_position_y;

char *pho_speaker[16];
int pho_speakerN;
static int new_select_idx_tsin_space_opt;


struct {
  char *keystr;
  int keynum;
} imkeys[] = {
  {"Control-Space", Control_Space},
  {"Shift-Space", Shift_Space},
  {"Alt-Space", Alt_Space},
  {"Windows-Space", Windows_Space},
  { NULL, 0},
};


static struct {
  char *name;
  int key;
} tsin_space_options[]={
  {N_("選擇同音字"), TSIN_SPACE_OPT_SELECT_CHAR},
  {N_("輸入空白"), TSIN_SPACE_OPT_INPUT},
};
int tsin_space_optionsN = sizeof(tsin_space_options) / sizeof(tsin_space_options[0]);


static void callback_button_clicked_tsin_space_opt( GtkWidget *widget, gpointer data)
{
  new_select_idx_tsin_space_opt = (int) data;
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


struct {
  char *str;
  int num;
} spc_opts[] = {
  {N_("由 .gtab 指定"), GTAB_space_auto_first_none},
  {N_("按空白立即送出第一字(嘸蝦米、大易)"), GTAB_space_auto_first_any},
  {N_("按滿按空白送出第一字"), GTAB_space_auto_first_full},
  {N_("按滿按空白不送出第一字(倉頡, 行列)"), GTAB_space_auto_first_nofull},
  { NULL, 0},
};


static void cb_save_tsin_cursor_color(GtkWidget *widget, gpointer user_data)
{
  GtkColorSelectionDialog *color_selector = (GtkColorSelectionDialog *)user_data;
  gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(color_selector->colorsel), &tsin_cursor_gcolor);

  gtk_widget_modify_bg(da_cursor, GTK_STATE_NORMAL, &tsin_cursor_gcolor);
}


static GtkWidget *create_spc_opts()
{
  GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
  GtkWidget *label = gtk_label_new(_("空白鍵選項"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  opt_spc_opts = gtk_option_menu_new ();
  gtk_box_pack_start (GTK_BOX (hbox), opt_spc_opts, FALSE, FALSE, 0);
  GtkWidget *menu_spc_opts = gtk_menu_new ();

  int i, current_idx=0;

  for(i=0; spc_opts[i].str; i++) {
    GtkWidget *item = gtk_menu_item_new_with_label (_(spc_opts[i].str));

    if (spc_opts[i].num == gtab_space_auto_first)
      current_idx = i;

    gtk_menu_shell_append (GTK_MENU_SHELL (menu_spc_opts), item);
  }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (opt_spc_opts), menu_spc_opts);
  gtk_option_menu_set_history (GTK_OPTION_MENU (opt_spc_opts), current_idx);

  return hbox;
}


char utf8_edit[]=GCIN_SCRIPT_DIR"/utf8-edit";
char html_browse[]=GCIN_SCRIPT_DIR"/html-browser";


static gboolean cb_gtab_edit_append( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  load_gtab_list();
  char *fname = inmd[default_input_method].filename;
  if (!fname)
    return;
  char append_fname[128];
  sprintf(append_fname, "~/.gcin/%s.append", fname);

  char prepare[128];

  sprintf(prepare, GCIN_SCRIPT_DIR"/gtab.append_prepare %s", append_fname);
  system(prepare);

  char exec[128];

  sprintf(exec, "%s %s", utf8_edit, append_fname);
  dbg("exec %s\n", exec);
  system(exec);
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


typedef struct
{
  gchar *name;
  GdkPixbuf *icon;
  gchar *key;
  gchar *file;
  gboolean used;
  gboolean default_inmd;
  gboolean editable;
} Item;


enum
{
  COLUMN_NAME,
  COLUMN_ICON,
  COLUMN_KEY,
  COLUMN_FILE,
  COLUMN_USE,
  COLUMN_DEFAULT_INMD,
  COLUMN_EDITABLE,
  NUM_COLUMNS
};


static int qcmp_key(const void *aa, const void *bb)
{
  Item *a=(Item *)aa, *b=(Item *)bb;

  return gcin_switch_keys_lookup(a->key[0]) - gcin_switch_keys_lookup(b->key[0]);
}


static GArray *articles = NULL;

static void
add_items (void)
{
  Item foo;

  g_return_if_fail (articles != NULL);

  load_gtab_list();

  int i;
  for (i=1; i <= MAX_GTAB_NUM_KEY; i++) {
    INMD *pinmd = &inmd[i];
    char *name = pinmd->cname;
    if (!name)
      continue;

    char key[2];
    char *file = pinmd->filename;
    char *icon = pinmd->icon;

    key[0] = gcin_switch_keys[i]; key[1]=0;

    foo.name = g_strdup(name);
    char icon_path[128];
    get_icon_path(icon, icon_path);
    GError *err = NULL;
    foo.icon = gdk_pixbuf_new_from_file(icon_path, &err);
    foo.key = g_strdup(key);

    foo.file = g_strdup(file);
    foo.used = (gcin_flags_im_enabled & (1 << i)) != 0;
    foo.default_inmd =  default_input_method == i;
    foo.editable = FALSE;
    g_array_append_vals (articles, &foo, 1);
  }

  g_array_sort (articles,qcmp_key);
}


static GtkTreeModel *
create_model (void)
{
  gint i = 0;
  GtkListStore *model;
  GtkTreeIter iter;

  /* create array */
  articles = g_array_sized_new (FALSE, FALSE, sizeof (Item), 1);

  add_items ();

  /* create list store */
  model = gtk_list_store_new (NUM_COLUMNS,G_TYPE_STRING, GDK_TYPE_PIXBUF,
                              G_TYPE_STRING,
                              G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
                              G_TYPE_BOOLEAN);

  /* add items */
  for (i = 0; i < articles->len; i++) {
      gtk_list_store_append (model, &iter);

      gtk_list_store_set (model, &iter,
			  COLUMN_NAME,
			  g_array_index (articles, Item, i).name,
			  COLUMN_ICON,
			  g_array_index (articles, Item, i).icon,
			  COLUMN_KEY,
			  g_array_index (articles, Item, i).key,
			  COLUMN_FILE,
			  g_array_index (articles, Item, i).file,
			  COLUMN_USE,
                          g_array_index (articles, Item, i).used,
			  COLUMN_DEFAULT_INMD,
                          g_array_index (articles, Item, i).default_inmd,
                          COLUMN_EDITABLE,
                          g_array_index (articles, Item, i).editable,
                          -1);
  }

  return GTK_TREE_MODEL (model);
}


static void clear_all(GtkTreeModel *model)
{
  GtkTreeIter iter;

  if (!gtk_tree_model_get_iter_first(model, &iter))
    return;

  do {
    char *tkey;

    gtk_tree_model_get(model,&iter, COLUMN_KEY, &tkey, -1);

    gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_DEFAULT_INMD, 0, -1);

  } while (gtk_tree_model_iter_next(model, &iter));
}


static gboolean toggled (GtkCellRendererToggle *cell, gchar *path_string, gpointer data)
{
  GtkTreeModel *model = GTK_TREE_MODEL (data);
  GtkTreeIter iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
  gboolean value;

  printf("toggled\n");

  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, COLUMN_USE, &value, -1);
  int i = gtk_tree_path_get_indices (path)[0];
  char *key=g_array_index (articles, Item, i).key;
  int in_no = gcin_switch_keys_lookup(key[0]);

  if (in_no < 0)
    return;

  gcin_flags_im_enabled ^= 1 << in_no;
  value ^= 1;

  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_USE, value, -1);
  gtk_tree_path_free (path);

  return TRUE;
}


static gboolean toggled_default_inmd(GtkCellRendererToggle *cell, gchar *path_string, gpointer data)
{
  GtkTreeModel *model = GTK_TREE_MODEL (data);
  clear_all(model);
  GtkTreeIter iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);

//  dbg("toggled_default_inmd\n");
  gtk_tree_model_get_iter (model, &iter, path);
  int i = gtk_tree_path_get_indices (path)[0];
  char *key=g_array_index (articles, Item, i).key;
  default_input_method = gcin_switch_keys_lookup(key[0]);
  dbg("default_input_method %d %c\n", default_input_method, key[0]);

  if (default_input_method < 0)
    default_input_method = 6;

  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_DEFAULT_INMD, TRUE, -1);
  gtk_tree_path_free (path);

  return TRUE;
}


static void
add_columns (GtkTreeView *treeview)
{
  GtkCellRenderer *renderer;
  GtkTreeModel *model = gtk_tree_view_get_model (treeview);

  /* name column */

  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", (gint *)COLUMN_NAME);
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, _("名稱"), renderer,
                                               "text", COLUMN_NAME,
                                               "editable", COLUMN_EDITABLE,
                                               NULL);

  renderer = gtk_cell_renderer_pixbuf_new();
  g_object_set_data (G_OBJECT (renderer), "column", (gint *)COLUMN_ICON);
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, _("icon"), renderer,
                                               "pixbuf", COLUMN_ICON,
//                                               "editable", COLUMN_EDITABLE,
                                               NULL);


  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", (gint *)COLUMN_KEY);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, _("Ctrl-Alt-鍵"), renderer,
                                               "text", COLUMN_KEY,
                                               "editable", COLUMN_EDITABLE,
                                               NULL);

  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", (gint *)COLUMN_FILE);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, _("檔案名"), renderer,
                                               "text", COLUMN_FILE,
                                               "editable", COLUMN_EDITABLE,
                                               NULL);
  // use column
  renderer = gtk_cell_renderer_toggle_new ();
  g_signal_connect (G_OBJECT (renderer), "toggled",
                    G_CALLBACK (toggled), model);

  g_object_set (G_OBJECT (renderer), "xalign", 0.0, NULL);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, _("Ctrl-Shift 循環"),
                                               renderer,
                                               "active", COLUMN_USE,
                                               NULL);

  // default_inmd column
  renderer = gtk_cell_renderer_toggle_new ();
  g_signal_connect (G_OBJECT (renderer), "toggled",
                    G_CALLBACK (toggled_default_inmd), model);

  g_object_set (G_OBJECT (renderer), "xalign", 0.0, NULL);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, _("第一次內定"),
                                               renderer,
                                               "active", COLUMN_DEFAULT_INMD,
                                               NULL);
}


static void cb_save_tsin_phrase_line_color(GtkWidget *widget, gpointer user_data)
{
  GtkColorSelectionDialog *color_selector = (GtkColorSelectionDialog *)user_data;
  gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(color_selector->colorsel), &tsin_phrase_line_gcolor);

  gtk_widget_modify_bg(da_phrase_line, GTK_STATE_NORMAL, &tsin_phrase_line_gcolor);
}


static void cb_save_gcin_sel_key_color(GtkWidget *widget, gpointer user_data)
{
  GtkColorSelectionDialog *color_selector = (GtkColorSelectionDialog *)user_data;
  gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(color_selector->colorsel), &gcin_sel_key_gcolor);

  gtk_widget_modify_bg(da_sel_key, GTK_STATE_NORMAL, &gcin_sel_key_gcolor);
}


static gboolean cb_gcin_sel_key_color( GtkWidget *widget, gpointer data)
{
   GtkColorSelectionDialog *color_selector = (GtkColorSelectionDialog *)gtk_color_selection_dialog_new (_("選擇鍵的顏色"));

   gtk_color_selection_set_current_color(
           GTK_COLOR_SELECTION(color_selector->colorsel),
           &gcin_sel_key_gcolor);

   g_signal_connect (GTK_OBJECT (color_selector->ok_button),
                     "clicked",
                     G_CALLBACK (cb_save_gcin_sel_key_color),
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


static int get_currnet_eng_ch_sw_idx()
{
  int i;
  for(i=0; i < tsin_eng_ch_swN; i++)
    if (tsin_eng_ch_sw[i].key == tsin_chinese_english_toggle_key)
      return i;

  p_err("tsin-chinese-english-switch->%d is not valid", tsin_chinese_english_toggle_key);
  return -1;
}


int get_current_speaker_idx()
{
  int i;

  for(i=0; i < pho_speakerN; i++)
    if (!strcmp(pho_speaker[i], phonetic_speak_sel))
      return i;

  return 0;
}


static GtkWidget *create_eng_ch_opts()
{

  GtkWidget *hbox = gtk_hbox_new (FALSE, 0);

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


static GtkWidget *create_kbm_opts()
{

  GtkWidget *hbox = gtk_hbox_new (FALSE, 0);

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


static GtkWidget *create_im_toggle_keys()
{

  GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
  GtkWidget *label = gtk_label_new(_("輸入視窗(開啟/關閉)切換"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  opt_im_toggle_keys = gtk_option_menu_new ();
  gtk_box_pack_start (GTK_BOX (hbox), opt_im_toggle_keys, FALSE, FALSE, 0);
  GtkWidget *menu_im_toggle_keys = gtk_menu_new ();

  int i, current_idx=0;

  for(i=0; imkeys[i].keystr; i++) {
    GtkWidget *item = gtk_menu_item_new_with_label (imkeys[i].keystr);

    if (imkeys[i].keynum == gcin_im_toggle_keys)
      current_idx = i;

    gtk_menu_shell_append (GTK_MENU_SHELL (menu_im_toggle_keys), item);
  }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (opt_im_toggle_keys), menu_im_toggle_keys);
  gtk_option_menu_set_history (GTK_OPTION_MENU (opt_im_toggle_keys), current_idx);

  return hbox;
}


static GtkWidget *create_speaker_opts()
{
  GtkWidget *hbox = gtk_hbox_new (FALSE, 0);

  opt_speaker_opts = gtk_option_menu_new ();
  gtk_box_pack_start (GTK_BOX (hbox), opt_speaker_opts, FALSE, FALSE, 0);
  GtkWidget *menu_speaker_opts = gtk_menu_new ();

  int i;
  int current_idx = get_current_speaker_idx();

  for(i=0; i<pho_speakerN; i++) {
    GtkWidget *item = gtk_menu_item_new_with_label (pho_speaker[i]);

    if (imkeys[i].keynum == gcin_im_toggle_keys)
      current_idx = i;

    gtk_menu_shell_append (GTK_MENU_SHELL (menu_speaker_opts), item);
  }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (opt_speaker_opts), menu_speaker_opts);
  gtk_option_menu_set_history (GTK_OPTION_MENU (opt_speaker_opts), current_idx);

  return hbox;
}

static GtkWidget *create_gcb_pos_opts()
{
  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);

  opt_gcb_pos = gtk_option_menu_new ();
  gtk_box_pack_start (GTK_BOX (hbox), opt_gcb_pos, FALSE, FALSE, 0);
  GtkWidget *menu_gcb_pos = gtk_menu_new ();

  int i;

  for(i=0; i<sizeof(gcb_pos)/sizeof(gcb_pos[0]); i++) {
    GtkWidget *item = gtk_menu_item_new_with_label (_(gcb_pos[i]));

    gtk_menu_shell_append (GTK_MENU_SHELL (menu_gcb_pos), item);
  }

  gtk_option_menu_set_menu (GTK_OPTION_MENU (opt_gcb_pos), menu_gcb_pos);
  gtk_option_menu_set_history (GTK_OPTION_MENU (opt_gcb_pos), gcb_position);

  return hbox;
}

typedef struct {
  GdkColor *color;
  char **color_str;
  GtkColorSelectionDialog *color_selector;
  char *title;
} COLORSEL;

COLORSEL colorsel[2] =
  { {&gcin_win_gcolor_fg, &gcin_win_color_fg, "前景顏色"},
    {&gcin_win_gcolor_bg, &gcin_win_color_bg, "背景顏色"}
  };

static gboolean close_application( GtkWidget *widget,
                                   GdkEvent  *event,
                                   gpointer   data )
{
  exit(0);
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
  char *restr = res ? N_("結果失敗"):N_("結果成功");
  main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  GtkWidget *button = gtk_button_new_with_label(_(restr));
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
   snprintf(cmd, sizeof(cmd), GCIN_BIN_DIR"/tsd2a32 %s/tsin32 > %s", gcin_dir, selected_filename);
   int res = system(cmd);
   create_result_win(res);
}


static void cb_ts_export()
{
   /* Create the selector */

   GtkWidget *file_selector = gtk_file_selection_new (_("請輸入要匯出的檔案名稱"));

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
      "cd %s/.gcin && "GCIN_BIN_DIR"/tsd2a32 tsin32 > tmpfile && cat %s >> tmpfile && "GCIN_BIN_DIR"/tsa2d32 tmpfile",
      getenv("HOME"), selected_filename);
   int res = system(cmd);
   create_result_win(res);
}


static void cb_ts_import()
{
   /* Create the selector */

   GtkWidget *file_selector = gtk_file_selection_new (_("請輸入要匯入的檔案名稱"));

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


static void cb_ts_edit()
{
  char tt[512];

  sprintf(tt, "( cd ~/.gcin && "GCIN_BIN_DIR"/tsd2a32 tsin32 > tmpfile && %s tmpfile && "GCIN_BIN_DIR"/tsa2d32 tmpfile ) &", utf8_edit);
  dbg("exec %s\n", tt);
  system(tt);
}


static void cb_ts_import_sys()
{
  char tt[512];

  sprintf(tt, "cd ~/.gcin && "GCIN_BIN_DIR"/tsd2a32 tsin32 > tmpfile && "GCIN_BIN_DIR"/tsd2a32 %s/tsin32 >> tmpfile && "GCIN_BIN_DIR"/tsa2d32 tmpfile", GCIN_TABLE_DIR);
  dbg("exec %s\n", tt);
  system(tt);
}


static void cb_alt_shift()
{
  char tt[512];

  sprintf(tt, "( cd ~/.gcin && %s phrase.table ) &", utf8_edit);
  system(tt);
}


static void cb_ctrl()
{
  char tt[512];

  sprintf(tt, "( cd ~/.gcin && %s phrase-ctrl.table ) &", utf8_edit);
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

int html_browser(char *fname)
{
  char tt[256];

  sprintf(tt, "%s %s", html_browse, fname);
  dbg("%s\n", tt);
  return system(tt);
}

static void cb_help()
{
  html_browser(DOC_DIR"/README.html");
}


static void cb_gb_output_toggle()
{
  send_gcin_message(GDK_DISPLAY(), GB_OUTPUT_TOGGLE);
  exit(0);
}


static void cb_gb_translate_toggle()
{
  system(GCIN_BIN_DIR"/sim2trad &");
  exit(0);
}


static void cb_big5_translate_toggle()
{
  system(GCIN_BIN_DIR"/trad2sim &");
  exit(0);
}


static void cb_juying_learn()
{
  system(GCIN_BIN_DIR"/juyin-learn &");
  exit(0);
}


int gcin_pid;

static void cb_gcin_exit()
{
  kill(gcin_pid, 9);
}


static void cb_savecb_gcin_win_color_fg(GtkWidget *widget, gpointer user_data)
{
  COLORSEL *sel = user_data;
  GtkColorSelectionDialog *color_selector = sel->color_selector;
  GdkColor *col = sel->color;
  gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(color_selector->colorsel), col);

  if (sel->color == &gcin_win_gcolor_fg)
    gtk_widget_modify_fg(label_win_color_test, GTK_STATE_NORMAL, col);
  else {
    gtk_widget_modify_bg(event_box_win_color_test, GTK_STATE_NORMAL, col);
  }
}


static gboolean cb_gcin_win_color_fg( GtkWidget *widget,
                                   gpointer   data)
{
  COLORSEL *sel = data;
  GtkColorSelectionDialog *color_selector =
  (GtkColorSelectionDialog *)gtk_color_selection_dialog_new (sel->title);

  gdk_color_parse(*sel->color_str, sel->color);

  gtk_color_selection_set_current_color(
          GTK_COLOR_SELECTION(color_selector->colorsel),
          sel->color);

  g_signal_connect (GTK_OBJECT (color_selector->ok_button),
                    "clicked",
                    G_CALLBACK (cb_savecb_gcin_win_color_fg),
                    (gpointer) sel);

  sel->color_selector = color_selector;

  g_signal_connect_swapped (GTK_OBJECT (color_selector->ok_button),
                            "clicked",
                            G_CALLBACK (gtk_widget_destroy),
                            (gpointer) color_selector);

  g_signal_connect_swapped (GTK_OBJECT (color_selector->cancel_button),
                            "clicked",
                            G_CALLBACK (gtk_widget_destroy),
                            (gpointer) color_selector);

  gtk_widget_show((GtkWidget*)color_selector);
  return TRUE;
}


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

  int font_size = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcin_font_size));
  save_gcin_conf_int(GCIN_FONT_SIZE, font_size);

#if GTK_24
  char fname[128];
  strcpy(fname, gtk_font_button_get_font_name(GTK_FONT_BUTTON(font_sel)));
  int len = strlen(fname)-1;

  while (len > 0 && isdigit(fname[len])) {
       fname[len--]=0;
  }

  while (len > 0 && fname[len]==' ') {
       fname[len--]=0;
  }

  save_gcin_conf_str(GCIN_FONT_NAME, fname);
#endif

  int font_size_tsin_presel = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcin_font_size_tsin_presel));
  save_gcin_conf_int(GCIN_FONT_SIZE_TSIN_PRESEL, font_size_tsin_presel);

  int font_size_symbol = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcin_font_size_symbol));
  save_gcin_conf_int(GCIN_FONT_SIZE_SYMBOL, font_size_symbol);

  int font_size_tsin_pho_in = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcin_font_size_tsin_pho_in));
  save_gcin_conf_int(GCIN_FONT_SIZE_TSIN_PHO_IN, font_size_tsin_pho_in);

  int font_size_pho_near = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcin_font_size_pho_near));
  save_gcin_conf_int(GCIN_FONT_SIZE_PHO_NEAR, font_size_pho_near);

  int font_size_gtab_in = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcin_font_size_gtab_in));
  save_gcin_conf_int(GCIN_FONT_SIZE_GTAB_IN, font_size_gtab_in);

  int gcin_pop_up_win = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_pop_up_win));
  save_gcin_conf_int(GCIN_POP_UP_WIN, gcin_pop_up_win);

  int gcin_root_x = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_root_style_x));
  save_gcin_conf_int(GCIN_ROOT_X, gcin_root_x);

  int gcin_root_y = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_root_style_y));
  save_gcin_conf_int(GCIN_ROOT_Y, gcin_root_y);

  int style = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_root_style_use)) ?
            InputStyleRoot : InputStyleOverSpot;
  save_gcin_conf_int(GCIN_INPUT_STYLE, style);

  save_gcin_conf_int(GCIN_INNER_FRAME, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_inner_frame)));
#if TRAY_ENABLED
  save_gcin_conf_int(GCIN_STATUS_TRAY, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_status_tray)));
#endif

  cstr = gtk_color_selection_palette_to_string(&gcin_win_gcolor_fg, 1);
  dbg("color fg %s\n", cstr);
  save_gcin_conf_str(GCIN_WIN_COLOR_FG, cstr);
  g_free(cstr);

  cstr = gtk_color_selection_palette_to_string(&gcin_win_gcolor_bg, 1);
  dbg("color bg %s\n", cstr);
  save_gcin_conf_str(GCIN_WIN_COLOR_BG, cstr);
  g_free(cstr);

  save_gcin_conf_int(GCIN_WIN_COLOR_USE, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_win_color_use)));


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

  save_gcin_conf_int(GTAB_DISP_KEY_CODES,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_disp_key_codes)));

  save_gcin_conf_int(GTAB_DISP_IM_NAME,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_disp_im_name)));

  save_gcin_conf_int(GTAB_INVALID_KEY_IN,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_invalid_key_in)));

  save_gcin_conf_int(GTAB_SHIFT_PHRASE_KEY,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_shift_phrase_key)));

  save_gcin_conf_int(GTAB_HIDE_ROW2,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_hide_row2)));

  save_gcin_conf_int(GTAB_IN_ROW1,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_in_row1)));

  save_gcin_conf_int(GTAB_CAPSLOCK_IN_ENG,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_capslock_in_eng)));


  save_gcin_conf_int(GTAB_VERTICAL_SELECT,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_vertical_select)));


  save_gcin_conf_int(GTAB_UNIQUE_AUTO_SEND,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_unique_auto_send)));

  save_gcin_conf_int(GTAB_QUE_WILD_CARD,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gtab_que_wild_card)));

  idx = gtk_option_menu_get_history (GTK_OPTION_MENU (opt_spc_opts));
  save_gcin_conf_int(GTAB_SPACE_AUTO_FIRST, spc_opts[idx].num);

  send_gcin_message(GDK_DISPLAY(), CHANGE_FONT_SIZE);


  save_gcin_conf_int(DEFAULT_INPUT_METHOD, default_input_method);

  idx = gtk_option_menu_get_history (GTK_OPTION_MENU (opt_im_toggle_keys));
  save_gcin_conf_int(GCIN_IM_TOGGLE_KEYS, imkeys[idx].keynum);
  save_gcin_conf_int(GCIN_FLAGS_IM_ENABLED, gcin_flags_im_enabled);
  save_gcin_conf_int(GCIN_REMOTE_CLIENT,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_remote_client)));
  save_gcin_conf_int(GCIN_SHIFT_SPACE_ENG_FULL,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_shift_space_eng_full)));

  save_gcin_conf_int(GCIN_INIT_IM_ENABLED,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_init_im_enabled)));

  save_gcin_conf_int(GCIN_ENG_PHRASE_ENABLED,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_eng_phrase_enabled)));

  if (opt_speaker_opts) {
    idx = gtk_option_menu_get_history (GTK_OPTION_MENU (opt_speaker_opts));
    save_gcin_conf_str(PHONETIC_SPEAK_SEL, pho_speaker[idx]);
  }

  idx = gtk_option_menu_get_history (GTK_OPTION_MENU (opt_gcb_pos));
  save_gcin_conf_int(GCB_POSITION, idx);
  int pos_x = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcb_position_x));
  save_gcin_conf_int(GCB_POSITION_X, pos_x);
  int pos_y = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcb_position_y));
  save_gcin_conf_int(GCB_POSITION_Y, pos_y);

  save_gcin_conf_int(PHONETIC_SPEAK,
     gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_phonetic_speak)));

  save_gcin_conf_int(GCIN_WIN_SYM_CLICK_CLOSE,
     gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_win_sym_click_close)));

  cstr = gtk_color_selection_palette_to_string(&gcin_sel_key_gcolor, 1);
  dbg("selkey color %s\n", cstr);
  save_gcin_conf_str(GCIN_SEL_KEY_COLOR, cstr);
  g_free(cstr);


  send_gcin_message(GDK_DISPLAY(), "reload");

  exit(0);
}


void create_about_window();


static void create_main_win()
{
  GtkWidget *notebook = gtk_notebook_new();
  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 0);
  GtkWidget *vbox;
  GtkWidget *notebooklabel;

  main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  g_signal_connect (G_OBJECT (main_window), "delete_event",
                    G_CALLBACK (close_application),
                    NULL);

  gtk_container_add(GTK_CONTAINER(main_window), vbox_top);
  gtk_container_add(GTK_CONTAINER(vbox_top), notebook);

  GtkWidget *hbox_help_close = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_help_close, FALSE, FALSE, 0);

  GtkWidget *button_help = gtk_button_new_from_stock (GTK_STOCK_HELP);
  gtk_box_pack_start (GTK_BOX (hbox_help_close), button_help, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_help), "clicked",
                            G_CALLBACK (cb_help), NULL);

  GtkWidget *button_close = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  gtk_box_pack_start (GTK_BOX (hbox_help_close), button_close, TRUE, TRUE, 0);
  g_signal_connect_swapped (G_OBJECT (button_close), "clicked",
                            G_CALLBACK (cb_ok), NULL);

/* label 1 */

  vbox = gtk_vbox_new(FALSE, 0);
  notebooklabel = gtk_label_new(_("雜項"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, notebooklabel);

  GtkWidget *button_alt_shift = gtk_button_new_with_label(_("alt-shift 片語編輯"));
  gtk_box_pack_start (GTK_BOX (vbox), button_alt_shift, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_alt_shift), "clicked",
                    G_CALLBACK (cb_alt_shift), NULL);

  GtkWidget *button_ctrl = gtk_button_new_with_label(_("ctrl 片語編輯"));
  gtk_box_pack_start (GTK_BOX (vbox), button_ctrl, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_ctrl), "clicked",
                    G_CALLBACK (cb_ctrl), NULL);

  GtkWidget *button_symbol_table = gtk_button_new_with_label(_("符號表編輯"));
  gtk_box_pack_start (GTK_BOX (vbox), button_symbol_table, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_symbol_table), "clicked",
                    G_CALLBACK (cb_symbol_table), NULL);

  GtkWidget *button_gb_output_toggle = gtk_button_new_with_label(_("簡體字輸出切換"));
  gtk_box_pack_start (GTK_BOX (vbox), button_gb_output_toggle, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_gb_output_toggle), "clicked",
                    G_CALLBACK (cb_gb_output_toggle), NULL);

  GtkWidget *button_gb_translate_toggle = gtk_button_new_with_label(_("剪貼區 簡體字->正體字"));
  gtk_box_pack_start (GTK_BOX (vbox), button_gb_translate_toggle, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_gb_translate_toggle), "clicked",
                    G_CALLBACK (cb_gb_translate_toggle), NULL);

  GtkWidget *button_big5_translate_toggle = gtk_button_new_with_label(_("剪貼區 正體字->簡體字"));
  gtk_box_pack_start (GTK_BOX (vbox), button_big5_translate_toggle, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_big5_translate_toggle), "clicked",
                    G_CALLBACK (cb_big5_translate_toggle), NULL);

  GtkWidget *button_juying_learn_toggle = gtk_button_new_with_label(_("剪貼區 注音查詢"));
  gtk_box_pack_start (GTK_BOX (vbox), button_juying_learn_toggle, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_juying_learn_toggle), "clicked",
                    G_CALLBACK (cb_juying_learn), NULL);

  GtkWidget *button_ts_export = gtk_button_new_with_label(_("詞庫匯出"));
  gtk_box_pack_start (GTK_BOX (vbox), button_ts_export, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_ts_export), "clicked",
                    G_CALLBACK (cb_ts_export), NULL);

  GtkWidget *button_ts_import = gtk_button_new_with_label(_("詞庫匯入"));
  gtk_box_pack_start (GTK_BOX (vbox), button_ts_import, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_ts_import), "clicked",
                    G_CALLBACK (cb_ts_import), NULL);

  GtkWidget *button_ts_edit = gtk_button_new_with_label(_("詞庫編輯"));
  gtk_box_pack_start (GTK_BOX (vbox), button_ts_edit, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_ts_edit), "clicked",
                    G_CALLBACK (cb_ts_edit), NULL);

  GtkWidget *button_tslearn = gtk_button_new_with_label(_("讓詞音從文章學習詞"));
  gtk_box_pack_start (GTK_BOX (vbox), button_tslearn, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_tslearn), "clicked",
                    G_CALLBACK (cb_tslearn), NULL);

  GtkWidget *button_ts_import_sys = gtk_button_new_with_label(_("匯入系統的詞庫"));
  gtk_box_pack_start (GTK_BOX (vbox), button_ts_import_sys, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_ts_import_sys), "clicked",
                    G_CALLBACK (cb_ts_import_sys), NULL);

  GtkWidget *button_about = gtk_button_new_with_label(_("關於 gcin"));
  gtk_box_pack_start (GTK_BOX (vbox), button_about, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_about), "clicked",
                    G_CALLBACK (create_about_window),  NULL);

  char *pid = getenv("GCIN_PID");

  if (pid && (gcin_pid = atoi(pid))) {
    GtkWidget *button_gcin_exit = gtk_button_new_with_label (_("結束 gcin"));
    gtk_box_pack_start (GTK_BOX (vbox), button_gcin_exit, TRUE, TRUE, 0);
    g_signal_connect (G_OBJECT (button_gcin_exit), "clicked",
                      G_CALLBACK (cb_gcin_exit), NULL);
  }

/* label 2 */

  vbox = gtk_vbox_new(FALSE, 0);
  notebooklabel = gtk_label_new(_("通用"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, notebooklabel);

  GtkWidget *hbox_gcin_font_size = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gcin_font_size, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcin_font_size =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_gcin_font_size = gtk_spin_button_new (adj_gcin_font_size, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size), spinner_gcin_font_size, FALSE, FALSE, 0);
  GtkWidget *label_gcin_font_size = gtk_label_new(_("字型大小"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size), label_gcin_font_size, FALSE, FALSE, 0);

  GtkWidget *hbox_gcin_font_size_symbol = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gcin_font_size_symbol, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcin_font_size_symbol =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size_symbol, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_gcin_font_size_symbol = gtk_spin_button_new (adj_gcin_font_size_symbol, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_symbol), spinner_gcin_font_size_symbol, FALSE, FALSE, 0);
  GtkWidget *label_gcin_font_size_symbol = gtk_label_new(_("符號選擇視窗字型大小"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_symbol), label_gcin_font_size_symbol, FALSE, FALSE, 0);

  GtkWidget *hbox_gcin_font_size_tsin_presel = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gcin_font_size_tsin_presel, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcin_font_size_tsin_presel =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size_tsin_presel, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_gcin_font_size_tsin_presel = gtk_spin_button_new (adj_gcin_font_size_tsin_presel, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_tsin_presel), spinner_gcin_font_size_tsin_presel, FALSE, FALSE, 0);
  GtkWidget *label_gcin_font_size_tsin_presel = gtk_label_new(_("詞音預選詞視窗字型大小"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_tsin_presel), label_gcin_font_size_tsin_presel, FALSE, FALSE, 0);

  GtkWidget *hbox_gcin_font_size_tsin_pho_in = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gcin_font_size_tsin_pho_in, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcin_font_size_tsin_pho_in =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size_tsin_pho_in, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_gcin_font_size_tsin_pho_in = gtk_spin_button_new (adj_gcin_font_size_tsin_pho_in, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_tsin_pho_in), spinner_gcin_font_size_tsin_pho_in, FALSE, FALSE, 0);
  GtkWidget *label_gcin_font_size_tsin_pho_in = gtk_label_new(_("詞音注音輸入區字型大小"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_tsin_pho_in), label_gcin_font_size_tsin_pho_in, FALSE, FALSE, 0);

  GtkWidget *hbox_gcin_font_size_pho_near = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gcin_font_size_pho_near, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcin_font_size_pho_near =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size_pho_near, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_gcin_font_size_pho_near = gtk_spin_button_new (adj_gcin_font_size_pho_near, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_pho_near), spinner_gcin_font_size_pho_near, FALSE, FALSE, 0);
  GtkWidget *label_gcin_font_size_pho_near = gtk_label_new(_("詞音近似音顯示字型大小"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_pho_near), label_gcin_font_size_pho_near, FALSE, FALSE, 0);

  GtkWidget *hbox_gcin_font_size_gtab_in = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gcin_font_size_gtab_in, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcin_font_size_gtab_in =
   (GtkAdjustment *) gtk_adjustment_new (gcin_font_size_gtab_in, 8.0, 32.0, 1.0, 1.0, 0.0);
  spinner_gcin_font_size_gtab_in = gtk_spin_button_new (adj_gcin_font_size_gtab_in, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_gtab_in), spinner_gcin_font_size_gtab_in, FALSE, FALSE, 0);
  GtkWidget *label_gcin_font_size_gtab_in = gtk_label_new(_("gtab(倉頡…)輸入區字型大小"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_font_size_gtab_in), label_gcin_font_size_gtab_in, FALSE, FALSE, 0);

#if GTK_24
  char tt[128];
  sprintf(tt, "%s %d", gcin_font_name, gcin_font_size);
  font_sel = gtk_font_button_new_with_font (tt);
  gtk_box_pack_start (GTK_BOX (vbox), font_sel, FALSE, FALSE, 0);
#endif

  GtkWidget *hbox_gcin_pop_up_win = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX(vbox), hbox_gcin_pop_up_win, FALSE, FALSE, 0);
  check_button_gcin_pop_up_win = gtk_check_button_new ();
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_pop_up_win),
       gcin_pop_up_win);
  gtk_box_pack_start (GTK_BOX(hbox_gcin_pop_up_win), check_button_gcin_pop_up_win, FALSE, FALSE, 0);
  GtkWidget *label_gcin_pop_up_win = gtk_label_new(_("彈出式輸入視窗"));
  gtk_box_pack_start (GTK_BOX(hbox_gcin_pop_up_win), label_gcin_pop_up_win, FALSE, FALSE, 0);

  GtkWidget *hbox_root_style_use = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX(vbox), hbox_root_style_use, FALSE, FALSE, 0);
  check_button_root_style_use = gtk_check_button_new ();
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_root_style_use),
       gcin_input_style == InputStyleRoot);
  gtk_box_pack_start (GTK_BOX(hbox_root_style_use), check_button_root_style_use, FALSE, FALSE, 0);
  GtkWidget *label_root_style_use = gtk_label_new(_("固定 gcin 視窗位置"));
  gtk_box_pack_start (GTK_BOX(hbox_root_style_use), label_root_style_use, FALSE, FALSE, 0);

  GtkAdjustment *adj_root_style_x =
   (GtkAdjustment *) gtk_adjustment_new (gcin_root_x, 0.0, 1600.0, 1.0, 1.0, 0.0);
  spinner_root_style_x = gtk_spin_button_new (adj_root_style_x, 0, 0);
  gtk_container_add (GTK_CONTAINER (hbox_root_style_use), spinner_root_style_x);

  GtkAdjustment *adj_root_style_y =
   (GtkAdjustment *) gtk_adjustment_new (gcin_root_y, 0.0, 1200.0, 1.0, 1.0, 0.0);
  spinner_root_style_y = gtk_spin_button_new (adj_root_style_y, 0, 0);
  gtk_container_add (GTK_CONTAINER (hbox_root_style_use), spinner_root_style_y);

  GtkWidget *hbox_gcin_inner_frame = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX(vbox), hbox_gcin_inner_frame, FALSE, FALSE, 0);
  check_button_gcin_inner_frame = gtk_check_button_new ();
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_inner_frame),
       gcin_inner_frame);
  gtk_box_pack_start (GTK_BOX(hbox_gcin_inner_frame), check_button_gcin_inner_frame, FALSE, FALSE, 0);
  GtkWidget *label_gcin_inner_frame = gtk_label_new(_("顯示內框"));
  gtk_box_pack_start (GTK_BOX(hbox_gcin_inner_frame), label_gcin_inner_frame, FALSE, FALSE, 0);

#if TRAY_ENABLED
  GtkWidget *hbox_gcin_status_tray = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX(vbox), hbox_gcin_status_tray, FALSE, FALSE, 0);
  check_button_gcin_status_tray = gtk_check_button_new ();
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_status_tray),
       gcin_status_tray);
  gtk_box_pack_start (GTK_BOX(hbox_gcin_status_tray), check_button_gcin_status_tray, FALSE, FALSE, 0);
  GtkWidget *label_gcin_status_tray = gtk_label_new(_("面板狀態(tray)"));
  gtk_box_pack_start (GTK_BOX(hbox_gcin_status_tray), label_gcin_status_tray, FALSE, FALSE, 0);
#endif

  GtkWidget *hbox_win_color_fbg = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX(vbox), hbox_win_color_fbg, FALSE, FALSE, 0);

  check_button_gcin_win_color_use = gtk_check_button_new ();
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_win_color_use),
       gcin_win_color_use);
  gtk_box_pack_start (GTK_BOX(hbox_win_color_fbg), check_button_gcin_win_color_use, FALSE, FALSE, 0);
  GtkWidget *label_win_color_use = gtk_label_new(_("顏色選擇"));
  gtk_box_pack_start (GTK_BOX(hbox_win_color_fbg), label_win_color_use, FALSE, FALSE, 0);

  GtkWidget *button_fg = gtk_button_new_with_label(_("前景顏色"));
  gtk_box_pack_start (GTK_BOX(hbox_win_color_fbg), button_fg, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_fg), "clicked",
                    G_CALLBACK (cb_gcin_win_color_fg), &colorsel[0]);

  GtkWidget *button_bg = gtk_button_new_with_label(_("背景顏色"));
  gtk_box_pack_start (GTK_BOX(hbox_win_color_fbg), button_bg, TRUE, TRUE, 0);
  g_signal_connect (G_OBJECT (button_bg), "clicked",
                    G_CALLBACK (cb_gcin_win_color_fg), &colorsel[1]);

  event_box_win_color_test = gtk_event_box_new();
  gtk_box_pack_start (GTK_BOX(vbox), event_box_win_color_test, FALSE, FALSE, 0);
  label_win_color_test = gtk_label_new(_("測試目前狀態"));
  gtk_container_add (GTK_CONTAINER(event_box_win_color_test), label_win_color_test);

  GdkColor color;
  gdk_color_parse(gcin_win_color_fg, &gcin_win_gcolor_fg);
  gtk_widget_modify_fg(label_win_color_test, GTK_STATE_NORMAL, &gcin_win_gcolor_fg);
  gdk_color_parse(gcin_win_color_bg, &gcin_win_gcolor_bg);
  gtk_widget_modify_bg(event_box_win_color_test, GTK_STATE_NORMAL, &gcin_win_gcolor_bg);

  GtkWidget *hbox_gcin_sel_key_color = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gcin_sel_key_color , FALSE, FALSE, 0);
  GtkWidget *label_gcin_sel_key_color = gtk_label_new(_("選擇鍵的顏色"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_sel_key_color), label_gcin_sel_key_color , FALSE, FALSE, 0);
  GtkWidget *button_gcin_sel_key_color = gtk_button_new();
  g_signal_connect (G_OBJECT (button_gcin_sel_key_color), "clicked",
                    G_CALLBACK (cb_gcin_sel_key_color), G_OBJECT (main_window));
  da_sel_key =  gtk_drawing_area_new();
  gtk_container_add (GTK_CONTAINER (button_gcin_sel_key_color), da_sel_key);
  gdk_color_parse(gcin_sel_key_color, &gcin_sel_key_gcolor);
  gtk_widget_modify_bg(da_sel_key, GTK_STATE_NORMAL, &gcin_sel_key_gcolor);
  gtk_widget_set_size_request(da_sel_key, 16, 2);
  gtk_container_add (GTK_CONTAINER (hbox_gcin_sel_key_color), button_gcin_sel_key_color);

  gtk_box_pack_start (GTK_BOX (vbox), create_im_toggle_keys(), FALSE, FALSE, 0);

  GtkWidget *hbox_gcin_remote_client = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gcin_remote_client, FALSE, FALSE, 0);
  check_button_gcin_remote_client = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcin_remote_client),check_button_gcin_remote_client,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_remote_client),
     gcin_remote_client);
  GtkWidget *label_gcin_remote_client = gtk_label_new(_("遠端 client 程式支援 (port 9999-)"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_remote_client), label_gcin_remote_client,  FALSE, FALSE, 0);

  GtkWidget *hbox_gcin_init_im_enabled = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gcin_init_im_enabled, FALSE, FALSE, 0);
  check_button_gcin_init_im_enabled = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcin_init_im_enabled),check_button_gcin_init_im_enabled,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_init_im_enabled),
     gcin_init_im_enabled);
  GtkWidget *label_gcin_init_im_enabled = gtk_label_new(_("直接進入中文輸入狀態(限非XIM)"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_init_im_enabled), label_gcin_init_im_enabled, FALSE, FALSE, 0);

  GtkWidget *hbox_gcin_shift_space_eng_full = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gcin_shift_space_eng_full, FALSE, FALSE, 0);
  check_button_gcin_shift_space_eng_full = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcin_shift_space_eng_full),check_button_gcin_shift_space_eng_full,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_shift_space_eng_full),
     gcin_shift_space_eng_full);
  GtkWidget *label_gcin_shift_space_eng_full = gtk_label_new(_("shift-space 進入全形英文模式"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_shift_space_eng_full), label_gcin_shift_space_eng_full,  FALSE, FALSE, 0);

  GtkWidget *hbox_gcin_eng_phrase_enabled = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gcin_eng_phrase_enabled, FALSE, FALSE, 0);
  check_button_gcin_eng_phrase_enabled = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcin_eng_phrase_enabled),check_button_gcin_eng_phrase_enabled,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_eng_phrase_enabled),
     gcin_eng_phrase_enabled);
  GtkWidget *label_gcin_eng_phrase_enabled = gtk_label_new(_("英數狀態使用 alt-shift 片語"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_eng_phrase_enabled), label_gcin_eng_phrase_enabled,  FALSE, FALSE, 0);

  GtkWidget *hbox_gcin_capslock_lower = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gcin_capslock_lower , FALSE, FALSE, 0);
  check_button_gcin_capslock_lower = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcin_capslock_lower), check_button_gcin_capslock_lower, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_gcin_capslock_lower), gcin_capslock_lower);
  GtkWidget *label_gcin_capslock_lower = gtk_label_new(_("Capslock英數用小寫"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_capslock_lower), label_gcin_capslock_lower , FALSE, FALSE, 0);

  GtkWidget *hbox_phonetic_speak = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_phonetic_speak , FALSE, FALSE, 0);
  check_button_phonetic_speak = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_phonetic_speak), check_button_phonetic_speak, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_phonetic_speak), phonetic_speak);
  GtkWidget *label_phonetic_speak = gtk_label_new(_("輸入時念出發音"));
  gtk_box_pack_start (GTK_BOX (hbox_phonetic_speak), label_phonetic_speak , FALSE, FALSE, 0);

  DIR *dir;
  if (dir=opendir(GCIN_OGG_DIR"/ㄧ")) {
    struct dirent *dire;

    pho_speakerN = 0;
    while (dire=readdir(dir)) {
      char *name = dire->d_name;

      if (name[0]=='.')
        continue;
      pho_speaker[pho_speakerN++]=strdup(name);
    }
    closedir(dir);

    dbg("pho_speakerN:%d\n", pho_speakerN);

    if (pho_speakerN) {
      GtkWidget *labelspeaker = gtk_label_new(_("，發音選擇"));
      gtk_box_pack_start (GTK_BOX (hbox_phonetic_speak), labelspeaker, FALSE, FALSE, 0);
      gtk_container_add (GTK_CONTAINER (hbox_phonetic_speak), create_speaker_opts());
    }
  }

  GtkWidget *hbox_gcin_win_sym_click_close = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gcin_win_sym_click_close, FALSE, FALSE, 0);
  check_button_gcin_win_sym_click_close = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcin_win_sym_click_close),check_button_gcin_win_sym_click_close,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_win_sym_click_close),
     gcin_win_sym_click_close);
  GtkWidget *label_gcin_win_sym_click_close = gtk_label_new(_("符號視窗點選後自動關閉"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_win_sym_click_close), label_gcin_win_sym_click_close,  FALSE, FALSE, 0);

  GtkWidget *hbox_gcb_pos = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gcb_pos, FALSE, FALSE, 0);
  GtkWidget *label_gcb_pos = gtk_label_new(_("剪貼區管理視窗位置&開關"));
  gtk_box_pack_start (GTK_BOX (hbox_gcb_pos), label_gcb_pos,  FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcb_pos), create_gcb_pos_opts(),  FALSE, FALSE, 0);
  GtkAdjustment *adj_gcb_position_x =
   (GtkAdjustment *) gtk_adjustment_new (gcb_position_x, 0.0, 100.0, 1.0, 1.0, 0.0);
  spinner_gcb_position_x = gtk_spin_button_new (adj_gcb_position_x, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcb_pos), spinner_gcb_position_x, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcb_position_y =
   (GtkAdjustment *) gtk_adjustment_new (gcb_position_y, 0.0, 100.0, 1.0, 1.0, 0.0);
  spinner_gcb_position_y = gtk_spin_button_new (adj_gcb_position_y, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcb_pos), spinner_gcb_position_y, FALSE, FALSE, 0);

/* label 3 */

  vbox = gtk_vbox_new(FALSE, 0);
  notebooklabel = gtk_label_new(_("注音/詞音"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, notebooklabel);

  GtkWidget *hbox_kbm_sw = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_kbm_sw, FALSE, FALSE, 0);
  GtkWidget *label_kbm_sw = gtk_label_new(_("鍵盤排列方式"));
  gtk_box_pack_start (GTK_BOX (hbox_kbm_sw), label_kbm_sw, FALSE, FALSE, 0);
  gtk_container_add (GTK_CONTAINER (hbox_kbm_sw), create_kbm_opts());

  GtkWidget *hbox_tsin_sw = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_tsin_sw, FALSE, FALSE, 0);
  GtkWidget *label_tsin_sw = gtk_label_new(_("詞音輸入[中/英]切換"));
  gtk_box_pack_start (GTK_BOX (hbox_tsin_sw), label_tsin_sw,  FALSE, FALSE, 0);
  gtk_container_add (GTK_CONTAINER (hbox_tsin_sw), create_eng_ch_opts());

  GtkWidget *frame_tsin_space_opt = gtk_frame_new(_("詞音輸入空白鍵選項"));
  gtk_box_pack_start (GTK_BOX (vbox), frame_tsin_space_opt, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame_tsin_space_opt), 0);

  GtkWidget *box_tsin_space_opt = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame_tsin_space_opt), box_tsin_space_opt);
  gtk_container_set_border_width (GTK_CONTAINER (box_tsin_space_opt), 0);

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
  gtk_box_pack_start (GTK_BOX (vbox), hbox_tsin_phrase_pre_select , FALSE, FALSE, 0);
  check_button_tsin_phrase_pre_select = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_tsin_phrase_pre_select), check_button_tsin_phrase_pre_select, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_tsin_phrase_pre_select), tsin_phrase_pre_select);
  GtkWidget *label_tsin_phrase_pre_select = gtk_label_new(_("詞音輸入預選詞視窗"));
  gtk_box_pack_start (GTK_BOX (hbox_tsin_phrase_pre_select), label_tsin_phrase_pre_select , FALSE, FALSE, 0);

  GtkWidget *hbox_phonetic_char_dynamic_sequence = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_phonetic_char_dynamic_sequence , FALSE, FALSE, 0);
  check_button_phonetic_char_dynamic_sequence = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_phonetic_char_dynamic_sequence), check_button_phonetic_char_dynamic_sequence, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_phonetic_char_dynamic_sequence), phonetic_char_dynamic_sequence);
  GtkWidget *label_phonetic_char_dynamic_sequence = gtk_label_new(_("依使用頻率調整字的順序"));
  gtk_box_pack_start (GTK_BOX (hbox_phonetic_char_dynamic_sequence), label_phonetic_char_dynamic_sequence , FALSE, FALSE, 0);

  GtkWidget *hbox_pho_hide_row2 = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_pho_hide_row2 , FALSE, FALSE, 0);
  check_button_pho_hide_row2 = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_pho_hide_row2), check_button_pho_hide_row2, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_pho_hide_row2), pho_hide_row2);
  GtkWidget *label_pho_hide_row2 = gtk_label_new(_("注音隱藏第二列(按鍵顯示)"));
  gtk_box_pack_start (GTK_BOX (hbox_pho_hide_row2), label_pho_hide_row2 , FALSE, FALSE, 0);

  GtkWidget *hbox_pho_in_row1 = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_pho_in_row1 , FALSE, FALSE, 0);
  check_button_pho_in_row1 = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_pho_in_row1), check_button_pho_in_row1, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_pho_in_row1), pho_in_row1);
  GtkWidget *label_pho_in_row1 = gtk_label_new(_("注音輸入顯示移至第一列"));
  gtk_box_pack_start (GTK_BOX (hbox_pho_in_row1), label_pho_in_row1 , FALSE, FALSE, 0);

  GtkWidget *hbox_phonetic_huge_tab = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_phonetic_huge_tab , FALSE, FALSE, 0);
  check_button_phonetic_huge_tab = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_phonetic_huge_tab), check_button_phonetic_huge_tab, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_phonetic_huge_tab), phonetic_huge_tab);
  GtkWidget *label_phonetic_huge_tab = gtk_label_new(_("使用巨大 UTF-8 字集"));
  gtk_box_pack_start (GTK_BOX (hbox_phonetic_huge_tab), label_phonetic_huge_tab , FALSE, FALSE, 0);

  GtkWidget *hbox_tsin_tone_char_input = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_tsin_tone_char_input , FALSE, FALSE, 0);
  check_button_tsin_tone_char_input = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_tsin_tone_char_input), check_button_tsin_tone_char_input, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_tsin_tone_char_input), tsin_tone_char_input);
  GtkWidget *label_tsin_tone_char_input = gtk_label_new(_("詞音輸入注音聲調符號"));
  gtk_box_pack_start (GTK_BOX (hbox_tsin_tone_char_input), label_tsin_tone_char_input , FALSE, FALSE, 0);

  GtkWidget *hbox_tsin_tab_phrase_end = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_tsin_tab_phrase_end , FALSE, FALSE, 0);
  check_button_tsin_tab_phrase_end = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_tsin_tab_phrase_end), check_button_tsin_tab_phrase_end, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_tsin_tab_phrase_end), tsin_tab_phrase_end);
  GtkWidget *label_tsin_tab_phrase_end = gtk_label_new(_("詞音/Escape Tab 斷詞"));
  gtk_box_pack_start (GTK_BOX (hbox_tsin_tab_phrase_end), label_tsin_tab_phrase_end , FALSE, FALSE, 0);

  GtkWidget *hbox_tsin_tail_select_key = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_tsin_tail_select_key , FALSE, FALSE, 0);
  check_button_tsin_tail_select_key = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_tsin_tail_select_key), check_button_tsin_tail_select_key, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_tsin_tail_select_key), tsin_tail_select_key);
  GtkWidget *label_tsin_tail_select_key = gtk_label_new(_("同音字詞選擇按鍵在後"));
  gtk_box_pack_start (GTK_BOX (hbox_tsin_tail_select_key), label_tsin_tail_select_key , FALSE, FALSE, 0);

  GtkWidget *hbox_tsin_buffer_editing_mode = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_tsin_buffer_editing_mode , FALSE, FALSE, 0);
  check_button_tsin_buffer_editing_mode = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_tsin_buffer_editing_mode), check_button_tsin_buffer_editing_mode, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_tsin_buffer_editing_mode), tsin_buffer_editing_mode);
  GtkWidget *label_tsin_buffer_editing_mode = gtk_label_new(_("啟用詞音緩衝區編輯模式"));
  gtk_box_pack_start (GTK_BOX (hbox_tsin_buffer_editing_mode), label_tsin_buffer_editing_mode, FALSE, FALSE, 0);

  GtkWidget *hbox_tsin_buffer_size = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_tsin_buffer_size , FALSE, FALSE, 0);
  GtkWidget *label_tsin_buffer_size = gtk_label_new(_("詞音編輯緩衝區大小"));
  gtk_box_pack_start (GTK_BOX (hbox_tsin_buffer_size), label_tsin_buffer_size , FALSE, FALSE, 0);
  GtkAdjustment *adj_gtab_in =
   (GtkAdjustment *) gtk_adjustment_new (tsin_buffer_size, 10.0, MAX_PH_BF, 1.0, 1.0, 0.0);
  spinner_tsin_buffer_size = gtk_spin_button_new (adj_gtab_in, 0, 0);
  gtk_container_add (GTK_CONTAINER (hbox_tsin_buffer_size), spinner_tsin_buffer_size);

  GtkWidget *hbox_tsin_phrase_line_color = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_tsin_phrase_line_color , FALSE, FALSE, 0);
  GtkWidget *label_tsin_phrase_line_color = gtk_label_new(_("詞音標示詞的底線顏色"));
  gtk_box_pack_start (GTK_BOX (hbox_tsin_phrase_line_color), label_tsin_phrase_line_color , FALSE, FALSE, 0);
  GtkWidget *button_tsin_phrase_line_color = gtk_button_new();
  g_signal_connect (G_OBJECT (button_tsin_phrase_line_color), "clicked",
                    G_CALLBACK (cb_tsin_phrase_line_color), G_OBJECT (main_window));
  da_phrase_line =  gtk_drawing_area_new();
  gtk_container_add (GTK_CONTAINER (button_tsin_phrase_line_color), da_phrase_line);
  gdk_color_parse(tsin_phrase_line_color, &tsin_phrase_line_gcolor);
  gtk_widget_modify_bg(da_phrase_line, GTK_STATE_NORMAL, &tsin_phrase_line_gcolor);
  gtk_widget_set_size_request(da_phrase_line, 16, 2);
  gtk_container_add (GTK_CONTAINER (hbox_tsin_phrase_line_color), button_tsin_phrase_line_color);

  GtkWidget *hbox_tsin_cursor_color = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_tsin_cursor_color , FALSE, FALSE, 5);
  GtkWidget *label_tsin_cursor_color = gtk_label_new(_("詞音游標的顏色"));
  gtk_box_pack_start (GTK_BOX (hbox_tsin_cursor_color), label_tsin_cursor_color , FALSE, FALSE, 0);
  GtkWidget *button_tsin_cursor_color = gtk_button_new();
  g_signal_connect (G_OBJECT (button_tsin_cursor_color), "clicked",
                    G_CALLBACK (cb_tsin_cursor_color), G_OBJECT (main_window));
  da_cursor =  gtk_drawing_area_new();
  gtk_container_add (GTK_CONTAINER (button_tsin_cursor_color), da_cursor);
  gdk_color_parse(tsin_cursor_color, &tsin_cursor_gcolor);
  gtk_widget_modify_bg(da_cursor, GTK_STATE_NORMAL, &tsin_cursor_gcolor);
  gtk_widget_set_size_request(da_cursor, 16, 2);
  gtk_container_add (GTK_CONTAINER (hbox_tsin_cursor_color), button_tsin_cursor_color);

/* label 4 */

  vbox = gtk_vbox_new(FALSE, 0);
  notebooklabel = gtk_label_new(_("倉頡/行列"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, notebooklabel);

  GtkWidget *hbox_gtab_pre_select = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gtab_pre_select, FALSE, FALSE, 0);
  check_button_gtab_pre_select = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gtab_pre_select),check_button_gtab_pre_select,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_pre_select),
     gtab_pre_select);
  GtkWidget *label_gtab_pre_select = gtk_label_new(_("預覽/預選 字"));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_pre_select), label_gtab_pre_select,  FALSE, FALSE, 0);

  GtkWidget *hbox_gtab_disp_partial_match = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gtab_disp_partial_match, FALSE, FALSE, 0);
  check_button_gtab_disp_partial_match = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gtab_disp_partial_match), check_button_gtab_disp_partial_match,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_disp_partial_match),
     gtab_disp_partial_match);
  GtkWidget *label_gtab_gtab_disp_partial_match = gtk_label_new(_("預選列中顯示部份符合的字"));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_disp_partial_match), label_gtab_gtab_disp_partial_match,  FALSE, FALSE, 0);

  GtkWidget *hbox_gtab_disp_key_codes = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gtab_disp_key_codes, FALSE, FALSE, 0);
  check_button_gtab_disp_key_codes = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gtab_disp_key_codes), check_button_gtab_disp_key_codes,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_disp_key_codes),
     gtab_disp_key_codes);
  GtkWidget *label_gtab_gtab_disp_key_codes = gtk_label_new(_("顯示拆字"));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_disp_key_codes), label_gtab_gtab_disp_key_codes,  FALSE, FALSE, 0);

  GtkWidget *hbox_gtab_disp_im_name = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gtab_disp_im_name, FALSE, FALSE, 0);
  check_button_gtab_disp_im_name = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gtab_disp_im_name), check_button_gtab_disp_im_name,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_disp_im_name),
     gtab_disp_im_name);
  GtkWidget *label_gtab_gtab_disp_im_name = gtk_label_new(_("顯示輸入法名稱"));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_disp_im_name), label_gtab_gtab_disp_im_name,  FALSE, FALSE, 0);

  GtkWidget *hbox_gtab_hide_row2 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gtab_hide_row2, FALSE, FALSE, 0);
  check_button_gtab_hide_row2 = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gtab_hide_row2), check_button_gtab_hide_row2,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_hide_row2),
     gtab_hide_row2);
  GtkWidget *label_gtab_hide_row2 = gtk_label_new(_("隱藏第二列(輸入鍵…)"));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_hide_row2), label_gtab_hide_row2,  FALSE, FALSE, 0);

  GtkWidget *hbox_gtab_in_row1 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gtab_in_row1, FALSE, FALSE, 0);
  check_button_gtab_in_row1 = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gtab_in_row1), check_button_gtab_in_row1,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_in_row1),
     gtab_in_row1);
  GtkWidget *label_gtab_in_row1 = gtk_label_new(_("輸入鍵顯示移至第一列"));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_in_row1), label_gtab_in_row1,  FALSE, FALSE, 0);

  GtkWidget *hbox_gtab_vertical_select = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gtab_vertical_select, FALSE, FALSE, 0);
  check_button_gtab_vertical_select = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gtab_vertical_select), check_button_gtab_vertical_select,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_vertical_select),
     gtab_vertical_select);
  GtkWidget *label_gtab_vertical_select = gtk_label_new(_("垂直選擇"));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_vertical_select), label_gtab_vertical_select,  FALSE, FALSE, 0);

  GtkWidget *hbox_gtab_press_full_auto_send = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gtab_press_full_auto_send, FALSE, FALSE, 0);
  check_button_gtab_press_full_auto_send = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gtab_press_full_auto_send), check_button_gtab_press_full_auto_send,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_press_full_auto_send),
     gtab_press_full_auto_send);
  GtkWidget *label_gtab_gtab_press_full_auto_send = gtk_label_new(_("按滿自動送字"));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_press_full_auto_send), label_gtab_gtab_press_full_auto_send,  FALSE, FALSE, 0);

  GtkWidget *hbox_gtab_auto_select_by_phrase = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gtab_auto_select_by_phrase, FALSE, FALSE, 0);
  check_button_gtab_auto_select_by_phrase = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gtab_auto_select_by_phrase),check_button_gtab_auto_select_by_phrase,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_auto_select_by_phrase),
     gtab_auto_select_by_phrase);
  GtkWidget *label_gtab_auto_select = gtk_label_new(_("由詞庫自動選擇字"));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_auto_select_by_phrase), label_gtab_auto_select,  FALSE, FALSE, 0);

  GtkWidget *hbox_gtab_dup_select_bell = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gtab_dup_select_bell, FALSE, FALSE, 0);
  check_button_gtab_dup_select_bell = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gtab_dup_select_bell),check_button_gtab_dup_select_bell,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_dup_select_bell),
     gtab_dup_select_bell);
  GtkWidget *label_gtab_sele = gtk_label_new(_("重複字選擇鈴聲"));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_dup_select_bell), label_gtab_sele,  FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (vbox), create_spc_opts(), FALSE, FALSE, 0);

  GtkWidget *hbox_gtab_invalid_key_in = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gtab_invalid_key_in, FALSE, FALSE, 0);
  check_button_gtab_invalid_key_in = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gtab_invalid_key_in), check_button_gtab_invalid_key_in,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_invalid_key_in),
     gtab_invalid_key_in);
  GtkWidget *label_gtab_gtab_invalid_key_in = gtk_label_new(_("允許錯誤鍵進入(傳統)"));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_invalid_key_in), label_gtab_gtab_invalid_key_in,  FALSE, FALSE, 0);

  GtkWidget *hbox_gtab_shift_phrase_key = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gtab_shift_phrase_key, FALSE, FALSE, 0);
  check_button_gtab_shift_phrase_key = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gtab_shift_phrase_key), check_button_gtab_shift_phrase_key,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_shift_phrase_key),
     gtab_shift_phrase_key);
  GtkWidget *label_gtab_gtab_shift_phrase_key = gtk_label_new(_("Shift 用來輸入片語(Alt-Shift)"));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_shift_phrase_key), label_gtab_gtab_shift_phrase_key,  FALSE, FALSE, 0);

  GtkWidget *hbox_gtab_capslock_in_eng = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gtab_capslock_in_eng, FALSE, FALSE, 0);
  check_button_gtab_capslock_in_eng = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gtab_capslock_in_eng), check_button_gtab_capslock_in_eng,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_capslock_in_eng),
     gtab_capslock_in_eng);
  GtkWidget *label_gtab_capslock_in_eng = gtk_label_new(_("CapsLock 打開輸入英數"));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_capslock_in_eng), label_gtab_capslock_in_eng,  FALSE, FALSE, 0);

  GtkWidget *hbox_gtab_unique_auto_send = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gtab_unique_auto_send, FALSE, FALSE, 0);
  check_button_gtab_unique_auto_send = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gtab_unique_auto_send), check_button_gtab_unique_auto_send,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_unique_auto_send),
     gtab_unique_auto_send);
  GtkWidget *label_gtab_unique_auto_send = gtk_label_new(_("唯一選擇時自動送出"));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_unique_auto_send), label_gtab_unique_auto_send,  FALSE, FALSE, 0);

  GtkWidget *hbox_gtab_que_wild_card = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gtab_que_wild_card, FALSE, FALSE, 0);
  check_button_gtab_que_wild_card = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gtab_que_wild_card), check_button_gtab_que_wild_card,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gtab_que_wild_card),
     gtab_que_wild_card);
  GtkWidget *label_gtab_que_wild_card = gtk_label_new(_("使用？萬用字元"));
  gtk_box_pack_start (GTK_BOX (hbox_gtab_que_wild_card), label_gtab_que_wild_card,  FALSE, FALSE, 0);

  GtkWidget *button_edit_append = gtk_button_new_with_label(_("編輯內定輸入法的使用者外加字詞"));
  gtk_box_pack_start (GTK_BOX (vbox), button_edit_append, FALSE, FALSE, 0);
  g_signal_connect_swapped (G_OBJECT (button_edit_append), "clicked",
                            G_CALLBACK (cb_gtab_edit_append), NULL);

/* label 5 */

  vbox = gtk_vbox_new(FALSE, 0);
  notebooklabel = gtk_label_new(_("內定輸入法"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, notebooklabel);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),
                                       GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);

  /* create model */
  GtkTreeModel *model = create_model ();

  /* create tree view */
  treeview = gtk_tree_view_new_with_model (model);
  g_object_unref (G_OBJECT (model));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);

  GtkTreeSelection *tree_selection =
     gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  gtk_tree_selection_set_mode (tree_selection, GTK_SELECTION_SINGLE);

  add_columns (GTK_TREE_VIEW (treeview));

  gtk_container_add (GTK_CONTAINER (sw), treeview);

/* label end */

  gtk_widget_show_all(main_window);
}


void init_TableDir(), exec_setup_scripts();

int main(int argc, char **argv)
{
  exec_setup_scripts();

  init_TableDir();

  load_setttings();

#if GCIN_i18n_message
  gtk_set_locale();
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);
#endif

  gtk_init(&argc, &argv);

  create_main_win();

  // once you invoke gcin-setup, the left-right buton tips is disabled
  save_gcin_conf_int(LEFT_RIGHT_BUTTON_TIPS, 0);

  pclipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);

  gtk_main();

  return 0;
}
