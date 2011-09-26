#include "gcin.h"

#if WIN32
 #pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif

int main()
{
  gdk_init(NULL, NULL);

#if UNIX
  Display *dpy = GDK_DISPLAY();
  if (find_gcin_window(dpy)==None)
    return;
  send_gcin_message(dpy, GCIN_EXIT_MESSAGE);
#else
  if (!find_gcin_window())
    return;
  send_gcin_message(GCIN_EXIT_MESSAGE);
#endif

  return 0;
}
