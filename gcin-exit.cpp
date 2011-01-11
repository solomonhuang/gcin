#include "gcin.h"

#if WIN32
 #pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif

int main()
{
  gdk_init(NULL, NULL);
#if UNIX
  send_gcin_message(GDK_DISPLAY(), GCIN_EXIT_MESSAGE);
#else
  send_gcin_message(GCIN_EXIT_MESSAGE);
#endif

  return 0;
}
