#include "gcin.h"
#include "gcin-conf.h"

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

static GtkWidget *gtablist_window = NULL;
static GtkWidget *vbox;
static GtkWidget *hbox;
static GtkWidget *sw;
static GtkWidget *treeview;
static GtkWidget *button;
static GtkWidget *opt_im_toggle_keys, *check_button_gcin_remote_client,
       *check_button_gcin_shift_space_eng_full;

typedef struct
{
  gchar *name;
  gchar *key;
  gchar *file;
  gboolean used;
  gboolean default_inmd;
  gboolean editable;
} Item;

enum
{
  COLUMN_NAME,
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

  return atoi(a->key) - atoi(b->key);
}

extern char *TableDir;

static void
add_items (void)
{
  Item foo;

  g_return_if_fail (articles != NULL);

  char ttt[128];
  FILE *fp;

  strcat(strcpy(ttt, TableDir),"/gtab.list");

  dbg("TableDir %s\n", TableDir);

  if ((fp=fopen(ttt, "r"))==NULL)
    exit(-1);

  while (!feof(fp)) {
    char name[32];
    char key[32];
    char file[32];

    name[0]=0;
    key[0]=0;
    file[0]=0;

    fscanf(fp, "%s %s %s", name, key, file);

    if (strlen(name) < 1)
      break;

    if (name[0]=='#')
      continue;

    foo.name = g_strdup(name);
    foo.key = g_strdup(key);
    int in_no = atoi(foo.key);
    foo.file = g_strdup(file);
    foo.used = (gcin_flags_im_enabled & (1 << in_no)) != 0;
    foo.default_inmd = in_no == default_input_method;
    foo.editable = FALSE;
    g_array_append_vals (articles, &foo, 1);
  }


  fclose(fp);
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
  model = gtk_list_store_new (NUM_COLUMNS,G_TYPE_STRING, G_TYPE_STRING,
                              G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN,
                              G_TYPE_BOOLEAN);

  /* add items */
  for (i = 0; i < articles->len; i++) {
      gtk_list_store_append (model, &iter);

      gtk_list_store_set (model, &iter,
			  COLUMN_NAME,
			  g_array_index (articles, Item, i).name,
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
  int in_no = atoi(key);

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
  default_input_method = atoi(key);

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
                                               -1, "名稱", renderer,
                                               "text", COLUMN_NAME,
                                               "editable", COLUMN_EDITABLE,
                                               NULL);

  /* number column */
  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", (gint *)COLUMN_KEY);


  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, "Ctrl-Alt-數字鍵", renderer,
                                               "text", COLUMN_KEY,
                                               "editable", COLUMN_EDITABLE,
                                               NULL);

  /* frequency column */

  renderer = gtk_cell_renderer_text_new ();
  g_object_set_data (G_OBJECT (renderer), "column", (gint *)COLUMN_FILE);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, "檔案名", renderer,
                                               "text", COLUMN_FILE,
                                               "editable", COLUMN_EDITABLE,
                                               NULL);
  // use column
  renderer = gtk_cell_renderer_toggle_new ();
  g_signal_connect (G_OBJECT (renderer), "toggled",
                    G_CALLBACK (toggled), model);

  g_object_set (G_OBJECT (renderer), "xalign", 0.0, NULL);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, "Ctrl-Shift 循環",
                                               renderer,
                                               "active", COLUMN_USE,
                                               NULL);

  // default_inmd column
  renderer = gtk_cell_renderer_toggle_new ();
  g_signal_connect (G_OBJECT (renderer), "toggled",
                    G_CALLBACK (toggled_default_inmd), model);

  g_object_set (G_OBJECT (renderer), "xalign", 0.0, NULL);

  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (treeview),
                                               -1, "第一次內定",
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
  GtkWidget *label = gtk_label_new("輸入視窗(開啟/關閉)切換");
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


void create_gtablist_window (void)
{
  if (gtablist_window) {
    gtk_window_present(GTK_WINDOW(gtablist_window));
    return;
  }

  /* create gtab_list_window, etc */
  gtablist_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (gtablist_window), "輸入法選擇");
  gtk_container_set_border_width (GTK_CONTAINER (gtablist_window), 5);

  g_signal_connect (G_OBJECT (gtablist_window), "destroy",
                    G_CALLBACK (gtk_widget_destroyed), &gtablist_window);

  g_signal_connect (G_OBJECT (gtablist_window), "delete_event",
                      G_CALLBACK (callback_win_delete), NULL);

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_container_add (GTK_CONTAINER (gtablist_window), vbox);

  gtk_box_pack_start (GTK_BOX (vbox),
                      gtk_label_new ("gcin 輸入法選擇"),
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
  GtkWidget *label_gcin_remote_client = gtk_label_new("遠端 client 程式支援 (port 9999-)");
  gtk_box_pack_start (GTK_BOX (hbox_gcin_remote_client), label_gcin_remote_client,  FALSE, FALSE, 0);
  check_button_gcin_remote_client = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcin_remote_client),check_button_gcin_remote_client,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_remote_client),
     gcin_remote_client);


  GtkWidget *hbox_gcin_shift_space_eng_full = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_gcin_shift_space_eng_full, FALSE, FALSE, 0);
  GtkWidget *label_gcin_shift_space_eng_full = gtk_label_new("shift-space 進入全形英文模式");
  gtk_box_pack_start (GTK_BOX (hbox_gcin_shift_space_eng_full), label_gcin_shift_space_eng_full,  FALSE, FALSE, 0);
  check_button_gcin_shift_space_eng_full = gtk_check_button_new ();
  gtk_box_pack_start (GTK_BOX (hbox_gcin_shift_space_eng_full),check_button_gcin_shift_space_eng_full,  FALSE, FALSE, 0);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_button_gcin_shift_space_eng_full),
     gcin_shift_space_eng_full);


  /* some buttons */
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

  gtk_window_set_default_size (GTK_WINDOW (gtablist_window), 550, 450);

  g_signal_connect (G_OBJECT (gtablist_window), "delete_event",
                    G_CALLBACK (gtk_main_quit), NULL);

  gtk_widget_show_all (gtablist_window);

  set_selection_by_key(default_input_method);

#if 0
  g_signal_connect (G_OBJECT(tree_selection), "changed",
                    G_CALLBACK (callback_row_selected), NULL);
#endif
}
