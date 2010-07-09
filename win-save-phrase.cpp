#include "gcin.h"
#include "pho.h"
#include "win-save-phrase.h"
#include "gtab.h"

extern int c_len;
extern gboolean test_mode;

static void wsp_str(WSP_S *wsp, int wspN, char *out)
{
  int i;
  int ofs=0;

  for(i=0;i<wspN;i++) {
    utf8_putchar(wsp[i].ch);
	int n = utf8cpy(out+ofs, wsp[i].ch);
	ofs+=n;
  }

//  dbg(" c_len:%d wsp %s\n", c_len, out);
}

static  WSP_S *mywsp;
static int mywspN;

static void free_mywsp()
{
  free(mywsp); mywsp=NULL;
}


static gboolean close_win_save_phrase(GtkWidget *widget, gpointer data)
{
  gtk_widget_destroy((GtkWidget *)data);
  free_mywsp();
  return TRUE;
}

static gint delete_event( GtkWidget *widget,
                   GdkEvent  *event,
                   gpointer   data )
{
  free_mywsp();
  return FALSE;
}

extern int ph_key_sz;

static gboolean cb_ok(GtkWidget *widget, gpointer data)
{
  int i;
  phokey_t pho[MAX_PHRASE_LEN];
  u_int pho32[MAX_PHRASE_LEN];
  u_int64_t pho64[MAX_PHRASE_LEN];
  char tt[512];
  void *dat;
  wsp_str(mywsp, mywspN, tt);

  if (ph_key_sz==2) {
    for(i=0;i<mywspN;i++)
      pho[i] = mywsp[i].key;
    dat = pho;
  }
  else
  if (ph_key_sz==4) {
    for(i=0;i<mywspN;i++) {
      pho32[i] = mywsp[i].key;
    }
    dat = pho32;
  }
  else
  if (ph_key_sz==8) {
    for(i=0;i<mywspN;i++)
      pho64[i] = mywsp[i].key;
    dat = pho64;
  }

#if 1
  save_phrase_to_db(dat, tt, mywspN, 1);
  if (current_method_type() == method_type_TSIN) {
  }
#endif

  gtk_widget_destroy((GtkWidget *)data);

  free(mywsp); mywsp=NULL;
  return TRUE;
}


void create_win_save_phrase(WSP_S *wsp, int wspN)
{
  if (test_mode)
	  return;

  GtkWidget *main_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  gtk_window_set_position (GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);

  gtk_window_set_default_size(GTK_WINDOW (main_window), 200, 100);

  gtk_window_set_title(GTK_WINDOW(main_window), _(_L("加片語到詞庫")));

  g_signal_connect (G_OBJECT (main_window), "delete_event",
                     G_CALLBACK (delete_event),
                     main_window);

  GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (main_window), vbox);

  char tt[512];
  wsp_str(wsp, wspN, tt);

  gtk_box_pack_start (GTK_BOX (vbox), gtk_label_new(tt), FALSE, FALSE, 0);

  int i;
  for(i=0; i<wspN; i++) {
    if (ph_key_sz==2)
      strcat(tt, phokey_to_str(wsp[i].key));
    strcat(tt, " ");
  }
  gtk_box_pack_start (GTK_BOX (vbox), gtk_label_new(tt), FALSE, FALSE, 0);

  mywsp = tmalloc(WSP_S, wspN);
  memcpy(mywsp, wsp, sizeof(WSP_S)*wspN);
  mywspN = wspN;

  GtkWidget *hbox_cancel_ok = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox), hbox_cancel_ok , FALSE, FALSE, 5);

  GtkWidget *button_ok = gtk_button_new_from_stock (GTK_STOCK_OK);
  gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_ok, TRUE, TRUE, 5);

  GtkWidget *button_cancel = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  gtk_box_pack_start (GTK_BOX (hbox_cancel_ok), button_cancel, TRUE, TRUE, 0);

  gtk_widget_show_all(main_window);

#if 0
#if WIN32
  set_no_focus(main_window);
#endif
  gtk_widget_realize(main_window);

#if UNIX
  set_no_focus(main_window);
#else
  win32_init_win(main_window);
#endif
#endif

#if GTK_CHECK_VERSION(2,17,5)
  gtk_widget_set_can_default (button_ok, TRUE);
#else
  GTK_WIDGET_SET_FLAGS (button_ok, GTK_CAN_DEFAULT);
#endif
  gtk_widget_grab_default (button_ok);

//  dbg("main_window %x\n", main_window);
  g_signal_connect (G_OBJECT (button_cancel), "clicked",
                            G_CALLBACK (close_win_save_phrase),
                            G_OBJECT (main_window));

  g_signal_connect (G_OBJECT (button_ok), "clicked",
                            G_CALLBACK (cb_ok),
                            G_OBJECT (main_window));

//  gtk_window_present(GTK_WINDOW(main_window));
  gtk_window_set_keep_above(GTK_WINDOW(main_window), TRUE);
//  gtk_window_set_modal(GTK_WINDOW(main_window), TRUE);
}
