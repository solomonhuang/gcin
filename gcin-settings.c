#include "gcin.h"

extern int gcin_font_size;

void load_setttings()
{
  gcin_font_size = get_gcin_conf_int(GCIN_FONT_SIZE, 14);
}
