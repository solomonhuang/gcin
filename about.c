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

void create_about_window()
{
    if (about_window) {
      gtk_window_present(GTK_WINDOW(about_window));
      return;
    }

    /* GtkWidget is the storage type for widgets */
    GtkWidget *button;
    GtkWidget *vbox = gtk_vbox_new(FALSE,3);
    GtkWidget *hbox;

    /* Create a new about_window */
    about_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title (GTK_WINDOW (about_window), "關於 gcin");

    /* It's a good idea to do this for all windows. */
    g_signal_connect (G_OBJECT (about_window), "destroy",
	              G_CALLBACK (callback_close), NULL);

    g_signal_connect (G_OBJECT (about_window), "delete_event",
	 	      G_CALLBACK (callback_close), NULL);

    /* Sets the border width of the about_window. */
    gtk_container_set_border_width (GTK_CONTAINER (about_window), 10);

    align_with_ui_window(about_window);

    /* Create a new button */
    button = gtk_button_new ();

    /* Connect the "clicked" signal of the button to our callback */
    g_signal_connect (G_OBJECT (button), "clicked",
		      G_CALLBACK (callback_close), (gpointer) "cool button");

    GtkWidget *label_version;
    GtkWidget *image;

    /* Create box for image and label */
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 3);

    GtkWidget *separator = gtk_hseparator_new ();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 3);

    GtkWidget *label_sf = gtk_label_new ("http://www.csie.nctu.edu.tw/~cp76/gcin\n討論區 http://cle.linux.org.tw/gcin");
    gtk_box_pack_start(GTK_BOX(vbox), label_sf, FALSE, FALSE, 0);


    /* Now on to the image stuff */
    image = gtk_image_new_from_file (SYS_ICON_DIR"/gcin.png");

    /* Create a label for the button */
    label_version = gtk_label_new ("version " GCIN_VERSION);

    /* Pack the image and label into the box */
    gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 3);
    gtk_box_pack_start (GTK_BOX (hbox), label_version, FALSE, FALSE, 3);


    gtk_container_add (GTK_CONTAINER (button), vbox);

    gtk_container_add (GTK_CONTAINER (about_window), button);

    gtk_widget_show_all (about_window);

    return;
}
