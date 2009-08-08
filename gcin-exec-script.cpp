#include <stdio.h>
#include <stdlib.h>
#include "os-dep.h"

#if UNIX
static void exec_script(char *name)
{
  char scr[512];

  sprintf(scr, GCIN_SCRIPT_DIR"/%s", name);
  system(scr);
}
#endif

void exec_setup_scripts()
{
#if WIN32
  win32exec_script("gcin-user-setup.bat");
#else
  exec_script("gcin-user-setup "GCIN_TABLE_DIR" "GCIN_BIN_DIR);
#endif
}
