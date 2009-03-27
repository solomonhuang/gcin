#include "gcin.h"
#include "pho.h"

GtkWidget *hbox_buttons;
char current_str[MAX_PHRASE_LEN*CH_SZ+1];


GtkWidget *mainwin;
GtkTextBuffer *buffer;

static char **phrase;
static int phraseN=0;

static int qcmp_str(const void *aa, const void *bb)
{
  char *a = * (char **)aa, *b = * (char **)bb;

  return strcmp(a,b);
}


void load_ts_phrase()
{
  FILE *fp;
  char fname[256];

  int i;
  for(i=0; i < phraseN; i++) {
    free(phrase[i]);
  }
  free(phrase); phrase = NULL;
  phraseN = 0;

  get_gcin_dir(fname);
  strcat(fname,"/tsin");

  if ((fp=fopen(fname, "r"))==NULL) {
    printf("Cannot open %s", fname);
    exit(-1);
  }


  while (!feof(fp)) {
    phokey_t phbuf[MAX_PHRASE_LEN];
    char chbuf[MAX_PHRASE_LEN * CH_SZ + 1];
    u_char clen;
    char usecount;

    clen = 0;

    fread(&clen,1,1,fp);

    if (clen > MAX_PHRASE_LEN)
      p_err("bad tsin db clen %d > MAX_PHRASE_LEN %d\n", clen, MAX_PHRASE_LEN);

    fread(&usecount,1,1,fp);
    fread(phbuf,sizeof(phokey_t), clen, fp);
    fread(chbuf, CH_SZ, clen,fp);

    if (clen < 2)
      continue;

    chbuf[clen * CH_SZ]=0;
    phrase = trealloc(phrase, char *, phraseN+1);

    if (strlen(chbuf) < CH_SZ*2)
      p_err("error %d] %s clen:%d", phraseN, chbuf, clen);

    phrase[phraseN++] = strdup(chbuf);
  }

  fclose(fp);

  qsort(phrase, phraseN, sizeof(char *), qcmp_str);

  dbg("phraseN: %d\n", phraseN);
}

gboolean pharse_search(char *s)
{
  return bsearch(&s, phrase, phraseN, sizeof(char *), qcmp_str) != NULL;
}

void all_wrap()
{
  GtkTextIter mstart,mend;

  gtk_text_buffer_get_bounds (buffer, &mstart, &mend);
  gtk_text_buffer_apply_tag_by_name (buffer, "char_wrap", &mstart, &mend);
}


static void cb_button_parse(GtkButton *button, gpointer user_data)
{
  int char_count = gtk_text_buffer_get_char_count (buffer);

  int i;

  load_ts_phrase();

  char_count = gtk_text_buffer_get_char_count (buffer);

  all_wrap();

  dbg("parse char_count:%d\n", char_count);

  for(i=0; i < char_count; ) {
    int len;

    for(len=MAX_PHRASE_LEN; len>=2 ; len--) {
      u_char txt[MAX_PHRASE_LEN*CH_SZ + 1];
      int txtN=0;

      gboolean b_ignore = FALSE;
      int k;
      for(k=0; k<len && i+k < char_count; k++) {
        GtkTextIter start,end;
        gtk_text_buffer_get_iter_at_offset (buffer, &start, i+k);
        gtk_text_buffer_get_iter_at_offset (buffer, &end, i+k+1);
        char *utf8 = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

        if (!(utf8[0] & 128))
          b_ignore = TRUE;

        int wn = strlen(utf8);

        memcpy(&txt[txtN], utf8, wn);

        txtN+= wn;
      }

      if (b_ignore || txtN < CH_SZ*2)
        continue;

      txt[txtN] = 0;
//      dbg("try len:%d txtN:%d %s\n", len, txtN, txt);
      if (!pharse_search(txt))
        continue;

//      dbg("match .... %d %d\n", i, len);

      GtkTextIter mstart,mend;

      gtk_text_buffer_get_iter_at_offset (buffer, &mstart, i);
      gtk_text_buffer_get_iter_at_offset (buffer, &mend, i+len);
      gtk_text_buffer_apply_tag_by_name (buffer, "blue_background", &mstart, &mend);
#if 1
      // why do I have to repeat this
      gtk_text_buffer_get_iter_at_offset (buffer, &mstart, i);
      gtk_text_buffer_get_iter_at_offset (buffer, &mend, i+len);
      gtk_text_buffer_apply_tag_by_name (buffer, "blue_background", &mstart, &mend);
#endif
      gdk_flush();
    }

    i+=len;
  }
}

#define MAX_SAME_CHAR_PHO (16)

typedef struct {
  phokey_t phokeys[MAX_SAME_CHAR_PHO];
  int phokeysN;
  GtkWidget *opt_menu;
} big5char_pho;

static big5char_pho bigpho[MAX_PHRASE_LEN];
static int bigphoN;

static GtkWidget *hbox_pho_sel;

GtkWidget *destroy_pho_sel_area()
{
  gtk_widget_destroy(hbox_pho_sel);
}

static void cb_button_ok(GtkButton *button, gpointer user_data)
{
  phokey_t pharr[MAX_PHRASE_LEN];

  int i;

  for(i=0; i < bigphoN; i++) {
    int idx = gtk_option_menu_get_history(GTK_OPTION_MENU(bigpho[i].opt_menu));
    pharr[i] = bigpho[i].phokeys[idx];
  }

  save_phrase_to_db(pharr, current_str, bigphoN);

  destroy_pho_sel_area();

  GtkTextMark *selebound =  gtk_text_buffer_get_selection_bound(buffer);
  gtk_text_mark_set_visible(selebound, FALSE);

  cb_button_parse(NULL, NULL);

}

static void cb_button_cancel(GtkButton *button, gpointer user_data)
{
  destroy_pho_sel_area();
}


GtkWidget *create_pho_sel_area()
{
  hbox_pho_sel = gtk_hbox_new (FALSE, 0);

  int i;

  for(i=0; i < bigphoN; i++) {
    bigpho[i].opt_menu = gtk_option_menu_new ();
    gtk_box_pack_start (GTK_BOX (hbox_pho_sel), bigpho[i].opt_menu, FALSE, FALSE, 0);
    GtkWidget *menu = gtk_menu_new ();

    int j;
    for(j=0; j < bigpho[i].phokeysN; j++) {
      char *phostr = phokey_to_str(bigpho[i].phokeys[j]);

      GtkWidget *item = gtk_menu_item_new_with_label (phostr);
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
    }

    gtk_option_menu_set_menu (GTK_OPTION_MENU (bigpho[i].opt_menu), menu);

  }


  GtkWidget *button_ok = gtk_button_new_with_label("OK to add");
  gtk_box_pack_start (GTK_BOX (hbox_pho_sel), button_ok, FALSE, FALSE, 20);
  g_signal_connect (G_OBJECT (button_ok), "clicked",
     G_CALLBACK (cb_button_ok), NULL);

  GtkWidget *button_cancel = gtk_button_new_with_label("cancel");
  gtk_box_pack_start (GTK_BOX (hbox_pho_sel), button_cancel, FALSE, FALSE, 20);
  g_signal_connect (G_OBJECT (button_cancel), "clicked",
     G_CALLBACK (cb_button_cancel), NULL);


  return hbox_pho_sel;
}




static void cb_button_add(GtkButton *button, gpointer user_data)
{
  GtkTextIter start, end;

  if (!gtk_text_buffer_get_selection_bounds(buffer, &start, &end))
    return;

  char *utf8 = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
  strcpy(current_str, utf8);
  g_free(utf8);

  int current_strN = strlen(utf8);

  bigphoN = 0;
  int i;
  for(i=0; i < current_strN; i+=CH_SZ) {
    phokey_t phokey[64];
    big5char_pho *pbigpho = &bigpho[bigphoN++];

    pbigpho->phokeysN = big5_pho_chars(&current_str[i], pbigpho->phokeys);

    if (!pbigpho->phokeysN) {
      dbg(" no mapping to pho\n");
      return;
    }

    dbg(" phokeyN: %d", bigpho[i].phokeysN);
  }

  dbg("\n");

  GtkWidget *sel =  create_pho_sel_area();
  gtk_box_pack_start (GTK_BOX (hbox_buttons), sel, FALSE, FALSE, 20);

  gtk_widget_show_all(hbox_buttons);

}

Display *dpy;

void do_exit()
{
  send_gcin_message(dpy, "reload");

  exit(0);
}


int main(int argc, char **argv)
{
  gtk_init (&argc, &argv);

  pho_load();
  load_tsin_db();
  dpy = GDK_DISPLAY();

  mainwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW (mainwin), 640, 220);

  GtkWidget *sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
                                  GTK_POLICY_AUTOMATIC,
                                  GTK_POLICY_AUTOMATIC);

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER(mainwin), vbox_top);

  GtkWidget *view = gtk_text_view_new ();
  gtk_container_add (GTK_CONTAINER(sw), view);

  gtk_box_pack_start (GTK_BOX (vbox_top), sw, TRUE, TRUE, 0);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));

  char *text = "按滑鼠中鍵, 貼上你要 tslearn 學習的文章。";

  gtk_text_buffer_set_text (buffer, text, -1);

  gtk_text_buffer_create_tag (buffer,
     "blue_background", "background", "blue", "foreground", "white", NULL);

  gtk_text_buffer_create_tag (buffer, "char_wrap",
			      "wrap_mode", GTK_WRAP_CHAR, NULL);

  hbox_buttons = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_buttons, FALSE, FALSE, 0);

  GtkWidget *button_parse = gtk_button_new_with_label("Mark");
  gtk_box_pack_start (GTK_BOX (hbox_buttons), button_parse, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_parse), "clicked",
     G_CALLBACK (cb_button_parse), NULL);

  GtkWidget *button_add = gtk_button_new_with_label("Add phrase");
  gtk_box_pack_start (GTK_BOX (hbox_buttons), button_add, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_add), "clicked",
     G_CALLBACK (cb_button_add), NULL);


  g_signal_connect (G_OBJECT (mainwin), "delete_event",
                    G_CALLBACK (do_exit), NULL);

  all_wrap();

  gtk_widget_show_all(mainwin);

  gtk_main();
}
