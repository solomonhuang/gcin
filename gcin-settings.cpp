#include "gcin.h"
#include "gtab.h"

extern gboolean is_chs;

int gcin_font_size, gcin_font_size_tsin_presel, gcin_font_size_symbol;
int gcin_font_size_pho_near, gcin_font_size_gtab_in, gcin_font_size_win_kbm, gcin_font_size_win_kbm_en;
int gcin_win_color_use, gcin_single_state;
int gcin_remote_client;
char *default_input_method_str;
int default_input_method;
int left_right_button_tips;
int gcin_im_toggle_keys, gcin_bell_off;
int gcin_capslock_lower, gcin_eng_phrase_enabled, gcin_init_im_enabled;
int gcin_win_sym_click_close, gcin_edit_display, gcin_win32_icon;
int gcin_on_the_spot_key, gcin_tray_hf_win_kbm, gcin_punc_auto_send;

int gtab_dup_select_bell;
int gtab_space_auto_first;
int gtab_auto_select_by_phrase;
int gtab_press_full_auto_send;
int gtab_pre_select, gtab_phrase_pre_select;
int gtab_disp_partial_match;
int gtab_disp_key_codes;
int gtab_disp_im_name;
int gtab_invalid_key_in;
int gtab_shift_phrase_key;
int gtab_hide_row2;
int gtab_in_row1;
int gtab_capslock_in_eng;
int gtab_vertical_select;
int gtab_unique_auto_send;
int gtab_que_wild_card, gtab_in_area_button;

int tsin_phrase_pre_select, tsin_tone_char_input;
int tsin_capslock_upper, tsin_use_pho_near;
int phonetic_char_dynamic_sequence;
int phonetic_huge_tab;
int phonetic_speak;
char *phonetic_speak_sel, *gcin_str_im_cycle;
int tsin_chinese_english_toggle_key;
int gcin_font_size_tsin_pho_in;
int tsin_space_opt;
int tsin_buffer_size, tsin_tail_select_key;
int tsin_buffer_editing_mode;
int gcin_shift_space_eng_full;
char *tsin_phrase_line_color;
char *tsin_cursor_color, *gcin_sel_key_color;
unich_t eng_full_str[]=_L("[英/全]");
unich_t cht_full_str[]=_L("[全]");
char *eng_color_full_str, *cht_color_full_str;
int tsin_tab_phrase_end;
int gcin_input_style, gcin_root_x, gcin_root_y, gcin_pop_up_win;
int gcin_inner_frame;
char *gcin_font_name, *gcin_win_color_fg, *gcin_win_color_bg;
#if TRAY_ENABLED
int gcin_status_tray;
#endif

int pho_hide_row2, pho_in_row1;
#if USE_GCB
int gcb_enabled, gcb_position, gcb_position_x, gcb_position_y, gcb_button_n, gcb_history_n;
#endif
int gcin_bell_volume;
int gcin_sound_play_overlap, gcin_enable_ctrl_alt_switch;
char *pho_kbm_name, *pho_selkey;
int pho_candicate_col_N, pho_candicate_R2L;


int get_gcin_conf_int(char *name, int default_value);

void load_setttings()
{
  gcin_font_size = get_gcin_conf_int(GCIN_FONT_SIZE, 16);
#if UNIX || 1
  get_gcin_conf_str(GCIN_FONT_NAME, &gcin_font_name, "Sans");
#else
  get_gcin_conf_str(GCIN_FONT_NAME, &gcin_font_name, "MingLiU Bold");
#endif
  gcin_font_size_tsin_presel = get_gcin_conf_int(GCIN_FONT_SIZE_TSIN_PRESEL, 16);
  gcin_font_size_symbol = get_gcin_conf_int(GCIN_FONT_SIZE_SYMBOL, 12);
  gcin_font_size_tsin_pho_in = get_gcin_conf_int(GCIN_FONT_SIZE_TSIN_PHO_IN, 13);
  gcin_font_size_gtab_in = get_gcin_conf_int(GCIN_FONT_SIZE_GTAB_IN, 10);
  gcin_font_size_pho_near = get_gcin_conf_int(GCIN_FONT_SIZE_PHO_NEAR, 14);
  gcin_font_size_win_kbm = get_gcin_conf_int(GCIN_FONT_SIZE_WIN_KBM, 10);
  gcin_font_size_win_kbm_en = get_gcin_conf_int(GCIN_FONT_SIZE_WIN_KBM_EN, 8);
  gcin_input_style = get_gcin_conf_int(GCIN_INPUT_STYLE, InputStyleOverSpot);
  gcin_root_x = get_gcin_conf_int(GCIN_ROOT_X, 1600);
  gcin_root_y = get_gcin_conf_int(GCIN_ROOT_Y, 1200);
  gcin_pop_up_win = get_gcin_conf_int(GCIN_POP_UP_WIN, 1);
  gcin_inner_frame = get_gcin_conf_int(GCIN_INNER_FRAME, 1);
  gcin_eng_phrase_enabled = get_gcin_conf_int(GCIN_ENG_PHRASE_ENABLED, 1);
  gcin_tray_hf_win_kbm = get_gcin_conf_int(GCIN_TRAY_HF_WIN_KBM, 0);
#if UNIX
  gcin_init_im_enabled = get_gcin_conf_int(GCIN_INIT_IM_ENABLED, 1);
#else
  gcin_init_im_enabled = true;
#endif

  gcin_single_state = get_gcin_conf_int(GCIN_SINGLE_STATE, 0);
  gcin_punc_auto_send = get_gcin_conf_int(GCIN_PUNC_AUTO_SEND, 1);


  get_gcin_conf_str(GCIN_STR_IM_CYCLE, &gcin_str_im_cycle, "1234567890-=[]\\");
  gcin_remote_client = get_gcin_conf_int(GCIN_REMOTE_CLIENT, 0);
  gcin_shift_space_eng_full = get_gcin_conf_int(GCIN_SHIFT_SPACE_ENG_FULL, 1);
  gcin_capslock_lower = get_gcin_conf_int(GCIN_CAPSLOCK_LOWER, 1);

  get_gcin_conf_str(DEFAULT_INPUT_METHOD, &default_input_method_str, "6");
  left_right_button_tips = get_gcin_conf_int(LEFT_RIGHT_BUTTON_TIPS, 1);
  gcin_im_toggle_keys = get_gcin_conf_int(GCIN_IM_TOGGLE_KEYS, 0);
#if TRAY_ENABLED
  gcin_status_tray = get_gcin_conf_int(GCIN_STATUS_TRAY, 1);
#endif
  gcin_win_sym_click_close = get_gcin_conf_int(GCIN_WIN_SYM_CLICK_CLOSE, 1);
#if WIN32
  gcin_win32_icon = 1;
#else
  gcin_win32_icon = get_gcin_conf_int(GCIN_WIN32_ICON, 1);
#endif

  gtab_dup_select_bell = get_gcin_conf_int(GTAB_DUP_SELECT_BELL, 0);
  gtab_space_auto_first = get_gcin_conf_int(GTAB_SPACE_AUTO_FIRST, GTAB_space_auto_first_none);
  gtab_auto_select_by_phrase = get_gcin_conf_int(GTAB_AUTO_SELECT_BY_PHRASE, GTAB_OPTION_AUTO);
  gtab_pre_select = get_gcin_conf_int(GTAB_PRE_SELECT, GTAB_OPTION_AUTO);
  gtab_press_full_auto_send = get_gcin_conf_int(GTAB_PRESS_FULL_AUTO_SEND, GTAB_OPTION_AUTO);
  gtab_disp_partial_match = get_gcin_conf_int(GTAB_DISP_PARTIAL_MATCH, GTAB_OPTION_AUTO);
  gtab_disp_key_codes = get_gcin_conf_int(GTAB_DISP_KEY_CODES, 1);
  gtab_disp_im_name = get_gcin_conf_int(GTAB_DISP_IM_NAME, 1);
  gtab_invalid_key_in = get_gcin_conf_int(GTAB_INVALID_KEY_IN, 1);
  gtab_shift_phrase_key = get_gcin_conf_int(GTAB_SHIFT_PHRASE_KEY, 0);
  gtab_hide_row2 = get_gcin_conf_int(GTAB_HIDE_ROW2, 0);
  gtab_in_row1 = get_gcin_conf_int(GTAB_IN_ROW1, 0);
  gtab_capslock_in_eng = get_gcin_conf_int(GTAB_CAPSLOCK_IN_ENG, 1);
  gtab_vertical_select = get_gcin_conf_int(GTAB_VERTICAL_SELECT, GTAB_OPTION_AUTO);
  gtab_unique_auto_send = get_gcin_conf_int(GTAB_UNIQUE_AUTO_SEND, GTAB_OPTION_AUTO);
  gtab_que_wild_card = get_gcin_conf_int(GTAB_QUE_WILD_CARD, 0);
  gtab_phrase_pre_select = get_gcin_conf_int(GTAB_PHRASE_PRE_SELECT, 1);
  gtab_in_area_button = get_gcin_conf_int(GTAB_IN_AREA_BUTTON, 1);

  tsin_phrase_pre_select = get_gcin_conf_int(TSIN_PHRASE_PRE_SELECT, 1);
  tsin_chinese_english_toggle_key = get_gcin_conf_int(TSIN_CHINESE_ENGLISH_TOGGLE_KEY,
                                    TSIN_CHINESE_ENGLISH_TOGGLE_KEY_Shift);
  tsin_tone_char_input = get_gcin_conf_int(TSIN_TONE_CHAR_INPUT, 0);

  tsin_space_opt = get_gcin_conf_int(TSIN_SPACE_OPT, TSIN_SPACE_OPT_SELECT_CHAR);
  tsin_buffer_size = get_gcin_conf_int(TSIN_BUFFER_SIZE, 40);
  tsin_tab_phrase_end = get_gcin_conf_int(TSIN_TAB_PHRASE_END, 1);
  tsin_tail_select_key = get_gcin_conf_int(TSIN_TAIL_SELECT_KEY, 0);
  tsin_buffer_editing_mode = get_gcin_conf_int(TSIN_BUFFER_EDITING_MODE, 1);
  tsin_use_pho_near = get_gcin_conf_int(TSIN_USE_PHO_NEAR, 1);

  phonetic_char_dynamic_sequence = get_gcin_conf_int(PHONETIC_CHAR_DYNAMIC_SEQUENCE, 1);
  phonetic_huge_tab = get_gcin_conf_int(PHONETIC_HUGE_TAB, 0);
  phonetic_speak = get_gcin_conf_int(PHONETIC_SPEAK, 0);
  get_gcin_conf_str(PHONETIC_SPEAK_SEL, &phonetic_speak_sel, "3.ogg");

  pho_hide_row2 = get_gcin_conf_int(PHO_HIDE_ROW2, 0);
  pho_in_row1 = get_gcin_conf_int(PHO_IN_ROW1, 1);

  get_gcin_conf_str(TSIN_PHRASE_LINE_COLOR, &tsin_phrase_line_color, "blue");
  get_gcin_conf_str(TSIN_CURSOR_COLOR, &tsin_cursor_color, "blue");
  get_gcin_conf_str(GCIN_SEL_KEY_COLOR, &gcin_sel_key_color, "blue");

  if (eng_color_full_str) {
    g_free(eng_color_full_str);
    g_free(cht_color_full_str);
  }

  eng_color_full_str = g_strdup_printf("<span foreground=\"%s\">%s</span>", gcin_sel_key_color, _(eng_full_str));
  cht_color_full_str = g_strdup_printf("<span foreground=\"%s\">%s</span>", gcin_sel_key_color, _(cht_full_str));

  get_gcin_conf_str(GCIN_WIN_COLOR_FG, &gcin_win_color_fg, "white");
  get_gcin_conf_str(GCIN_WIN_COLOR_BG, &gcin_win_color_bg, "#005BFF");
  gcin_win_color_use = get_gcin_conf_int(GCIN_WIN_COLOR_USE, 0);
  gcin_bell_off = get_gcin_conf_int(GCIN_BELL_OFF, 0);


#if USE_GCB
  gcb_enabled = get_gcin_conf_int(GCB_ENABLED, 0);
  gcb_position = get_gcin_conf_int(GCB_POSITION, 4);
  gcb_position_x = get_gcin_conf_int(GCB_POSITION_X, 0);
  gcb_position_y = get_gcin_conf_int(GCB_POSITION_Y, 0);
  gcb_button_n = get_gcin_conf_int(GCB_BUTTON_N, 3);
  gcb_history_n = get_gcin_conf_int(GCB_HISTORY_N, 10);
#endif
  gcin_bell_volume = get_gcin_conf_int(GCIN_BELL_VOLUME, -97);
  gcin_sound_play_overlap = get_gcin_conf_int(GCIN_SOUND_PLAY_OVERLAP, 0);
  gcin_enable_ctrl_alt_switch = get_gcin_conf_int(GCIN_ENABLE_CTRL_ALT_SWITCH, 1);
#if 1
  gcin_edit_display = get_gcin_conf_int(GCIN_EDIT_DISPLAY, GCIN_EDIT_DISPLAY_BOTH);
#elif 0
  gcin_edit_display = get_gcin_conf_int(GCIN_EDIT_DISPLAY, GCIN_EDIT_DISPLAY_ON_THE_SPOT);
#else
  gcin_edit_display = get_gcin_conf_int(GCIN_EDIT_DISPLAY, GCIN_EDIT_DISPLAY_OVER_THE_SPOT);
#endif

  gcin_on_the_spot_key = get_gcin_conf_int(GCIN_ON_THE_SPOT_KEY, 1);
  if (gcin_on_the_spot_key)
    gcin_edit_display = GCIN_EDIT_DISPLAY_ON_THE_SPOT;


  char phokbm[MAX_GCIN_STR];
#define ASDF "asdfghjkl;"
#define N1234 "123456789"

#if DEBUG
  char *gcin_pho_kbm = getenv("GCIN_PHO_KBM");
  if (gcin_pho_kbm)
    strcpy(phokbm, gcin_pho_kbm);
  else
#endif
  {
    char *kbm_str = is_chs?"pinyin "N1234" 1 1":"zo "N1234" 1 1";
    get_gcin_conf_fstr(PHONETIC_KEYBOARD, phokbm, kbm_str);
  }

  char phokbm_name[32], selkey[32];
  pho_candicate_col_N=0; pho_candicate_R2L=0;
  sscanf(phokbm, "%s %s %d %d",phokbm_name, selkey, &pho_candicate_col_N, &pho_candicate_R2L);

  if (pho_candicate_col_N <= 0)
    pho_candicate_col_N = 1;
  if (pho_candicate_col_N > strlen(selkey))
    pho_candicate_col_N =strlen(selkey);

  if (pho_candicate_R2L<0 || pho_candicate_R2L>1)
    pho_candicate_R2L = 0;

  if (pho_selkey)
    free(pho_selkey);
  pho_selkey = strdup(selkey);

  if (pho_kbm_name)
    free(pho_kbm_name);
  pho_kbm_name = strdup(phokbm_name);
}
