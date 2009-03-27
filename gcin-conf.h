#define CHANGE_FONT_SIZE "change font size"

#define GCIN_FONT_SIZE "gcin-font-size"
#define GCIN_FONT_SIZE_TSIN_PRESEL "gcin-font-size-tsin-presel"
#define GCIN_FONT_SIZE_SYMBOL "gcin-font-size-symbol"
#define GCIN_FONT_SIZE_TSIN_PHO_IN "gcin-font-size-tsin-pho-in"
#define GCIN_IM_TOGGLE_KEYS "gcin-im-toggle-keys"
#define DEFAULT_INPUT_METHOD "default-input-method"
#define LEFT_RIGHT_BUTTON_TIPS "left-right-button-tips"
#define GCIN_CHARS_BIG5_ONLY "gcin-chars-big5-only"
#define GCIN_FLAGS_IM_ENABLED "gcin-flags-im-enabled"
#define GCIN_REMOTE_CLIENT "gcin-remote-client"
#define GCIN_SHIFT_SPACE_ENG_FULL "gcin-shift-space-eng-full"


#define GTAB_DUP_SELECT_BELL "gtab-dup-select-bell"
#define GTAB_SPACE_AUTO_FIRST "gtab-space-auto-first"
#define GTAB_AUTO_SELECT_BY_PHRASE "gtab-auto-select-by_phrase"
#define GTAB_PRE_SELECT "gtab-pre-select"
#define GTAB_PRESS_FULL_AUTO_SEND "gtab-press-full-auto-send"
#define GTAB_DISP_PARTIAL_MATCH "gtab-disp-partial-match"
#define GTAB_SIMPLE_WIN "gtab-simple-win"
#define GTAB_DISP_KEY_CODES "gtab-disp-key-codes"
#define GTAB_DISP_IM_NAME "gtab-disp-im-name"

#define TSIN_PHRASE_PRE_SELECT "tsin-phrase-pre-select"
#define TSIN_CHINESE_ENGLISH_TOGGLE_KEY "tsin-chinese-english-toggle_key"
#define TSIN_CHINESE_ENGLISH_SWITCH "tsin-chinese-english-switch"
#define TSIN_DISP_STATUS_ROW "tsin-disp-status-row"
#define TSIN_SPACE_OPT "tsin-space-opt"


#define PHONETIC_KEYBOARD "phonetic-keyboard"
#define PHONETIC_CHAR_DYNAMIC_SEQUENCE "phonetic-char-dynamic-sequence"
#define PHONETIC_HUGE_TAB "phonetic-huge-tab"


extern int gcin_font_size, gcin_font_size_tsin_presel, gcin_font_size_symbol,
           gcin_font_size_tsin_pho_in, gcin_chars_big5_only,
           gcin_flags_im_enabled, gcin_remote_client, gtab_simple_win,
           gtab_disp_key_codes, gtab_disp_im_name, gcin_shift_space_eng_full;

extern int default_input_method;
extern int left_right_button_tips;
extern int gtab_dup_select_bell;
extern int gtab_space_auto_first;
extern int gtab_auto_select_by_phrase;
extern int gcin_im_toggle_keys;
extern int gtab_pre_select;
extern int gtab_press_full_auto_send;
extern int gtab_disp_partial_match;
extern int tsin_ec_toggle_key;
extern int tsin_disp_status_row;

extern int tsin_phrase_pre_select;
extern int tsin_chinese_english_toggle_key;

extern int phonetic_char_dynamic_sequence;
extern int phonetic_huge_tab;
extern int tsin_space_opt;


void get_gcin_user_fname(char *name, char fname[]);
void get_gcin_conf_str(char *name, char rstr[], char *default_str);
void save_gcin_conf_str(char *name, char *str);
void save_gcin_conf_int(char *name, int val);
void load_setttings();
