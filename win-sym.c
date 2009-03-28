#include <sys/stat.h>
#include "gcin.h"
#include "pho.h"
#include "gtab.h"
#include "win-sym.h"

static GtkWidget *gwin_sym = NULL;
static int cur_in_method;

typedef struct {
  char **sym;
  int symN;
} SYM_ROW;

static SYM_ROW *syms;
static int symsN;

typedef struct {
  SYM_ROW *syms;
  int symsN;
} PAGE;

static PAGE *pages;
static int pagesN;
static int idx;

extern char *TableDir;

FILE *watch_fopen(char *filename, time_t *pfile_modify_time)
{
  FILE *fp;
  char fname[256];

  if (!getenv("GCIN_TABLE_DIR"))
    get_gcin_user_fname(filename, fname);
  else
    get_sys_table_file_name(filename, fname);

  if ((fp=fopen(fname, "r"))==NULL) {
    strcat(strcat(strcpy(fname, TableDir), "/"), filename);

    if ((fp=fopen(fname, "r"))==NULL)
     return NULL;
  }

  struct stat st;
  fstat(fileno(fp), &st);

  if (st.st_mtime == *pfile_modify_time) {
    fclose(fp);
    return NULL;
  }

  *pfile_modify_time = st.st_mtime;
  return fp;
}

static void save_page()
{
  if (!symsN)
    return;

  pages=trealloc(pages, PAGE, pagesN+1);
  pages[pagesN].syms = syms;
  pages[pagesN].symsN = symsN;
  pagesN++;
  syms = NULL;
  symsN = 0;
}


static gboolean read_syms()
{
  FILE *fp;
  static char symbol_table[] = "symbol-table";
  static time_t file_modify_time;

  if ((fp=watch_fopen(symbol_table, &file_modify_time))==NULL)
    return FALSE;

  int pg;
  for(pg=0; pg < pagesN; pg++) {
    syms = pages[pg].syms;
    symsN = pages[pg].symsN;

    int i;
    for(i=0; i < symsN; i++) {
      int j;
      for(j=0; j < syms[i].symN; j++)
        if (syms[i].sym[j])
          free(syms[i].sym[j]);
    }
    free(syms);
  }
  pagesN = 0; pages = NULL;
  syms = NULL; symsN = 0;


  while (!feof(fp)) {
    char tt[1024];

    bzero(tt, sizeof(tt));
    fgets(tt, sizeof(tt), fp);
//    dbg(tt);
    int len=strlen(tt);

    if (!len)
      continue;

    if (tt[len-1]=='\n') {
      tt[len-1]=0;
      if (tt[0]==0)
        save_page();
    } else
      break;

    if (tt[0]=='#')
      continue;

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

    if (!psym->symN) {
      free(syms);
      syms=NULL;
      symsN=0;
    }
  }

  if (symsN)
    save_page();

  fclose(fp);

  idx = 0;
  syms = pages[idx].syms;
  symsN = pages[idx].symsN;

  return TRUE;
}


gboolean add_to_tsin_buf(char *str, phokey_t *pho, int len);
void send_text_call_back(char *text);
void tsin_reset_in_pho(), reset_gtab_all(), clr_in_area_pho();

static void cb_button_sym(GtkButton *button, char *str)
{
  phokey_t pho[256];
  bzero(pho, sizeof(pho));

  if (current_CS->in_method == 6 && current_CS->im_state != GCIN_STATE_DISABLED) {
    add_to_tsin_buf(str, pho, utf8_str_N(str));
  }
  else
    send_text_call_back(str);

  switch (current_CS->in_method) {
    case 3:
       clr_in_area_pho();
       break;
    case 6:
       tsin_reset_in_pho();
       break;
    default:
       reset_gtab_all();
       break;
  }
}

void update_active_in_win_geom();
extern int win_status_y;

void move_win_sym()
{
#if 0
  dbg("move_win_sym %d\n", current_CS->in_method);
#endif
  if (!gwin_sym)
    return;

  int wx, wy;
#if 0
  if (gcin_pop_up_win) {
    wx = dpy_xl;
  } else
#endif
  {
  //  dbg("win_y: %d  %d\n", win_y, win_yl);
    update_active_in_win_geom();

    wx = win_x; wy = win_y + win_yl;
  }

  int winsym_xl, winsym_yl;
  get_win_size(gwin_sym, &winsym_xl, &winsym_yl);

  if (wx + winsym_xl > dpy_xl)
    wx = dpy_xl - winsym_xl;
  if (wx < 0)
    wx = 0;

#if 0
  if (gcin_pop_up_win) {
    wy = win_status_y - winsym_yl;
  } else
#endif
  {
    if (wy + winsym_yl > dpy_yl)
      wy = win_y - winsym_yl;
    if (wy < 0)
      wy = 0;
  }

  gtk_window_move(GTK_WINDOW(gwin_sym), wx, wy);
}

static gboolean win_sym_enabled=0;

void hide_win_sym()
{
  if (!gwin_sym)
    return;

  gtk_widget_hide(gwin_sym);
}

void show_win_sym()
{
  if (!current_CS)
    return;

  if (!gwin_sym || !win_sym_enabled || current_CS->im_state == GCIN_STATE_DISABLED)
    return;
#if 0
  dbg("show_win_sym\n");
#endif
  gtk_widget_show_all(gwin_sym);
  move_win_sym(gwin_sym);
}


void lookup_gtab(char *ch, char out[]);
void str_to_all_phokey_chars(char *b5_str, char *out);

static void sym_lookup_key(char *instr, char *outstr)
{
  if (current_CS->in_method == 3 || current_CS->in_method == 6) {
    str_to_all_phokey_chars(instr, outstr);
  } else {
    outstr[0]=0;

    while (*instr) {
      char tt[512];

      lookup_gtab(instr, tt);
      strcat(outstr, tt);

      instr+= utf8_sz(instr);

      if (*instr)
          strcat(outstr, " | ");
    }
  }
}

static void destory_win()
{
  if (gwin_sym)
    gtk_widget_destroy(gwin_sym);
  gwin_sym = NULL;
}


gboolean  button_scroll_event(GtkWidget *widget,GdkEventScroll *event, gpointer user_data)
{
  int winx,winy,i;

  if (pagesN < 2)
    return;

  switch (event->direction) {
    case GDK_SCROLL_UP:
      idx--;
      if (idx < 0)
        idx = pagesN - 1;
      break;
    case GDK_SCROLL_DOWN:
      idx = (idx+1) % pagesN;
      break;
  }

  syms = pages[idx].syms;
  symsN = pages[idx].symsN;
  destory_win();
  win_sym_enabled = 0;
  create_win_sym();
}


void create_win_sym()
{
  if (!current_CS) {
    dbg("create_win_sym, null CS\n");
    return;
  }

  if (current_CS->in_method < 0 || current_CS->in_method >= MAX_GTAB_NUM_KEY) {
    p_err("bad current_CS %d\n", current_CS->in_method);
  }

  if (current_CS->in_method != 3 && current_CS->in_method != 6 && !cur_inmd)
    return;

  if (read_syms() || cur_in_method != current_CS->in_method) {
    destory_win();
  } else {
    if (!syms)
      return;
  }

  win_sym_enabled^=1;

  if (gwin_sym) {
    if (win_sym_enabled) {
      show_win_sym(gwin_sym);
    }
    else
      hide_win_sym(gwin_sym);

    return;
  }

  gwin_sym = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  cur_in_method = current_CS->in_method;

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

      if (utf8_str_N(str) > 0) {
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

  if (win_sym_enabled)
    gtk_widget_show_all(gwin_sym);

  g_signal_connect (G_OBJECT (gwin_sym), "scroll-event",
    G_CALLBACK (button_scroll_event), NULL);

 move_win_sym(gwin_sym);
#if 0
  dbg("in_method:%d\n", current_CS->in_method);
#endif
  return;
}


void change_win_sym_font_size()
{
}
