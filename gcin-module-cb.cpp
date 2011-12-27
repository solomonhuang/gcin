#include "gcin.h"
#include "gtab.h"
#include "pho.h"
#include "tsin.h"
#include "gst.h"
#include "im-client/gcin-im-client-attr.h"
#include "win1.h"
#include "gcin-module.h"
#include "gcin-module-cb.h"
#include "util.h"


#if UNIX
#include <dlfcn.h>
#endif

GCIN_module_callback_functions *init_GCIN_module_callback_functions(char *sofile)
{
#if UNIX
  void *handle;
  char *error;

  if (!(handle = dlopen(sofile, RTLD_LAZY))) {
    if ((error = dlerror()) != NULL)
      box_warn(error);
    return NULL;
  }
#else
  HMODULE handle = LoadLibraryA(sofile);
  if (!handle) {
    box_warn("LoadLibraryA %s error %s", sofile, sys_err_strA());
    return NULL;
  }
#define dlsym GetProcAddress
#endif

  GCIN_module_callback_functions st;
  *(void **) (&st.module_init_win) = dlsym(handle, "module_init_win");
  if (!st.module_init_win)
    p_err("module_init_win() not found in %s", sofile);

  *(void **) (&st.module_get_win_geom) = dlsym(handle, "module_get_win_geom");
  *(void **) (&st.module_reset) = dlsym(handle, "module_reset");
  *(void **) (&st.module_get_preedit) = dlsym(handle, "module_get_preedit");
  *(void **) (&st.module_feedkey) = dlsym(handle, "module_feedkey");
  *(void **) (&st.module_feedkey_release) = dlsym(handle, "module_feedkey_release");
  *(void **) (&st.module_move_win) = dlsym(handle, "module_move_win");
  *(void **) (&st.module_change_font_size) = dlsym(handle, "module_change_font_size");
  *(void **) (&st.module_show_win) = dlsym(handle, "module_show_win");
  *(void **) (&st.module_hide_win) = dlsym(handle, "module_hide_win");
  *(void **) (&st.module_win_visible) = dlsym(handle, "module_win_visible");
  *(void **) (&st.module_flush_input) = dlsym(handle, "module_flush_input");
  *(void **) (&st.module_setup_window_create) = dlsym(handle, "module_setup_window_create");

  return tmemdup(&st, GCIN_module_callback_functions, 1);
}
