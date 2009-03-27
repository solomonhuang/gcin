#include "gcin.h"

int gcin_font_size;
int default_input_method;
int left_right_button_tips;
int gtab_dup_select_bell;
int gtab_full_space_auto_first;
int gtab_auto_select_by_phrase;
int gcin_im_toggle_keys;
int gtab_pre_select;

void load_setttings()
{
  gcin_font_size = get_gcin_conf_int(GCIN_FONT_SIZE, 14);
  default_input_method = get_gcin_conf_int(DEFAULT_INPUT_METHOD, 6);
  left_right_button_tips = get_gcin_conf_int(LEFT_RIGHT_BUTTON_TIPS, 1);
  gtab_dup_select_bell = get_gcin_conf_int(GTAB_DUP_SELECT_BELL, 0);
  gtab_full_space_auto_first = get_gcin_conf_int(GTAB_FULL_SPACE_AUTO_FIRST, 0);
  gtab_auto_select_by_phrase = get_gcin_conf_int(GTAB_AUTO_SELECT_BY_PHRASE, 1);
  gtab_auto_select_by_phrase = get_gcin_conf_int(GTAB_AUTO_SELECT_BY_PHRASE, 1);
  gcin_im_toggle_keys = get_gcin_conf_int(GCIN_IM_TOGGLE_KEYS, 0);
  gtab_pre_select = get_gcin_conf_int(GTAB_PRE_SELECT, 1);
}
