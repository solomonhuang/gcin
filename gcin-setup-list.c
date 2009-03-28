#include "gcin.h"
#include "gcin-conf.h"
#include "gtab.h"

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

char *gcb_pos[] = {
  N_("關閉"), N_("左下"), N_("左上"), N_("右下"), N_("右上")
};

static GtkWidget *gtablist_window = NULL;
static GtkWidget *vbox;
static GtkWidget *hbox;
static GtkWidget *sw;
static GtkWidget *treeview;
static GtkWidget *button, *check_button_phonetic_speak, *opt_speaker_opts;
static GtkWidget *opt_im_toggle_keys, *check_button_gcin_remote_client, *opt_gcb_pos,
       *check_button_gcin_shift_space_eng_full,
       *check_button_gcin_init_im_enabled,
       *check_button_gcin_eng_phrase_enabled,
       *check_button_gcin_win_sym_click_close,
       *spinner_gcb_position_x, *spinner_gcb_position_y;

char *pho_speaker[16];
int pho_speakerN;

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


static GArray *articles = NULL;

static int qcmp_key(const void *aa, const void *bb)
{
  Item *a=(Item *)aa, *b=(Item *)bb;

  return gcin_switch_keys_lookup(a->key[0]) - gcin_switch_keys_lookup(b->key[0]);
}

extern char *TableDir;
void get_icon_path(char *iconame, char fname[]);

extern char gcin_switch_keys[];

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

#define DEFAULT_INPUT_METHOD "default-input-method"


static void cb_ok (GtkWidget *button, gpointer data)
{
  save_gcin_conf_int(DEFAULT_INPUT_METHOD, default_input_method);

  int idx = gtk_option_menu_get_history (GTK_OPTION_MENU (opt_im_toggle_keys));
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

  save_gcin_conf_int(GCIN_WIN_SYM_CLICK_CLOSE,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_win_sym_click_close)));

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

  gtk_widget_destroy(gtablist_window); gtablist_window = NULL;

  send_gcin_message(GDK_DISPLAY(), "reload");
}

static void cb_cancel (GtkWidget *widget, gpointer data)
{
  gtk_widget_destroy(gtablist_window); gtablist_window = NULL;
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


static void callback_win_delete()
{
  gtk_widget_destroy(gtablist_window); gtablist_window = NULL;
}

void set_selection_by_key(int key)
{
  if (!treeview)
    return;

  GtkTreeSelection *selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
  GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
  GtkTreeIter iter;
  gboolean found=FALSE;

  if (!gtk_tree_model_get_iter_first(model, &iter))
    return;

  do {
    char *tkey;

    gtk_tree_model_get(model,&iter, COLUMN_KEY, &tkey,-1);

    if (atoi(tkey) == key) {
      found=TRUE;
      break;
    }
  } while (gtk_tree_model_iter_next(model, &iter));

  if (found)
    gtk_tree_selection_select_iter(selection,&iter);
}


static GtkWidget *create_im_toggle_keys()
{

  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);
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
  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);

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


void create_gtablist_window (void)
{
  if (gtablist_window) {
    gtk_window_present(GTK_WINDOW(gtablist_window));
    return;
  }

  /* create gtab_list_window, etc */
  gtablist_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (gtablist_window), _("輸入法選擇"));
  gtk_container_set_border_width (GTK_CONTAINER (gtablist_window), 1);

  g_signal_connect (G_OBJECT (gtablist_window), "destroy",
                    G_CALLBACK (gtk_widget_destroyed), &gtablist_window);

  g_signal_connect (G_OBJECT (gtablist_window), "delete_event",
                      G_CALLBACK (callback_win_delete), NULL);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (gtablist_window), vbox);

  gtk_box_pack_start (GTK_BOX (vbox),
                      gtk_label_new (_("gcin 輸入法選擇")),
                      FALSE, FALSE, 0);

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

  gtk_box_pack_start (GTK_BOX (vbox), create_im_toggle_keys(), FALSE, FALSE, 0);

  GtkWidget *hbox_gcin_remote_client = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gcin_remote_client, FALSE, FALSE, 0);
  GtkWidget *label_gcin_remote_client = gtk_label_new(_("遠端 client 程式支援 (port 9999-)"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_remote_client), label_gcin_remote_client,  FALSE, FALSE, 0);
  check_button_gcin_remote_client = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcin_remote_client),check_button_gcin_remote_client,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_remote_client),
     gcin_remote_client);


  GtkWidget *hbox_gcin_init_im_enabled = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gcin_init_im_enabled, FALSE, FALSE, 0);
  GtkWidget *label_gcin_init_im_enabled = gtk_label_new(_("直接進入中文輸入狀態(限非XIM)"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_init_im_enabled), label_gcin_init_im_enabled,  FALSE, FALSE, 0);
  check_button_gcin_init_im_enabled = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcin_init_im_enabled),check_button_gcin_init_im_enabled,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_init_im_enabled),
     gcin_init_im_enabled);


  GtkWidget *hbox_gcin_shift_space_eng_full = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gcin_shift_space_eng_full, FALSE, FALSE, 0);
  GtkWidget *label_gcin_shift_space_eng_full = gtk_label_new(_("shift-space 進入全形英文模式"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_shift_space_eng_full), label_gcin_shift_space_eng_full,  FALSE, FALSE, 0);
  check_button_gcin_shift_space_eng_full = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcin_shift_space_eng_full),check_button_gcin_shift_space_eng_full,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_shift_space_eng_full),
     gcin_shift_space_eng_full);


  GtkWidget *hbox_gcin_eng_phrase_enabled = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gcin_eng_phrase_enabled, FALSE, FALSE, 0);
  GtkWidget *label_gcin_eng_phrase_enabled = gtk_label_new(_("英數狀態使用 alt-shift 片語"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_eng_phrase_enabled), label_gcin_eng_phrase_enabled,  FALSE, FALSE, 0);
  check_button_gcin_eng_phrase_enabled = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcin_eng_phrase_enabled),check_button_gcin_eng_phrase_enabled,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_eng_phrase_enabled),
     gcin_eng_phrase_enabled);

  GtkWidget *hbox_phonetic_speak = gtk_hbox_new(FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_phonetic_speak , FALSE, FALSE, 0);
  GtkWidget *label_phonetic_speak = gtk_label_new(_("輸入時念出發音"));
  gtk_box_pack_start (GTK_BOX (hbox_phonetic_speak), label_phonetic_speak , FALSE, FALSE, 0);
  check_button_phonetic_speak = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_phonetic_speak), check_button_phonetic_speak, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_phonetic_speak), phonetic_speak);


  GtkWidget *hbox_gcin_win_sym_click_close = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gcin_win_sym_click_close, FALSE, FALSE, 0);
  GtkWidget *label_gcin_win_sym_click_close = gtk_label_new(_("符號視窗點選後自動關閉"));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_win_sym_click_close), label_gcin_win_sym_click_close,  FALSE, FALSE, 0);
  check_button_gcin_win_sym_click_close = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcin_win_sym_click_close),check_button_gcin_win_sym_click_close,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_win_sym_click_close),
     gcin_win_sym_click_close);


  GtkWidget *hbox_gcb_pos = gtk_hbox_new (FALSE, 10);
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

#include <dirent.h>
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
      GtkWidget *labelspeaker = gtk_label_new(_("發音選擇"));
      gtk_box_pack_start (GTK_BOX (hbox_phonetic_speak), labelspeaker, FALSE, FALSE, 0);
      gtk_container_add (GTK_CONTAINER (hbox_phonetic_speak), create_speaker_opts());
    }
  }



  hbox = gtk_hbox_new (TRUE, 4);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  button = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (cb_cancel), treeview);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  button = gtk_button_new_from_stock (GTK_STOCK_OK);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (cb_ok), model);
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  gtk_window_set_default_size (GTK_WINDOW (gtablist_window), 520, 450);

  g_signal_connect (G_OBJECT (gtablist_window), "delete_event",
                    G_CALLBACK (gtk_main_quit), NULL);

  gtk_widget_show_all (gtablist_window);

  set_selection_by_key(default_input_method);

#if 0
  g_signal_connect (G_OBJECT(tree_selection), "changed",
                    G_CALLBACK (callback_row_selected), NULL);
#endif
}
