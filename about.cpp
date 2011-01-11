#include "gcin.h"
#include "gcin-version.h"

static GtkWidget *about_window;

/* Our usual callback function */
static void callback_close( GtkWidget *widget, gpointer   data )
{
   gtk_widget_destroy(about_window);
   about_window = NULL;
}

extern GtkWidget *main_window;

void align_with_ui_window(GtkWidget *win)
{
    gint uix, uiy;

    gtk_window_get_position(GTK_WINDOW(main_window), &uix, &uiy);

    gtk_window_move(GTK_WINDOW(win), uix, uiy);
}


void align_with_ui_window(GtkWidget *win);
int html_browser(char *fname);

static void callback_forum( GtkWidget *widget, gpointer   data)
{
#if WIN32
#define FURL "http://hyperrate.com/dir.php?eid=215"
	ShellExecuteA(NULL, "open", FURL, NULL, NULL, SW_SHOWNORMAL);
#else
#define FURL "http://hyperrate.com/dir.php?eid=67"
	html_browser(FURL);
#endif
}


static void callback_changelog( GtkWidget *widget, gpointer   data)
{
#define LOG_URL "http://cle.linux.org.tw/gcin/download/Changelog.html"
#if WIN32
	ShellExecuteA(NULL, "open", LOG_URL, NULL, NULL, SW_SHOWNORMAL);
#else
	html_browser(LOG_URL);
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
    GtkWidget *hbox;

    /* Create a new about_window */
    about_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_has_resize_grip(GTK_WINDOW(about_window), FALSE);

    gtk_window_set_title (GTK_WINDOW (about_window), _(_L("關於 gcin")));

    /* It's a good idea to do this for all windows. */
    g_signal_connect (G_OBJECT (about_window), "destroy",
	              G_CALLBACK (callback_close), NULL);

    g_signal_connect (G_OBJECT (about_window), "delete_event",
	 	      G_CALLBACK (callback_close), NULL);

    /* Sets the border width of the about_window. */
    gtk_container_set_border_width (GTK_CONTAINER (about_window), 10);

    align_with_ui_window(about_window);

    GtkWidget *label_version;
    GtkWidget *image;

    /* Create box for image and label */
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 3);

    GtkWidget *separator = gtk_hseparator_new ();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 3);

#if GTK_CHECK_VERSION(2,18,9)
   GtkWidget *label = gtk_label_new(_(_L("<a href='http://hyperrate.com?eid=67'>點選連結前往 gcin 討論區</a>\n"
_L("<a href='http://hyperrate.com?eid=215'>gcin也有 Windows版</a>\n")
_L("<a href='http://cle.linux.org.tw/gcin/download/Changelog.html'>gcin改變記錄</a>\n")
)));
   gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
#else
    GtkWidget *button_forum = gtk_button_new_with_label(_(_L("討論區")));
    gtk_box_pack_start(GTK_BOX(vbox), button_forum, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (button_forum), "clicked",
		      G_CALLBACK (callback_forum), NULL);
    GtkWidget *button_changelog = gtk_button_new_with_label(_(_L("gcin改變記錄")));
    gtk_box_pack_start(GTK_BOX(vbox), button_changelog, FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (button_changelog), "clicked",
		      G_CALLBACK (callback_changelog), NULL);
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
