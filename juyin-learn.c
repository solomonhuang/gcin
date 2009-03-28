#include "gcin.h"
#include "pho.h"
#include "config.h"
#include <libintl.h>

GtkWidget *hbox_buttons;
char current_str[MAX_PHRASE_LEN*CH_SZ+1];
PIN_JUYIN *pin_juyin;
int pin_juyinN;
PHOKBM phkbm;
char inph[8];
u_char typ_pho[4];
int text_pho_N;

gboolean b_pinyin;

static GtkClipboard *pclipboard;

GtkWidget *mainwin;
GtkTextBuffer *buffer;

void bell()
{
}

void key_typ_pho(phokey_t phokey, u_char rtyp_pho[])
{
}

void do_exit()
{
  exit(0);
}


#define MAX_SAME_PHO 32

void all_wrap()
{
  GtkTextIter mstart,mend;

  gtk_text_buffer_get_bounds (buffer, &mstart, &mend);
  gtk_text_buffer_apply_tag_by_name (buffer, "char_wrap", &mstart, &mend);
}

char *phokey2pinyin(phokey_t k)
{
  static char tt[32];
  phokey_t tonemask = 7;

  int i;
  for(i=0; i < pin_juyinN; i++) {

    if ((k & ~tonemask) == pin_juyin[i].key)
      break;
  }

  if (i==pin_juyinN)
    strcpy(tt, "??");
  else {
static char tone[2];
    tone[0] = (k & tonemask) + '0';
    strcpy(tt, pin_juyin[i].pinyin);

    if (tone[0]!='0')
      strcat(tt, tone);
  }

  return tt;
}

static void selection_received(GtkClipboard *pclip, const gchar *text, gpointer data)
{
  if (!text)
    return;

  char *outtext = NULL;
  int outlen = 0;

  while (*text) {
    char tt[CH_SZ * 5 * MAX_SAME_PHO + 1];
    int len = u8cpy(tt, (char *)text);
    tt[len]=0;

    phokey_t phokeys[MAX_SAME_PHO];
    int phokeysN = utf8_pho_keys((char *)text, phokeys);

    int i;
    for(i=0; i <phokeysN ;i++) {
      char *phostr;

      if (b_pinyin)
        phostr = phokey2pinyin(phokeys[i]);
      else
        phostr = phokey_to_str(phokeys[i]);

      strcat(tt, phostr);
    }

    int ttlen = strlen(tt);
    outtext = realloc(outtext, outlen + ttlen + 1);

    memcpy(outtext + outlen, tt, ttlen);
    outlen+=ttlen;

    text += len;
  }

  outtext[outlen]=0;

  gtk_text_buffer_set_text (buffer, outtext, -1);
  free(outtext);

  all_wrap();
}

void req_clipboard()
{
  gtk_clipboard_request_text(pclipboard, selection_received, mainwin);
}


gboolean cb_button_fetch()
{
  req_clipboard();
  return TRUE;
}

int main(int argc, char **argv)
{

#if GCIN_i18n_message
  gtk_set_locale();
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);
#endif

  gtk_init (&argc, &argv);

  char kbm_str[32];
  get_gcin_conf_fstr(PHONETIC_KEYBOARD, kbm_str, "zo-asdf");
#if 1
  b_pinyin = strstr(kbm_str, "pinyin") != NULL;
#else
  b_pinyin = 1;
#endif

  if (b_pinyin)
    load_pin_juyin();

  pho_load();

  mainwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW (mainwin), 640, 220);
  gtk_window_set_icon_from_file(GTK_WINDOW(mainwin), SYS_ICON_DIR"/gcin.png", NULL);

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

  gtk_text_buffer_create_tag (buffer,
     "blue_background", "background", "blue", "foreground", "white", NULL);

  gtk_text_buffer_create_tag (buffer, "char_wrap",
			      "wrap_mode", GTK_WRAP_CHAR, NULL);

  hbox_buttons = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_top), hbox_buttons, FALSE, FALSE, 0);

  GtkWidget *button_fetch = gtk_button_new_with_label(_("自剪貼區更新"));
  gtk_box_pack_start (GTK_BOX (hbox_buttons), button_fetch, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_fetch), "clicked",
     G_CALLBACK (cb_button_fetch), NULL);

  GtkWidget *button_exit = gtk_button_new_with_label(_("離開"));
  gtk_box_pack_start (GTK_BOX (hbox_buttons), button_exit, FALSE, FALSE, 0);
  g_signal_connect (G_OBJECT (button_exit), "clicked",
     G_CALLBACK (do_exit), NULL);


  g_signal_connect (G_OBJECT (mainwin), "delete_event",
                    G_CALLBACK (do_exit), NULL);

  gtk_widget_show_all(mainwin);

  pclipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);

  req_clipboard();

  gtk_main();
  return 0;
}
