#include <sys/stat.h>
#include "gcin.h"
#include "pho.h"
#include "gtab.h"
#include "win-sym.h"

static GtkWidget *gwin_pho_near = NULL;

void create_win_pho_near(phokey_t pho)
{
  gwin_pho_near = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  GtkWidget *vbox_top = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (gwin_pho_near), vbox_top);

  gtk_container_set_border_width (GTK_CONTAINER (vbox_top), 0);
}
