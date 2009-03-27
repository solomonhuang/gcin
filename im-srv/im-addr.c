#include "gcin.h"
#include <string.h>

char *get_gcin_xim_name();

char *get_gcin_im_srv_sock_path()
{
  static char tstr[128];

  get_gcin_dir(tstr);
  strcat(strcat(tstr,"/gcin-socket-"), get_gcin_xim_name());

  return tstr;
}
