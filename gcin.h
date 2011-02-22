#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "os-dep.h"
#include <gtk/gtk.h>
#include <string.h>
#if UNIX
#include "IMdkit.h"
#include "Xi18n.h"
#endif
#if GCIN_i18n_message
#include <libintl.h>
#define _(STRING) gettext(STRING)
#else
#if UNIX
#define _(STRING) (STRING)
#else
#define _(x) __utf16_8(x)
#endif
#endif

#define N_(STRING) STRING

#if !GTK_CHECK_VERSION(2,13,4)
#define gtk_widget_get_window(x) (x)->window
#define gtk_color_selection_dialog_get_color_selection(x) (x)->colorsel
#endif

#if !GTK_CHECK_VERSION(2,15,0)
#define gtk_status_icon_set_tooltip_text(x,y) gtk_status_icon_set_tooltip(x,y)
#endif

#if GTK_CHECK_VERSION(2,17,5)
#undef GTK_WIDGET_NO_WINDOW
#define GTK_WIDGET_NO_WINDOW !gtk_widget_get_has_window
#undef GTK_WIDGET_SET_FLAGS
#define GTK_WIDGET_SET_FLAGS(x,y) gtk_widget_set_can_default(x,1)
#endif

#if GTK_CHECK_VERSION(2,17,7)
#undef GTK_WIDGET_VISIBLE
#define GTK_WIDGET_VISIBLE gtk_widget_get_visible
#endif

#if GTK_CHECK_VERSION(2,17,10)
#undef GTK_WIDGET_DRAWABLE
#define GTK_WIDGET_DRAWABLE gtk_widget_is_drawable
#endif

#if GTK_CHECK_VERSION(2,19,5)
#undef GTK_WIDGET_REALIZED
#define GTK_WIDGET_REALIZED gtk_widget_get_realized
#endif

#if GTK_CHECK_VERSION(2,21,8)
#undef GDK_DISPLAY
#define GDK_DISPLAY() GDK_DISPLAY_XDISPLAY(gdk_display_get_default())
#endif

#if GTK_CHECK_VERSION(2,91,0)
#define GTK_OBJECT
#endif

#if !GTK_CHECK_VERSION(2,91,1)
#define gtk_window_set_has_resize_grip(x,y);
#endif

#ifndef GTK_COMBO_BOX_TEXT
#define GTK_COMBO_BOX_TEXT GTK_COMBO_BOX
#endif

#if GTK_CHECK_VERSION(2,91,2)
#define gtk_combo_box_new_text gtk_combo_box_text_new
#define gtk_combo_box_append_text gtk_combo_box_text_append_text
#define gtk_widget_hide_all gtk_widget_hide
#endif

#if GTK_CHECK_VERSION(2,91,6)
#define GDK_WINDOW_XWINDOW GDK_WINDOW_XID
#endif

#if GTK_CHECK_VERSION(2,91,7)
#define gdk_window_lookup_for_display gdk_x11_window_lookup_for_display
#endif

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
#define zmalloc __gcin_zmalloc
#endif

#include "util.h"

#define tmalloc(type,n)  (type*)malloc(sizeof(type) * (n))
void *zmalloc(int n);
void *memdup(void *p, int n);
#define tzmalloc(type,n)  (type*)zmalloc(sizeof(type) * (n))
#define trealloc(p,type,n)  (type*)realloc(p, sizeof(type) * (n+1))
#define tmemdup(p,type,n) (type*)memdup(p, sizeof(type) * n)
#if UNIX
extern Display *dpy;
#endif


extern GtkWidget *gwin0;
extern GdkWindow *gdkwin0;
extern Window xwin0;
extern Window root;
#if UNIX
void loadIC();
IC *FindIC(CARD16 icid);
#endif
extern ClientState *current_CS;

enum {
  InputStyleOverSpot = 1,
  InputStyleRoot = 2,
  InputStyleOnSpot = 4
};

typedef enum {
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
};

typedef enum {
  TSIN_SPACE_OPT_SELECT_CHAR = 1,
  TSIN_SPACE_OPT_INPUT = 2,
} TSIN_SPACE_OPT;

enum {
  GCIN_EDIT_DISPLAY_OVER_THE_SPOT=1,
  GCIN_EDIT_DISPLAY_ON_THE_SPOT=2,
  GCIN_EDIT_DISPLAY_BOTH=4,
};

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
char *myfgets(char *buf, int bufN, FILE *fp);
void get_gcin_dir(char *tt);
#if UNIX
Atom get_gcin_atom(Display *dpy);
#endif
void get_sys_table_file_name(char *name, char *fname);
char *half_char_to_full_char(KeySym xkey);
void send_text(char *text);
void sendkey_b5(char *bchar);
void send_ascii(char key);
void bell();
void set_label_font_size(GtkWidget *label, int size);
#if UNIX
void send_gcin_message(Display *dpy, char *s);
#else
void send_gcin_message(char *s);
#endif
void check_CS();
gint64 current_time();
void get_win_size(GtkWidget *win, int *width, int *height);
void change_win_fg_bg(GtkWidget *win, GtkWidget *label);
void set_no_focus(GtkWidget *win);
void change_win_bg(GtkWidget *win);
gboolean gcin_edit_display_ap_only();
void char_play(char *utf8);
void skip_utf8_sigature(FILE *fp);
#if WIN32
char *__utf16_8(wchar_t *s);
void win32_init_win(GtkWidget *win);
#endif

#define BITON(flag, bit) ((flag) & (bit))

extern int gcin_switch_keysN;
extern char gcin_switch_keys[];

typedef int usecount_t;

#define MAX_CIN_PHR (100*CH_SZ + 1)
