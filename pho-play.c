#include "gcin.h"
#include "pho.h"

int pho_play(phokey_t key)
{
  if (!phonetic_speak)
    return;
  static int pid;
  static time_t last_time;
  time_t t = time(NULL);
  if (!gcin_sound_play_overlap) {
    if (pid && t - last_time < 2)
      kill(pid, 9);
  }
  char *ph = phokey_to_str2(key, 1);
  char tt[512];
  sprintf(tt, GCIN_OGG_DIR"/%s/%s", ph, phonetic_speak_sel);

  if (access(tt, R_OK))
    return 0;

  last_time = t;

  if (pid = fork()) {
    if (pid < 0)
      dbg("cannot fork ?");
    return 1;
  }

  close(1);
  close(2);
  execlp("ogg123", "ogg123", tt, NULL);
}


void char_play(char *utf8)
{
  if (!phonetic_speak || !(utf8[0]&128))
    return;

  if (!ch_pho)
    load_tab_pho_file();

  phokey_t phos[16];
  int phosN = utf8_pho_keys(utf8, phos);

  if (!phosN)
    return;

  int i;
  for(i=0; i < phosN; i++)
    if (pho_play(phos[i]))
      break;
}
