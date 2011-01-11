#include "gcin.h"
#include "gcin-im-client.h"

#if UNIX
void send_gcin_message(Display *dpy, char *s)
#else
void send_gcin_message(char *s)
#endif
{
  GCIN_client_handle *handle = gcin_im_client_open(dpy);
  gcin_im_client_message(handle, s);

  gcin_im_client_close(handle);
}
