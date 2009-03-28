#include "gcin.h"
#include "pho.h"
#include "gtab.h"
#include "win-sym.h"
#include "eggtrayicon.h"

static GtkWidget *image;

static void
cb_pref (GtkAction * action)
{
  system(GCIN_BIN_DIR"/gcin-setup &");
}

char *gcin_icons[]={SYS_ICON_DIR"/gcin-tray.png", SYS_ICON_DIR"/gcin-tray-sim.png"};


void toggle_gb_output();
extern gboolean gb_output;

void cb_trad_sim_toggle()
{
  toggle_gb_output();
  gtk_image_set_from_file(GTK_IMAGE(image), gcin_icons[gb_output]);
}

static void cb_sim2trad()
{
  system(GCIN_BIN_DIR"/sim2trad &");
}


static void cb_trad2sim()
{
  system(GCIN_BIN_DIR"/trad2sim &");
}

struct {
  char *name;
  char *stock_id;
  void (*cb)();
} mitems[] = {
  {"設定", GTK_STOCK_PREFERENCES, cb_pref},
  {"正->簡體", NULL, cb_trad2sim},
  {"簡->正體", NULL, cb_sim2trad},
};


GtkWidget *menu;

static void create_menu()
{
  menu = gtk_menu_new ();

  int i;
  for(i=0; i < sizeof(mitems)/ sizeof(mitems[0]); i++) {
    GtkWidget *item;

    if (mitems[i].stock_id) {
      item = gtk_image_menu_item_new_with_label (mitems[i].name);
      gtk_image_menu_item_set_image(item, gtk_image_new_from_stock(mitems[i].stock_id, GTK_ICON_SIZE_MENU));
    }
    else
      item = gtk_menu_item_new_with_label (mitems[i].name);

    g_signal_connect (G_OBJECT (item), "activate",
                      G_CALLBACK (mitems[i].cb), NULL);

    gtk_widget_show(item);

    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  }

  return;
}

gint inmd_switch_popup_handler (GtkWidget *widget, GdkEvent *event);

gboolean
tray_button_press_event_cb (GtkWidget * button, GdkEventButton * event, gpointer userdata)
{
  switch (event->button) {
    case 1:
      cb_trad_sim_toggle(NULL);
      break;
    case 2:
      inmd_switch_popup_handler(NULL, (GdkEvent *)event);
      break;
    case 3:
      if (!menu)
        create_menu();

      gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL,
         event->button, event->time);
      break;
  }

  return TRUE;
}

void create_tray()
{
  EggTrayIcon *tray_icon = egg_tray_icon_new ("gcin");
  GtkWidget *event_box = gtk_event_box_new ();

  GtkTooltips *tips = gtk_tooltips_new ();
  gtk_tooltips_set_tip (GTK_TOOLTIPS (tips), event_box, "左:正/簡體 中:輸入法 右:選項", NULL);

  gtk_container_add (GTK_CONTAINER (tray_icon), event_box);

  g_signal_connect (G_OBJECT (event_box), "button-press-event",
                    G_CALLBACK (tray_button_press_event_cb), NULL);

  image = gtk_image_new_from_file(gcin_icons[gb_output]);


  gtk_container_add (GTK_CONTAINER (event_box), image);

  gtk_widget_show_all (GTK_WIDGET (tray_icon));
}
