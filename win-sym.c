#include "gcin.h"
#include "pho.h"
#include <sys/stat.h>

static GtkWidget *gwin_sym = NULL;

typedef struct {
  char **sym;
  int symN;
} SYM_ROW;

static SYM_ROW *syms;
static int symsN;
static time_t file_modify_time;

extern char *TableDir;


static gboolean read_syms()
{
  FILE *fp;
  char fname[256];
  static char symbol_table[] = "symbol-table";

  get_gcin_conf_fname(symbol_table, fname);

  if ((fp=fopen(fname, "r"))==NULL) {
    strcat(strcat(strcpy(fname, TableDir), "/"), symbol_table);

    if ((fp=fopen(fname, "r"))==NULL)
     return FALSE;
  }

  struct stat st;
  fstat(fileno(fp), &st);

  if (file_modify_time) {
    if (st.st_mtime == file_modify_time) {
      fclose(fp);
      return FALSE;
    }
  }

  file_modify_time = st.st_mtime;

  int i;
  for(i=0; i < symsN; i++) {
    int j;
    for(j=0; j < syms[i].symN; j++)
      free(syms[i].sym[j]);
  }
  free(syms); syms=NULL;
  symsN=0;


  while (!feof(fp)) {
    char tt[1024];

    bzero(tt, sizeof(tt));
    fgets(tt, sizeof(tt), fp);
    int len=strlen(tt);

    if (!len)
      continue;

    if (tt[len-1]=='\n')
      tt[len-1]=0;

    if (!strlen(tt))
      return;

    char *p=tt;

    syms=realloc(syms, sizeof(SYM_ROW) * (symsN+1));
    SYM_ROW *psym = &syms[symsN++];
    bzero(psym, sizeof(SYM_ROW));


    while (*p) {
      char *n = p;

      while (*n && *n!='\t')
        n++;

      *n = 0;

      psym->sym=realloc(psym->sym, (psym->symN+1) * sizeof(char *));
      psym->sym[psym->symN++] = strdup(p);

      p = n + 1;
    }
  }

  fclose(fp);
  return TRUE;
}

gboolean add_to_tsin_buf(char *str, phokey_t *pho, int len);

static void cb_button_sym(GtkButton *button, char *str)
{
//  dbg("select %s\n", str);
  phokey_t pho[256];
  int len = strlen(str);

  bzero(pho, sizeof(pho));
  int i;
  for(i=0; i < len; i+=2)
    big5_pho_chars(str+i, &pho[i>>1]);

  add_to_tsin_buf(str, pho, strlen(str) / 2);
}



void move_win_sym()
{
  int winsym_xl, winsym_yl;
  GtkRequisition sz;

//  dbg("win_y: %d  %d\n", win_y, win_yl);
  update_active_in_win_geom();

  gtk_widget_size_request(gwin_sym, &sz);
  winsym_xl = sz.width;  winsym_yl = sz.height;
  int wx = win_x, wy = win_y + win_yl;

  if (wx + winsym_xl > dpy_xl)
    wx = dpy_xl - winsym_xl;
  if (wx < 0)
    wx = 0;

  if (wy + winsym_yl > dpy_yl)
    wy = win_y - winsym_yl;
  if (wy < 0)
    wy = 0;

  gtk_window_move(GTK_WINDOW(gwin_sym), wx, wy);
}

static gboolean win_sym_enabled=1;

void hide_win_sym()
{
  if (!gwin_sym)
    return;

  gtk_widget_hide(gwin_sym);
}


void show_win_sym()
{
  if (!gwin_sym || !win_sym_enabled)
    return;

  gtk_widget_show(gwin_sym);
  move_win_sym();
}

static void str_to_all_phokey_chars(char *b5_str, char *out)
{
  int len=strlen(b5_str);

  out[0]=0;

  int h;

  for(h=0; h < strlen(b5_str); h+=CH_SZ) {
    phokey_t phos[32];

    int n=big5_pho_chars(&b5_str[h], phos);

    int i;
    for(i=0; i < n; i++) {
      char *pstr = phokey_to_str(phos[i]);
      strcat(out, pstr);
      if (i < n -1)
        strcat(out, " ");
    }

    if (h < len - CH_SZ)
      strcat(out, " | ");
  }
}


void create_win_sym()
{
  if (read_syms()) {
    if (gwin_sym)
      gtk_widget_destroy(gwin_sym);
    gwin_sym = NULL;
  } else {
    if (!syms)
      return;
  }

  if (gwin_sym) {
    win_sym_enabled^=1;

    if (win_sym_enabled) {
      move_win_sym();
      show_win_sym();
    }
    else
      hide_win_sym();

    return;
  }

  gwin_sym = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (gwin_sym), vbox_top);

  gtk_container_set_border_width (GTK_CONTAINER (vbox_top), 0);


  int i;
  for(i=0; i < symsN; i++) {
    SYM_ROW *psym = &syms[i];
    GtkWidget *hbox_row = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox_top), hbox_row, FALSE, FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox_row), 0);

    int j;
    for(j=0; j < psym->symN; j++) {
      GError *err = NULL;
      char *str = psym->sym[j];
      int rn, wn;

      char *utf8 = g_locale_to_utf8 (str, strlen(str), &rn, &wn, &err);

      if (!utf8)
         continue;

      GtkWidget *button = gtk_button_new_with_label(utf8);
      g_free(utf8);

      gtk_container_set_border_width (GTK_CONTAINER (button), 0);
      gtk_box_pack_start (GTK_BOX (hbox_row), button, FALSE, FALSE, 0);

      if (strlen(str)>=CH_SZ) {
        char *pho_utf8 = NULL;
        char phos[32];

        str_to_all_phokey_chars(str, phos);
        int phos_len = strlen(phos);

        if (phos_len) {
          pho_utf8 = g_locale_to_utf8 (phos, phos_len, &rn, &wn, &err);
          GtkTooltips *button_pho_tips = gtk_tooltips_new ();
          gtk_tooltips_set_tip (GTK_TOOLTIPS (button_pho_tips), button, pho_utf8, NULL);
          g_free(pho_utf8);
        }
      }

      g_signal_connect (G_OBJECT (button), "clicked",
         G_CALLBACK (cb_button_sym), str);
    }
  }

  gtk_widget_realize (gwin_sym);
  GdkWindow *gdkwin_sym = gwin_sym->window;
  gdk_window_set_override_redirect(gdkwin_sym, TRUE);

  gtk_widget_show_all(gwin_sym);
  move_win_sym();
}
