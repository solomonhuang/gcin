#include "gcin.h"
#include "gcin-im-client.h"

void send_gcin_message(Display *dpy, char *s)
{
  GCIN_client_handle *handle = gcin_im_client_open(dpy);
  gcin_im_client_message(handle, s);

  gcin_im_client_close(handle);
}
