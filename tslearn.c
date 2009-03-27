#include "gcin.h"
#include "pho.h"

GtkWidget *hbox_buttons;
char current_str[MAX_PHRASE_LEN*2+1];


GtkWidget *mainwin;
GtkTextBuffer *buffer;

static char **phrase;
static int phraseN=0;

static int qcmp_str(const void *aa, const void *bb)
{
  char *a = * (char **)aa, *b = * (char **)bb;

  return strcmp(a,b);
}


void load_phrase()
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
    char chbuf[MAX_PHRASE_LEN * 2 + 1];
    u_char clen;
    char usecount;

    clen = 0;

    fread(&clen,1,1,fp);
    fread(&usecount,1,1,fp);
    fread(phbuf,sizeof(phokey_t), clen, fp);
    fread(chbuf,2,clen,fp);

    if (clen < 2)
      continue;

    chbuf[clen*2]=0;
    phrase = realloc(phrase, sizeof(char *) * (phraseN+1));

    if (strlen(chbuf) < 4)
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

  load_phrase();
#if 0
  static gboolean compacted = FALSE;
  if (!compacted) {
    for(i=0; i < char_count; ) {
      GtkTextIter start, end;

      gtk_text_buffer_get_iter_at_offset (buffer, &start, i);
      gtk_text_buffer_get_iter_at_offset (buffer, &end, i+1);

      char *utf8 = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
      if (utf8[0]=='\n') {
        gtk_text_buffer_delete(buffer, &start, &end);
        char_count--;
      } else
        i++;

      g_free(utf8);
    }

    compacted = TRUE;
  }
#endif

  char_count = gtk_text_buffer_get_char_count (buffer);

  all_wrap();

  dbg("parse char_count:%d\n", char_count);

  for(i=0; i < char_count; ) {
    int len;

    for(len=MAX_PHRASE_LEN; len>=2 ; len--) {
      u_char txt[MAX_PHRASE_LEN*2 + 1];
      int txtN=0;

      gboolean b_ignore = FALSE;
      int k;
      for(k=0; k<len && i+k < char_count; k++) {
        GtkTextIter start,end;
        gtk_text_buffer_get_iter_at_offset (buffer, &start, i+k);
        gtk_text_buffer_get_iter_at_offset (buffer, &end, i+k+1);
        char *utf8 = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
        int rn, wn;
        GError *error = NULL;

        u_char *locstr = g_locale_from_utf8(utf8, strlen(utf8), &rn, &wn, &error);
        g_free(utf8);


        if (error) {
          dbg("utf8->locale convert err %s %s\n", utf8, error->message);
          g_error_free(error);
          b_ignore = TRUE;
          continue;
        }

        if (locstr[0] < 128)
          b_ignore = TRUE;

#if 0
        if (wn)
          dbg("locstr %s %d\n", locstr, writen);
#endif
        memcpy(&txt[txtN], locstr, wn);
        txtN+= wn;
        g_free(locstr);
      }

      if (b_ignore || txtN < 4)
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
      int rn, wn;
      GError *err = NULL;
      char *utf8 = g_locale_to_utf8 (phostr, strlen(phostr), &rn, &wn, &err);

      GtkWidget *item = gtk_menu_item_new_with_label (utf8);
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

      g_free(utf8);
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
  int rn, wn;
  GError *error = NULL;

  u_char *locstr = g_locale_from_utf8(utf8, strlen(utf8), &rn, &wn, &error);
  g_free(utf8);

  strcpy(current_str, locstr);
  int locstrN = strlen(locstr);

  if (locstrN & 1) {
    dbg("not all big5 chars");
    goto ret;
  }

  dbg("locstr: %s", locstr);

  bigphoN = 0;
  int i;
  for(i=0; i< locstrN; i+=2) {
    phokey_t phokey[64];
    big5char_pho *pbigpho = &bigpho[bigphoN++];

    pbigpho->phokeysN = big5_pho_chars(&locstr[i], pbigpho->phokeys);

    if (!pbigpho->phokeysN) {
      dbg(" no mapping to pho\n");
      goto ret;
    }

    dbg(" phokeyN: %d", bigpho[i].phokeysN);
  }

  dbg("\n");

  GtkWidget *sel =  create_pho_sel_area();
  gtk_box_pack_start (GTK_BOX (hbox_buttons), sel, FALSE, FALSE, 20);

  gtk_widget_show_all(hbox_buttons);

ret:
  g_free(locstr);
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

  GError *err = NULL;
  char *text = "按滑鼠中鍵, 貼上你要 tslearn 學習的文章。";
  int rn, wn;
  char *utf8 = g_locale_to_utf8 (text, strlen(text), &rn, &wn, &err);

  if (err) {
    dbg("conver err %s %s\n", text, err->message);
    g_error_free(err);
  }

  gtk_text_buffer_set_text (buffer, utf8, -1);

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
