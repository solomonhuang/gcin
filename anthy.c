#include "gcin.h"
#include "pho.h"
#include <anthy/anthy.h>
static anthy_context_t ac;
void (*f_anthy_resize_segment)(anthy_context_t ac, int, int);
void (*f_anthy_get_stat)(anthy_context_t ac, struct anthy_conv_stat *acs);
void (*f_anthy_get_segment)(anthy_context_t ac, int,int,char *, int);
void (*f_anthy_get_segment_stat)(anthy_context_t ac, int, struct anthy_segment_stat *);
void (*f_anthy_commit_segment)(anthy_context_t ac, int, int);
void (*f_anthy_set_string)(anthy_context_t ac, char *);
extern int eng_ph;
extern gint64 key_press_time;
static GtkWidget *event_box_anthy;
gint64 current_time();

struct {
  char *en;
  char *ro;
} anthy_romaji_map[] = {
{"xtu",	"っ"},
{"xtsu",	"っ"},
{"ltu",	"っ"},
{"ltsu",	"っ"},

{"-",	"ー"},
{"a",	"あ"},
{"i",	"い"},
{"u",	"う"},
{"e",	"え"},
{"o",	"お"},

{"xa",	"ぁ"},
{"xi",	"ぃ"},
{"xu",	"ぅ"},
{"xe",	"ぇ"},
{"xo",	"ぉ"},

{"la",	"ぁ"},
{"li",	"ぃ"},
{"lu",	"ぅ"},
{"le",	"ぇ"},
{"lo",	"ぉ"},

{"wi",	"うぃ"},
{"we",	"うぇ"},
{"wha",	"うぁ"},
{"whi",	"うぃ"},
{"whe",	"うぇ"},
{"who",	"うぉ"},

{"va",	"う゛ぁ"},
{"vi",	"う゛ぃ"},
{"vu",	"う゛"},
{"ve",	"う゛ぇ"},
{"vo",	"う゛ぉ"},

{"ka",	"か"},
{"ki",	"き"},
{"ku",	"く"},
{"ke",	"け"},
{"ko",	"こ"},

{"ga",	"が"},
{"gi",	"ぎ"},
{"gu",	"ぐ"},
{"ge",	"げ"},
{"go",	"ご"},

{"kya",	"きゃ"},
{"kyi",	"きぃ"},
{"kyu",	"きゅ"},
{"kye",	"きぇ"},
{"kyo",	"きょ"},
{"gya",	"ぎゃ"},
{"gyi",	"ぎぃ"},
{"gyu",	"ぎゅ"},
{"gye",	"ぎぇ"},
{"gyo",	"ぎょ"},

{"sa",	"さ"},
{"si",	"し"},
{"su",	"す"},
{"se",	"せ"},
{"so",	"そ"},

{"za",	"ざ"},
{"zi",	"じ"},
{"zu",	"ず"},
{"ze",	"ぜ"},
{"zo",	"ぞ"},

{"sya",	"しゃ"},
{"syi",	"しぃ"},
{"syu",	"しゅ"},
{"sye",	"しぇ"},
{"syo",	"しょ"},
{"sha",	"しゃ"},
{"shi",	"し"},
{"shu",	"しゅ"},
{"she",	"しぇ"},
{"sho",	"しょ"},
{"zya",	"じゃ"},
{"zyi",	"じぃ"},
{"zyu",	"じゅ"},
{"zye",	"じぇ"},
{"zyo",	"じょ"},
{"ja",	"じゃ"},
{"jya", "じゃ"},
{"ji",	"じ"},
{"jyi", "じぃ"},
{"ju",	"じゅ"},
{"jyu",	"じゅ",},
{"je",	"じぇ"},
{"jye",	"じぇ"},
{"jo",	"じょ"},
{"jyo",	"じょ"},
{"ta",	"た"},
{"ti",	"ち"},
{"tu",	"つ"},
{"tsu",	"つ"},
{"te",	"て"},
{"to",	"と"},

{"da",	"だ"},
{"di",	"ぢ"},
{"du",	"づ"},
{"de",	"で"},
{"do",	"ど"},


{"tya",	"ちゃ"},
{"tyi",	"ちぃ"},
{"tyu",	"ちゅ"},
{"tye",	"ちぇ"},
{"tyo",	"ちょ"},

{"cha",	"ちゃ"},
{"chi",	"ち"},
{"chu",	"ちゅ"},
{"che",	"ちぇ"},
{"cho",	"ちょ"},

{"dya",	"ぢゃ"},
{"dyi",	"ぢぃ"},
{"dyu",	"ぢゅ"},
{"dye",	"ぢぇ"},
{"dyo",	"ぢょ"},

{"tha",	"てゃ"},
{"thi",	"てぃ"},
{"thu",	"てゅ"},
{"the",	"てぇ"},
{"tho",	"てょ"},

{"dha",	"でゃ"},
{"dhi",	"でぃ"},
{"dhu",	"でゅ"},
{"dhe",	"でぇ"},
{"dho",	"でょ"},

{"na",	"な"},
{"ni",	"に"},
{"nu",	"ぬ"},
{"ne",	"ね"},
{"no",	"の"},
{"nya",	"にゃ"},
{"nyi",	"にぃ"},
{"nyu",	"にゅ"},
{"nye",	"にぇ"},
{"nyo",	"にょ"},

{"ha",	"は"},
{"hi",	"ひ"},
{"hu",	"ふ"},
{"he",	"へ"},
{"ho",	"ほ"},

{"ba",	"ば"},
{"bi",	"び"},
{"bu",	"ぶ"},
{"be",	"べ"},
{"bo",	"ぼ"},

{"pa",	"ぱ"},
{"pi",	"ぴ"},
{"pu",	"ぷ"},
{"pe",	"ぺ"},
{"po",	"ぽ"},

{"hya",	"ひゃ"},
{"hyi",	"ひぃ"},
{"hyu",	"ひゅ"},
{"hye",	"ひぇ"},
{"hyo",	"ひょ"},
{"bya",	"びゃ"},
{"byi",	"びぃ"},
{"byu",	"びゅ"},
{"bye",	"びぇ"},
{"byo",	"びょ"},
{"pya",	"ぴゃ"},
{"pyi",	"ぴぃ"},
{"pyu",	"ぴゅ"},
{"pye",	"ぴぇ"},
{"pyo",	"ぴょ"},

{"fa",	"ふぁ"},
{"fi",	"ふぃ"},
{"fu",	"ふ"},
{"fe",	"ふぇ"},
{"fo",	"ふぉ"},

{"ma",	"ま"},
{"mi",	"み"},
{"mu",	"む"},
{"me",	"め"},
{"mo",	"も"},

{"mya",	"みゃ"},
{"myi",	"みぃ"},
{"myu",	"みゅ"},
{"mye",	"みぇ"},
{"myo",	"みょ"},
{"lya",	"ゃ"},
{"xya",	"ゃ"},
{"ya",	"や"},
{"lyu",	"ゅ"},
{"xyu",	"ゅ"},
{"yu",	"ゆ"},
{"lyo",	"ょ"},
{"xyo",	"ょ"},
{"yo",	"よ"},

{"ra",	"ら"},
{"ri",	"り"},
{"ru",	"る"},
{"re",	"れ"},
{"ro",	"ろ"},

{"rya",	"りゃ"},
{"ryi",	"りぃ"},
{"ryu",	"りゅ"},
{"rye",	"りぇ"},
{"ryo",	"りょ"},
{"xwa",	"ゎ"},
{"wa",	"わ"},
{"wo",	"を"},
{"n'",	"ん"},
{"nn",	"ん"},
{"n",	"ん"},
{"m",	"ん"},  // tombo
{"wyi",	"ゐ"},
{"wye",	"ゑ"},
{",",	"、"},
{".",	"。"},
{"[",	"「"},
{"]",	"」"},
{"/",	"／"},
{"\\",	"＼"},
{"=",	"＝"},
{"+",	"＋"},
{"_",	"＿"},
{"~",	"〜"},
{"!",	"！"},
{"@",	"＠"},
{"#",	"＃"},
{"$",	"＄"},
{"%",	"％"},
{"^",	"＾"},
{"&",	"＆"},
{"*",	"＊"},
{"(",	"（"},
{")",	"）"},
{"<",	"＜"},
{">",	"＞"},
{"{",	"｛"},
{"}",	"｝"},
{"|",	"｜"},
{"'",	"’"},
{"\"",	"”"},
{"`",	"‘"},
{"?",	"？"},
{":",	"："},
{";",	"；"},
{"0",	"0"},
{"1",	"1"},
{"2",	"2"},
{"3",	"3"},
{"4",	"4"},
{"5",	"5"},
{"6",	"6"},
{"7",	"7"},
{"8",	"8"},
{"9",	"9"},
};


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

static char keys[32];
static short int keysN;
static unsigned char jp[128];
static short int jpN=0;
static short selN, pageidx;

#define MAX_SEG_N 80
typedef struct {
  GtkWidget *label;
  unsigned char selidx, selN;
} SEG;
static SEG seg[MAX_SEG_N];
static short segN, segNa;
static short cursor;
enum {
  STATE_ROMANJI=1,
  STATE_CONVERT=2,
  STATE_SELECT=4,
};
static char state = STATE_ROMANJI;

static GtkWidget *win_anthy;

static int is_empty()
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

static void append_jp(u_char rom_idx)
{
//  printf("append %d %s\n", rom_idx, anthy_romaji_map[rom_idx].ro);
  jp[jpN++]=rom_idx;
  cursor = jpN;
}

void parse_key()
{
  int i;
  int preN=0, eqN=0, sendpreN=0;
  unsigned char eq, sendpre_i = 255;
  static char ch2[]="kstzdhbrpfgvcjmwy";

  if (keysN==2 && keys[0]==keys[1] && strchr(ch2, keys[0])) {
    append_jp(0);
    keys[1]=0;
    keysN=1;
    return;
  }

  for(i=0; i < anthy_romaji_mapN; i++) {
    char *en = anthy_romaji_map[i].en;
    char *ro = anthy_romaji_map[i].ro;
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

    append_jp(eq);

    keys[0]=0;
    keysN=0;
    return;
  }

  if (sendpre_i != 255) {
    char *en = anthy_romaji_map[sendpre_i].en;
    int len =strlen(en);
    int nlen = keysN - len;
    memmove(keys, keys+len, nlen);
    keys[nlen] = 0;
    keysN = nlen;

    append_jp(sendpre_i);
  }
}

static void clear_seg_label()
{
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

static void disp_input()
{
  int i;

  if (gcin_edit_display & GCIN_EDIT_DISPLAY_ON_THE_SPOT)
    return;

//  printf("cursor %d\n", cursor);
  clear_seg_label();
  for(i=0; i < jpN; i++) {
    if (i==cursor)
      cursor_markup(i, anthy_romaji_map[jp[i]].ro);
    else
      gtk_label_set_text(GTK_LABEL(seg[i].label), anthy_romaji_map[jp[i]].ro);
  }

  char tt[2];
  tt[1]=0;
  for(i=0; i < keysN; i++) {
    tt[0]=keys[i];
    gtk_label_set_text(GTK_LABEL(seg[jpN+i].label), tt);
  }

  if (cursor==jpN)
    cursor_markup(jpN+keysN, " ");

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

void delete_jpstr(idx)
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

static char merge_jp(char out[])
{
  int i;
  for(i=0, out[0]=0; i < jpN; i++)
    strcat(out, anthy_romaji_map[jp[i]].ro);
}


static gboolean send_jp()
{
  char out[512];
  merge_jp(out);

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
    set_sele_text(i - pageidx, buf, strlen(buf));
  }

  if (pageidx)
    disp_arrow_up();
  if (i < seg[cursor].selN)
    disp_arrow_down();

  int x,y;
  get_widget_xy(win_anthy, seg[cursor].label, &x, &y);
//  printf("%x cusor %d %d\n", win_anthy, cursor, x);
  y = gcin_edit_display==GCIN_EDIT_DISPLAY_ON_THE_SPOT?win_y:win_y+win_yl;
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


static int flush_input()
{
  hide_selections_win();

  int val;
  if (state==STATE_CONVERT) {
    val = TRUE;
    send_seg();
  } else {
    val = send_jp();
  }

  jpN=0;
  keysN = 0;
  cursor = 0;
  disp_input();
  state = STATE_ROMANJI;
  auto_hide();
  return val;
}

gboolean feedkey_anthy(int kv, int kvstate)
{
  int lkv = tolower(kv);
  int shift_m=(kvstate&ShiftMask) > 0;
//  printf("%x %c  %d\n", kv, kv, shift_m);

  if (kv==XK_Shift_L||kv==XK_Shift_R) {
    puts("shift");
    key_press_time = current_time();
  }

  if (!eng_ph)
    return 0;

  gboolean is_empty = !keysN && !jpN && !segN;

//  printf("empty %d\n", is_empty);

  switch (kv) {
    case XK_F11:
      system("kasumi &");
      return TRUE;
    case XK_F12:
      system("kasumi -a &");
      return TRUE;
    case XK_Return:
      if (is_empty)
        return FALSE;
send:
      return flush_input();
    case XK_Escape:
        if (state==STATE_SELECT) {
          state = STATE_CONVERT;
          clear_sele();
        }
        else
        if (state==STATE_CONVERT)
          goto rom;
      return FALSE;
    case XK_BackSpace:
      if (is_empty)
        return FALSE;

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
      if (is_empty)
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
      if (is_empty)
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
      if (is_empty)
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
      if (is_empty)
        return FALSE;
      cursor = 0;
      if (state&STATE_ROMANJI) {
        disp_input();
      } else
      if (state&STATE_CONVERT) {
        disp_convert();
      }
      return;
    case XK_End:
      if (is_empty)
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
      pageidx -= phkbm.selkeyN;
      if (pageidx < 0)
        pageidx = 0;
      disp_select();
      return TRUE;
    case XK_Next:
      next_page();
      return TRUE;
    case ' ':
      if (is_empty)
        return FALSE;
      goto lab1;
    default:
      if (state==STATE_SELECT) {
        char *pp;
        if ((pp=strchr(phkbm.selkey, lkv))) {
          int c=pp-phkbm.selkey;
          int idx = pageidx + c;

          if (idx < seg[cursor].selN) {
            char buf[256];
            (*f_anthy_get_segment)(ac, cursor, idx, buf, sizeof(buf));
            gtk_label_set_text(GTK_LABEL(seg[cursor].label), buf);
            seg[cursor].selidx = idx;

            state = STATE_CONVERT;
            hide_selections_win();
            if (segN==1) {
              goto send;
            }
          }
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
      merge_jp(tt);
      (*f_anthy_set_string)(ac, tt);

      load_seg();
    } else
    if (state==STATE_CONVERT) {
      state = STATE_SELECT;
//      puts("STATE_SELECT");
      disp_select();
    } else
    if (state==STATE_SELECT) {
      next_page();
    }
  }

  return TRUE;
}

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

int init_win_anthy()
{
  char* so[] = {"libanthy.so", "libanthy.so.0", NULL};
  void *handle;
  char *error;

  eng_ph = 1;

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

  int (*f_anthy_create_context)();
  *(void **) (&f_anthy_create_context) = dlsym(handle, "anthy_create_context");
  ac = (anthy_context_t)(*f_anthy_create_context)();
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
  gtk_window_set_default_size(GTK_WINDOW (win_anthy), 40, 50);


  gtk_widget_realize (win_anthy);
  set_no_focus(win_anthy);

  event_box_anthy = gtk_event_box_new();

  gtk_container_add(GTK_CONTAINER(win_anthy), event_box_anthy);

  GtkWidget *hbox_top = gtk_hbox_new (FALSE, 0);
  gtk_container_add(GTK_CONTAINER(event_box_anthy), hbox_top);

  g_signal_connect(G_OBJECT(event_box_anthy),"button-press-event",
                   G_CALLBACK(mouse_button_callback), NULL);

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
  if (gcin_edit_display & GCIN_EDIT_DISPLAY_ON_THE_SPOT)
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
          flush_input();
          key_press_time = 0;
          hide_selections_win();
          tsin_set_eng_ch(!eng_ph);
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
  int tn=0;

//  dbg("anthy_get_preedit\n");
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
      char *s = gtk_label_get_text(GTK_LABEL(seg[i].label));
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

    for(i=0;i < jpN; i++) {
      char *s=anthy_romaji_map[jp[i]].ro;
      int N = utf8_str_N(s);
      ch_N+=N;
      if (i < cursor)
        *pcursor+= N;
      if (i==cursor) {
        attr[1].ofs0=*pcursor;
        attr[1].ofs1=*pcursor+N;
        attr[1].flag=GCIN_PREEDIT_ATTR_FLAG_REVERSE;
        attrN++;
      }
      strcat(str, s);
    }

    strcat(str, keys);
    attr[0].ofs1 = ch_N + keysN;
  }

ret:
  return attrN;
}


void gcin_anthy_reset()
{
  if (!win_anthy)
    return;
  clear_all();
}

void get_win_anthy_geom()
{
  if (!win_anthy)
    return;
  gtk_window_get_position(GTK_WINDOW(win_anthy), &win_x, &win_y);

  get_win_size(win_anthy, &win_xl, &win_yl);
}
