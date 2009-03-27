#define CHANGE_FONT_SIZE "change font size"

#define GCIN_FONT_SIZE "gcin-font-size"
#define GCIN_FONT_SIZE_TSIN_PRESEL "gcin-font-size-tsin-presel"
#define GCIN_FONT_SIZE_SYMBOL "gcin-font-size-symbol"
#define GCIN_FONT_SIZE_TSIN_PHO_IN "gcin-font-size-tsin-pho-in"
#define DEFAULT_INPUT_METHOD "default-input-method"
#define LEFT_RIGHT_BUTTON_TIPS "left-right-button-tips"
#define GTAB_DUP_SELECT_BELL "gtab-dup-select-bell"
#define GTAB_SPACE_AUTO_FIRST "gtab-space-auto-first"
#define GTAB_AUTO_SELECT_BY_PHRASE "gtab-auto-select-by_phrase"
#define GCIN_IM_TOGGLE_KEYS "gcin-im-toggle-keys"
#define GTAB_PRE_SELECT "gtab-pre-select"
#define GTAB_PRESS_FULL_AUTO_SEND "gtab-press-full-auto-send"
#define GTAB_DISP_PARTIAL_MATCH "gtab-disp-partial-match"

#define TSIN_PHRASE_PRE_SELECT "tsin-phrase-pre-select"
#define TSIN_CHINESE_ENGLISH_TOGGLE_KEY "tsin-chinese-english-toggle_key"
#define TSIN_CHINESE_ENGLISH_SWITCH "tsin-chinese-english-switch"


#define PHONETIC_KEYBOARD "phonetic-keyboard"

#define PHONETIC_CHAR_DYNAMIC_SEQUENCE "phonetic-char-dynamic-sequence"


extern int gcin_font_size, gcin_font_size_tsin_presel, gcin_font_size_symbol,
           gcin_font_size_tsin_pho_in;
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

extern int tsin_phrase_pre_select;
extern int tsin_chinese_english_toggle_key;

extern int phonetic_char_dynamic_sequence;


void get_gcin_user_fname(char *name, char fname[]);
void get_gcin_conf_str(char *name, char rstr[], char *default_str);
void save_gcin_conf_str(char *name, char *str);
void save_gcin_conf_int(char *name, int val);
void load_setttings();
