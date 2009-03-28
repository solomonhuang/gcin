#include "gcin.h"

int main(int argc, char **argv)
{
  int i;
  char text[128];
  char icon[128];
  int duration = 3000;

  strcpy(text, "-");
  strcpy(icon, "-");

  for(i=1; i < argc; i+=2) {
    if (!strcmp(argv[i], "-icon")) {
      strcpy(icon, argv[i+1]);
    } else
    if (!strcmp(argv[i], "-text")) {
      strcpy(text, argv[i+1]);
    } else
    if (!strcmp(argv[i], "-duration")) {
      duration = atoi(argv[i+1]);
    } else {
      dbg("unknown opt %s", argv[i]);
      p_err("usage: %s -icon file_name -text string -duration milli_seconds\n", argv[0]);
    }
  }

  char message[512];

  sprintf(message, "#gcin_message %s %s %d", icon, text, duration);


  gdk_init(NULL, NULL);
  send_gcin_message(GDK_DISPLAY(), message);

  return 0;
}
