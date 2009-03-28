#include "gcin.h"
#include "pho.h"

void pho_play(phokey_t key)
{
  if (!phonetic_speak)
    return;
  static int pid;
  static time_t last_time;
  time_t t = time(NULL);

  if (pid && t - last_time < 2)
    kill(pid, 9);

  char *ph = phokey_to_str2(key, 1);
  char tt[512];

  last_time = t;

  if (pid = fork()) {
    if (pid < 0)
      dbg("cannot fork ?");
    return;
  }

  sprintf(tt, GCIN_OGG_DIR"/%s/%s", ph, phonetic_speak_sel);
  close(1);
  close(2);
  execlp("ogg123", "ogg123", tt, NULL);
}
