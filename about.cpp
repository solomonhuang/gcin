#include "gcin.h"
#include "gcin-version.h"

static GtkWidget *about_window;

/* Our usual callback function */
static void callback_close( GtkWidget *widget, gpointer   data )
{
   gtk_widget_destroy(about_window);
   about_window = NULL;
}


void align_with_ui_window(GtkWidget *win);
int html_browser(char *fname);


#if WIN32
#define FURL "http://hyperrate.com/dir.php?eid=215"
#else
#define FURL "http://hyperrate.com/dir.php?eid=67"
#endif

#define LOG_URL "http://www.csie.nctu.edu.tw/~cp76/gcin/download/Changelog.html"
#define ET26_URL "http://hyperrate.com/thread.php?tid=22661"
#define PUNC_URL "http://hyperrate.com/thread.php?tid=19444#19444"
#define ADD_PHRASE_URL "http://hyperrate.com/thread.php?tid=23991#23991"
#define SPLIT_PHRASE_URL "http://hyperrate.com/thread.php?tid=26361"

static void callback_url( GtkWidget *widget, gpointer   data)
{
#if WIN32
	ShellExecuteA(NULL, "open", (char *)data, NULL, NULL, SW_SHOWNORMAL);
#else
	html_browser(data);
#endif
}



void sys_icon_fname(char *iconame, char fname[]);
void create_about_window()
{
    if (about_window) {
      gtk_window_present(GTK_WINDOW(about_window));
      return;
    }

    GtkWidget *vbox = gtk_vbox_new(FALSE,3);
    gtk_orientable_set_orientation(GTK_ORIENTABLE(vbox), GTK_ORIENTATION_VERTICAL);
    GtkWidget *hbox;

    /* Create a new about_window */
    about_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(about_window), GTK_WIN_POS_CENTER);

    gtk_window_set_has_resize_grip(GTK_WINDOW(about_window), FALSE);

    gtk_window_set_title (GTK_WINDOW (about_window), _(_L("關於 gcin")));


    g_signal_connect (G_OBJECT (about_window), "destroy",
	              G_CALLBACK (callback_close), NULL);

    g_signal_connect (G_OBJECT (about_window), "delete_event",
	 	      G_CALLBACK (callback_close), NULL);

    /* Sets the border width of the about_window. */
    gtk_container_set_border_width (GTK_CONTAINER (about_window), 10);


    GtkWidget *label_version;
    GtkWidget *image;

    /* Create box for image and label */
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 3);

    GtkWidget *separator = gtk_hseparator_new ();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 3);

// this doesn't work on win32
#if GTK_CHECK_VERSION(2,18,9) && UNIX && 1
   char tmp[1024];
   sprintf(tmp, "<a href='http://hyperrate.com/dir.php?eid=67'>%s</a>\n"
                "<a href='http://hyperrate.com/dir.php?eid=215'>%s</a>\n"
                "<a href='"LOG_URL"'>%s</a>\n"
                "<a href='"ET26_URL"'>%s</a>\n"
                "<a href='"PUNC_URL"'>%s</a>\n"
                "<a href='"ADD_PHRASE_URL"'>%s</a>\n"
                "<a href='"SPLIT_PHRASE_URL"'>%s</a>\n",
                _(_L("前往 gcin 討論區")),
                _(_L("gcin也有 Windows版")),
                _(_L("gcin改變記錄")),
                _(_L("推薦使用26鍵的注音鍵盤")),
                _(_L("使用詞音輸入標點符號")),
                _(_L("如何新增詞")),
                _(_L("如何手動斷詞"))

          );
   GtkWidget *label = gtk_label_new(tmp);
   gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
#else
    GtkWidget *button_forum = gtk_button_new_with_label(_(_L("討論區")));
    gtk_box_pack_start(GTK_BOX(vbox), button_forum, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (button_forum), "clicked",
		      G_CALLBACK (callback_url), FURL);

    GtkWidget *button_changelog = gtk_button_new_with_label(_(_L("gcin改變記錄")));
    gtk_box_pack_start(GTK_BOX(vbox), button_changelog, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (button_changelog), "clicked",
		      G_CALLBACK (callback_url), LOG_URL);

    GtkWidget *button_et26 = gtk_button_new_with_label(_(_L("推薦使用26鍵的注音鍵盤")));
    gtk_box_pack_start(GTK_BOX(vbox), button_et26, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (button_et26), "clicked",
		      G_CALLBACK (callback_url), ET26_URL);

    GtkWidget *button_punc = gtk_button_new_with_label(_(_L("使用詞音輸入標點符號")));
    gtk_box_pack_start(GTK_BOX(vbox), button_punc, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (button_punc), "clicked",
		      G_CALLBACK (callback_url), PUNC_URL);

    GtkWidget *button_add_phrase = gtk_button_new_with_label(_(_L("如何新增詞")));
    gtk_box_pack_start(GTK_BOX(vbox), button_add_phrase, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (button_add_phrase), "clicked",
		      G_CALLBACK (callback_url), ADD_PHRASE_URL);

    GtkWidget *button_split_phrase = gtk_button_new_with_label(_(_L("如何手動斷詞")));
    gtk_box_pack_start(GTK_BOX(vbox), button_split_phrase, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (button_split_phrase), "clicked",
		      G_CALLBACK (callback_url), SPLIT_PHRASE_URL);

#endif


#if UNIX
    image = gtk_image_new_from_file (SYS_ICON_DIR"/gcin.png");
#else
    char gcin_png[128];
    sys_icon_fname("gcin.png", gcin_png);
    image = gtk_image_new_from_file (gcin_png);
#endif

    label_version = gtk_label_new ("version " GCIN_VERSION "  " __DATE__);

    gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 3);
    gtk_box_pack_start (GTK_BOX (hbox), label_version, FALSE, FALSE, 3);


    gtk_container_add (GTK_CONTAINER (about_window), vbox);

    GtkWidget *button = gtk_button_new_with_label (_(_L("關閉")));
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 3);
    g_signal_connect (G_OBJECT (button), "clicked",
		      G_CALLBACK (callback_close), (gpointer) "cool button");

    gtk_widget_show_all (about_window);

    return;
}
