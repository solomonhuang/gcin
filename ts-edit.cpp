#include "gcin.h"
#include "pho.h"
#include "config.h"
#if GCIN_i18n_message
#include <libintl.h>
#endif
#include "lang.h"
#include "tsin.h"
#include "gtab.h"
#include <gdk/gdkkeysyms.h>
#if GTK_CHECK_VERSION(2,90,7)
#include <gdk/gdkkeysyms-compat.h>
#endif

gboolean b_contrib;

void load_tsin_contrib();
char private_file_src[128];
char contributed_file_src[128];
char downloaded_file_src[128];

char txt[128];

extern char *current_tsin_fname;
typedef unsigned int u_int32_t;

#define PAGE_LEN 20

void init_TableDir();
void init_gtab(int inmdno);
gboolean init_tsin_table_fname(INMD *p, char *fname);
void load_tsin_db0(char *infname, gboolean is_gtab_i);
void load_tsin_db_ex(TSIN_HANDLE *ptsin_hand, char *infname, gboolean is_gtab_i, gboolean read_only, gboolean use_idx);

GtkWidget *vbox_top;
INMD *pinmd;
char gtab_tsin_fname[256];
char is_gtab;

char *phokey2pinyin(phokey_t k);
gboolean b_pinyin;
GtkWidget *hbox_buttons;
char current_str[MAX_PHRASE_LEN*CH_SZ+1];
extern gboolean is_chs;

GtkWidget *mainwin;

static int *ts_idx;
int tsN;
int page_ofs;
GtkWidget *labels[PAGE_LEN];
GtkWidget *button_check[PAGE_LEN];
GtkWidget *last_row, *find_textentry;
GtkWidget *scroll_bar;
int del_ofs[1024];
int del_ofsN;

void cp_ph_key(void *in, int idx, void *dest)
{
  if (ph_key_sz==2) {
    phokey_t *pharr = (phokey_t *)in;
    in = &pharr[idx];
  } else
  if (ph_key_sz==4) {
    u_int32_t *pharr4 = (u_int32_t *)in;
    in = &pharr4[idx];
  } else {
    u_int64_t *pharr8 = (u_int64_t *)in;
    in = &pharr8[idx];
  }

  memcpy(dest, in, ph_key_sz);
}

void *get_ph_key_ptr(void *in, int idx)
{
  if (ph_key_sz==2) {
    phokey_t *pharr = (phokey_t *)in;
    return &pharr[idx];
  } else
  if (ph_key_sz==4) {
    u_int32_t *pharr4 = (u_int32_t *)in;
    return &pharr4[idx];
  } else {
    u_int64_t *pharr8 = (u_int64_t *)in;
    return &pharr8[idx];
  }
}

int lookup_gtab_key(char *ch, void *out)
{
  int outN=0;
  INMD *tinmd = &inmd[default_input_method];

  int i;
  for(i=0; i < tinmd->DefChars; i++) {
    char *chi = (char *)tblch2(tinmd, i);

    if (!(chi[0] & 0x80))
      continue;
    if (!utf8_eq(chi, ch))
      continue;

    u_int64_t key = CONVT2(tinmd, i);
    if (ph_key_sz==4) {
      u_int32_t key32 = (u_int32_t)key;
      memcpy(get_ph_key_ptr(out, outN), &key32, ph_key_sz);
    } else
      memcpy(get_ph_key_ptr(out, outN), &key, ph_key_sz);
    outN++;
  }

  return outN;
}


static int qcmp_str(const void *aa, const void *bb)
{
  char *a = * (char **)aa, *b = * (char **)bb;

  return strcmp(a,b);
}

extern FILE *fph;

void load_ts_phrase()
{
  FILE *fp = tsin_hand.fph;

  int i;
  dbg("fname %s\n", current_tsin_fname);

  int ofs = is_gtab ? sizeof(TSIN_GTAB_HEAD):0;
  fseek(fp, ofs, SEEK_SET);

  tsN=0;
  free(ts_idx); ts_idx=NULL;

  while (!feof(fp)) {
    ts_idx = trealloc(ts_idx, int, tsN);
    ts_idx[tsN] = ftell(fp);
    u_int64_t phbuf[MAX_PHRASE_LEN];
    char chbuf[MAX_PHRASE_LEN * CH_SZ + 1];
    u_char clen;
    usecount_t usecount;
    clen = 0;

    fread(&clen,1,1,fp);

    if (clen > MAX_PHRASE_LEN)
      p_err("bad tsin db clen %d > MAX_PHRASE_LEN %d\n", clen, MAX_PHRASE_LEN);

    fread(&usecount,sizeof(usecount_t), 1, fp);
    fread(phbuf, ph_key_sz, clen, fp);
    int tlen = 0;

    for(i=0; i < clen; i++) {
      int n = fread(&chbuf[tlen], 1, 1, fp);
      if (n<=0)
        goto stop;
      int len=utf8_sz(&chbuf[tlen]);
      fread(&chbuf[tlen+1], 1, len-1, fp);
      tlen+=len;
    }

    if (clen < 2)
      continue;

    chbuf[tlen]=0;
    tsN++;
  }

  page_ofs = tsN - PAGE_LEN;
  if (page_ofs < 0)
    page_ofs = 0;

stop:
   dbg("load_ts_phrase\n");
//  fclose(fp);

}

int gtab_key2name(INMD *tinmd, u_int64_t key, char *t, int *rtlen);
void get_key_str(void *key, int idx, char *out_str)
{
  char t[128];
  char *phostr;

  if (is_gtab) {
     int tlen;
     u_int64_t key64;
     if (ph_key_sz == 4) {
       u_int32_t key32;
       cp_ph_key(key, idx, &key32);
       key64 = key32;
     } else
       cp_ph_key(key, idx, &key64);
     gtab_key2name(pinmd, key64, t, &tlen);
     phostr = t;
   } else {
     phokey_t k;
     cp_ph_key(key, idx, &k);
     phostr = b_pinyin?
     phokey2pinyin(k):phokey_to_str(k);
   }

   strcpy(out_str, phostr);
}

void load_tsin_entry0_ex(TSIN_HANDLE *ptsin_hand, char *len, usecount_t *usecount, void *pho, u_char *ch);

void load_tsin_at_ts_idx(int ts_row, char *len, usecount_t *usecount, void *pho, u_char *ch)
{
    int ofs = ts_idx[ts_row];
    fseek(tsin_hand.fph, ofs, SEEK_SET);

    load_tsin_entry0_ex(&tsin_hand, len, usecount, pho, ch);
}

void disp_page()
{
  if (!tsN)
    return;

  int li;
  for(li=0;li<PAGE_LEN;li++) {
    char line[256];
    line[0];
    int ts_row = page_ofs + li;
    if (ts_row >= tsN) {
      gtk_label_set_text(GTK_LABEL(labels[li]), "-");
      gtk_widget_hide(button_check[li]);
      continue;
    }


    u_int64_t phbuf[MAX_PHRASE_LEN];
    char chbuf[MAX_PHRASE_LEN * CH_SZ + 1];
    char clen;
    usecount_t usecount;
    int i;

    load_tsin_at_ts_idx(ts_row, &clen, &usecount, phbuf, (u_char *)chbuf);

    char *t;
    strcpy(line, t=g_markup_escape_text(chbuf, -1));
    g_free(t);
    strcat(line, " <span foreground=\"blue\">");


    char tt[512];

    for(i=0; i < clen; i++) {
      get_key_str(phbuf, i, tt);
//      dbg("tt %s\n", tt);
      strcat(line, t=g_markup_escape_text(tt, -1));
      g_free(t);
      strcat(line, " ");
    }

    sprintf(tt, " %d", usecount);
    strcat(line, tt);
    strcat(line, "</span>");

//    dbg("%s\n", line);
    gtk_label_set_markup(GTK_LABEL(labels[li]), line);
    gtk_widget_show(button_check[li]);
  }

  gtk_range_set_value(GTK_RANGE(scroll_bar), page_ofs);
}


void load_tsin_contrib();
void write_tsin_src(FILE *fw, char len, phokey_t *pho, char *s);

static void cb_button_delete(GtkButton *button, gpointer user_data)
{
  int i;
  if (b_contrib) {
  FILE *fw;

  if ((fw=fopen(private_file_src, "a"))==NULL)
    p_err("cannot write %s", private_file_src);

  for(i=0; i < PAGE_LEN; i++) {
    if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button_check[i])))
      continue;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_check[i]), FALSE);

    char len;
    usecount_t usecount;
    phokey_t pho[MAX_PHRASE_LEN];
    unsigned char str[MAX_PHRASE_STR_LEN];
    load_tsin_at_ts_idx(page_ofs+i, &len, &usecount, pho, str);

    write_tsin_src(fw, len, pho, (char*)str);
  }

  fclose(fw);
  load_tsin_contrib();
  load_ts_phrase();
} else {
  for(i=0; i < PAGE_LEN; i++) {
    if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button_check[i])))
      continue;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_check[i]), FALSE);

    del_ofs[del_ofsN++] = ts_idx[page_ofs+i];
    ts_idx[page_ofs+i]=-1;
  }

  int ntsN=0;

  for(i=0;i<tsN;i++)
    if (ts_idx[i]>=0)
      ts_idx[ntsN++]=ts_idx[i];
  tsN = ntsN;
}
  disp_page();
}

static void cb_button_find_ok(GtkButton *button, gpointer user_data)
{
  txt[0]=0;
  strcpy(txt, gtk_entry_get_text(GTK_ENTRY(find_textentry)));
//  gtk_widget_destroy(last_row);
//  last_row = NULL;
  if (!txt[0])
    return;
  int row;
  for(row=page_ofs+1; row < tsN; row++) {
    u_int64_t phbuf[MAX_PHRASE_LEN];
    char chbuf[MAX_PHRASE_LEN * CH_SZ + 1];
    char clen;
    usecount_t usecount;

    load_tsin_at_ts_idx(row, &clen, &usecount, phbuf, (u_char *)chbuf);
    if (strstr(chbuf, txt))
      break;
  }

  if (row==tsN) {
    GtkWidget *dia = gtk_message_dialog_new(NULL,GTK_DIALOG_MODAL,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,"%s not found",
      txt);
    gtk_dialog_run (GTK_DIALOG (dia));
    gtk_widget_destroy (dia);
  } else {
    page_ofs = row;
    disp_page();
  }
}

static void cb_button_close(GtkButton *button, gpointer user_data)
{
  if (last_row)
    gtk_widget_destroy(last_row);
  last_row = NULL;
  gtk_window_resize(GTK_WINDOW(mainwin), 1, 1);
}

static void cb_button_find(GtkButton *button, gpointer user_data)
{
  if (last_row)
    gtk_widget_destroy(last_row);

  last_row = gtk_hbox_new (FALSE, 0);
  GtkWidget *lab = gtk_label_new("Find");
  gtk_box_pack_start (GTK_BOX (last_row), lab, FALSE, FALSE, 0);
  find_textentry = gtk_entry_new();
  gtk_box_pack_start (GTK_BOX (last_row), find_textentry, FALSE, FALSE, 5);
  GtkWidget *button_ok = gtk_button_new_from_stock (GTK_STOCK_OK);
  g_signal_connect (G_OBJECT (button_ok), "clicked",
     G_CALLBACK (cb_button_find_ok), NULL);
  gtk_box_pack_start (GTK_BOX (last_row), button_ok, FALSE, FALSE, 5);

  GtkWidget *button_close = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  g_signal_connect (G_OBJECT (button_close), "clicked",
     G_CALLBACK (cb_button_close), NULL);
  gtk_box_pack_start (GTK_BOX (last_row), button_close, FALSE, FALSE, 5);


  gtk_box_pack_start (GTK_BOX (vbox_top), last_row, FALSE, FALSE, 0);

  gtk_entry_set_text(GTK_ENTRY(find_textentry), txt);

  gtk_widget_show_all(last_row);
}

static void cb_button_edit(GtkButton *button, gpointer user_data)
{
}

void ts_upload();

static void cb_button_save(GtkButton *button, gpointer user_data)
{
if (b_contrib) {
  ts_upload();
} else {
  int i;
  for(i=0;i<del_ofsN;i++) {
    fseek(tsin_hand.fph, del_ofs[i], SEEK_SET);
    char clen;
    fread(&clen, 1, 1, tsin_hand.fph);
    if (clen > 0) {
      clen = -clen;
      fseek(tsin_hand.fph, del_ofs[i], SEEK_SET);
      fwrite(&clen, 1, 1, tsin_hand.fph);
    }
  }
  fflush(tsin_hand.fph);

#if UNIX
  unix_exec(GCIN_BIN_DIR"/tsd2a32 %s -o tsin.tmp", current_tsin_fname);
  unix_exec(GCIN_BIN_DIR"/tsa2d32 tsin.tmp %s", current_tsin_fname);
#else
  win32exec_va("tsd2a32", current_tsin_fname, "-o", "tsin.tmp", NULL);
  win32exec_va("tsa2d32", "tsin.tmp",  current_tsin_fname, NULL);
#endif
}
  exit(0);
}

#define MAX_SAME_CHAR_PHO (16)

typedef struct {
  u_int64_t phokeys[MAX_SAME_CHAR_PHO];
  int phokeysN;
  GtkWidget *opt_menu;
} char_pho;



static char_pho bigpho[MAX_PHRASE_LEN];
static int bigphoN;

static GtkWidget *hbox_pho_sel;

void destroy_pho_sel_area()
{
  gtk_widget_destroy(hbox_pho_sel);
}

static void cb_button_ok(GtkButton *button, gpointer user_data)
{
  u_int64_t pharr8[MAX_PHRASE_LEN];

  int i;
  for(i=0; i < bigphoN; i++) {
    int idx = gtk_combo_box_get_active(GTK_COMBO_BOX(bigpho[i].opt_menu));
    void *dest = get_ph_key_ptr(pharr8, i);

    cp_ph_key(bigpho[i].phokeys, idx, dest);
  }

  save_phrase_to_db(pharr8, current_str, bigphoN, 0);

  destroy_pho_sel_area();
}

static void cb_button_cancel(GtkButton *button, gpointer user_data)
{
}

int gtab_key2name(INMD *tinmd, u_int64_t key, char *t, int *rtlen);
GtkWidget *create_pho_sel_area()
{
  hbox_pho_sel = gtk_hbox_new (FALSE, 0);

  int i;

  for(i=0; i < bigphoN; i++) {
    bigpho[i].opt_menu = gtk_combo_box_new_text ();
#if !GTK_CHECK_VERSION(2,4,0)
    GtkWidget *menu = gtk_menu_new ();
#endif
    gtk_box_pack_start (GTK_BOX (hbox_pho_sel), bigpho[i].opt_menu, FALSE, FALSE, 0);

    int j;
    for(j=0; j < bigpho[i].phokeysN; j++) {
      char t[128];
      char *phostr;

      if (is_gtab) {
        int tlen;
        u_int64_t key64;
        if (ph_key_sz == 4) {
          u_int32_t key32;
          cp_ph_key(bigpho[i].phokeys,j, &key32);
          key64 = key32;
        } else
          cp_ph_key(bigpho[i].phokeys,j, &key64);

        gtab_key2name(pinmd, key64, t, &tlen);
//        dbg("%d,%d] %s\n", i,j, t);
        phostr = t;
      } else {
        phokey_t k;
        cp_ph_key(bigpho[i].phokeys, j, &k);
        phostr = b_pinyin?
        phokey2pinyin(k):phokey_to_str(k);
      }

#if GTK_CHECK_VERSION(2,4,0)
      gtk_combo_box_append_text (GTK_COMBO_BOX_TEXT (bigpho[i].opt_menu), phostr);
#else
      GtkWidget *item = gtk_menu_item_new_with_label (phostr);
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
#endif
    }

#if GTK_CHECK_VERSION(2,4,0)
    gtk_combo_box_set_active (GTK_COMBO_BOX(bigpho[i].opt_menu), 0);
#else
    gtk_option_menu_set_menu (GTK_OPTION_MENU (bigpho[i].opt_menu), menu);
#endif

  }


  GtkWidget *button_ok = gtk_button_new_with_label("OK to add");
  gtk_box_pack_start (GTK_BOX (hbox_pho_sel), button_ok, FALSE, FALSE, 20);
  g_signal_connect (G_OBJECT (button_ok), "clicked",
     G_CALLBACK (cb_button_ok), NULL);

  GtkWidget *button_cancel = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  gtk_box_pack_start (GTK_BOX (hbox_pho_sel), button_cancel, FALSE, FALSE, 20);
  g_signal_connect (G_OBJECT (button_cancel), "clicked",
     G_CALLBACK (cb_button_cancel), NULL);

  return hbox_pho_sel;
}


static void cb_button_add(GtkButton *button, gpointer user_data)
{
  bigphoN = 0;
  char *p = current_str;
  while (*p) {
    char_pho *pbigpho = &bigpho[bigphoN++];

    if (ph_key_sz==2) {
      pbigpho->phokeysN = utf8_pho_keys(p, (phokey_t*)pbigpho->phokeys);
    } else {
      pbigpho->phokeysN = lookup_gtab_key(p, pbigpho->phokeys);
    }

    p+=utf8_sz(p);

    if (!pbigpho->phokeysN) {
      dbg(" no mapping to pho\n");
      return;
    }
  }

  GtkWidget *sel =  create_pho_sel_area();
  gtk_box_pack_start (GTK_BOX (hbox_buttons), sel, FALSE, FALSE, 20);

  gtk_widget_show_all(hbox_buttons);

}

Display *dpy;

void do_exit()
{
  send_gcin_message(
#if UNIX
    dpy,
#endif
     RELOAD_TSIN_DB);

  exit(0);
}

void load_tsin_db();
void set_window_gcin_icon(GtkWidget *window);
#if WIN32
void init_gcin_program_files();
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif

static gboolean  scroll_event(GtkWidget *widget,GdkEventScroll *event, gpointer user_data)
{
  dbg("scroll_event\n");

  if (event->direction!=GDK_SCROLL_DOWN)
    return TRUE;

  return FALSE;
}

gboolean key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
  if (last_row)
    return FALSE;
//  dbg("key_press_event %x\n", event->keyval);
  switch (event->keyval) {
    case GDK_Up:
      if (page_ofs>0)
        page_ofs--;
      break;
    case GDK_Down:
      if (page_ofs<tsN-1)
        page_ofs++;
      break;
    case GDK_Page_Up:
      page_ofs -= PAGE_LEN;
      if (page_ofs < 0)
        page_ofs = 0;
      break;
    case GDK_Page_Down:
      page_ofs += PAGE_LEN;
      if (page_ofs>= tsN)
        page_ofs = tsN-1;
      break;
    case GDK_Home:
      page_ofs = 0;
      break;
    case GDK_End:
      page_ofs = tsN - PAGE_LEN;
      break;
  }

  disp_page();

  return TRUE;
}

gboolean is_pinyin_kbm();

#if WIN32
#include <direct.h>
#endif

gboolean  cb_scroll_bar(GtkRange *range, GtkScrollType scroll,
                        gdouble value, gpointer user_data)
{
  page_ofs = (int)value;
  if (page_ofs > tsN - PAGE_LEN)
    page_ofs = tsN - PAGE_LEN;
  if (page_ofs < 0)
    page_ofs = 0;
  disp_page();
  return TRUE;
}

static void gen_bin(char *name, char *full_src, char *minus)
{
  // doesn't have to use full path because chdir(gcin user dir)
  minus[0]=0;
  char src[128];
  strcat(strcpy(src, name),".src");

  dbg("gen_bin %s %s\n", name, src);

  if (get_gcin_user_fname(src, full_src)) {
    dbg("%s %s\n", name, full_src);
#if UNIX
    putenv("GCIN_NO_RELOAD=");
    unix_exec(GCIN_BIN_DIR"/tsa2d32 %s %s", src, name);
#else
    _putenv("GCIN_NO_RELOAD=Y");
    win32exec_va("tsa2d32", src, name, NULL);
#endif
    sprintf(minus, " -minus %s", name);
  } else
    dbg("not exist %s\n", full_src);
}


void free_tsin();
#if WIN32
int win32exec_argv(char *s, int argc, char *argv[]);
#endif
void load_tsin_contrib()
{
  char minus_priv[128];
#define PRIVATE_PHRASES "private-phrases"
  gen_bin(PRIVATE_PHRASES, private_file_src, minus_priv);

#define CONTRIBUTED_PHRASES "contributed-phrases"
  char minus_contributed[128];
  gen_bin(CONTRIBUTED_PHRASES, contributed_file_src, minus_contributed);

#define DOWNLOADED_PHRASES "downloaded-phrases"
  char minus_downloaded[128];
  gen_bin(DOWNLOADED_PHRASES, downloaded_file_src, minus_downloaded);

  char sys_tsfname[128], contrib_temp[128];
  get_sys_table_file_name(tsin32_f, sys_tsfname);
  get_gcin_user_fname("contrib-temp", contrib_temp);
#if UNIX
  unix_exec(GCIN_BIN_DIR"/tsd2a32 -b -minus %s -o %s%s%s%s",
    sys_tsfname, contrib_temp, minus_priv, minus_contributed, minus_downloaded);
#else
  char *argv[32];
  int argc=0;
  argv[argc++]="-b";

  argv[argc++]="-o";
  argv[argc++]=contrib_temp;

  argv[argc++]="-minus";
  argv[argc++]=sys_tsfname;

  if (minus_priv[0]) {
    argv[argc++]="-minus";
    argv[argc++]=PRIVATE_PHRASES;
  }
  if (minus_contributed[0]) {
    argv[argc++]="-minus";
    argv[argc++]=CONTRIBUTED_PHRASES;
  }
  if (minus_downloaded[0]) {
    argv[argc++]="-minus";
    argv[argc++]=DOWNLOADED_PHRASES;
  }

  win32exec_argv("tsd2a32.exe", argc, argv);
#endif
  free_tsin();
  load_tsin_db0(contrib_temp, FALSE);
}


void ts_download();

static void cb_button_download(GtkButton *button, gpointer user_data)
{
  ts_download();
  load_ts_phrase();
  gtk_range_set_range(GTK_RANGE(scroll_bar), 0, tsN);
  disp_page();
}

int main(int argc, char **argv)
{
#if WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
#endif

  dbg("%s\n", argv[0]);
  b_contrib = strstr(argv[0], "ts-contribute")!=NULL;

  if (b_contrib)
    dbg("b_contrib\n");

  set_is_chs();
  init_TableDir();
  b_pinyin = is_pinyin_kbm();

  gtk_init (&argc, &argv);
  load_setttings();
  load_gtab_list(TRUE);

  char gcin_dir[512];
  get_gcin_dir(gcin_dir);

  if (argc < 2) {
#if UNIX
  chdir(gcin_dir);
#else
  _chdir(gcin_dir);
#endif
  }

#if GCIN_i18n_message
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);
#endif

  get_gcin_user_fname(DOWNLOADED_PHRASES, downloaded_file_src);
  strcat(downloaded_file_src, ".src");

  pinmd = &inmd[default_input_method];

  if (pinmd->method_type == method_type_TSIN) {
    dbg("is tsin\n");
    pho_load();
    if (b_contrib) {
      load_tsin_contrib();
    } else {
      if (argc > 1)
        load_tsin_db_ex(&tsin_hand, argv[1], FALSE, FALSE, FALSE);
      else
        load_tsin_db();
    }

    ph_key_sz = 2;
  } else
  if (pinmd->filename) {
    if (b_contrib)
      p_err("Currently %s only supports tsin", argv[0]);

    dbg("gtab filename %s\n", pinmd->filename);
    init_gtab(default_input_method);
    is_gtab = TRUE;
    init_tsin_table_fname(pinmd, gtab_tsin_fname);
    load_tsin_db0(gtab_tsin_fname, TRUE);
  } else
    p_err("Your default input method %s doesn't use phrase database",
      pinmd->cname);

  dbg("ph_key_sz: %d\n", ph_key_sz);

#if UNIX
  dpy = GDK_DISPLAY();
#endif

  load_ts_phrase();

  mainwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(mainwin), GTK_WIN_POS_CENTER);

  g_signal_connect (G_OBJECT (mainwin), "key-press-event",
                   G_CALLBACK (key_press_event), NULL);

  gtk_window_set_has_resize_grip(GTK_WINDOW(mainwin), FALSE);
//  gtk_window_set_default_size(GTK_WINDOW (mainwin), 640, 520);
  set_window_gcin_icon(mainwin);

  vbox_top = gtk_vbox_new (FALSE, 0);
  gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox_top), GTK_ORIENTATION_VERTICAL);
  gtk_container_add (GTK_CONTAINER(mainwin), vbox_top);

  GtkWidget *hbox_page = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_page, TRUE, FALSE, 0);

  GtkWidget *align_page = gtk_alignment_new(0, 0, 1.0, 1.0);
  gtk_box_pack_start (GTK_BOX (hbox_page), align_page, TRUE, TRUE, 0);

  GtkWidget *vbox_page = gtk_vbox_new (TRUE, 0);
  gtk_container_add (GTK_CONTAINER(align_page), vbox_page);

  scroll_bar = gtk_vscrollbar_new(
  GTK_ADJUSTMENT(gtk_adjustment_new(tsN - PAGE_LEN,0, tsN, 1,PAGE_LEN,PAGE_LEN)));
  gtk_box_pack_start (GTK_BOX (hbox_page), scroll_bar, FALSE, FALSE, 0);

  g_signal_connect(G_OBJECT(scroll_bar), "change-value", G_CALLBACK(cb_scroll_bar), NULL);

  int i;
  for(i=0;i<PAGE_LEN;i++) {
    GtkWidget *hbox;
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (vbox_page), hbox, TRUE, TRUE, 0);
    button_check[i] = gtk_check_button_new();
    gtk_box_pack_start (GTK_BOX (hbox), button_check[i], FALSE, FALSE, 0);

    labels[i]=gtk_label_new(NULL);
#if 0
    g_signal_connect (G_OBJECT (labels[i]), "scroll-event",
                      G_CALLBACK (scroll_event), NULL);
#endif
    GtkWidget *align = gtk_alignment_new (0, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), labels[i]);
    gtk_box_pack_start (GTK_BOX (hbox), align, FALSE, FALSE, 0);
  }


  hbox_buttons = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_buttons, FALSE, FALSE, 0);

  GtkWidget *button_delete = b_contrib ? gtk_button_new_with_label(_(_L("私密詞不上載"))):gtk_button_new_from_stock (GTK_STOCK_DELETE);
  gtk_box_pack_start (GTK_BOX (hbox_buttons), button_delete, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_delete), "clicked",
     G_CALLBACK (cb_button_delete), NULL);

  GtkWidget *button_find = gtk_button_new_from_stock (GTK_STOCK_FIND);
  gtk_box_pack_start (GTK_BOX (hbox_buttons), button_find, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_find), "clicked",
     G_CALLBACK (cb_button_find), NULL);

#if 0
  GtkWidget *button_edit = gtk_button_new_from_stock (GTK_STOCK_EDIT);
  gtk_box_pack_start (GTK_BOX (hbox_buttons), button_edit, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_edit), "clicked",
     G_CALLBACK (cb_button_edit), NULL);
#endif

  GtkWidget *button_save =  b_contrib ? gtk_button_new_with_label(_(_L("上載詞")))
  :gtk_button_new_from_stock (GTK_STOCK_SAVE);

  gtk_box_pack_start (GTK_BOX (hbox_buttons), button_save, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_save), "clicked",
     G_CALLBACK (cb_button_save), NULL);


  GtkWidget *button_quit = gtk_button_new_from_stock (GTK_STOCK_QUIT);
  gtk_box_pack_start (GTK_BOX (hbox_buttons), button_quit, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_quit), "clicked",
     G_CALLBACK (do_exit), NULL);

  if (!b_contrib && !is_gtab) {
    GtkWidget *button_download = gtk_button_new_with_label(_(_L("下載共享詞庫")));
    gtk_box_pack_start (GTK_BOX (hbox_buttons), button_download, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (button_download), "clicked",
       G_CALLBACK (cb_button_download), NULL);
  }

  g_signal_connect (G_OBJECT (mainwin), "delete_event",
                    G_CALLBACK (do_exit), NULL);

  gtk_widget_show_all(mainwin);

//  load_ts_phrase();

  disp_page();

  gtk_main();
  return 0;
}
