#define CHANGE_FONT_SIZE "change font size"
#define GB_OUTPUT_TOGGLE "gb output toggle"


#define GCIN_FONT_SIZE "gcin-font-size"
#define GCIN_FONT_NAME "gcin-font-name"
#define GCIN_FONT_SIZE_TSIN_PRESEL "gcin-font-size-tsin-presel"
#define GCIN_FONT_SIZE_SYMBOL "gcin-font-size-symbol"
#define GCIN_FONT_SIZE_TSIN_PHO_IN "gcin-font-size-tsin-pho-in"
#define GCIN_FONT_SIZE_GTAB_IN "gcin-font-size-gtab-in"
#define GCIN_FONT_SIZE_PHO_NEAR "gcin-font-size-pho-near"
#define GCIN_INPUT_STYLE "gcin-input-style"
#define GCIN_ROOT_X      "gcin-root-x"
#define GCIN_ROOT_Y      "gcin-root-y"
#define GCIN_POP_UP_WIN "gcin-pop-up-win"
#define GCIN_POP_UP_WIN_ABS_CORNER "gcin-pop-up-win-abs-corner"
#define GCIN_INNER_FRAME "gcin-inner-frame"

#define GCIN_IM_TOGGLE_KEYS "gcin-im-toggle-keys"
#define DEFAULT_INPUT_METHOD "default-input-method"
#define LEFT_RIGHT_BUTTON_TIPS "left-right-button-tips"
#define GCIN_CHARS_BIG5_ONLY "gcin-chars-big5-only"
#define GCIN_FLAGS_IM_ENABLED "gcin-flags-im-enabled"
#define GCIN_REMOTE_CLIENT "gcin-remote-client"
#define GCIN_SHIFT_SPACE_ENG_FULL "gcin-shift-space-eng-full"
#define GCIN_STATUS_TRAY "gcin-status-tray"
#define GCIN_WIN_COLOR_FG "gcin-win-color-fg"
#define GCIN_WIN_COLOR_BG "gcin-win-color-bg"
#define GCIN_WIN_COLOR_USE "gcin-win-color-use"


#define GTAB_DUP_SELECT_BELL "gtab-dup-select-bell"
#define GTAB_SPACE_AUTO_FIRST "gtab-space-auto-first"
#define GTAB_AUTO_SELECT_BY_PHRASE "gtab-auto-select-by_phrase"
#define GTAB_PRE_SELECT "gtab-pre-select"
#define GTAB_PRESS_FULL_AUTO_SEND "gtab-press-full-auto-send"
#define GTAB_DISP_PARTIAL_MATCH "gtab-disp-partial-match"
#define GTAB_SIMPLE_WIN "gtab-simple-win"
#define GTAB_DISP_KEY_CODES "gtab-disp-key-codes"
#define GTAB_DISP_IM_NAME "gtab-disp-im-name"
#define GTAB_INVALID_KEY_IN "gtab-invalid-key-in"
#define GTAB_SHIFT_PHRASE_KEY "gtab-shift-phrase-key"
#define GTAB_HIDE_ROW2 "gtab-hide-row2"
#define GTAB_IN_ROW1 "gtab-in-row1"
#define GTAB_CAPSLOCK_IN_ENG "gtab-capslock-in-eng"
#define GTAB_VERTICAL_SELECT "gtab-vertical-select"

#define PHO_SIMPLE_WIN "pho-simple-window" // common to both pho and tsin
#define TSIN_PHRASE_PRE_SELECT "tsin-phrase-pre-select"
#define TSIN_CHINESE_ENGLISH_TOGGLE_KEY "tsin-chinese-english-toggle_key"
#define TSIN_CHINESE_ENGLISH_SWITCH "tsin-chinese-english-switch"
#define TSIN_SPACE_OPT "tsin-space-opt"
#define TSIN_BUFFER_SIZE "tsin-buffer-size"
#define TSIN_PHRASE_LINE_COLOR "tsin-phrase-line-color"
#define TSIN_CURSOR_COLOR "tsin-cursor-color"
#define TSIN_TONE_CHAR_INPUT "tsin-tone-char-input"
#define TSIN_TAB_PHRASE_END "tsin-tab-phrase-end"
#define TSIN_TAIL_SELECT_KEY "tsin-tail-select-key"

#define PHO_HIDE_ROW2 "pho-hide-row2"
#define PHO_IN_ROW1 "pho-in-row1"


#define PHONETIC_KEYBOARD "phonetic-keyboard"
#define PHONETIC_CHAR_DYNAMIC_SEQUENCE "phonetic-char-dynamic-sequence"
#define PHONETIC_HUGE_TAB "phonetic-huge-tab"


extern int gcin_font_size, gcin_font_size_tsin_presel, gcin_font_size_symbol,
           gcin_font_size_tsin_pho_in, gcin_font_size_pho_near, gcin_chars_big5_only,
           gcin_font_size_gtab_in, gcin_inner_frame,
           gcin_flags_im_enabled, gcin_remote_client, gtab_simple_win,
           gtab_disp_key_codes, gtab_disp_im_name, gcin_shift_space_eng_full,
           gtab_invalid_key_in, gtab_hide_row2, gtab_in_row1, gtab_capslock_in_eng,
           pho_hide_row2,
           pho_in_row1;

extern int default_input_method;
extern int left_right_button_tips;
extern int gtab_dup_select_bell;
extern int gtab_space_auto_first;
extern int gtab_auto_select_by_phrase;
extern int gcin_im_toggle_keys;
extern int gtab_pre_select;
extern int gtab_press_full_auto_send;
extern int gtab_disp_partial_match;
extern int gtab_shift_phrase_key;
extern int gtab_vertical_select;
extern int tsin_ec_toggle_key;
extern int pho_simple_win, tsin_buffer_size;
extern int gcin_input_style, gcin_root_x, gcin_root_y, gcin_pop_up_win, gcin_pop_up_win_abs_corner;
extern int gcin_status_tray;

extern int tsin_phrase_pre_select;
extern int tsin_chinese_english_toggle_key;
extern int tsin_tab_phrase_end, tsin_tail_select_key;

extern int phonetic_char_dynamic_sequence;
extern int phonetic_huge_tab;
extern int tsin_space_opt, tsin_tone_char_input;

extern char *tsin_phrase_line_color, *tsin_cursor_color, *gcin_font_name;
extern char *gcin_win_color_fg, *gcin_win_color_bg;
extern int gcin_win_color_use;

void get_gcin_user_fname(char *name, char fname[]);
void get_gcin_conf_str(char *name, char **rstr, char *default_str);
void get_gcin_conf_fstr(char *name, char rstr[], char *default_str);
void save_gcin_conf_str(char *name, char *str);
void save_gcin_conf_int(char *name, int val);
void load_setttings();

#define TRAY_ENABLED 1
