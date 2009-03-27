#include "gcin.h"
#include "gtab.h"

int gcin_font_size, gcin_font_size_tsin_presel, gcin_font_size_symbol;
int default_input_method;
int left_right_button_tips;
int gcin_im_toggle_keys;

int gtab_dup_select_bell;
int gtab_space_auto_first;
int gtab_auto_select_by_phrase;
int gtab_press_full_auto_send;
int gtab_pre_select;
int gtab_disp_partial_match;

int tsin_phrase_pre_select;
int phonetic_char_dynamic_sequence;
int tsin_chinese_english_toggle_key;
int gcin_font_size_tsin_pho_in;

int get_gcin_conf_int(char *name, int default_value);

void load_setttings()
{
  gcin_font_size = get_gcin_conf_int(GCIN_FONT_SIZE, 16);
  gcin_font_size_tsin_presel = get_gcin_conf_int(GCIN_FONT_SIZE_TSIN_PRESEL, 16);
  gcin_font_size_symbol = get_gcin_conf_int(GCIN_FONT_SIZE_SYMBOL, 12);
  gcin_font_size_tsin_pho_in = get_gcin_conf_int(GCIN_FONT_SIZE_TSIN_PHO_IN, 10);

  default_input_method = get_gcin_conf_int(DEFAULT_INPUT_METHOD, 6);
  left_right_button_tips = get_gcin_conf_int(LEFT_RIGHT_BUTTON_TIPS, 1);
  gcin_im_toggle_keys = get_gcin_conf_int(GCIN_IM_TOGGLE_KEYS, 0);

  gtab_dup_select_bell = get_gcin_conf_int(GTAB_DUP_SELECT_BELL, 0);
  gtab_space_auto_first = get_gcin_conf_int(GTAB_SPACE_AUTO_FIRST, GTAB_space_auto_first_full);
  gtab_auto_select_by_phrase = get_gcin_conf_int(GTAB_AUTO_SELECT_BY_PHRASE, 1);
  gtab_pre_select = get_gcin_conf_int(GTAB_PRE_SELECT, 1);
  gtab_press_full_auto_send = get_gcin_conf_int(GTAB_PRESS_FULL_AUTO_SEND, 1);
  gtab_disp_partial_match = get_gcin_conf_int(GTAB_DISP_PARTIAL_MATCH, 1);

  tsin_phrase_pre_select = get_gcin_conf_int(TSIN_PHRASE_PRE_SELECT, 1);
  tsin_chinese_english_toggle_key = get_gcin_conf_int(TSIN_CHINESE_ENGLISH_TOGGLE_KEY,
                                    TSIN_CHINESE_ENGLISH_TOGGLE_KEY_CapsLock);

  phonetic_char_dynamic_sequence = get_gcin_conf_int(PHONETIC_CHAR_DYNAMIC_SEQUENCE, 1);
}
