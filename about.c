#include "gcin.h"

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

#if 0
void cb_link_button(GtkLinkButton *button, const gchar *link_,
                           gpointer user_data)
{
  GError *err;
  gtk_show_uri(NULL, link_, GDK_CURRENT_TIME, &err);
}
#endif

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

    gtk_window_set_title (GTK_WINDOW (about_window), _("關於 gcin"));

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

    char tt[512];

#if 1
    GtkWidget *label_sf = gtk_label_new ("http://hyperrate.com/dir.php?eid=67\n");
    gtk_label_set_selectable(GTK_LABEL(label_sf), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), label_sf, FALSE, FALSE, 0);
#else
    GtkWidget *button_url = gtk_link_button_new_with_label("http://hyperrate.com/dir.php?eid=67", "forum");
    gtk_box_pack_start(GTK_BOX(vbox), button_url, FALSE, FALSE, 0);
    gtk_link_button_set_uri_hook(cb_link_button, NULL, NULL);
#endif

    image = gtk_image_new_from_file (SYS_ICON_DIR"/gcin.png");

    label_version = gtk_label_new ("version " GCIN_VERSION);

    gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 3);
    gtk_box_pack_start (GTK_BOX (hbox), label_version, FALSE, FALSE, 3);


    gtk_container_add (GTK_CONTAINER (about_window), vbox);

    /* Create a new button */
    GtkWidget *button = gtk_button_new_with_label (_("關閉"));
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 3);
    /* Connect the "clicked" signal of the button to our callback */
    g_signal_connect (G_OBJECT (button), "clicked",
		      G_CALLBACK (callback_close), (gpointer) "cool button");

    gtk_widget_show_all (about_window);

    return;
}
