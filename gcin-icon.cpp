#include "gcin.h"
#if WIN32
#include <io.h>
#endif

void sys_icon_fname(char *iconame, char fname[])
{
#if WIN32
  sprintf(fname, "%s\\icons\\%s",gcin_program_files_path, iconame);
#else
  sprintf(fname, GCIN_ICON_DIR"/%s", iconame);
#endif
}

void get_icon_path(char *iconame, char fname[])
{
  char uu[128];
#if UNIX
  sprintf(uu, "icons/%s", iconame);
#else
  sprintf(uu, "icons\\%s", iconame);
#endif
  get_gcin_user_fname(uu, fname);

#if UNIX
  if (access(fname, R_OK)) {
#else
  if (_access(fname, 04)) {
#endif
    sys_icon_fname(iconame, fname);
  }
}


void set_window_gcin_icon(GtkWidget *window)
{
#if WIN32
  char tt[128];
  sys_icon_fname("gcin.png", tt);
  gtk_window_set_icon_from_file(GTK_WINDOW(window), tt, NULL);
#else
  gtk_window_set_icon_from_file(GTK_WINDOW(window), SYS_ICON_DIR"/gcin.png", NULL);
#endif
}
