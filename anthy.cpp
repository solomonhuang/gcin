#include "gcin.h"
#include "pho.h"
#include "gst.h"
#include <anthy/anthy.h>
extern gboolean test_mode;
static anthy_context_t ac;
void (*f_anthy_resize_segment)(anthy_context_t ac, int, int);
void (*f_anthy_get_stat)(anthy_context_t ac, struct anthy_conv_stat *acs);
void (*f_anthy_get_segment)(anthy_context_t ac, int,int,char *, int);
void (*f_anthy_get_segment_stat)(anthy_context_t ac, int, struct anthy_segment_stat *);
void (*f_anthy_commit_segment)(anthy_context_t ac, int, int);
void (*f_anthy_set_string)(anthy_context_t ac, char *);
extern gint64 key_press_time;
static GtkWidget *event_box_anthy;
gint64 current_time();

void show_win_anthy();
void hide_win_anthy();
void change_anthy_font_size();

enum {
  STATE_hira=0,
  STATE_kata=1,
  STATE_half_kata=2
};

static int state_hira_kata=STATE_hira;

struct {
  char *en;
  char *hira;
  char *kata;
  char *half_kata;
} anthy_romaji_map[] = {
{"xtu",	"っ",  "ッ", "ｯ"},
{"xtsu",	"っ",  "ッ", "ｯ"},
{"ltu",	"っ",  "ッ", "ｯ"},
{"ltsu",	"っ",  "ッ", "ｯ"},

{"-",	"ー", "ー", "ｰ"},
{"a",	"あ", "ア", "ｱ"},
{"i",	"い", "イ", "ｲ"},
{"u",	"う", "ウ", "ｳ"},
{"e",	"え", "エ", "ｴ"},
{"o",	"お", "オ", "ｵ"},

{"xa",	"ぁ", "ァ", "ｧ"},
{"xi",	"ぃ", "ィ", "ｨ"},
{"xu",	"ぅ", "ゥ", "ｩ"},
{"xe",	"ぇ", "ェ", "ｪ"},
{"xo",	"ぉ", "ォ", "ｫ"},

{"la",	"ぁ", "ァ", "ｧ"},
{"li",	"ぃ", "ィ", "ｨ"},
{"lu",	"ぅ", "ゥ", "ｩ"},
{"le",	"ぇ", "ェ", "ｪ"},
{"lo",	"ぉ", "ォ", "ｫ"},

{"wi",	"うぃ", "ウィ", "ｳｨ"},
{"we",	"うぇ", "ウェ", "ｳｪ"},
{"wha",	"うぁ", "ウァ", "ｳｧ"},
{"whi",	"うぃ", "ウィ", "ｳｨ"},
{"whe",	"うぇ", "ウェ", "ｳｪ"},
{"who",	"うぉ", "ウォ", "ｳｫ"},

{"va",	"う゛ぁ", "ヴァ", "ｳﾞｧ"},
{"vi",	"う゛ぃ", "ヴィ", "ｳﾞｨ"},
{"vu",	"う゛", "ヴ", "ｳﾞ"},
{"ve",	"う゛ぇ", "ヴェ", "ｳﾞｪ"},
{"vo",	"う゛ぉ", "ヴォ", "ｳﾞｫ"},

{"ka",	"か", "カ", "ｶ"},
{"ki",	"き", "キ", "ｷ"},
{"ku",	"く", "ク", "ｸ"},
{"ke",	"け", "ケ", "ｹ"},
{"ko",	"こ", "コ", "ｺ"},

{"ga",	"が", "ガ", "ｶﾞ"},
{"gi",	"ぎ", "ギ", "ｷﾞ"},
{"gu",	"ぐ", "グ", "ｸﾞ"},
{"ge",	"げ", "ゲ", "ｹﾞ"},
{"go",	"ご", "ゴ", "ｺﾞ"},

{"kya",	"きゃ", "キャ", "ｷｬ"},
{"kyi",	"きぃ", "キィ", "ｷｨ"},
{"kyu",	"きゅ", "キュ", "ｷｭ"},
{"kye",	"きぇ", "キェ", "ｷｪ"},
{"kyo",	"きょ", "キョ", "ｷｮ"},
{"gya",	"ぎゃ", "ギャ", "ｷﾞｬ"},
{"gyi",	"ぎぃ", "ギィ", "ｷﾞｨ"},
{"gyu",	"ぎゅ", "ギュ", "ｷﾞｭ"},
{"gye",	"ぎぇ", "ギェ", "ｷﾞｪ"},
{"gyo",	"ぎょ", "ギョ", "ｷﾞｮ"},

{"sa",	"さ", "サ", "ｻ"},
{"si",	"し", "シ", "ｼ"},
{"su",	"す", "ス", "ｽ"},
{"se",	"せ", "セ", "ｾ"},
{"so",	"そ", "ソ", "ｿ"},

{"za",	"ざ", "ザ", "ｻﾞ"},
{"zi",	"じ", "ジ", "ｼﾞ"},
{"zu",	"ず", "ズ", "ｽﾞ"},
{"ze",	"ぜ", "ゼ", "ｾﾞ"},
{"zo",	"ぞ", "ゾ", "ｿﾞ"},

{"sya",	"しゃ", "シャ", "ｼｬ"},
{"syi",	"しぃ", "シィ", "ｼｨ"},
{"syu",	"しゅ", "シュ", "ｼｭ"},
{"sye",	"しぇ", "シェ", "ｼｪ"},
{"syo",	"しょ", "ショ", "ｼｮ"},
{"sha",	"しゃ", "シャ", "ｼｬ"},
{"shi",	"し", "シ", "ｼ"},
{"shu",	"しゅ", "シュ", "ｼｭ"},
{"she",	"しぇ", "シェ", "ｼｪ"},
{"sho",	"しょ", "ショ", "ｼｮ"},
{"zya",	"じゃ", "ジャ", "ｼﾞｬ"},
{"zyi",	"じぃ", "ジィ", "ｼﾞｨ"},
{"zyu",	"じゅ", "ジュ", "ｼﾞｭ"},
{"zye",	"じぇ", "ジェ", "ｼﾞｪ"},
{"zyo",	"じょ", "ジョ", "ｼﾞｮ"},
{"ja",	"じゃ", "ジャ", "ｼﾞｬ"},
{"jya", "じゃ", "ジャ", "ｼﾞｬ"},
{"ji",	"じ", "ジ", "ｼﾞ"},
{"jyi",	"じぃ", "ジィ", "ｼﾞｨ"},
{"ju",	"じゅ", "ジュ", "ｼﾞｭ"},
{"jyu",	"じゅ", "ジュ", "ｼﾞｭ"},
{"je",	"じぇ", "ジェ", "ｼﾞｪ"},
{"jye",	"じぇ", "ジェ", "ｼﾞｪ"},
{"jo",	"じょ", "ジョ", "ｼﾞｮ"},
{"jyo",	"じょ", "ジョ", "ｼﾞｮ"},
{"ta",	"た", "タ", "ﾀ"},
{"ti",	"ち", "チ", "ﾁ"},
{"tu",	"つ", "ツ", "ﾂ"},
{"tsu",	"つ", "ツ", "ﾂ"},
{"te",	"て", "テ", "ﾃ"},
{"to",	"と", "ト", "ﾄ"},

{"da",	"だ", "ダ", "ﾀﾞ"},
{"di",	"ぢ", "ヂ", "ﾁﾞ"},
{"du",	"づ", "ヅ", "ﾂﾞ"},
{"de",	"で", "デ", "ﾃﾞ"},
{"do",	"ど", "ド", "ﾄﾞ"},


{"tya",	"ちゃ", "チャ", "ﾁｬ"},
{"tyi",	"ちぃ", "チィ", "ﾁｨ"},
{"tyu",	"ちゅ", "チュ", "ﾁｭ"},
{"tye",	"ちぇ", "チェ", "ﾁｪ"},
{"tyo",	"ちょ", "チョ", "ﾁｮ"},

{"cha",	"ちゃ", "チャ", "ﾁｬ"},
{"chi",	"ち", "チ", "ﾁ"},
{"chu",	"ちゅ", "チュ", "ﾁｭ"},
{"che",	"ちぇ", "チェ", "ﾁｪ"},
{"cho",	"ちょ", "チョ", "ﾁｮ"},

{"dya",	"ぢゃ", "ヂャ", "ﾁﾞｬ"},
{"dyi",	"ぢぃ", "ヂィ", "ﾁﾞｨ"},
{"dyu",	"ぢゅ", "ヂュ", "ﾁﾞｭ"},
{"dye",	"ぢぇ", "ヂェ", "ﾁﾞｪ"},
{"dyo",	"ぢょ", "ヂョ", "ﾁﾞｮ"},

{"tha",	"てゃ", "テャ", "ﾃｬ"},
{"thi",	"てぃ", "ティ", "ﾃｨ"},
{"thu",	"てゅ", "テュ", "ﾃｭ"},
{"the",	"てぇ", "テェ", "ﾃｪ"},
{"tho",	"てょ", "テョ", "ﾃｮ"},

{"dha",	"でゃ", "デャ", "ﾃﾞｬ"},
{"dhi",	"でぃ", "ディ", "ﾃﾞｨ"},
{"dhu",	"でゅ", "デュ", "ﾃﾞｭ"},
{"dhe",	"でぇ", "デェ", "ﾃﾞｪ"},
{"dho",	"でょ", "デョ", "ﾃﾞｮ"},

{"na",	"な", "ナ", "ﾅ"},
{"ni",	"に", "ニ", "ﾆ"},
{"nu",	"ぬ", "ヌ", "ﾇ"},
{"ne",	"ね", "ネ", "ﾈ"},
{"no",	"の", "ノ", "ﾉ"},
{"nya",	"にゃ", "ニャ", "ﾆｬ"},
{"nyi",	"にぃ", "ニィ", "ﾆｨ"},
{"nyu",	"にゅ", "ニュ", "ﾆｭ"},
{"nye",	"にぇ", "ニェ", "ﾆｪ"},
{"nyo",	"にょ", "ニョ", "ﾆｮ"},

{"ha",	"は", "ハ", "ﾊ"},
{"hi",	"ひ", "ヒ", "ﾋ"},
{"hu",	"ふ", "フ", "ﾌ"},
{"he",	"へ", "ヘ", "ﾍ"},
{"ho",	"ほ", "ホ", "ﾎ"},

{"ba",	"ば", "バ", "ﾊﾞ"},
{"bi",	"び", "ビ", "ﾋﾞ"},
{"bu",	"ぶ", "ブ", "ﾌﾞ"},
{"be",	"べ", "ベ", "ﾍﾞ"},
{"bo",	"ぼ", "ボ", "ﾎﾞ"},

{"pa",	"ぱ", "パ", "ﾊﾟ"},
{"pi",	"ぴ", "ピ", "ﾋﾟ"},
{"pu",	"ぷ", "プ", "ﾌﾟ"},
{"pe",	"ぺ", "ペ", "ﾍﾟ"},
{"po",	"ぽ", "ポ", "ﾎﾟ"},

{"hya",	"ひゃ", "ヒャ", "ﾋｬ"},
{"hyi",	"ひぃ", "ヒィ", "ﾋｨ"},
{"hyu",	"ひゅ", "ヒュ", "ﾋｭ"},
{"hye",	"ひぇ", "ヒェ", "ﾋｪ"},
{"hyo",	"ひょ", "ヒョ", "ﾋｮ"},
{"bya",	"びゃ", "ビャ", "ﾋﾞｬ"},
{"byi",	"びぃ", "ビィ", "ﾋﾞｨ"},
{"byu",	"びゅ", "ビュ", "ﾋﾞｭ"},
{"bye",	"びぇ", "ビェ", "ﾋﾞｪ"},
{"byo",	"びょ", "ビョ", "ﾋﾞｮ"},
{"pya",	"ぴゃ", "ピャ", "ﾋﾟｬ"},
{"pyi",	"ぴぃ", "ピィ", "ﾋﾟｨ"},
{"pyu",	"ぴゅ", "ピュ", "ﾋﾟｭ"},
{"pye",	"ぴぇ", "ピェ", "ﾋﾟｪ"},
{"pyo",	"ぴょ", "ピョ", "ﾋﾟｮ"},

{"fa",	"ふぁ", "ファ", "ﾌｧ"},
{"fi",	"ふぃ", "フィ", "ﾌｨ"},
{"fu",	"ふ", "フ", "ﾌ"},
{"fe",	"ふぇ", "フェ", "ﾌｪ"},
{"fo",	"ふぉ", "フォ", "ﾌｫ"},

{"ma",	"ま", "マ", "ﾏ"},
{"mi",	"み", "ミ", "ﾐ"},
{"mu",	"む", "ム", "ﾑ"},
{"me",	"め", "メ", "ﾒ"},
{"mo",	"も", "モ", "ﾓ"},

{"mya",	"みゃ", "ミャ", "ﾐｬ"},
{"myi",	"みぃ", "ミィ", "ﾐｨ"},
{"myu",	"みゅ", "ミュ", "ﾐｭ"},
{"mye",	"みぇ", "ミェ", "ﾐｪ"},
{"myo",	"みょ", "ミョ", "ﾐｮ"},
{"lya",	"ゃ", "ャ", "ｬ"},
{"xya",	"ゃ", "ャ", "ｬ"},
{"ya",	"や", "ヤ", "ﾔ"},
{"lyu",	"ゅ", "ュ", "ｭ"},
{"xyu",	"ゅ", "ュ", "ｭ"},
{"yu",	"ゆ", "ユ", "ﾕ"},
{"lyo",	"ょ", "ョ", "ｮ"},
{"xyo",	"ょ", "ョ", "ｮ"},
{"yo",	"よ", "ヨ", "ﾖ"},

{"ra",	"ら", "ラ", "ﾗ"},
{"ri",	"り", "リ", "ﾘ"},
{"ru",	"る", "ル", "ﾙ"},
{"re",	"れ", "レ", "ﾚ"},
{"ro",	"ろ", "ロ", "ﾛ"},

{"rya",	"りゃ", "リャ", "ﾘｬ"},
{"ryi",	"りぃ", "リィ", "ﾘｨ"},
{"ryu",	"りゅ", "リュ", "ﾘｭ"},
{"rye",	"りぇ", "リェ", "ﾘｪ"},
{"ryo",	"りょ", "リョ", "ﾘｮ"},
{"xwa",	"ゎ", "ヮ", "ﾜ"},
{"wa",	"わ", "ワ", "ﾜ"},
{"wo",	"を", "ヲ", "ｦ"},
{"n'",	"ん", "ン", "ﾝ"},
{"nn",	"ん", "ン", "ﾝ"},
{"n",	"ん", "ン", "ﾝ"},
{"m",	"ん", "ン", "ﾝ"}, // tombo
{"wyi",	"ゐ", "ヰ", "ｨ"},
{"wye",	"ゑ", "ヱ", "ｪ"},
{"cya",	"ちゃ", "チャ", "ﾁｬ"},
{"cye",	"ちぇ", "チェ", "ﾁｪ"},
{"cyi",	"ちぃ", "チィ", "ﾁｨ"},
{"cyo",	"ちょ", "チョ", "ﾁｮ"},
{"cyu",	"ちゅ", "チュ", "ﾁｭ"},
{"fya",	"ふゃ", "フャ", "ﾌｬ"},
{"fye",	"ふぇ", "フェ", "ﾌｪ"},
{"fyi",	"ふぃ", "フィ", "ﾌｨ"},
{"fyo",	"ふょ", "フョ", "ﾌｮ"},
{"fyu",	"ふゅ", "フュ", "ﾌｭ"},
{"lye",	"ぇ", "ェ", "ｪ"},
{"lyi",	"ぃ", "ィ", "ｨ"},
{"qa",	"くぁ", "クァ", "ｸｧ"},
{"qe",	"くぇ", "クェ", "ｸｪ"},
{"qi",	"くぃ", "クィ", "ｸｨ"},
{"qo",	"くぉ", "クォ", "ｸｫ"},
{"qu",	"く", "ク", "ｸ"},
{"tsa",	"つぁ", "ツァ", "ﾂｧ"},
{"tse",	"つぇ", "ツェ", "ﾂｪ"},
{"tsi",	"つぃ", "ツィ", "ﾂｨ"},
{"tso",	"つぉ", "ツォ", "ﾂｫ"},
{"vya",	"う゛ゃ", "ヴャ", "ｳﾞｬ"},
{"vyo",	"う゛ょ", "ヴョ", "ｳﾞｮ"},
{"vyu",	"う゛ゅ", "ヴュ", "ｳﾞｭ"},
{"whu",	"う", "ウ", "ｳ"},
{"wu",	"う", "ウ", "ｳ"},
{"xca",	"ヵ", "ヵ", "ｶ"},
{"xka",	"ヵ", "ヵ", "ｶ"},
{"xke",	"ヶ", "ヶ", "ｹ"},
{"xye",	"ぇ", "ェ", "ｪ"},
{"xyi",	"ぃ", "ィ", "ｨ"},
{"ye",	"いぇ", "イェ", "ｲｪ"},
{"z,",	"‥", "‥", ""},
{"z-",	"〜", "〜", ""},
{"z.",	"…", "…", ""},
{"z/",	"・", "・", "･"},
{"z[",	"『", "『", ""},
{"z]",	"』", "』", ""},
{"zh",	"←", "←", ""},
{"zj",	"↓", "↓", ""},
{"zk",	"↑", "↑", ""},
{"zl",	"→", "→", ""},
{"xwi",	"ゐ", "ヰ", "ｨ"},
{"xwe",	"ゑ", "ヱ", "ｪ"},
{"bwa",	"ぶぁ", "ブァ", "ﾌﾞｧ"},
{"bwi",	"ぶぃ", "ブィ", "ﾌﾞｨ"},
{"bwu",	"ぶぅ", "ブゥ", "ﾌﾞｩ"},
{"bwe",	"ぶぇ", "ブェ", "ﾌﾞｪ"},
{"bwo",	"ぶぉ", "ブォ", "ﾌﾞｫ"},
{"d'i",	"でぃ", "ディ", "ﾃﾞｨ"},
{"d'yu",	"でゅ", "デュ", "ﾃﾞｭ"},
{"d'u",	"どぅ", "ドゥ", "ﾄﾞｩ"},
{"dsu",	"づ", "ヅ", "ﾂﾞ"},
{"dwa",	"どぁ", "ドァ", "ﾄﾞｧ"},
{"dwi",	"どぃ", "ドィ", "ﾄﾞｨ"},
{"dwu",	"どぅ", "ドゥ", "ﾄﾞｩ"},
{"dwe",	"どぇ", "ドェ", "ﾄﾞｪ"},
{"dwo",	"どぉ", "ドォ", "ﾄﾞｫ"},
{"fwa",	"ふぁ", "ファ", "ﾌｧ"},
{"fwi",	"ふぃ", "フィ", "ﾌｨ"},
{"fwu",	"ふぅ", "フゥ", "ﾌｩ"},
{"fwe",	"ふぇ", "フェ", "ﾌｪ"},
{"fwo",	"ふぉ", "フォ", "ﾌｫ"},
{"gwa",	"ぐぁ", "グァ", "ｸﾞｧ"},
{"gwi",	"ぐぃ", "グィ", "ｸﾞｨ"},
{"gwu",	"ぐぅ", "グゥ", "ｸﾞｩ"},
{"gwe",	"ぐぇ", "グェ", "ｸﾞｪ"},
{"gwo",	"ぐぉ", "グォ", "ｸﾞｫ"},
{"hwa",	"ふぁ", "ファ", "ﾌｧ"},
{"hwi",	"ふぃ", "フィ", "ﾌｨ"},
{"hwe",	"ふぇ", "フェ", "ﾌｪ"},
{"hwo",	"ふぉ", "フォ", "ﾌｫ"},
{"kwa",	"くぁ", "クァ", "ｸｧ"},
{"kwi",	"くぃ", "クィ", "ｸｨ"},
{"kwu",	"くぅ", "クゥ", "ｸｩ"},
{"kwe",	"くぇ", "クェ", "ｸｪ"},
{"kwo",	"くぉ", "クォ", "ｸｫ"},
{"mwa",	"むぁ", "ムァ", "ﾑｧ"},
{"mwi",	"むぃ", "ムィ", "ﾑｨ"},
{"mwu",	"むぅ", "ムゥ", "ﾑｩ"},
{"mwe",	"むぇ", "ムェ", "ﾑｪ"},
{"mwo",	"むぉ", "ムォ", "ﾑｫ"},
{"pwa",	"ぷぁ", "プァ", "ﾌﾟｧ"},
{"pwi",	"ぷぃ", "プィ", "ﾌﾟｨ"},
{"pwu",	"ぷぅ", "プゥ", "ﾌﾟｩ"},
{"pwe",	"ぷぇ", "プェ", "ﾌﾟｪ"},
{"pwo",	"ぷぉ", "プォ", "ﾌﾟｫ"},
{"qwa",	"くぁ", "クァ", "ｸｧ"},
{"qwi",	"くぃ", "クィ", "ｸｨ"},
{"qwu",	"くぅ", "クゥ", "ｸｩ"},
{"qwe",	"くぇ", "クェ", "ｸｪ"},
{"qwo",	"くぉ", "クォ", "ｸｫ"},
{"qya",	"くゃ", "クャ", "ｸｬ"},
{"qyi",	"くぃ", "クィ", "ｸｨ"},
{"qyu",	"くゅ", "クュ", "ｸｭ"},
{"qye",	"くぇ", "クェ", "ｸｪ"},
{"qyo",	"くょ", "クョ", "ｸｮ"},
{"rwa",	"るぁ", "ルァ", "ﾙｧ"},
{"rwi",	"るぃ", "ルィ", "ﾙｨ"},
{"rwu",	"るぅ", "ルゥ", "ﾙｩ"},
{"rwe",	"るぇ", "ルェ", "ﾙｪ"},
{"rwo",	"るぉ", "ルォ", "ﾙｫ"},
{"swa",	"すぁ", "スァ", "ｽｧ"},
{"swi",	"すぃ", "スィ", "ｽｨ"},
{"swu",	"すぅ", "スゥ", "ｽｩ"},
{"swe",	"すぇ", "スェ", "ｽｪ"},
{"swo",	"すぉ", "スォ", "ｽｫ"},
{"t'i",	"てぃ", "ティ", "ﾃｨ"},
{"t'yu",	"てゅ", "テュ", "ﾃｭ"},
{"t'u",	"とぅ", "トゥ", "ﾄｩ"},
{"twa",	"とぁ", "トァ", "ﾄｧ"},
{"twi",	"とぃ", "トィ", "ﾄｨ"},
{"twu",	"とぅ", "トゥ", "ﾄｩ"},
{"twe",	"とぇ", "トェ", "ﾄｪ"},
{"two",	"とぉ", "トォ", "ﾄｫ"},
{"ywa",	"ゆぁ", "ユァ", "ﾕｧ"},
{"ywi",	"ゆぃ", "ユィ", "ﾕｨ"},
{"ywu",	"ゆぅ", "ユゥ", "ﾕｩ"},
{"ywe",	"ゆぇ", "ユェ", "ﾕｪ"},
{"ywo",	"ゆぉ", "ユォ", "ﾕｫ"},
{"zwa",	"ずぁ", "ズァ", "ｽﾞｧ"},
{"zwi",	"ずぃ", "ズィ", "ｽﾞｨ"},
{"zwu",	"ずぅ", "ズゥ", "ｽﾞｩ"},
{"zwe",	"ずぇ", "ズェ", "ｽﾞｪ"},
{"zwo",	"ずぉ", "ズォ", "ｽﾞｫ"},
{",",	"、", "、", "､"},
{".",	"。", "。", "｡"},
{"[",	"「", "「", "｢"},
{"]",	"」", "」", "｣"},
{"/",	"／", "／", "/"},
{"\\",	"＼", "＼", "\\"},
{"=",	"＝", "＝", "="},
{"+",	"＋", "＋", "+"},
{"_",	"＿", "＿", "_"},
{"¥",	"￥", "￥", "¥"},
{"~",	"〜", "〜", "~"},
{"!",	"！", "！", "!"},
{"@",	"＠", "＠", "@"},
{"#",	"＃", "＃", "#"},
{"$",	"＄", "＄", "$"},
{"%",	"％", "％", "%"},
{"^",	"＾", "＾", "^"},
{"&",	"＆", "＆", "&"},
{"*",	"＊", "＊", "*"},
{"(",	"（", "（", "("},
{")",	"）", "）", ")"},
{"<",	"＜", "＜", "<"},
{">",	"＞", "＞", ">"},
{"{",	"｛", "｛", "{"},
{"}",	"｝", "｝", "}"},
{"|",	"｜", "｜", "|"},
{"'",	"’", "’", "'"},
{"\"",	"”", "”", "\""},
{"`",	"‘", "‘", "`"},
{"?",	"？", "？", "?"},
{":",	"：", "：", ":"},
{";",	"；", "；", ";"},
{"0",	"0", "0", "0"},
{"1",	"1", "1", "1"},
{"2",	"2", "2", "2"},
{"3",	"3", "3", "3"},
{"4",	"4", "4", "4"},
{"5",	"5", "5", "5"},
{"6",	"6", "6", "6"},
{"7",	"7", "7", "7"},
{"8",	"8", "8", "8"},
{"9",	"9", "9", "9"},
};

static char *idx_hira_kata(int idx, gboolean always_hira)
{
  char *s=NULL;

  if (!always_hira) {
    if (state_hira_kata==STATE_kata)
      s = anthy_romaji_map[idx].kata;
    else
    if (state_hira_kata==STATE_half_kata)
      s = anthy_romaji_map[idx].half_kata;
  }

  if (!s)
    s = anthy_romaji_map[idx].hira;

  return s;
}


static short int anthy_romaji_mapN = sizeof(anthy_romaji_map)/sizeof(anthy_romaji_map[0]);

static int is_legal_char(int k)
{
  int i;

  if (k==' ')
    return 1;
  for(i=0; i < anthy_romaji_mapN; i++)
    if (strchr(anthy_romaji_map[i].en, k))
      return 1;
  return 0;
}

#define MAX_KEYS 32

static char keys[MAX_KEYS];
static short int keysN;
static u_short *jp;
static short int jpN=0;
static short pageidx;

typedef struct {
  GtkWidget *label;
  unsigned char selidx, selN;
} SEG;
static SEG *seg;
static short segN;
#define MAX_SEG_N 100
static short cursor;
enum {
  STATE_ROMANJI=1,
  STATE_CONVERT=2,
  STATE_SELECT=4,
};
static char state = STATE_ROMANJI;

static GtkWidget *win_anthy;

static gboolean is_empty()
{
  return !jpN && !segN && !keysN;
}

static void auto_hide()
{
//  puts("auto hide");
  if (is_empty() && gcin_pop_up_win) {
//    puts("empty");
    hide_win_anthy();
  }
}

static void insert_jp(u_short rom_idx)
{
//  printf("append %d %s  cursor:%d\n", rom_idx, anthy_romaji_map[rom_idx].ro, cursor);
  jp = trealloc(jp, u_short, jpN);
  if (cursor < jpN)
    memmove(jp+cursor+1, jp+cursor, jpN - cursor);

  jp[cursor]=rom_idx;
  cursor++;
  jpN++;
}

#define SEND_PRE_INI 65535

void parse_key()
{
  int i;
  int preN=0, eqN=0, sendpreN=0;
  u_short eq, sendpre_i = SEND_PRE_INI;
  static char ch2[]="kstzdhbrpfgvcjmwy";

  if (keysN==2 && keys[0]==keys[1] && strchr(ch2, keys[0])) {
    insert_jp(0);
    keys[1]=0;
    keysN=1;
    return;
  }

  for(i=0; i < anthy_romaji_mapN; i++) {
    char *en = anthy_romaji_map[i].en;
    if (!strncmp(keys, en, keysN))
      preN++;

    if (!strncmp(keys, en, strlen(en))) {
      sendpre_i = i;
      sendpreN++;
    }

    if (!strcmp(keys, en)) {
      eq = i;
      eqN++;
    }
  }

  if (preN > 1)
    return;

  if (eqN) {
    if (eqN > 1) {
      puts("bug");
      exit(1);
    }

    insert_jp(eq);

    keys[0]=0;
    keysN=0;
    return;
  }

  if (sendpre_i != SEND_PRE_INI) {
    char *en = anthy_romaji_map[sendpre_i].en;
    int len =strlen(en);
    int nlen = keysN - len;
    memmove(keys, keys+len, nlen);
    keys[nlen] = 0;
    keysN = nlen;

    insert_jp(sendpre_i);
  }
}

static void clear_seg_label()
{
//  dbg("clear_seg_label\n");
  int i;
  for(i=0; i < MAX_SEG_N; i++) {
    gtk_label_set_text(GTK_LABEL(seg[i].label), NULL);
    seg[i].selidx = 0;
  }
}

static void cursor_markup(int idx, char *s)
{
  char cur[256];
  GtkWidget *lab = seg[idx].label;
  sprintf(cur, "<span background=\"%s\">%s</span>", tsin_cursor_color, s);
  gtk_label_set_markup(GTK_LABEL(lab), cur);
}

void minimize_win_anthy()
{
  if (!win_anthy)
    return;
  gtk_window_resize(GTK_WINDOW(win_anthy), 32, 12);
}

static void disp_keys(int idx)
{
  int i;
  char tt[2];
  tt[1]=0;
  for(i=0; i < keysN; i++) {
    tt[0]=keys[i];
    gtk_label_set_text(GTK_LABEL(seg[idx+i].label), tt);
  }
}


static void disp_input()
{
  int i;

  if (gcin_edit_display_ap_only())
    return;

  clear_seg_label();

  int idx;
  for(idx=i=0; i < jpN; i++) {
    if (i==cursor) {
      disp_keys(idx);
      idx+=keysN;
      cursor_markup(idx++, idx_hira_kata(jp[i], FALSE));
    }
    else
      gtk_label_set_text(GTK_LABEL(seg[idx++].label), idx_hira_kata(jp[i], FALSE));
  }

  if (cursor==jpN) {
    disp_keys(idx);
    idx+=keysN;
    cursor_markup(idx, " ");
  }

  minimize_win_anthy();
}

static void disp_convert()
{
  int i;

//  printf("cursor %d\n", cursor);
  for(i=0; i < segN; i++) {
    char tt[256];
    strcpy(tt, gtk_label_get_text(GTK_LABEL(seg[i].label)));

    if (i==cursor && segN > 1)
      cursor_markup(i, tt);
    else
      gtk_label_set_text(GTK_LABEL(seg[i].label), tt);
  }
}

void delete_jpstr(int idx)
{
  if (idx==jpN)
    return;
  memmove(jp+idx, jp+idx+1, jpN-1-idx);
  jpN--;
}

static void clear_all()
{
  clear_seg_label();
  jpN=0;
  keys[0]=0;
  keysN = 0;
  segN = 0;
  cursor=0;
  tss.sel_pho = FALSE;
  state_hira_kata = STATE_hira;
  auto_hide();
}


static void send_seg()
{
  char out[512];
  int i;
  for(i=0, out[0]=0; i < segN; i++) {
    strcat(out, gtk_label_get_text(GTK_LABEL(seg[i].label)));
    (*f_anthy_commit_segment)(ac, i, seg[i].selidx);
    seg[i].selidx = 0;
  }

//  printf("sent convert '%s'\n", out);
  send_text(out);
  clear_all();
}

static void merge_jp(char out[], gboolean always_hira)
{
  int i;
  for(i=0, out[0]=0; i < jpN; i++)
    strcat(out, idx_hira_kata(jp[i], always_hira));
}


static gboolean send_jp()
{
  char out[512];
  merge_jp(out, FALSE);

  if (!out[0])
    return FALSE;

  clear_seg_label();
  jpN=0;
  keysN = 0;

//  printf("sent romanji '%s'\n", out);
  send_text(out);
  segN = 0;
  return TRUE;
}

void clear_sele();
void set_sele_text(int tN, int i, char *text, int len);
void disp_arrow_up(),disp_arrow_down();
void get_widget_xy(GtkWidget *win, GtkWidget *widget, int *rx, int *ry);
void disp_selections(int x, int y);

static void disp_select()
{
//  puts("disp_select");
  clear_sele();
  int endn = pageidx + phkbm.selkeyN;
  if (endn >  seg[cursor].selN)
    endn = seg[cursor].selN;
  int i;
  for(i=pageidx; i<endn; i++) {
    char buf[256];
    (*f_anthy_get_segment)(ac, cursor, i, buf, sizeof(buf));
//    printf("%d %s\n", i, buf);
    set_sele_text(seg[cursor].selN, i - pageidx, buf, strlen(buf));
  }

  if (pageidx)
    disp_arrow_up();
  if (i < seg[cursor].selN)
    disp_arrow_down();

  int x,y;
  get_widget_xy(win_anthy, seg[cursor].label, &x, &y);
//  printf("%x cusor %d %d\n", win_anthy, cursor, x);
  y = gcin_edit_display_ap_only()?win_y:win_y+win_yl;
  disp_selections(x, y);
}

static void load_seg()
{
      clear_seg_label();
      struct anthy_conv_stat acs;
      (*f_anthy_get_stat)(ac, &acs);
      segN = 0;
      if (acs.nr_segment > 0) {
        char buf[256];
        int i;

        for(i=0; i < acs.nr_segment; i++) {
          (*f_anthy_get_segment)(ac, i, 0, buf, sizeof(buf));

          seg[i].selidx = 0;
          gtk_label_set_text(GTK_LABEL(seg[i].label), buf);

          struct anthy_segment_stat ss;
          (*f_anthy_get_segment_stat)(ac, i, &ss);

          seg[i].selN = ss.nr_candidate;
          segN++;
        }

        state=STATE_CONVERT;
//        cursor = 0;
        if (cursor >= acs.nr_segment)
          cursor = acs.nr_segment - 1;
        disp_convert();
      }
      keysN=0;
}

static void next_page()
{
  pageidx += phkbm.selkeyN;
  if (pageidx >= seg[cursor].selN)
    pageidx = 0;
  disp_select();
}

void hide_selections_win();

int flush_anthy_input()
{
  hide_selections_win();

  int val;
  if (state==STATE_CONVERT) {
    val = TRUE;
    send_seg();
  } else {
    val = send_jp();
  }

//  dbg("cursor %d\n", cursor);
  clear_all();
  return val;
}

static int page_N()
{
  int N = seg[cursor].selN - pageidx;
  if (N > phkbm.selkeyN)
    N = phkbm.selkeyN;
  return N;
}

static gboolean select_idx(int c)
{
  int idx = pageidx + c;

  if (idx < seg[cursor].selN) {
    char buf[256];
    (*f_anthy_get_segment)(ac, cursor, idx, buf, sizeof(buf));
    gtk_label_set_text(GTK_LABEL(seg[cursor].label), buf);
    seg[cursor].selidx = idx;

    state = STATE_CONVERT;
    hide_selections_win();
    return (segN==1);
  }

  return FALSE;
}


gboolean feedkey_anthy(int kv, int kvstate)
{
  int lkv = tolower(kv);
  int shift_m=(kvstate&ShiftMask) > 0;
//  printf("%x %c  %d\n", kv, kv, shift_m);

  if (kvstate & ControlMask)
    return FALSE;
  if (kvstate & (Mod1Mask|Mod4Mask|Mod5Mask))
    return FALSE;

  if (kv==XK_Shift_L||kv==XK_Shift_R) {
    key_press_time = current_time();
  }

  if (!tsin_pho_mode())
    return 0;

  gboolean b_is_empty = is_empty();

  switch (kv) {
    case XK_F7:
      if (is_empty())
        return FALSE;
      state = STATE_ROMANJI;
      if (state_hira_kata != STATE_kata)
        state_hira_kata = STATE_kata;
      else
        state_hira_kata = STATE_hira;
      disp_input();
      return TRUE;
    case XK_F8:
      if (is_empty())
        return FALSE;
      state = STATE_ROMANJI;
      if (state_hira_kata != STATE_half_kata)
        state_hira_kata = STATE_half_kata;
      else
        state_hira_kata = STATE_hira;
      disp_input();
      return TRUE;
    case XK_F11:
      system("kasumi &");
      return TRUE;
    case XK_F12:
      system("kasumi -a &");
      return TRUE;
    case XK_Up:
      if (b_is_empty)
        return FALSE;
      if (state==STATE_SELECT) {
        int N = page_N();
        tss.pho_menu_idx--;
        if (tss.pho_menu_idx < 0)
          tss.pho_menu_idx = N - 1;
        disp_select();
      }
      return TRUE;
    case XK_Down:
      if (b_is_empty)
        return FALSE;
      if (state==STATE_CONVERT) {
        state = STATE_SELECT;
        tss.sel_pho = TRUE;
  //      puts("STATE_SELECT");
        disp_select();
      } else
      if (state==STATE_SELECT) {
        int N = page_N();
        tss.pho_menu_idx=(tss.pho_menu_idx+1)% N;
        disp_select();
      }
      return TRUE;
    case XK_Return:
      if (b_is_empty)
        return FALSE;
      if (state==STATE_SELECT) {
        if (select_idx(tss.pho_menu_idx))
          goto send;
        return TRUE;
      }
send:
      return flush_anthy_input();
    case XK_Escape:
        if (state==STATE_SELECT) {
          state = STATE_CONVERT;
          tss.sel_pho = FALSE;
          clear_sele();
        }
        else
        if (state==STATE_CONVERT)
          goto rom;
      return FALSE;
    case XK_BackSpace:
      if (b_is_empty) {
        state_hira_kata = STATE_hira;
        return FALSE;
      }

      hide_selections_win();

      if (state&(STATE_CONVERT|STATE_SELECT)) {
rom:
//        puts("romanji");
        state = STATE_ROMANJI;
        cursor = jpN;
        segN = 0;
        disp_input();
        return TRUE;
      }

//      puts("back");
      if (keysN) {
        keysN--;
        keys[keysN]=0;
      }
      else
      if (jpN && cursor) {
        delete_jpstr(cursor-1);
        cursor--;
      } else
        return FALSE;
      disp_input();
      auto_hide();
      return TRUE;
    case XK_Delete:
      if (b_is_empty)
        return FALSE;
      if (state&STATE_ROMANJI) {
        if (keysN)
          return TRUE;
        delete_jpstr(cursor);
        disp_input();
      }
      auto_hide();
      return TRUE;
    case XK_Left:
      if (b_is_empty)
        return FALSE;
      if (state&STATE_ROMANJI) {
        if (keysN)
          keysN = 0;
        else {
          if (cursor)
            cursor--;
        }
        disp_input();
      } else
      if (state&STATE_CONVERT) {
        if (shift_m) {
          (*f_anthy_resize_segment)(ac, cursor, -1);
          load_seg();
        } else {
          if (cursor)
            cursor--;
        }
        disp_convert();
      }
      return TRUE;
    case XK_Right:
      if (b_is_empty)
        return FALSE;
      if (state&STATE_ROMANJI) {
        if (cursor < jpN)
          cursor++;
        disp_input();
      } else
      if (state&STATE_CONVERT) {
        if (shift_m) {
          (*f_anthy_resize_segment)(ac, cursor, 1);
          load_seg();
        } else {
          if (cursor < segN-1)
            cursor++;
        }
        disp_convert();
      }
      return TRUE;
    case XK_Home:
      if (b_is_empty)
        return FALSE;
      cursor = 0;
      if (state&STATE_ROMANJI) {
        disp_input();
      } else
      if (state&STATE_CONVERT) {
        disp_convert();
      }
      return TRUE;
    case XK_End:
      if (b_is_empty)
        return FALSE;
      if (state&STATE_ROMANJI) {
        cursor = jpN;
        disp_input();
      } else
      if (state&STATE_CONVERT) {
        cursor = segN-1;
        disp_convert();
      }
      return TRUE;
    case XK_Prior:
      if (state!=STATE_SELECT)
        return FALSE;
      pageidx -= phkbm.selkeyN;
      if (pageidx < 0)
        pageidx = 0;
      disp_select();
      return TRUE;
    case XK_Next:
      if (state!=STATE_SELECT)
        return FALSE;
      next_page();
      return TRUE;
    case ' ':
      if (b_is_empty)
        return FALSE;
      goto lab1;
    default:
      if (state==STATE_SELECT) {
        char *pp;
        if ((pp=strchr(pho_selkey, lkv))) {
          int c=pp-pho_selkey;
          if (select_idx(c))
            goto send;
        }
        return TRUE;
      }
  }

//  printf("kv %d\n", kv);
  if (!is_legal_char(kv))
    return FALSE;

  kv = lkv;

  if (state==STATE_CONVERT && kv!=' ') {
    send_seg();
    state = STATE_ROMANJI;
  }

lab1:
  if (state==STATE_ROMANJI) {
    if (keysN < MAX_KEYS)
      keys[keysN++]=kv;

    keys[keysN]=0;
    parse_key();
    disp_input();
  }

  show_win_anthy();

  if (kv==' ') {
    if (state==STATE_ROMANJI) {
      char tt[512];
      clear_seg_label();
      merge_jp(tt, TRUE);
//      dbg("tt %s %d\n", tt, strlen(tt));
      (*f_anthy_set_string)(ac, tt);
      load_seg();
    } else
    if (state==STATE_CONVERT) {
      state = STATE_SELECT;
      tss.sel_pho = TRUE;
//      puts("STATE_SELECT");
      disp_select();
    } else
    if (state==STATE_SELECT) {
      next_page();
    }
  }

  return TRUE;
}

void toggle_win_sym(), exec_gcin_setup();

static void mouse_button_callback( GtkWidget *widget,GdkEventButton *event, gpointer data)
{
//  dbg("mouse_button_callback %d\n", event->button);
  switch (event->button) {
    case 1:
      toggle_win_sym();
      break;
    case 2:
      inmd_switch_popup_handler(widget, (GdkEvent *)event);
      break;
    case 3:
      exec_gcin_setup();
      break;
  }
}


#include <dlfcn.h>

void create_win1(), create_win1_gui(), load_tab_pho_file(), show_win_sym();
void hide_win_sym();
int init_win_anthy()
{
  char* so[] = {"libanthy.so", "libanthy.so.0", NULL};
  void *handle;
  char *error;

  set_tsin_pho_mode();

  if (win_anthy)
    return TRUE;

  int i;
  for (i=0; so[i]; i++)
    if (handle = dlopen(so[i], RTLD_LAZY))
      break;

  if (!handle) {
    GtkWidget *dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_ERROR,
                                     GTK_BUTTONS_CLOSE,
                                     "Error loading %s %s. Please install anthy", so[0], dlerror());
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
    return FALSE;
  }
  dlerror();    /* Clear any existing error */

  int (*f_anthy_init)();
  *(void **) (&f_anthy_init) = dlsym(handle, "anthy_init");

  if ((error = dlerror()) != NULL)  {
    fprintf(stderr, "%s\n", error);
    return FALSE;
  }

  if ((*f_anthy_init)() == -1) {
    GtkWidget *dialog = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL,
                                     GTK_MESSAGE_ERROR,
                                     GTK_BUTTONS_CLOSE,
                                     "Cannot init anthy. incompatible anthy.so ?");
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
    return FALSE;
  }

  anthy_context_t (*f_anthy_create_context)();
  *(void **) (&f_anthy_create_context) = dlsym(handle, "anthy_create_context");
  ac = (*f_anthy_create_context)();
  if (!ac) {
    printf("anthy_create_context err\n");
    return FALSE;
  }

  int (*f_anthy_context_set_encoding)(anthy_context_t, int);
  *(void **) (&f_anthy_context_set_encoding) = dlsym(handle, "anthy_context_set_encoding");
  (*f_anthy_context_set_encoding)(ac, ANTHY_UTF8_ENCODING);

  *(void **) (&f_anthy_resize_segment) = dlsym(handle, "anthy_resize_segment");
  *(void **) (&f_anthy_get_stat) = dlsym(handle, "anthy_get_stat");
  *(void **) (&f_anthy_get_segment) = dlsym(handle, "anthy_get_segment");
  *(void **) (&f_anthy_get_segment_stat) = dlsym(handle, "anthy_get_segment_stat");
  *(void **) (&f_anthy_commit_segment) = dlsym(handle, "anthy_commit_segment");
  *(void **) (&f_anthy_set_string) = dlsym(handle, "anthy_set_string");


  win_anthy = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_has_resize_grip(GTK_WINDOW(win_anthy), FALSE);
  gtk_window_set_default_size(GTK_WINDOW (win_anthy), 40, 50);


  gtk_widget_realize (win_anthy);
  set_no_focus(win_anthy);

  event_box_anthy = gtk_event_box_new();

  gtk_container_add(GTK_CONTAINER(win_anthy), event_box_anthy);

  GtkWidget *hbox_top = gtk_hbox_new (FALSE, 0);
  gtk_container_add(GTK_CONTAINER(event_box_anthy), hbox_top);

  g_signal_connect(G_OBJECT(event_box_anthy),"button-press-event",
                   G_CALLBACK(mouse_button_callback), NULL);

  if (!seg)
    seg=tzmalloc(SEG,MAX_SEG_N);

  for(i=0; i < MAX_SEG_N; i++) {
    seg[i].label = gtk_label_new(NULL);
    gtk_widget_show(seg[i].label);
    gtk_box_pack_start (GTK_BOX (hbox_top), seg[i].label, FALSE, FALSE, 0);
  }

  gtk_widget_show_all(win_anthy);

  create_win1();
  create_win1_gui();
  change_anthy_font_size();

  if (!phkbm.selkeyN)
    load_tab_pho_file();

  hide_win_anthy();

  return TRUE;
}

int anthy_visible()
{
  return GTK_WIDGET_VISIBLE(win_anthy);
}

extern gboolean force_show;
void show_win_anthy()
{
  if (gcin_edit_display_ap_only())
    return;
  if (!gcin_pop_up_win || !is_empty() || force_show ) {
    if (!anthy_visible())
      gtk_widget_show(win_anthy);
    show_win_sym();
  }
}

void hide_win_anthy()
{
  if (state == STATE_SELECT) {
    state = STATE_CONVERT;
    hide_selections_win();
  }
  gtk_widget_hide(win_anthy);
  hide_win_sym();
}

void change_anthy_font_size()
{
  GdkColor fg;
  gdk_color_parse(gcin_win_color_fg, &fg);
  change_win_bg(win_anthy);
  change_win_bg(event_box_anthy);

  int i;
  for(i=0; i < MAX_SEG_N; i++) {
    GtkWidget *label = seg[i].label;
    set_label_font_size(label, gcin_font_size);
    if (gcin_win_color_use) {
      gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &fg);
    }
  }
}

void move_win_sym();
void move_win_anthy(int x, int y)
{
#if 0
  best_win_x = x;
  best_win_y = y;
#endif
  gtk_window_get_size(GTK_WINDOW(win_anthy), &win_xl, &win_yl);

  if (x + win_xl > dpy_xl)
    x = dpy_xl - win_xl;
  if (x < 0)
    x = 0;

  if (y + win_yl > dpy_yl)
    y = dpy_yl - win_yl;
  if (y < 0)
    y = 0;

  gtk_window_move(GTK_WINDOW(win_anthy), x, y);
  win_x = x;
  win_y = y;

  move_win_sym();
}

void tsin_set_eng_ch(int nmod);

int feedkey_anthy_release(KeySym xkey, int kbstate)
{
  switch (xkey) {
     case XK_Shift_L:
     case XK_Shift_R:
        if (
(  (tsin_chinese_english_toggle_key == TSIN_CHINESE_ENGLISH_TOGGLE_KEY_Shift) ||
   (tsin_chinese_english_toggle_key == TSIN_CHINESE_ENGLISH_TOGGLE_KEY_ShiftL
     && xkey == XK_Shift_L) ||
   (tsin_chinese_english_toggle_key == TSIN_CHINESE_ENGLISH_TOGGLE_KEY_ShiftR
     && xkey == XK_Shift_R))
          &&  current_time() - key_press_time < 300000) {
          if (!test_mode) {
            flush_anthy_input();
            key_press_time = 0;
            hide_selections_win();
            tsin_set_eng_ch(!tsin_pho_mode());
          }
          return 1;
        } else
          return 0;
     default:
        return 0;
  }
}

#include "im-client/gcin-im-client-attr.h"

int anthy_get_preedit(char *str, GCIN_PREEDIT_ATTR attr[], int *pcursor)
{
  int i;

//  dbg("anthy_get_preedit %d\n", cursor);
  str[0]=0;
  *pcursor=0;

  attr[0].flag=GCIN_PREEDIT_ATTR_FLAG_UNDERLINE;
  attr[0].ofs0=0;
  int attrN=0;
  int ch_N=0;

  if (state==STATE_CONVERT) {
    if (segN)
      attrN=1;

    for(i=0; i < segN; i++) {
      char *s = (char *)gtk_label_get_text(GTK_LABEL(seg[i].label));
      int N = utf8_str_N(s);
      ch_N+=N;
      if (i < cursor)
        *pcursor+=N;
      if (i==cursor) {
        attr[1].ofs0=*pcursor;
        attr[1].ofs1=*pcursor+N;
        attr[1].flag=GCIN_PREEDIT_ATTR_FLAG_REVERSE;
        attrN++;
      }
      strcat(str, s);
    }

    attr[0].ofs1 = ch_N;
  } else {
    if (jpN)
      attrN=1;

    keys[keysN]=0;

    int idx;
    for(i=0;i < jpN; i++) {
      char *s=idx_hira_kata(jp[i], FALSE);
      int N = utf8_str_N(s);
//      dbg("%d]%s N:%d\n", i, s, N);
      if (i==cursor) {
        strcat(str, keys);
        ch_N+=keysN;
        *pcursor = ch_N;
        attr[1].ofs0=ch_N;
        attr[1].ofs1=ch_N+N;
        attr[1].flag=GCIN_PREEDIT_ATTR_FLAG_REVERSE;
        attrN++;
      }
      strcat(str, s);
      ch_N+=N;
    }

    if (cursor==jpN) {
      *pcursor = ch_N;
      strcat(str, keys);
      ch_N+=keysN;
    }

    attr[0].ofs1 = ch_N;
//    dbg("cursor %d  ch_N:%d  '%s'\n", *pcursor, ch_N, str);
  }

ret:
  return attrN;
}


int gcin_anthy_reset()
{
  if (!win_anthy)
    return 0;
  int v = !is_empty();

  clear_all();
  return v;
}

void get_win_anthy_geom()
{
  if (!win_anthy)
    return;
  gtk_window_get_position(GTK_WINDOW(win_anthy), &win_x, &win_y);

  get_win_size(win_anthy, &win_xl, &win_yl);
}
