#include "gcin.h"
#include "gcin-conf.h"
#include "gtab.h"
#include "gtab-list.h"

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

extern char *default_input_method_str;

#if USE_GCB
unich_t *gcb_pos[] = {
N_(_L("左下")), N_(_L("左上")), N_(_L("右下")), N_(_L("右上"))
};
#endif

static GtkWidget *gtablist_window = NULL;
static GtkWidget *vbox;
static GtkWidget *hbox;
static GtkWidget *sw;
static GtkWidget *treeview;
static GtkWidget *button, *button2, *check_button_phonetic_speak, *opt_speaker_opts, *check_button_gcin_bell_off;
static GtkWidget *opt_im_toggle_keys, *check_button_gcin_remote_client,
#if USE_GCB
       *check_button_gcb_enabled,
       *opt_gcb_pos,
#endif
       *check_button_gcin_shift_space_eng_full,
       *check_button_gcin_init_im_enabled,
       *check_button_gcin_eng_phrase_enabled,
       *check_button_gcin_win_sym_click_close,
       *check_button_gcin_punc_auto_send;
#if USE_GCB
static GtkWidget *spinner_gcb_position_x, *spinner_gcb_position_y;
static GtkWidget *spinner_gcb_history_n, *spinner_gcb_button_n;
#endif
#if UNIX
static GtkWidget *check_button_gcin_single_state;
#endif
extern gboolean button_order;

char *pho_speaker[16];
int pho_speakerN;

typedef struct
{
  gchar *name;
  GdkPixbuf *icon;
  gchar *key;
  gchar *file;
  gboolean cycle;
  gboolean default_inmd;
  gboolean use;
  gboolean editable;
  INMD *pinmd;
} Item;

enum
{
  COLUMN_NAME,
  COLUMN_ICON,
  COLUMN_KEY,
  COLUMN_FILE,
  COLUMN_CYCLE,
  COLUMN_DEFAULT_INMD,
  COLUMN_USE,
  COLUMN_EDITABLE,
  COLUMN_PINMD,
  NUM_COLUMNS
};


static GArray *articles = NULL;
int gcin_switch_keys_lookup(int key);

static int qcmp_key(const void *aa, const void *bb)
{
  Item *a=(Item *)aa, *b=(Item *)bb;

  return gcin_switch_keys_lookup(a->key[0]) - gcin_switch_keys_lookup(b->key[0]);
}

extern char *TableDir;
void get_icon_path(char *iconame, char fname[]);

static void
add_items (void)
{
  Item foo;

  g_return_if_fail (articles != NULL);

  load_gtab_list(FALSE);

  int i;
  for (i=0; i < inmdN; i++) {
    INMD *pinmd = &inmd[i];
    char *name = pinmd->cname;
    if (!name)
      continue;

    char key[2];
    char *file = pinmd->filename;
    char *icon = pinmd->icon;

    key[0] = pinmd->key_ch;
    key[1]=0;

    foo.name = g_strdup(name);
    char icon_path[128];
    get_icon_path(icon, icon_path);
    GError *err = NULL;
    foo.icon = gdk_pixbuf_new_from_file(icon_path, &err);
    foo.key = g_strdup(key);
    foo.file = g_strdup(file);
//    dbg("%d] %d\n",i,pinmd->in_cycle);
    foo.default_inmd =  default_input_method == i ;
    foo.use = !pinmd->disabled;
    foo.cycle = pinmd->in_cycle && foo.use;
    foo.editable = FALSE;
    foo.pinmd = pinmd;
    g_array_append_vals (articles, &foo, 1);
  }

//  g_array_sort (articles,qcmp_key);
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
                              G_TYPE_BOOLEAN,
                              G_TYPE_BOOLEAN, G_TYPE_POINTER);

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
                          COLUMN_CYCLE,
                          g_array_index (articles, Item, i).cycle,
                          COLUMN_DEFAULT_INMD,
                          g_array_index (articles, Item, i).default_inmd,
                          COLUMN_USE,
                          g_array_index (articles, Item, i).use,
                          COLUMN_EDITABLE,
                          g_array_index (articles, Item, i).editable,
                          COLUMN_PINMD,
                          g_array_index (articles, Item, i).pinmd,
                          -1);
  }

  return GTK_TREE_MODEL (model);
}


extern char gtab_list[];

static void save_gtab_list()
{
  char ttt[128];
  get_gcin_user_fname(gtab_list, ttt);

  FILE *fp;

  if ((fp=fopen(ttt, "w"))==NULL)
    p_err("cannot write to %s\n", ttt);

  int i;
  for (i=0; i < inmdN; i++) {
    INMD *pinmd = &inmd[i];
    char *name = pinmd->cname;
    if (!name)
      continue;

    char *file = pinmd->filename;
    char *icon = pinmd->icon;
    char *disabled = pinmd->disabled?"!":"";

    fprintf(fp, "%s%s %c %s %s\n", disabled,name, pinmd->key_ch, file, icon);
  }

  fclose(fp);
}


static void cb_ok (GtkWidget *button, gpointer data)
{
  GtkTreeModel *model = GTK_TREE_MODEL (data);

  GtkTreeIter iter;
  if (!gtk_tree_model_get_iter_first(model, &iter))
    return;

  do {
    char *tkey;
    gtk_tree_model_get(model,&iter, COLUMN_KEY, &tkey, -1);
    gboolean cycle, default_inmd, use;
    gtk_tree_model_get (model, &iter, COLUMN_CYCLE, &cycle, -1);
    gtk_tree_model_get (model, &iter, COLUMN_DEFAULT_INMD, &default_inmd, -1);
    gtk_tree_model_get (model, &iter, COLUMN_USE, &use, -1);
    INMD *pinmd;
    gtk_tree_model_get (model, &iter, COLUMN_PINMD, &pinmd, -1);
    pinmd->in_cycle = cycle;
    pinmd->disabled = !use;
  } while (gtk_tree_model_iter_next(model, &iter));

  dbg("default_input_method_str %s\n",default_input_method_str);
  save_gcin_conf_str(DEFAULT_INPUT_METHOD, default_input_method_str);

  int idx;
#if UNIX
  idx = gtk_combo_box_get_active (GTK_COMBO_BOX (opt_im_toggle_keys));
  save_gcin_conf_int(GCIN_IM_TOGGLE_KEYS, imkeys[idx].keynum);
#else
  save_gcin_conf_int(GCIN_IM_TOGGLE_KEYS, Control_Space);
#endif

  free(gcin_str_im_cycle);

  int i;
  char tt[512];
  int ttN=0;
  for(i=0;i<inmdN;i++) {
    if (inmd[i].in_cycle) {
//      dbg("in %d %c\n", i, inmd[i].key_ch);
      tt[ttN++]=inmd[i].key_ch;
    }
  }
  tt[ttN]=0;
  gcin_str_im_cycle = strdup(tt);
  save_gcin_conf_str(GCIN_STR_IM_CYCLE, gcin_str_im_cycle);
  dbg("gcin_str_im_cycle ttN:%d '%s' '%s'\n", ttN, gcin_str_im_cycle, tt);

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

  save_gcin_conf_int(GCIN_BELL_OFF,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_bell_off)));

  save_gcin_conf_int(GCIN_PUNC_AUTO_SEND,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_punc_auto_send)));
#if UNIX
  save_gcin_conf_int(GCIN_SINGLE_STATE,
    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcin_single_state)));
#endif
  if (opt_speaker_opts) {
    idx = gtk_combo_box_get_active (GTK_COMBO_BOX (opt_speaker_opts));
    save_gcin_conf_str(PHONETIC_SPEAK_SEL, pho_speaker[idx]);
  }


#if USE_GCB
  save_gcin_conf_int(GCB_ENABLED, gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_gcb_enabled)));
  idx = gtk_combo_box_get_active (GTK_COMBO_BOX (opt_gcb_pos));
  save_gcin_conf_int(GCB_POSITION, idx+1); // for backward compatbility
  int pos_x = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcb_position_x));
  save_gcin_conf_int(GCB_POSITION_X, pos_x);
  int pos_y = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcb_position_y));
  save_gcin_conf_int(GCB_POSITION_Y, pos_y);
  int button_n = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcb_button_n));
  save_gcin_conf_int(GCB_BUTTON_N, button_n);
  int history_n = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(spinner_gcb_history_n));
  save_gcin_conf_int(GCB_HISTORY_N, history_n);
#endif

  save_gtab_list();

  save_gcin_conf_int(PHONETIC_SPEAK,
     gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button_phonetic_speak)));

  gtk_widget_destroy(gtablist_window); gtablist_window = NULL;

  send_gcin_message(
#if UNIX
	  GDK_DISPLAY(),
#endif
	  "reload");
}

static void cb_cancel (GtkWidget *widget, gpointer data)
{
  gtk_widget_destroy(gtablist_window); gtablist_window = NULL;
}

int gcin_switch_keys_lookup(int key);
#if 1
static gboolean toggled (GtkCellRendererToggle *cell, gchar *path_string, gpointer data)
{
  GtkTreeModel *model = GTK_TREE_MODEL (data);
  GtkTreeIter iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
  gboolean cycle;

  dbg("toggled\n");

  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, COLUMN_CYCLE, &cycle, -1);

  cycle ^= 1;
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_CYCLE, cycle, -1);

  if (cycle) {
    gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_USE, TRUE, -1);
  }

  gtk_tree_path_free (path);

  return TRUE;
}
#endif

static void clear_col_default_inmd(GtkTreeModel *model)
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
  clear_col_default_inmd(model);
  GtkTreeIter iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);

//  dbg("toggled_default_inmd\n");
  gtk_tree_model_get_iter (model, &iter, path);
  char *key;
  gtk_tree_model_get (model, &iter, COLUMN_KEY, &key, -1);
  char *file;
  gtk_tree_model_get (model, &iter, COLUMN_FILE, &file, -1);
  char tt[128];
  sprintf(tt, "%s %s", key, file);
  free(default_input_method_str);
  default_input_method_str = strdup(tt);
  dbg("default_input_method_str %s\n", default_input_method_str);
//  default_input_method = gcin_switch_keys_lookup(key[0]);

  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_DEFAULT_INMD, TRUE, -1);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_USE, TRUE, -1);

  gtk_tree_path_free (path);

  return TRUE;
}

static gboolean toggled_use(GtkCellRendererToggle *cell, gchar *path_string, gpointer data)
{
  GtkTreeModel *model = GTK_TREE_MODEL (data);
//  clear_all(model);
  GtkTreeIter iter;
  GtkTreePath *path = gtk_tree_path_new_from_string (path_string);

//  dbg("toggled_default_inmd\n");
  gtk_tree_model_get_iter (model, &iter, path);
  gboolean cycle, default_inmd, use;
  gtk_tree_model_get (model, &iter, COLUMN_CYCLE, &cycle, -1);
  gtk_tree_model_get (model, &iter, COLUMN_DEFAULT_INMD, &default_inmd, -1);
  gtk_tree_model_get (model, &iter, COLUMN_USE, &use, -1);
  use=!use;
  gboolean must_on = default_inmd;
  dbg("toggle %d %d %d\n", cycle, default_inmd, use);

  if (must_on && !use) {
    return TRUE;
  }

  gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_USE, use, -1);
  if (!use)
    gtk_list_store_set (GTK_LIST_STORE (model), &iter, COLUMN_CYCLE, FALSE, -1);

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
                                               -1, _(_L("名稱")), renderer,
                                               "text", COLUMN_NAME,
                                               "editable", COLUMN_EDITABLE,
                                               NULL);

  renderer = gtk_cell_renderer_pixbuf_new();
  g_object_set_data (G_OBJECT (renderer), "column", (gint *)COLUMN_ICON);
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, "icon", renderer,
                                               "pixbuf", COLUMN_ICON,
//                                               "editable", COLUMN_EDITABLE,
                                               NULL);

  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", (gint *)COLUMN_KEY);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, _(_L("Ctrl-Alt-鍵")), renderer,
                                               "text", COLUMN_KEY,
                                               "editable", COLUMN_EDITABLE,
                                               NULL);

  // cycle column
  renderer = gtk_cell_renderer_toggle_new ();
  g_signal_connect (G_OBJECT (renderer), "toggled",
                    G_CALLBACK (toggled), model);

  g_object_set (G_OBJECT (renderer), "xalign", 0.0, NULL);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),-1,
#if UNIX
	  _(_L("Ctrl-Shift\n循環")),
#else
	  _(_L("Ctrl-Shift 循環\n需關閉Windows按鍵")),
#endif
	  renderer, "active", COLUMN_CYCLE,
                                               NULL);

  // default_inmd column
  renderer = gtk_cell_renderer_toggle_new ();
  gtk_cell_renderer_toggle_set_radio(GTK_CELL_RENDERER_TOGGLE(renderer), TRUE);
  g_signal_connect (G_OBJECT (renderer), "toggled",
                    G_CALLBACK (toggled_default_inmd), model);

  g_object_set (G_OBJECT (renderer), "xalign", 0.0, NULL);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, _(_L("第一次\n內定")),

                                               renderer,
                                               "active", COLUMN_DEFAULT_INMD,
                                               NULL);

  // use
  renderer = gtk_cell_renderer_toggle_new ();
  g_signal_connect (G_OBJECT (renderer), "toggled",
                    G_CALLBACK (toggled_use), model);

  g_object_set (G_OBJECT (renderer), "xalign", 0.0, NULL);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, _(_L("使用")),
                                               renderer,
                                               "active", COLUMN_USE,
                                               NULL);

  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", (gint *)COLUMN_FILE);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, _(_L("檔案名")), renderer,
                                               "text", COLUMN_FILE,
                                               "editable", COLUMN_EDITABLE,
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

#if UNIX
static GtkWidget *create_im_toggle_keys()
{

  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);
  GtkWidget *label = gtk_label_new(_(_L("輸入視窗(開啟/關閉)切換")));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

  opt_im_toggle_keys = gtk_combo_box_new_text ();
#if !GTK_CHECK_VERSION(2,4,0)
  GtkWidget *menu_im_toggle_keys = gtk_menu_new ();
#endif
  gtk_box_pack_start (GTK_BOX (hbox), opt_im_toggle_keys, FALSE, FALSE, 0);

  int i, current_idx=0;

  for(i=0; imkeys[i].keystr; i++) {
#if !GTK_CHECK_VERSION(2,4,0)
    GtkWidget *item = gtk_menu_item_new_with_label (imkeys[i].keystr);
#endif

    if (imkeys[i].keynum == gcin_im_toggle_keys)
      current_idx = i;

#if GTK_CHECK_VERSION(2,4,0)
    gtk_combo_box_append_text (GTK_COMBO_BOX_TEXT (opt_im_toggle_keys), imkeys[i].keystr);
#else
    gtk_menu_shell_append (GTK_MENU_SHELL (menu_im_toggle_keys), item);
#endif
  }

#if !GTK_CHECK_VERSION(2,4,0)
  gtk_option_menu_set_menu (GTK_OPTION_MENU (opt_im_toggle_keys), menu_im_toggle_keys);
#endif
  gtk_combo_box_set_active (GTK_COMBO_BOX (opt_im_toggle_keys), current_idx);

  return hbox;
}
#endif

int get_current_speaker_idx();

static GtkWidget *create_speaker_opts()
{
  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);

  opt_speaker_opts = gtk_combo_box_new_text ();
#if !GTK_CHECK_VERSION(2,4,0)
  GtkWidget *menu_speaker_opts = gtk_menu_new ();
#endif
  gtk_box_pack_start (GTK_BOX (hbox), opt_speaker_opts, FALSE, FALSE, 0);

  int i;
  int current_idx = get_current_speaker_idx();

  for(i=0; i<pho_speakerN; i++) {
#if !GTK_CHECK_VERSION(2,4,0)
    GtkWidget *item = gtk_menu_item_new_with_label (pho_speaker[i]);
#endif

    if (imkeys[i].keynum == gcin_im_toggle_keys)
      current_idx = i;

#if GTK_CHECK_VERSION(2,4,0)
    gtk_combo_box_append_text (GTK_COMBO_BOX_TEXT (opt_speaker_opts), pho_speaker[i]);
#else
    gtk_menu_shell_append (GTK_MENU_SHELL (menu_speaker_opts), item);
#endif
  }

#if !GTK_CHECK_VERSION(2,4,0)
  gtk_option_menu_set_menu (GTK_OPTION_MENU (opt_speaker_opts), menu_speaker_opts);
#endif
  gtk_combo_box_set_active (GTK_COMBO_BOX (opt_speaker_opts), current_idx);

  return hbox;
}


#if USE_GCB
static GtkWidget *create_gcb_pos_opts()
{
  GtkWidget *hbox = gtk_hbox_new (FALSE, 1);

  opt_gcb_pos = gtk_combo_box_new_text ();
#if !GTK_CHECK_VERSION(2,4,0)
  GtkWidget *menu_gcb_pos = gtk_menu_new ();
#endif
  gtk_box_pack_start (GTK_BOX (hbox), opt_gcb_pos, FALSE, FALSE, 0);

  int i;

  for(i=0; i<sizeof(gcb_pos)/sizeof(gcb_pos[0]); i++) {
#if GTK_CHECK_VERSION(2,4,0)
    gtk_combo_box_append_text (GTK_COMBO_BOX_TEXT (opt_gcb_pos), _(gcb_pos[i]));
#else
    GtkWidget *item = gtk_menu_item_new_with_label (_(gcb_pos[i]));
    gtk_menu_shell_append (GTK_MENU_SHELL (menu_gcb_pos), item);
#endif
  }

#if !GTK_CHECK_VERSION(2,4,0)
  gtk_option_menu_set_menu (GTK_OPTION_MENU (opt_gcb_pos), menu_gcb_pos);
#endif
  gtk_combo_box_set_active (GTK_COMBO_BOX (opt_gcb_pos), gcb_position-1); // for backward compatibily

  return hbox;
}
#endif

#if UNIX
#include <dirent.h>
#endif

void create_gtablist_window (void)
{
  if (gtablist_window) {
    gtk_window_present(GTK_WINDOW(gtablist_window));
    return;
  }

  /* create gtab_list_window, etc */
  gtablist_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(gtablist_window), GTK_WIN_POS_MOUSE);

  gtk_window_set_has_resize_grip(GTK_WINDOW(gtablist_window), FALSE);
 gtk_window_set_title (GTK_WINDOW (gtablist_window), _(_L("gcin 輸入法選擇")));
  gtk_container_set_border_width (GTK_CONTAINER (gtablist_window), 1);

  g_signal_connect (G_OBJECT (gtablist_window), "destroy",
                    G_CALLBACK (gtk_widget_destroyed), &gtablist_window);

  g_signal_connect (G_OBJECT (gtablist_window), "delete_event",
                      G_CALLBACK (callback_win_delete), NULL);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox), GTK_ORIENTATION_VERTICAL);
  gtk_container_add (GTK_CONTAINER (gtablist_window), vbox);

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
  gtk_widget_set_hexpand (treeview, TRUE);
  gtk_widget_set_vexpand (treeview, TRUE);
  g_object_unref (G_OBJECT (model));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);

  GtkTreeSelection *tree_selection =
     gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
  gtk_tree_selection_set_mode (tree_selection, GTK_SELECTION_SINGLE);

  add_columns (GTK_TREE_VIEW (treeview));

  gtk_container_add (GTK_CONTAINER (sw), treeview);


  GtkWidget *hpan= gtk_hpaned_new ();
  gtk_box_pack_start (GTK_BOX (vbox), hpan, FALSE, FALSE, 0);


  GtkWidget *vboxL = gtk_vbox_new (FALSE, 0);
  GtkWidget *vboxR = gtk_vbox_new (FALSE, 0);
  gtk_paned_pack1 (GTK_PANED (hpan), vboxL, TRUE, TRUE);
  gtk_paned_pack2 (GTK_PANED (hpan), vboxR, TRUE, TRUE);

#if UNIX
  gtk_box_pack_start(GTK_BOX(vboxL), create_im_toggle_keys(), FALSE, FALSE, 0);
#endif

#if UNIX
  GtkWidget *hbox_gcin_remote_client = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vboxR), hbox_gcin_remote_client, FALSE, FALSE, 0);
  GtkWidget *label_gcin_remote_client = gtk_label_new(_(_L("遠端 client 程式支援 (port 9999-)")));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_remote_client), label_gcin_remote_client,  FALSE, FALSE, 0);
  check_button_gcin_remote_client = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcin_remote_client),check_button_gcin_remote_client,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_remote_client),
     gcin_remote_client);


  GtkWidget *hbox_gcin_init_im_enabled = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX(vboxL), hbox_gcin_init_im_enabled, FALSE, FALSE, 0);
  GtkWidget *label_gcin_init_im_enabled = gtk_label_new(_(_L("直接進入中文輸入狀態(限非XIM)")));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_init_im_enabled), label_gcin_init_im_enabled,  FALSE, FALSE, 0);
  check_button_gcin_init_im_enabled = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcin_init_im_enabled),check_button_gcin_init_im_enabled,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_init_im_enabled),
     gcin_init_im_enabled);
#endif


  GtkWidget *hbox_gcin_shift_space_eng_full = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vboxR), hbox_gcin_shift_space_eng_full, FALSE, FALSE, 0);
  GtkWidget *label_gcin_shift_space_eng_full = gtk_label_new(_(_L("shift-space 進入全形英文模式")));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_shift_space_eng_full), label_gcin_shift_space_eng_full,  FALSE, FALSE, 0);
  check_button_gcin_shift_space_eng_full = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcin_shift_space_eng_full),check_button_gcin_shift_space_eng_full,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_shift_space_eng_full),
     gcin_shift_space_eng_full);

#if UNIX
  GtkWidget *hbox_gcin_single_state = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vboxL), hbox_gcin_single_state, FALSE, FALSE, 0);
  GtkWidget *label_gcin_single_state = gtk_label_new(_(_L("不記憶個別程式的輸入法狀態")));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_single_state), label_gcin_single_state,  FALSE, FALSE, 0);
  check_button_gcin_single_state = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcin_single_state),check_button_gcin_single_state,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_single_state),
     gcin_single_state);
#endif

  GtkWidget *hbox_gcin_eng_phrase_enabled = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vboxR), hbox_gcin_eng_phrase_enabled, FALSE, FALSE, 0);
  GtkWidget *label_gcin_eng_phrase_enabled = gtk_label_new(_(_L("英數狀態使用 alt-shift 片語")));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_eng_phrase_enabled), label_gcin_eng_phrase_enabled,  FALSE, FALSE, 0);
  check_button_gcin_eng_phrase_enabled = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcin_eng_phrase_enabled),check_button_gcin_eng_phrase_enabled,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_eng_phrase_enabled),
     gcin_eng_phrase_enabled);

  GtkWidget *hbox_phonetic_speak = gtk_hbox_new(FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vboxL), hbox_phonetic_speak , FALSE, FALSE, 0);
  GtkWidget *label_phonetic_speak = gtk_label_new(_(_L("輸入時念出發音")));
  gtk_box_pack_start (GTK_BOX (hbox_phonetic_speak), label_phonetic_speak , FALSE, FALSE, 0);
  check_button_phonetic_speak = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_phonetic_speak), check_button_phonetic_speak, FALSE, FALSE, 0);
  gtk_toggle_button_set_active(
     GTK_TOGGLE_BUTTON(check_button_phonetic_speak), phonetic_speak);


  GtkWidget *hbox_gcin_win_sym_click_close = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vboxR), hbox_gcin_win_sym_click_close, FALSE, FALSE, 0);
  GtkWidget *label_gcin_win_sym_click_close = gtk_label_new(_(_L("符號視窗點選後自動關閉")));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_win_sym_click_close), label_gcin_win_sym_click_close,  FALSE, FALSE, 0);
  check_button_gcin_win_sym_click_close = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcin_win_sym_click_close),check_button_gcin_win_sym_click_close,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_win_sym_click_close),
     gcin_win_sym_click_close);


  GtkWidget *hbox_gcin_bell_off = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vboxR), hbox_gcin_bell_off, FALSE, FALSE, 0);
  GtkWidget *label_gcin_bell_off = gtk_label_new(_(_L("關閉鈴聲")));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_bell_off), label_gcin_bell_off,  FALSE, FALSE, 0);
  check_button_gcin_bell_off = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcin_bell_off),check_button_gcin_bell_off,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_bell_off),
     gcin_bell_off);


  GtkWidget *hbox_gcin_punc_auto_send = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vboxL), hbox_gcin_punc_auto_send, FALSE, FALSE, 0);
  GtkWidget *label_gcin_punc_auto_send = gtk_label_new(_(_L("結尾標點符號自動送出編輯區內容")));
  gtk_box_pack_start (GTK_BOX (hbox_gcin_punc_auto_send), label_gcin_punc_auto_send,  FALSE, FALSE, 0);
  check_button_gcin_punc_auto_send = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcin_punc_auto_send),check_button_gcin_punc_auto_send,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_punc_auto_send),
     gcin_punc_auto_send);


#if USE_GCB
  GtkWidget *hbox_gcb_pos = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gcb_pos, FALSE, FALSE, 0);
  GtkWidget *label_gcb_pos = gtk_label_new(_(_L("剪貼區管理視窗位置&開關")));
  gtk_box_pack_start (GTK_BOX (hbox_gcb_pos), label_gcb_pos,  FALSE, FALSE, 0);

  check_button_gcb_enabled = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcb_pos), check_button_gcb_enabled,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcb_enabled),
     gcb_enabled);

  gtk_box_pack_start (GTK_BOX (hbox_gcb_pos), create_gcb_pos_opts(),  FALSE, FALSE, 0);
  GtkAdjustment *adj_gcb_position_x =
   (GtkAdjustment *) gtk_adjustment_new (gcb_position_x, 0.0, 100.0, 1.0, 1.0, 0.0);
  spinner_gcb_position_x = gtk_spin_button_new (adj_gcb_position_x, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcb_pos), spinner_gcb_position_x, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcb_position_y =
   (GtkAdjustment *) gtk_adjustment_new (gcb_position_y, 0.0, 100.0, 1.0, 1.0, 0.0);
  spinner_gcb_position_y = gtk_spin_button_new (adj_gcb_position_y, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcb_pos), spinner_gcb_position_y, FALSE, FALSE, 0);


  GtkWidget *label_n = gtk_label_new(_(_L("儲藏/歷史 格數量 ")));
  gtk_box_pack_start (GTK_BOX (hbox_gcb_pos), label_n,  FALSE, FALSE, 0);

  GtkAdjustment *adj_gcb_button_n =
   (GtkAdjustment *) gtk_adjustment_new (gcb_button_n, 2.0, 9.0, 1.0, 1.0, 0.0);
  spinner_gcb_button_n = gtk_spin_button_new (adj_gcb_button_n, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcb_pos), spinner_gcb_button_n, FALSE, FALSE, 0);
  GtkAdjustment *adj_gcb_history_n =
   (GtkAdjustment *) gtk_adjustment_new (gcb_history_n, 5.0, 50.0, 1.0, 1.0, 0.0);
  spinner_gcb_history_n = gtk_spin_button_new (adj_gcb_history_n, 0, 0);
  gtk_box_pack_start (GTK_BOX (hbox_gcb_pos), spinner_gcb_history_n, FALSE, FALSE, 0);

#endif

#if UNIX
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

  }
#else
  wchar_t oggdir[256];
  wchar_t gcin16[256];
  utf8_to_16(gcin_program_files_path, gcin16, sizeof(gcin16));
  wsprintfW(oggdir, L"%s\\ogg\\ㄧ\\*", gcin16);

  WIN32_FIND_DATAW ffd;
  HANDLE hFind = FindFirstFileW(oggdir, &ffd);

  if (INVALID_HANDLE_VALUE != hFind) {
    do {
      char tt[256];
      utf16_to_8(ffd.cFileName, tt, sizeof(tt));
	  if (!strcmp(tt, ".") || !strcmp(tt, ".."))
		  continue;
	  dbg("--- %s\n", tt);
      pho_speaker[pho_speakerN++]=strdup(tt);
    } while (FindNextFileW(hFind, &ffd) != 0);

    FindClose(hFind);
  }
#endif

  if (pho_speakerN) {
    GtkWidget *labelspeaker = gtk_label_new(_(_L("發音選擇")));
    gtk_box_pack_start (GTK_BOX (hbox_phonetic_speak), labelspeaker, FALSE, FALSE, 0);
    gtk_container_add (GTK_CONTAINER (hbox_phonetic_speak), create_speaker_opts());
  }

  hbox = gtk_hbox_new (TRUE, 4);
  gtk_grid_set_column_homogeneous(GTK_GRID(hbox), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  button = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (cb_cancel), treeview);
  if (button_order)
    gtk_box_pack_end (GTK_BOX (hbox), button, TRUE, TRUE, 0);
  else
    gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 0);

  button2 = gtk_button_new_from_stock (GTK_STOCK_OK);
  g_signal_connect (G_OBJECT (button2), "clicked",
                    G_CALLBACK (cb_ok), model);
#if !GTK_CHECK_VERSION(2,91,2)
  if (button_order)
    gtk_box_pack_end (GTK_BOX (hbox), button2, TRUE, TRUE, 0);
  else
    gtk_box_pack_start (GTK_BOX (hbox), button2, TRUE, TRUE, 0);
#else
  if (button_order)
    gtk_grid_attach_next_to (GTK_BOX (hbox), button2, button, GTK_POS_LEFT, 1, 1);
  else
    gtk_grid_attach_next_to (GTK_BOX (hbox), button2, button, GTK_POS_RIGHT, 1, 1);
#endif
#if UNIX
  gtk_window_set_default_size (GTK_WINDOW (gtablist_window), 480, 450);
#else
  gtk_window_set_default_size (GTK_WINDOW (gtablist_window), 680, 450);
#endif

  g_signal_connect (G_OBJECT (gtablist_window), "delete_event",
                    G_CALLBACK (gtk_main_quit), NULL);

  gtk_widget_show_all (gtablist_window);

  set_selection_by_key(default_input_method);

#if 0
  g_signal_connect (G_OBJECT(tree_selection), "changed",
                    G_CALLBACK (callback_row_selected), NULL);
#endif
}
