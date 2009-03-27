#include <sys/stat.h>
#include "gcin.h"
#include "pho.h"
#include "gtab.h"
#include "win-sym.h"

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

  if (!getenv("GCIN_TABLE_DIR"))
    get_gcin_user_fname(symbol_table, fname);
  else
    get_sys_table_file_name(symbol_table, fname);

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
      break;

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
void send_text_call_back(char *text);

static void cb_button_sym(GtkButton *button, char *str)
{
//  dbg("select %s\n", str);
  phokey_t pho[256];
  int len = strlen(str);

  bzero(pho, sizeof(pho));
  int i;
  for(i=0; i < len; i+=CH_SZ)
    big5_pho_chars(str+i, &pho[i/CH_SZ]);

  if (current_IC->in_method == 6)
    add_to_tsin_buf(str, pho, strlen(str) / CH_SZ);
  else
    send_text_call_back(str);
}

void update_active_in_win_geom();

void move_win_sym()
{
  gwin_sym = win_syms[current_IC->in_method];
#if 0
  dbg("move %d gwin_sym:%x\n", current_IC->in_method, gwin_sym);
#endif
  if (!gwin_sym)
    return;

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
  gwin_sym = win_syms[current_IC->in_method];

  if (!gwin_sym)
    return;
#if 0
  dbg("hide_win_sym %x\n", gwin_sym);
#endif
  gtk_widget_hide(gwin_sym);
}


void show_win_sym()
{
  if (!current_IC)
    return;

  gwin_sym = win_syms[current_IC->in_method];



  if (!gwin_sym || !win_sym_enabled)
    return;
#if 0
  dbg("show_win_sym %x\n", gwin_sym);
#endif
  gtk_widget_show(gwin_sym);
  move_win_sym(gwin_sym);
}


void lookup_gtab(char *ch, char out[]);
void str_to_all_phokey_chars(char *b5_str, char *out);

static void sym_lookup_key(char *instr, char *outstr)
{
  if (current_IC->in_method == 3 || current_IC->in_method == 6) {
    str_to_all_phokey_chars(instr, outstr);
  } else {

    outstr[0]=0;

    int i;
    for(i=0; i < strlen(instr); i+= CH_SZ) {
      char tt[512];

      lookup_gtab(instr, tt);
      strcat(outstr, tt);
      if (i < strlen(instr) - CH_SZ)
        strcat(outstr, " | ");
    }
  }
}

extern INMD *cur_inmd;

void create_win_sym()
{
#if 0
  dbg("create_win_sym ..\n", create_win_sym);
#endif
  if (current_IC)
    gwin_sym = win_syms[current_IC->in_method];

  if (current_IC->in_method != 3 && current_IC->in_method != 6 && !cur_inmd) {
    return;
  }

  if (read_syms()) {
    if (gwin_sym)
      gtk_widget_destroy(gwin_sym);
    win_syms[current_IC->in_method] = gwin_sym = NULL;
  } else {
    if (!syms)
      return;
  }

  if (gwin_sym) {
    win_sym_enabled^=1;

    if (win_sym_enabled) {
//      move_win_sym(gwin_sym);
      show_win_sym(gwin_sym);
    }
    else
      hide_win_sym(gwin_sym);

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
      char *str = psym->sym[j];

      if (!str[0])
         continue;

      GtkWidget *button = gtk_button_new();
      GtkWidget *label = gtk_label_new(str);
      gtk_container_add(GTK_CONTAINER(button), label);
      set_label_font_size(label, gcin_font_size_symbol);

      gtk_container_set_border_width (GTK_CONTAINER (button), 0);
      gtk_box_pack_start (GTK_BOX (hbox_row), button, FALSE, FALSE, 0);

      if (strlen(str) >= CH_SZ) {
        char phos[512];

        sym_lookup_key(str, phos);

        int phos_len = strlen(phos);

        if (phos_len) {
          GtkTooltips *button_pho_tips = gtk_tooltips_new ();
          gtk_tooltips_set_tip (GTK_TOOLTIPS (button_pho_tips), button, phos, NULL);
        }
      }

      g_signal_connect (G_OBJECT (button), "clicked",
         G_CALLBACK (cb_button_sym), str);
    }
  }

  gtk_widget_realize (gwin_sym);
  GdkWindow *gdkwin_sym = gwin_sym->window;
  gdk_window_set_override_redirect(gdkwin_sym, TRUE);

  if (current_IC)
    win_syms[current_IC->in_method] = gwin_sym;

  gtk_widget_show_all(gwin_sym);
  move_win_sym(gwin_sym);
#if 1
  dbg("in_method:%d\n", current_IC->in_method);
#endif
  return;
}
