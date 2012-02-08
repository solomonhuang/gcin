#include "gcin.h"
#include "gtab.h"
#include "config.h"
#if UNIX
#include <signal.h>
#endif


int html_browser(char *fname)
{
#if WIN32
  LONG r = (LONG)ShellExecuteA(NULL, "open", fname, NULL, NULL, SW_SHOWNORMAL);
  return r;
#else
  static char html_browse[]=GCIN_SCRIPT_DIR"/html-browser";
#if 0
  char tt[256];
  sprintf(tt, "%s %s", html_browse, fname);
  dbg("%s\n", tt);
  return system(tt);
#else
  GError *error;
  error = NULL;
  gtk_show_uri (gdk_screen_get_default (),  fname, gtk_get_current_event_time (), &error);
#endif
#endif
}

