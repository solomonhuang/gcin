#include "gcin.h"
#include <X11/Xatom.h>

void send_gcin_message(Display *dpy, char *s)
{
  Atom atom = get_gcin_atom(dpy);
  Window gcin_win;

  if ((gcin_win=XGetSelectionOwner(dpy, atom))==None) {
    dbg("Cannot connect to gcin server\n");
    return;
  }

  Window mwin = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy),
                  0,0,90,90,1,0,0);

  XChangeProperty(dpy, mwin , atom, XA_STRING, 8,
     PropModeReplace, s, strlen(s)+1);

  XPropertyEvent eve;

  eve.type=PropertyNotify;
  eve.window=mwin;
  eve.state=PropertyNewValue;
  eve.atom=atom;
  XSendEvent(dpy, gcin_win, False, 0, (XEvent *)&eve);
  XSync(dpy,0);
  sleep(1);

  XDestroyWindow(dpy, mwin);
}
