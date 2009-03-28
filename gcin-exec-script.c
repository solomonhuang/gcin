#include <stdio.h>

static void exec_script(char *name)
{
  char scr[512];

  sprintf(scr, GCIN_SCRIPT_DIR"/%s", name);
  system(scr);
}

void exec_setup_scripts()
{
  exec_script("gcin-user-setup "GCIN_TABLE_DIR" "GCIN_BIN_DIR);
}
