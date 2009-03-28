#include <dlfcn.h>
void gdk_window_freeze_toplevel_updates_libgtk_only(){}
void gdk_window_thaw_toplevel_updates_libgtk_only(){}

#if 0
void __attribute__((constructor)) __open_init()
{
  puts("gtk_bug_fixe loaded");
}
#endif
