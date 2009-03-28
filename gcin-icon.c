#include "gcin.h"

void get_icon_path(char *iconame, char fname[])
{
  char uu[128];
  sprintf(uu, "icons/%s", iconame);
  get_gcin_user_fname(uu, fname);

  if (access(fname, R_OK))
    sprintf(fname, GCIN_ICON_DIR"/%s", iconame);
}
