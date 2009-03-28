#include "gcin.h"
#include "gtab.h"

int gcin_font_size, gcin_font_size_tsin_presel, gcin_font_size_symbol;
int gcin_font_size_pho_near, gcin_font_size_gtab_in, gcin_win_color_use;
int gcin_remote_client;
int default_input_method;
int left_right_button_tips;
int gcin_im_toggle_keys;
int gcin_capslock_lower, gcin_eng_phrase_enabled, gcin_init_im_enabled;
int gcin_win_sym_click_close;

int gtab_dup_select_bell;
int gtab_space_auto_first;
int gtab_auto_select_by_phrase;
int gtab_press_full_auto_send;
int gtab_pre_select;
int gtab_disp_partial_match;
int gtab_simple_win=1;
int gtab_disp_key_codes;
int gtab_disp_im_name;
int gtab_invalid_key_in;
int gtab_shift_phrase_key;
int gtab_hide_row2;
int gtab_in_row1;
int gtab_capslock_in_eng;
int gtab_vertical_select;
int gtab_unique_auto_send;
int gtab_que_wild_card;

int tsin_phrase_pre_select, tsin_tone_char_input;
int tsin_capslock_upper;
int phonetic_char_dynamic_sequence;
int phonetic_huge_tab;
int phonetic_speak;
char *phonetic_speak_sel;
int tsin_chinese_english_toggle_key;
int gcin_font_size_tsin_pho_in;
int pho_simple_win=1;
int tsin_space_opt;
int tsin_buffer_size, tsin_tail_select_key;
int tsin_buffer_editing_mode;
int gcin_flags_im_enabled;
int gcin_shift_space_eng_full;
char *tsin_phrase_line_color;
char *tsin_cursor_color, *gcin_sel_key_color;
int tsin_tab_phrase_end;
int gcin_input_style, gcin_root_x, gcin_root_y, gcin_pop_up_win;
int gcin_inner_frame;
char *gcin_font_name, *gcin_win_color_fg, *gcin_win_color_bg;
#if TRAY_ENABLED
int gcin_status_tray;
#endif

int pho_hide_row2, pho_in_row1;
int gcb_position, gcb_position_x, gcb_position_y, gcin_bell_volume;
int gcin_sound_play_overlap, gcin_enable_ctrl_alt_switch;


int get_gcin_conf_int(char *name, int default_value);

void load_setttings()
{
  gcin_font_size = get_gcin_conf_int(GCIN_FONT_SIZE, 16);
  get_gcin_conf_str(GCIN_FONT_NAME, &gcin_font_name, "sans");
  gcin_font_size_tsin_presel = get_gcin_conf_int(GCIN_FONT_SIZE_TSIN_PRESEL, 16);
  gcin_font_size_symbol = get_gcin_conf_int(GCIN_FONT_SIZE_SYMBOL, 12);
  gcin_font_size_tsin_pho_in = get_gcin_conf_int(GCIN_FONT_SIZE_TSIN_PHO_IN, 10);
  gcin_font_size_gtab_in = get_gcin_conf_int(GCIN_FONT_SIZE_GTAB_IN, 10);
  gcin_font_size_pho_near = get_gcin_conf_int(GCIN_FONT_SIZE_PHO_NEAR, 14);
  gcin_input_style = get_gcin_conf_int(GCIN_INPUT_STYLE, InputStyleOverSpot);
  gcin_root_x = get_gcin_conf_int(GCIN_ROOT_X, 2000);
  gcin_root_y = get_gcin_conf_int(GCIN_ROOT_Y, 2000);
  gcin_pop_up_win = get_gcin_conf_int(GCIN_POP_UP_WIN, 1);
  gcin_inner_frame = get_gcin_conf_int(GCIN_INNER_FRAME, 0);
  gcin_eng_phrase_enabled = get_gcin_conf_int(GCIN_ENG_PHRASE_ENABLED, 1);
  gcin_init_im_enabled = get_gcin_conf_int(GCIN_INIT_IM_ENABLED, 0);

  gcin_flags_im_enabled = get_gcin_conf_int(GCIN_FLAGS_IM_ENABLED,0x7fffffff);
  gcin_remote_client = get_gcin_conf_int(GCIN_REMOTE_CLIENT, 0);
  gcin_shift_space_eng_full = get_gcin_conf_int(GCIN_SHIFT_SPACE_ENG_FULL, 1);
  gcin_capslock_lower = get_gcin_conf_int(GCIN_CAPSLOCK_LOWER, 1);

  default_input_method = get_gcin_conf_int(DEFAULT_INPUT_METHOD, 6);
  left_right_button_tips = get_gcin_conf_int(LEFT_RIGHT_BUTTON_TIPS, 1);
  gcin_im_toggle_keys = get_gcin_conf_int(GCIN_IM_TOGGLE_KEYS, 0);
#if TRAY_ENABLED
  gcin_status_tray = get_gcin_conf_int(GCIN_STATUS_TRAY, 1);
#endif
  gcin_win_sym_click_close = get_gcin_conf_int(GCIN_WIN_SYM_CLICK_CLOSE, 1);

  gtab_dup_select_bell = get_gcin_conf_int(GTAB_DUP_SELECT_BELL, 0);
  gtab_space_auto_first = get_gcin_conf_int(GTAB_SPACE_AUTO_FIRST, GTAB_space_auto_first_none);
  gtab_auto_select_by_phrase = get_gcin_conf_int(GTAB_AUTO_SELECT_BY_PHRASE, 0);
  gtab_pre_select = get_gcin_conf_int(GTAB_PRE_SELECT, 1);
  gtab_press_full_auto_send = get_gcin_conf_int(GTAB_PRESS_FULL_AUTO_SEND, 1);
  gtab_disp_partial_match = get_gcin_conf_int(GTAB_DISP_PARTIAL_MATCH, 1);
  gtab_disp_key_codes = get_gcin_conf_int(GTAB_DISP_KEY_CODES, 1);
  gtab_disp_im_name = get_gcin_conf_int(GTAB_DISP_IM_NAME, 1);
  gtab_invalid_key_in = get_gcin_conf_int(GTAB_INVALID_KEY_IN, 1);
  gtab_shift_phrase_key = get_gcin_conf_int(GTAB_SHIFT_PHRASE_KEY, 0);
  gtab_hide_row2 = get_gcin_conf_int(GTAB_HIDE_ROW2, 0);
  gtab_in_row1 = get_gcin_conf_int(GTAB_IN_ROW1, 0);
  gtab_capslock_in_eng = get_gcin_conf_int(GTAB_CAPSLOCK_IN_ENG, 1);
  gtab_vertical_select = get_gcin_conf_int(GTAB_VERTICAL_SELECT, 0);
  gtab_unique_auto_send = get_gcin_conf_int(GTAB_UNIQUE_AUTO_SEND, 0);
  gtab_que_wild_card = get_gcin_conf_int(GTAB_QUE_WILD_CARD, 0);

  tsin_phrase_pre_select = get_gcin_conf_int(TSIN_PHRASE_PRE_SELECT, 1);
  tsin_chinese_english_toggle_key = get_gcin_conf_int(TSIN_CHINESE_ENGLISH_TOGGLE_KEY,
                                    TSIN_CHINESE_ENGLISH_TOGGLE_KEY_CapsLock);
  tsin_tone_char_input = get_gcin_conf_int(TSIN_TONE_CHAR_INPUT, 0);

  tsin_space_opt = get_gcin_conf_int(TSIN_SPACE_OPT, TSIN_SPACE_OPT_SELECT_CHAR);
  tsin_buffer_size = get_gcin_conf_int(TSIN_BUFFER_SIZE, 40);
  tsin_tab_phrase_end = get_gcin_conf_int(TSIN_TAB_PHRASE_END, 0);
  tsin_tail_select_key = get_gcin_conf_int(TSIN_TAIL_SELECT_KEY, 0);
  tsin_buffer_editing_mode = get_gcin_conf_int(TSIN_BUFFER_EDITING_MODE, 0);

  phonetic_char_dynamic_sequence = get_gcin_conf_int(PHONETIC_CHAR_DYNAMIC_SEQUENCE, 1);
  phonetic_huge_tab = get_gcin_conf_int(PHONETIC_HUGE_TAB, 0);
  phonetic_speak = get_gcin_conf_int(PHONETIC_SPEAK, 0);
  get_gcin_conf_str(PHONETIC_SPEAK_SEL, &phonetic_speak_sel, "1");

  pho_hide_row2 = get_gcin_conf_int(PHO_HIDE_ROW2, 0);
  pho_in_row1 = get_gcin_conf_int(PHO_IN_ROW1, 1);

  get_gcin_conf_str(TSIN_PHRASE_LINE_COLOR, &tsin_phrase_line_color, "blue");
  get_gcin_conf_str(TSIN_CURSOR_COLOR, &tsin_cursor_color, "blue");
  get_gcin_conf_str(GCIN_SEL_KEY_COLOR, &gcin_sel_key_color, "blue");

  get_gcin_conf_str(GCIN_WIN_COLOR_FG, &gcin_win_color_fg, "white");
  get_gcin_conf_str(GCIN_WIN_COLOR_BG, &gcin_win_color_bg, "#005BFF");
  gcin_win_color_use = get_gcin_conf_int(GCIN_WIN_COLOR_USE, 0);


  gcb_position = get_gcin_conf_int(GCB_POSITION, 4);
  gcb_position_x = get_gcin_conf_int(GCB_POSITION_X, 0);
  gcb_position_y = get_gcin_conf_int(GCB_POSITION_Y, 0);
  gcin_bell_volume = get_gcin_conf_int(GCIN_BELL_VOLUME, -97);
  gcin_sound_play_overlap = get_gcin_conf_int(GCIN_SOUND_PLAY_OVERLAP, 0);
  gcin_enable_ctrl_alt_switch = get_gcin_conf_int(GCIN_ENABLE_CTRL_ALT_SWITCH, 1);
}
