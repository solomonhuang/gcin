#include "gcin.h"
#include "gtab.h"


static  GtkWidget *menu;

void cb_update_menu_select(GtkWidget  *item,  gpointer data)
{
   int idx=GPOINTER_TO_INT(data);

   init_in_method(idx);
}

void create_inmd_switch()
{
  menu = gtk_menu_new ();

  int i;
  for(i=0; i < MAX_GTAB_NUM_KEY; i++) {
    if (!inmd[i].cname || !inmd[i].cname[0])
      continue;

    char tt[64];

    sprintf(tt, "%s ctrl-alt-%c", inmd[i].cname, gcin_switch_keys[i]);

    GtkWidget *item = gtk_menu_item_new_with_label (tt);

    g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (cb_update_menu_select), GINT_TO_POINTER(i));

    gtk_widget_show(item);

    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  }
}




gint inmd_switch_popup_handler (GtkWidget *widget, GdkEvent *event)
{
  if (!menu)
    create_inmd_switch();

  GdkEventButton *event_button;

  if (event->type == GDK_BUTTON_PRESS) {
    event_button = (GdkEventButton *) event;
    gtk_menu_popup (GTK_MENU(menu), NULL, NULL, NULL, NULL,
                    event_button->button, event_button->time);
    return TRUE;
  }

  return FALSE;
}
