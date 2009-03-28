#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include <X11/Xlocale.h>
#include <X11/keysym.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <string.h>
#include "IMdkit.h"
#include "Xi18n.h"

typedef enum {
  GCIN_STATE_DISABLED = 0,
  GCIN_STATE_ENG_FULL = 1,
  GCIN_STATE_CHINESE = 2
} GCIN_STATE_E;



/* change 3 to 4 if you want to use 4-byte UTF-8 characters, but you must
   regenerate *.gtab tsin
*/
#define CH_SZ (4)


#include "IC.h"

#if CLIENT_LIB
#define p_err __gcin_p_err
#define dbg __gcin_dbg
#define zmalloc __gcin_zmalloc
#endif


#define tmalloc(type,n)  (type*)malloc(sizeof(type) * (n))
void *zmalloc(int n);
#define tzmalloc(type,n)  (type*)zmalloc(sizeof(type) * (n))
#define trealloc(p,type,n)  (type*)realloc(p, sizeof(type) * (n+1))

extern Display *dpy;


void p_err(char *fmt,...);
void dbg(char *fmt,...);


extern GtkWidget *gwin0;
extern GdkWindow *gdkwin0;
extern Window xwin0;
extern Window root;
void loadIC();
IC *FindIC(CARD16 icid);
extern ClientState *current_CS;

enum {
  InputStyleOverSpot = 1,
  InputStyleRoot = 2,
  InputStyleOnSpot = 4
};

enum {
  Control_Space=0,
  Shift_Space=1,
  Alt_Space=2,
  Windows_Space=3,
} IM_TOGGLE_KEYS;

enum {
  TSIN_CHINESE_ENGLISH_TOGGLE_KEY_CapsLock=1,
  TSIN_CHINESE_ENGLISH_TOGGLE_KEY_Tab=2,
  TSIN_CHINESE_ENGLISH_TOGGLE_KEY_Shift=4,
  TSIN_CHINESE_ENGLISH_TOGGLE_KEY_ShiftL=8,
  TSIN_CHINESE_ENGLISH_TOGGLE_KEY_ShiftR=16,
} TSIN_CHINESE_ENGLISH_TOGGLE_KEY;

enum {
  TSIN_SPACE_OPT_SELECT_CHAR = 1,
  TSIN_SPACE_OPT_FLUSH_BUFFER = 2,
} TSIN_SPACE_OPT;


#define ROW_ROW_SPACING (2)


#define MAX_GCIN_STR (256)

#define PHO_KBM "phokbm"

extern int win_xl, win_yl;
extern int win_x, win_y;   // actual win x/y
extern int  current_in_win_x,  current_in_win_y;  // request x/y
extern int dpy_xl, dpy_yl;

extern int gcin_font_size;

void big5_utf8(char *s, char out[]);
void utf8_big5(char *s, char out[]);
gint inmd_switch_popup_handler (GtkWidget *widget, GdkEvent *event);

#include "gcin-conf.h"

#define bchcpy(a,b) memcpy(a,b, CH_SZ)
#define bchcmp(a,b) memcmp(a,b, CH_SZ)

int utf8_sz(char *s);
int utf8cpy(char *t, char *s);
int u8cpy(char *t, char *s);
int utf8_tlen(char *s, int N);
void utf8_putchar(char *s);
void utf8_putcharn(char *s, int n);
gboolean utf8_eq(char *a, char *b);
gboolean utf8_str_eq(char *a, char *b, int len);
void utf8cpyN(char *t, char *s, int N);
int utf8_str_N(char *str);
void utf8cpyn(char *t, char *s, int n);
void utf8cpy_bytes(char *t, char *s, int n);

void get_gcin_dir(char *tt);
Atom get_gcin_atom(Display *dpy);
void get_sys_table_file_name(char *name, char *fname);
char *half_char_to_full_char(KeySym xkey);
void send_text(char *text);
void sendkey_b5(char *bchar);
void bell();
void set_label_font_size(GtkWidget *label, int size);
void send_gcin_message(Display *dpy, char *s);
void check_CS();
void get_win_size(GtkWidget *win, int *width, int *height);
void change_win_fg_bg(GtkWidget *win, GtkWidget *label);


#define BITON(flag, bit) ((flag) & (bit))

extern int gcin_switch_keysN;
extern char gcin_switch_keys[];

#if GTK_MAJOR_VERSION >=2 && GTK_MINOR_VERSION >= 4
#define GTK_24 1
#endif

typedef int usecount_t;
