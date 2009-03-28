#include "gcin.h"
#include "pho.h"

#if USE_TSIN
int find_match();
extern gboolean flush_tsin_buffer();
#endif

PIN_JUYIN *pin_juyin;

char file_pin_float[] = GCIN_ICON_DIR"/pin-float16.png";

int c_len;
int text_pho_N=3;
int ts_gtabN;

gboolean b_use_full_space = TRUE;

static char text_pho[6][CH_SZ];

void bell()
{
#if 1
  XBell(dpy, -97);
#else
  gdk_beep();
#endif
//  abort();
}

void case_inverse(int *xkey, int shift_m)
{
  if (shift_m) {
    if (islower(*xkey))
      *xkey-=0x20;
  } else
  if (isupper(*xkey))
    *xkey+=0x20;
}

gint64 current_time()
{
  struct timeval tval;

  gettimeofday(&tval, NULL);
  return (gint64)tval.tv_sec * 1000000 + tval.tv_usec;
}

void disp_pho_sub(GtkWidget *label, int index, char *pho)
{
  if (index>=text_pho_N)
    return;


  if (pho[0]==' ' && !pin_juyin) {
    u8cpy(text_pho[index], "\xe3\x80\x80");
  }
  else {
    u8cpy(text_pho[index], pho);
  }

  char s[text_pho_N * CH_SZ+1];


  int tn = 0;
  int i;
  for(i=0; i < text_pho_N; i++) {
    int n = utf8cpy(s + tn, text_pho[i]);
    tn += n;
  }

  gtk_label_set_text(label, s);
}

void exec_gcin_setup()
{
#if DEBUG
  dbg("exec gcin\n");
#endif

  char pidstr[32];
  sprintf(pidstr, "GCIN_PID=%d",getpid());
  putenv(pidstr);
  system(GCIN_BIN_DIR"/gcin-setup &");
}

void set_label_font_size(GtkWidget *label, int size)
{
  if (!label)
    return;

  PangoContext *pango_context = gtk_widget_get_pango_context (label);
  PangoFontDescription* font=pango_context_get_font_description
       (pango_context);
  pango_font_description_set_family(font, gcin_font_name);
  pango_font_description_set_size(font, PANGO_SCALE * size);
  gtk_widget_modify_font(label, font);
}

// the width of ascii space in firefly song
void set_label_space(GtkWidget *label)
{
  gtk_label_set_text(GTK_LABEL(label), "\xe3\x80\x80");
  return;
}

void set_no_focus(GtkWidget *win)
{
  gdk_window_set_override_redirect(win->window, TRUE);
#if GTK_MAJOR_VERSION >=2 && GTK_MINOR_VERSION >= 6
  gtk_window_set_accept_focus(win, FALSE);
#endif
#if GTK_MAJOR_VERSION >=2 && GTK_MINOR_VERSION >= 6
  gtk_window_set_focus_on_map (win, FALSE);
#endif
}

#if !USE_TSIN
gboolean flush_tsin_buffer()
{
  tsin_reset_in_pho();

  if (gcin_pop_up_win)
    hide_win0();

  if (c_len) {
    putbuf(c_len);
    compact_win0_x();
    clear_ch_buf_sel_area();
    clear_tsin_buffer();
    return 1;
  }

  return 0;
}

// len is in CH_SZ
int find_match(char *str, int len, char *match_chars, int match_chars_max)
{
  if (!len)
    return 0;

  if (!ts_gtabN)
    build_ts_gtab();

  int bottom = 0;
  int top = ts_gtabN - 1;
  int mid, tlen;
  char tstr[MAX_PHRASE_STR_LEN];
  int matchN=0;

  if (match_chars)
      match_chars[0] = 0;

  do {
    mid = (bottom + top) /2;

//    dbg("tstr:%s  %d %d %d\n", tstr, bottom, mid, top);
    tlen = load_ts_gtab(mid, tstr);

    if (!tlen) {  // error in db
      dbg("error in db\n");
      build_ts_gtab();
      return 0;
    }

    int r = strncmp(str, tstr, len);

    if (r < 0) {
      top = mid - 1;
    }
    else
    if (r > 0 || strlen(tstr)==len) {
      bottom = mid + 1;
    } else {
      strcpy(str, tstr);

      if (!match_chars)
        return 1;

      bottom = mid;
      int i;

      int totlen=0;
      for(i=mid; i>=0; i--) {
        tlen = load_ts_gtab(i, tstr);

        if (strncmp(str, tstr, len) || tlen <= len)
          break;

        if (matchN >= match_chars_max)
          break;

        int slen= u8cpy(&match_chars[totlen], &tstr[len]);
        totlen+=slen;
        matchN++;
      }

      for(i=mid+1; i< ts_gtabN; i++) {
        tlen = load_ts_gtab(i, tstr);

        if (strncmp(str, tstr, len) || tlen <= len)
          break;

        if (matchN >= match_chars_max)
          break;

        int slen = u8cpy(&match_chars[totlen], &tstr[len]);
        totlen+=slen;
        matchN++;
      }

      match_chars[totlen] = 0;

      return matchN;
    }

  } while (bottom <= top);

//  dbg("%d %d\n", bottom, top);
  return 0;
}

void add_to_tsin_buf(){}
void add_to_tsin_buf_str(){}
void build_ts_gtab(){}
void change_tsin_color(){}
void change_tsin_font_size(){}
void change_win0_style(){}
void clear_ch_buf_sel_area(){}
void clear_tsin_buffer(){}
void compact_win0_x(){}
void destroy_win0(){}
void destroy_win1(){}
void free_tsin(){}
void hide_win0(){}
void load_ts_gtab(){}
void load_tsin_db(){}
void putbuf(){}
void tsin_remove_last(){}
void tsin_reset_in_pho(){}
void tsin_set_eng_ch(){}
void tsin_toggle_half_full(){}
#endif
