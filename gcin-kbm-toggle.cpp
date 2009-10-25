#include "gcin.h"

int main()
{
  gdk_init(NULL, NULL);
  send_gcin_message(GDK_DISPLAY(), KBM_TOGGLE);

  return 0;
}
