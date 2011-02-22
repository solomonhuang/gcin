#include "gcin.h"
#include "gtab.h"
#if UNIX
#include <signal.h>
#include <X11/extensions/XTest.h>
#endif
#include "gst.h"
#include "im-client/gcin-im-client-attr.h"

#define STRBUFLEN 64

extern Display *dpy;
#if USE_XIM
extern XIMS current_ims;
static IMForwardEventStruct *current_forward_eve;
#endif
extern gboolean win_kbm_inited;

static char *callback_str_buffer;
Window focus_win;
static int timeout_handle;
char *output_buffer;
int output_bufferN;
static char *output_buffer_raw, *output_buffer_raw_bak;
static int output_buffer_rawN;
gboolean test_mode;
#if WIN32
int last_input_method;
#endif


void init_gtab(int inmdno);

char current_method_type()
{
  if (!current_CS)
#if UNIX
    return inmd[default_input_method].method_type;
#else
  {
    if (!last_input_method)
		last_input_method = default_input_method;
	return inmd[last_input_method].method_type;
  }
#endif
  return inmd[current_CS->in_method].method_type;
}


#if WIN32
void win32_FakeKey(UINT vk, bool key_pressed);
#endif
void send_fake_key_eve(KeySym key)
{
#if WIN32
  win32_FakeKey(key, true);
  win32_FakeKey(key, false);
#else
  KeyCode kc = XKeysymToKeycode(dpy, key);
  XTestFakeKeyEvent(dpy, kc, True, CurrentTime);
  XTestFakeKeyEvent(dpy, kc, False, CurrentTime);
#endif
}

void fake_shift()
{
  send_fake_key_eve(XK_Shift_L);
}

void swap_ptr(char **a, char **b)
{
  char *t = *a;
  *a = *b;
  *b = t;
}

int force_preedit=0;
void force_preedit_shift()
{
  send_fake_key_eve(XK_Shift_L);
  force_preedit=1;
}

void send_text_call_back(char *text)
{
  callback_str_buffer = (char *)realloc(callback_str_buffer, strlen(text)+1);
  strcpy(callback_str_buffer, text);
  fake_shift();
}

void output_buffer_call_back()
{
  swap_ptr(&callback_str_buffer, &output_buffer);

  if (output_buffer)
    output_buffer[0] = 0;
  output_bufferN = 0;

  fake_shift();
}

ClientState *current_CS;
static ClientState temp_CS;

gboolean init_in_method(int in_no);


void clear_output_buffer()
{
  if (output_buffer)
    output_buffer[0] = 0;
  output_bufferN = 0;

  swap_ptr(&output_buffer_raw, &output_buffer_raw_bak);

  if (output_buffer_raw)
    output_buffer_raw[0] = 0;
  output_buffer_rawN = 0;
}

gboolean gb_output = FALSE;

void toggle_gb_output()
{
  gb_output = !gb_output;
}

static void append_str(char **buf, int *bufN, char *text, int len)
{
  int requiredN = len + 1 + *bufN;
  *buf = (char *)realloc(*buf, requiredN);
  (*buf)[*bufN] = 0;
  strcat(*buf, text);
  *bufN += len;
}

int trad2sim(char *str, int strN, char **out);
void add_ch_time_str(char *s);

void send_text(char *text)
{
#if WIN32
  if (test_mode)
    return;
#endif

  char *filter;

  if (!text)
    return;
  int len = strlen(text);

  add_ch_time_str(text);

  append_str(&output_buffer_raw, &output_buffer_rawN, text, len);

  char *utf8_gbtext = NULL;

  if (gb_output) {
    len = trad2sim(text, len, &utf8_gbtext);
    text = utf8_gbtext;
  }

direct:
#if UNIX
  filter = getenv("GCIN_OUTPUT_FILTER");
  char filter_text[512];

  if (filter) {
    int pfdr[2], pfdw[2];

    if (pipe(pfdr) == -1) {
      dbg("cannot pipe r\n");
      goto next;
    }

    if (pipe(pfdw) == -1) {
      dbg("cannot pipe w\n");
      goto next;
    }

    int pid = fork();

    if (pid < 0) {
      dbg("cannot fork filter\n");
      goto next;
    }

    if (pid) {
      close(pfdw[0]);
      close(pfdr[1]);
      write(pfdw[1], text, len);
      close(pfdw[1]);
      int rn = read(pfdr[0], filter_text, sizeof(filter_text) - 1);
      filter_text[rn] = 0;
//      puts(filter_text);
      close(pfdr[0]);
      text = filter_text;
      len = rn;
    } else {
      close(pfdr[0]);
      close(pfdw[1]);
      dup2(pfdw[0], 0);
      dup2(pfdr[1], 1);
      if (execl(filter, filter, NULL) < 0) {
        dbg("execl %s err", filter);
        goto next;
      }
    }

  }
#endif
next:
  if (len) {
    append_str(&output_buffer, &output_bufferN, text, len);
  }

  free(utf8_gbtext);
}

void send_output_buffer_bak()
{
  send_text(output_buffer_raw_bak);
}

void set_output_buffer_bak_to_clipboard()
{
  char *text, *utf8_gbtext=NULL;

  if (gb_output) {
    int len = trad2sim(output_buffer_raw_bak, strlen(output_buffer_raw_bak),
      &utf8_gbtext);
    text = utf8_gbtext;
  } else
    text = output_buffer_raw_bak;

#if UNIX && 0
  GtkClipboard *pclipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
#else
  GtkClipboard *pclipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
#endif

  gtk_clipboard_set_text(pclipboard, text, -1);

  free(utf8_gbtext);
}

void sendkey_b5(char *bchar)
{
  char tt[CH_SZ+1];
  int len = utf8_sz(bchar);

  memcpy(tt, bchar, len);
  tt[len]=0;

  send_text(tt);
}


void send_ascii(char key)
{
  sendkey_b5(&key);
}

#if USE_XIM
void export_text_xim()
{
  char *text = output_buffer;

  if (!output_bufferN)
    return;

  XTextProperty tp;
#if 0
  char outbuf[512];
  utf8_big5(output_buffer, outbuf);
  text = outbuf;
  XmbTextListToTextProperty(dpy, &text, 1, XCompoundTextStyle, &tp);
#else
  Xutf8TextListToTextProperty(dpy, &text, 1, XCompoundTextStyle, &tp);
#endif

#if DEBUG && 0
  dbg("sendkey_b5: %s\n", text);
#endif

  ((IMCommitStruct*)current_forward_eve)->flag |= XimLookupChars;
  ((IMCommitStruct*)current_forward_eve)->commit_string = (char *)tp.value;
  IMCommitString(current_ims, (XPointer)current_forward_eve);

  clear_output_buffer();

  XFree(tp.value);
}


static void bounce_back_key()
{
    IMForwardEventStruct forward_ev = *(current_forward_eve);
    IMForwardEvent(current_ims, (XPointer)&forward_ev);
}
#endif

void hide_win0();
void hide_win_gtab();
void hide_win_int();
void hide_win_pho();

int current_in_win_x = -1, current_in_win_y = -1;  // request x/y

void reset_current_in_win_xy()
{
  current_in_win_x = current_in_win_y = -1;
}

void hide_win_anthy();

void hide_in_win(ClientState *cs)
{
  if (!cs) {
#if 0
    dbg("hide_in_win: ic is null\n");
#endif
    return;
  }
#if 0
  dbg("hide_in_win %d\n", ic->in_method);
#endif

  if (timeout_handle) {
    g_source_remove(timeout_handle);
    timeout_handle = 0;
  }

  switch (current_method_type()) {
    case method_type_PHO:
      hide_win_pho();
      break;
#if USE_TSIN
    case method_type_TSIN:
//      flush_tsin_buffer();
      hide_win0();
      break;
#endif
    case method_type_INT_CODE:
      hide_win_int();
      break;
#if USE_ANTHY
    case method_type_ANTHY:
      hide_win_anthy();
      break;
#endif
    default:
      hide_win_gtab();
  }

  reset_current_in_win_xy();
}

void show_win_pho();
void show_win0();
void show_win_int();
void show_win_gtab(), show_win_anthy();

void check_CS()
{
  if (!current_CS) {
    current_CS = &temp_CS;

    temp_CS.input_style = InputStyleOverSpot;
  }
  else
    temp_CS = *current_CS;
}

gboolean force_show;

void show_in_win(ClientState *cs)
{
  if (!cs) {
#if 0
    dbg("show_in_win: ic is null");
#endif
    return;
  }

  switch (current_method_type()) {
    case method_type_PHO:
      show_win_pho();
      break;
#if USE_TSIN
    case method_type_TSIN:
      show_win0();
      break;
#endif
    case method_type_INT_CODE:
      show_win_int();
      break;
#if USE_ANTHY
    case method_type_ANTHY:
      show_win_anthy();
      break;
#endif
    default:
      show_win_gtab();
  }
#if 0
  show_win_stautus();
#endif
}


void move_win_gtab(int x, int y);
void move_win_int(int x, int y);
void move_win0(int x, int y);
void move_win_pho(int x, int y);
void move_win_anthy(int x, int y);

void move_in_win(ClientState *cs, int x, int y)
{
  check_CS();

  if (current_CS && current_CS->fixed_pos) {
    x = current_CS->fixed_x;
    y = current_CS->fixed_y;
  } else
  if (gcin_input_style == InputStyleRoot) {
    x = gcin_root_x;
    y = gcin_root_y;
  }

#if 0
  dbg("move_in_win %d %d\n",x, y);
#endif
#if 1
  if (current_in_win_x == x && current_in_win_y == y)
    return;
#endif
  current_in_win_x = x ; current_in_win_y = y;

  switch (current_method_type()) {
    case method_type_PHO:
      move_win_pho(x, y);
      break;
#if USE_TSIN
    case method_type_TSIN:
      move_win0(x, y);
      break;
#endif
    case method_type_INT_CODE:
      move_win_int(x, y);
      break;
#if USE_ANTHY
    case method_type_ANTHY:
      move_win_anthy(x, y);
      break;
#endif
    default:
      if (!cs->in_method)
        return;
      move_win_gtab(x, y);
  }
}
#if UNIX
static int xerror_handler(Display *d, XErrorEvent *eve)
{
  return 0;
}
#endif

void getRootXY(Window win, int wx, int wy, int *tx, int *ty)
{
  if (!win) {
	*tx = wx;
	*ty = wy;
	return;
  }

#if WIN32
  POINT pt;
  pt.x = wx; pt.y = wy;
  ClientToScreen((HWND)win, &pt);
  *tx = pt.x; *ty=pt.y;
#else
  Window ow;
  XErrorHandler olderr = XSetErrorHandler((XErrorHandler)xerror_handler);
  XTranslateCoordinates(dpy,win,root,wx,wy,tx,ty,&ow);
  XSetErrorHandler(olderr);
#endif
}


void move_IC_in_win(ClientState *cs)
{
#if 0
   dbg("move_IC_in_win\n");
#endif
   Window inpwin = cs->client_win;
#if UNIX
   if (!inpwin) {
//	   dbg("no inpwin\n");
      return;
   }
#endif
   // non focus win filtering is done in the client lib
   if (inpwin != focus_win && focus_win && !cs->b_gcin_protocol)
      return;

   int inpx = cs->spot_location.x;
   int inpy = cs->spot_location.y;

#if UNIX
   XWindowAttributes att;
   XGetWindowAttributes(dpy, inpwin, &att);
   if (att.override_redirect)
     return;

   if (inpx >= att.width)
     inpx = att.width - 1;
   if (inpy >= att.height)
     inpy = att.height - 1;
#else
   if (inpwin) {
     RECT rect;
	 GetClientRect((HWND)inpwin, &rect);
     if (inpx >= rect.right)
       inpx = rect.right - 1;
     if (inpy >= rect.bottom)
       inpy = rect.bottom - 1;
   }
//   dbg("GetClientRect %x %d,%d\n", inpwin, inpx, inpy);
#endif
   int tx,ty;
   getRootXY(inpwin, inpx, inpy, &tx, &ty);

#if 0
   dbg("move_IC_in_win inpxy:%d,%d txy:%d,%d\n", inpx, inpy, tx, ty);
#endif

   move_in_win(cs, tx, ty+1);
}


void update_in_win_pos()
{
//  dbg("update_in_win_pos %d\n", current_CS->input_style);

  check_CS();

  if (current_CS->input_style == InputStyleRoot) {
#if UNIX
    Window r_root, r_child;
    int winx, winy, rootx, rooty;
    u_int mask;

    XQueryPointer(dpy, root, &r_root, &r_child, &rootx, &rooty, &winx, &winy, &mask);

    winx++; winy++;

    Window inpwin = current_CS->client_win;
#if 0
    dbg("update_in_win_pos\n");
#endif
    if (inpwin) {
      int tx, ty;
      Window ow;

      XTranslateCoordinates(dpy, root, inpwin, winx, winy, &tx, &ty, &ow);

      current_CS->spot_location.x = tx;
      current_CS->spot_location.y = ty;
    }
#else
	  int winx=0, winy=0;
      current_CS->spot_location.x = 0;
      current_CS->spot_location.y = 0;
#endif

    move_in_win(current_CS, winx, winy);
  } else {
    move_IC_in_win(current_CS);
  }
}

void win_pho_disp_half_full();
void win_tsin_disp_half_full();
void win_gtab_disp_half_full();
extern char eng_full_str[], full_char_str[];
void update_tray_icon(), load_tray_icon(), load_tray_icon_win32();
static int current_gcin_win32_icon = -1;
void restart_gcin0();

#if TRAY_ENABLED
#if UNIX
void destroy_tray()
{
  if (current_gcin_win32_icon)
    destroy_tray_win32();
  else
    destroy_tray_icon();
}
#endif

void disp_tray_icon()
{
//  dbg("disp_tray_icon\n");
//dbg("disp_tray_icon %d %d\n", current_gcin_win32_icon, gcin_win32_icon);
#if UNIX
  if (current_gcin_win32_icon >= 0 && current_gcin_win32_icon != gcin_win32_icon) {
    destroy_tray();
  }

  current_gcin_win32_icon = gcin_win32_icon;

  if (gcin_win32_icon)
#endif

    load_tray_icon_win32();
#if UNIX
  else
    load_tray_icon();
#endif
}
#endif

void disp_im_half_full()
{
//  dbg("disp_im_half_full\n");
#if TRAY_ENABLED
  disp_tray_icon();
#endif

  switch (current_method_type()) {
    case method_type_PHO:
      win_pho_disp_half_full();
      break;
#if USE_TSIN
    case method_type_TSIN:
      win_tsin_disp_half_full();
      break;
#endif
    default:
      win_gtab_disp_half_full();
      break;
  }
}

void flush_tsin_buffer();
void reset_gtab_all();
void set_tsin_pho_mode();
void set_tsin_pho_mode0(ClientState *cs);

//static u_int orig_caps_state;

void init_state_chinese(ClientState *cs)
{
  cs->im_state = GCIN_STATE_CHINESE;
  set_tsin_pho_mode0(cs);
  if (!cs->in_method)
#if UNIX
    init_in_method(default_input_method);
#else
  if (!last_input_method)
    last_input_method = default_input_method;
  init_in_method(last_input_method);
#endif
}

gboolean output_gbuf();
void update_win_kbm();

void toggle_im_enabled()
{
//    dbg("toggle_im_enabled\n");
    check_CS();

    if (current_CS->in_method < 0 || current_CS->in_method > MAX_GTAB_NUM_KEY)
      p_err("err found");


    if (current_CS->im_state != GCIN_STATE_DISABLED) {
      if (current_CS->im_state == GCIN_STATE_ENG_FULL) {
        current_CS->im_state = GCIN_STATE_CHINESE;
        disp_im_half_full();
        return;
      }

      if (current_method_type() == method_type_TSIN) {
#if USE_TSIN
        flush_tsin_buffer();
#endif
      }
      else {
        output_gbuf();
        reset_gtab_all();
      }

      hide_in_win(current_CS);
#if 0
      hide_win_status();
#endif
      current_CS->im_state = GCIN_STATE_DISABLED;

      update_win_kbm();

#if TRAY_ENABLED
      disp_tray_icon();
#endif
    } else {
      if (!current_method_type())
        init_gtab(current_CS->in_method);


      init_state_chinese(current_CS);
      reset_current_in_win_xy();
#if 1
      show_in_win(current_CS);
      update_in_win_pos();
#else
      update_in_win_pos();
      show_in_win(current_CS);
#endif

      update_win_kbm();

#if TRAY_ENABLED
      disp_tray_icon();
#endif
    }
}

void get_win_gtab_geom();
void get_win0_geom();
void get_win_pho_geom();
void get_win_anthy_geom();
void get_win_int_geom();

void update_active_in_win_geom()
{
//  dbg("update_active_in_win_geom\n");
  switch (current_method_type()) {
    case method_type_PHO:
      get_win_pho_geom();
      break;
#if USE_TSIN
    case method_type_TSIN:
      get_win0_geom();
      break;
#endif
    case method_type_INT_CODE:
	  get_win_int_geom();
      break;
#if USE_ANTHY
    case method_type_ANTHY:
      get_win_anthy_geom();
      break;
#endif
    default:
      get_win_gtab_geom();
      break;
  }
}

extern GtkWidget *gwin_pho, *gwin0, *gwin_gtab, *gwin_int;
int anthy_visible();

gboolean win_is_visible()
{
  if (!current_CS)
    return FALSE;
  switch (current_method_type()) {
    case method_type_PHO:
      return gwin_pho && GTK_WIDGET_VISIBLE(gwin_pho);
#if USE_TSIN
    case method_type_TSIN:
      return gwin0 && GTK_WIDGET_VISIBLE(gwin0);
#endif
    case method_type_INT_CODE:
	  return gwin_int && GTK_WIDGET_VISIBLE(gwin_int);
#if USE_ANTHY
    case method_type_ANTHY:
      return anthy_visible();
#endif
    default:
      if (!gwin_gtab)
        return FALSE;
      return gwin_gtab && GTK_WIDGET_VISIBLE(gwin_gtab);
  }

  return FALSE;
}


void disp_gtab_half_full(gboolean hf);
void tsin_toggle_half_full();
extern char eng_full_str[];

void toggle_half_full_char()
{
#if WIN32
  if (test_mode)
    return;
#endif

  check_CS();

  if (!gcin_shift_space_eng_full) {
    current_CS->b_half_full_char = 0;
    tss.tsin_half_full=0;
    disp_im_half_full();
    return;
  }


//  dbg("toggle_half_full_char\n");

  if (current_method_type() == method_type_TSIN && current_CS->im_state == GCIN_STATE_CHINESE) {
    tsin_toggle_half_full();
  }
  else {
    if (current_CS->im_state == GCIN_STATE_ENG_FULL) {
      current_CS->im_state = GCIN_STATE_DISABLED;
      hide_in_win(current_CS);
    } else
    if (current_CS->im_state == GCIN_STATE_DISABLED) {
      toggle_im_enabled();
      current_CS->im_state = GCIN_STATE_ENG_FULL;
    } else
    if (current_CS->im_state == GCIN_STATE_CHINESE) {
      current_CS->b_half_full_char = !current_CS->b_half_full_char;
    }

//    dbg("current_CS->in_method %d\n", current_CS->in_method);
    disp_im_half_full();
  }
//  dbg("half full toggle\n");
}

void init_inter_code();
void init_tab_pp(gboolean init);
void init_tab_pho();


extern int b_show_win_kbm;

void hide_win_kbm();
extern gboolean win_sym_enabled;
int init_win_anthy();
void show_win_kbm();
void set_gtab_input_method_name(char *s);

void update_win_kbm_inited()
{
  if (win_kbm_inited)
    update_win_kbm();
}

gboolean init_in_method(int in_no)
{
  gboolean init_im = !(cur_inmd && (cur_inmd->flag & FLAG_GTAB_SYM_KBM));

  if (in_no < 0 || in_no > MAX_GTAB_NUM_KEY)
    return FALSE;

  check_CS();


  if (current_CS->in_method != in_no) {
    if (!(inmd[in_no].flag & FLAG_GTAB_SYM_KBM)) {
      if (current_method_type() == method_type_TSIN) {
#if USE_TSIN
      flush_tsin_buffer();
#endif
      } else
        output_gbuf();

      hide_in_win(current_CS);
    }

    if (cur_inmd && (cur_inmd->flag & FLAG_GTAB_SYM_KBM))
      hide_win_kbm();
  }

  reset_current_in_win_xy();

//  dbg("switch init_in_method %x %d\n", current_CS, in_no);

  switch (inmd[in_no].method_type) {
    case method_type_PHO:
      current_CS->in_method = in_no;
      init_tab_pho();
      break;
#if USE_TSIN
    case method_type_TSIN:
      current_CS->in_method = in_no;
      set_tsin_pho_mode0(current_CS);
      init_tab_pp(init_im);
      break;
#endif
    case method_type_INT_CODE:
      current_CS->in_method = in_no;
      init_inter_code();
      break;
#if USE_ANTHY
    case method_type_ANTHY:
      if (init_win_anthy()) {
        current_CS->in_method = in_no;
        show_win_anthy();
      } else {
        return FALSE;
      }
      break;
#endif
    default:
      init_gtab(in_no);
      if (!inmd[in_no].DefChars)
        return FALSE;
      current_CS->in_method = in_no;
      if (!(inmd[in_no].flag & FLAG_GTAB_SYM_KBM))
        show_win_gtab();
      else {
        win_kbm_inited = 1;
        show_win_kbm();
      }

      set_gtab_input_method_name(inmd[in_no].cname);
      break;
  }
#if WIN32
  if (current_CS && current_CS->in_method != last_input_method)
	  last_input_method = current_CS->in_method;
#endif

#if TRAY_ENABLED
  disp_tray_icon();
#endif

  update_in_win_pos();
  update_win_kbm_inited();

  return TRUE;
}


static void cycle_next_in_method()
{
  int i;
#if WIN32
  if (test_mode)
    return;
#endif
  for(i=0; i < MAX_GTAB_NUM_KEY; i++) {
    int v = ((current_CS->in_method + i) % MAX_GTAB_NUM_KEY) + 1;
    if (!(gcin_flags_im_enabled & (1<<v)))
      continue;
    if (!inmd[v].cname || !inmd[v].cname[0])
      continue;

    if (!init_in_method(v))
      continue;

    return;
  }
}

void add_to_tsin_buf_str(char *str);
gboolean gtab_phrase_on();
void insert_gbuf_nokey(char *s);

gboolean full_char_proc(KeySym keysym)
{
  char *s = half_char_to_full_char(keysym);

  if (!s)
    return 0;

  char tt[CH_SZ+1];

  utf8cpy(tt, s);

  if (current_CS->im_state  == GCIN_STATE_ENG_FULL) {
    send_text(tt);
    return 1;
  }

  if (current_method_type() == 6 && current_CS->im_state == GCIN_STATE_CHINESE)
    add_to_tsin_buf_str(tt);
  else
  if (gtab_phrase_on() && ggg.gbufN)
    insert_gbuf_nokey(tt);
  else
    send_text(tt);

  return 1;
}

int feedkey_pho(KeySym xkey, int kbstate);
int feedkey_pp(KeySym xkey, int state);
int feedkey_gtab(KeySym key, int kbstate);
int feed_phrase(KeySym ksym, int state);
int feedkey_intcode(KeySym key);
gboolean feedkey_anthy(int kv, int kvstate);
void tsin_set_eng_ch(int nmod);
static KeySym last_keysym;

gboolean timeout_raise_window(gpointer data)
{
//  dbg("timeout_raise_window\n");
  timeout_handle = 0;
  show_in_win(current_CS);
  return FALSE;
}

extern Window xwin_pho, xwin0, xwin_gtab;
void create_win_sym();
int gcin_switch_keys_lookup(int key);

// return TRUE if the key press is processed
gboolean ProcessKeyPress(KeySym keysym, u_int kev_state)
{
#if 0
  dbg("key press %x %x\n", keysym, kev_state);
#endif
  check_CS();

  if (current_CS->client_win)
    focus_win = current_CS->client_win;

  if (!test_mode && callback_str_buffer && strlen(callback_str_buffer)) {
    send_text(callback_str_buffer);
    callback_str_buffer[0]=0;
    return TRUE;
  }

  if (force_preedit) {
    force_preedit=0;
    return 1;
  }

  if (keysym == XK_space) {
#if 0
    dbg("state %x\n", kev->state);
    dbg("%x\n", Mod4Mask);
#endif
    if (
      ((kev_state & (ControlMask|Mod1Mask|ShiftMask))==ControlMask && gcin_im_toggle_keys==Control_Space) ||
      ((kev_state & Mod1Mask) && gcin_im_toggle_keys==Alt_Space) ||
      ((kev_state & ShiftMask) && gcin_im_toggle_keys==Shift_Space) ||
      ((kev_state & Mod4Mask) && gcin_im_toggle_keys==Windows_Space)
    ) {
      if (current_method_type() == method_type_TSIN) {
        tsin_set_eng_ch(1);
      }

      toggle_im_enabled();
#if UNIX
      return TRUE;
#else
      return FALSE;
#endif
    }
  }

  if (keysym == XK_space && (kev_state & ShiftMask)) {
    if (last_keysym != XK_Shift_L && last_keysym != XK_Shift_R)
      return FALSE;

    toggle_half_full_char();

    return TRUE;
  }


  if ((kev_state & (Mod1Mask|ShiftMask)) == (Mod1Mask|ShiftMask)) {
    if (current_CS->im_state != GCIN_STATE_DISABLED || gcin_eng_phrase_enabled)
      return feed_phrase(keysym, kev_state);
    else
      return 0;
  }

//  dbg("state %x\n", kev_state);
  if ((current_CS->im_state & (GCIN_STATE_ENG_FULL)) ) {
    return full_char_proc(keysym);
  }


  if ((kev_state & ControlMask) && (kev_state&(Mod1Mask|Mod5Mask))) {
    if (keysym == 'g' || keysym == 'r') {
      send_output_buffer_bak();
      return TRUE;
    }

    if (keysym == ',') {
#if 1
      if (current_CS->im_state == GCIN_STATE_CHINESE) {
        if (!win_is_visible())
          win_sym_enabled=1;
        else
          win_sym_enabled^=1;
      } else
        win_sym_enabled=0;
#else
      win_sym_enabled^=1;
#endif

      create_win_sym();
      if (win_sym_enabled) {
        force_show = TRUE;
        if (current_CS->im_state == GCIN_STATE_CHINESE)
          show_in_win(current_CS);
        force_show = FALSE;
      }
      return TRUE;
    }

    if (!gcin_enable_ctrl_alt_switch)
      return FALSE;

    int kidx = gcin_switch_keys_lookup(keysym);
    if (kidx <= 0)
      return FALSE;

    if (!inmd[kidx].cname)
      return FALSE;

    current_CS->im_state = GCIN_STATE_CHINESE;
#if WIN32
    if (!test_mode)
#endif
      init_in_method(kidx);

    return TRUE;
  }

  last_keysym = keysym;

  if (current_CS->im_state == GCIN_STATE_DISABLED) {
    return FALSE;
  }

  if (!current_CS->b_gcin_protocol) {
  if (((keysym == XK_Control_L || keysym == XK_Control_R)
                   && (kev_state & ShiftMask)) ||
      ((keysym == XK_Shift_L || keysym == XK_Shift_R)
                   && (kev_state & ControlMask))) {
     cycle_next_in_method();
     return TRUE;
  }
  }

  if (current_CS->b_raise_window && keysym>=' ' && keysym < 127) {
    if (timeout_handle)
      g_source_remove(timeout_handle);
    timeout_handle = g_timeout_add(200, timeout_raise_window, NULL);
  }

  if (kev_state & ControlMask) {
    if (feed_phrase(keysym, kev_state))
      return TRUE;
  }

  switch(current_method_type()) {
    case method_type_PHO:
      return feedkey_pho(keysym, kev_state);
#if USE_TSIN
    case method_type_TSIN:
      return feedkey_pp(keysym, kev_state);
#endif
    case method_type_INT_CODE:
      return feedkey_intcode(keysym);
#if USE_ANTHY
    case method_type_ANTHY:
      return feedkey_anthy(keysym, kev_state);
#endif
    default:
      return feedkey_gtab(keysym, kev_state);
  }

  return FALSE;
}

int feedkey_pp_release(KeySym xkey, int kbstate);
int feedkey_anthy_release(KeySym xkey, int kbstate);

// return TRUE if the key press is processed
gboolean ProcessKeyRelease(KeySym keysym, u_int kev_state)
{
  check_CS();
#if 0
  dbg_time("key release %x %x\n", keysym, kev_state);
#endif

  if (current_CS->im_state == GCIN_STATE_DISABLED)
    return FALSE;

#if 1
  if (current_CS->b_gcin_protocol && (last_keysym == XK_Shift_L ||
  last_keysym == XK_Shift_R || last_keysym == XK_Control_L || last_keysym == XK_Control_R)) {
    if (((keysym == XK_Control_L || keysym == XK_Control_R)
          && (kev_state & ShiftMask)) ||
        ((keysym == XK_Shift_L || keysym == XK_Shift_R)
          && (kev_state & ControlMask))) {
       cycle_next_in_method();
       return TRUE;
    }
  }
#endif

  switch(current_method_type()) {
    case method_type_TSIN:
      return feedkey_pp_release(keysym, kev_state);
#if USE_ANTHY
    case method_type_ANTHY:
      return feedkey_anthy_release(keysym, kev_state);
#endif
    default:
      return feedkey_gtab_release(keysym, kev_state);
  }

  return FALSE;
}


#if USE_XIM
int xim_ForwardEventHandler(IMForwardEventStruct *call_data)
{
    current_forward_eve = call_data;

    if (call_data->event.type != KeyPress && call_data->event.type != KeyRelease) {
#if DEBUG || 1
        dbg("bogus event type, ignored\n");
#endif
        return True;
    }

    char strbuf[STRBUFLEN];
    KeySym keysym;

    bzero(strbuf, STRBUFLEN);
    XKeyEvent *kev = (XKeyEvent*)&current_forward_eve->event;
    XLookupString(kev, strbuf, STRBUFLEN, &keysym, NULL);

    int ret;
    if (call_data->event.type == KeyPress)
      ret = ProcessKeyPress(keysym, kev->state);
    else
      ret = ProcessKeyRelease(keysym, kev->state);

    if (!ret)
      bounce_back_key();

    export_text_xim();

    return False;
}
#endif

#if UNIX
int skip_window(Window win)
{
  XWindowAttributes att;
  XGetWindowAttributes(dpy, win, &att);
#if 0
  dbg("hhh %d %d class:%d all:%x %x\n", att.width, att.height, att.class,
    att.all_event_masks, att.your_event_mask);
#endif
  if (att.override_redirect)
    return 1;

  return 0;
}
#endif

void gcin_reset();

int gcin_FocusIn(ClientState *cs)
{
  Window win = cs->client_win;

  gcin_reset();
#if UNIX
  if (skip_window(win))
    return FALSE;
#endif

  reset_current_in_win_xy();

  if (cs) {
    Window win = cs->client_win;

    if (focus_win != win) {
      hide_in_win(current_CS);
      focus_win = win;
    }
  }

  current_CS = cs;

//  dbg("current_CS %x %d %d\n", cs, cs->im_state, current_CS->im_state);

  if (win == focus_win) {
    if (cs->im_state != GCIN_STATE_DISABLED) {
      show_in_win(cs);
      move_IC_in_win(cs);
#if 0
      set_win_status_inmd(inmd[cs->in_method].cname);
#endif
    } else
      hide_in_win(cs);
  }

  update_win_kbm();
#if TRAY_ENABLED
  disp_tray_icon();
#endif
#if 0
  dbg_time("gcin_FocusIn %x %x\n",cs, current_CS);
#endif
  return True;
}


#if USE_XIM
IC *FindIC(CARD16 icid);
void load_IC(IC *rec);
CARD16 connect_id;

int xim_gcin_FocusIn(IMChangeFocusStruct *call_data)
{
    IC *ic = FindIC(call_data->icid);
    ClientState *cs = &ic->cs;
    connect_id = call_data->connect_id;

    if (ic) {
      gcin_FocusIn(cs);

      load_IC(ic);
    }

#if DEBUG && 0
    dbg("xim_gcin_FocusIn %d\n", call_data->icid);
#endif
    return True;
}
#endif



static gint64 last_focus_out_time;

int gcin_FocusOut(ClientState *cs)
{
  gint64 t = current_time();

  if (cs != current_CS)
     return FALSE;

#if UNIX
//  dbg("gcin_FocusOut\n");
  if (skip_window(cs->client_win))
    return FALSE;
#endif
  if (t - last_focus_out_time < 100000) {
    last_focus_out_time = t;
    return FALSE;
  }

  last_focus_out_time = t;

  if (cs == current_CS) {
    hide_in_win(cs);
  }

  reset_current_in_win_xy();

  if (cs == current_CS)
    temp_CS = *current_CS;

#if 0
  dbg("focus out\n");
#endif

  return True;
}

int tsin_get_preedit(char *str, GCIN_PREEDIT_ATTR attr[], int *cursor, int *sub_comp_len);
int anthy_get_preedit(char *str, GCIN_PREEDIT_ATTR attr[], int *pcursor);
int gtab_get_preedit(char *str, GCIN_PREEDIT_ATTR attr[], int *pcursor, int *sub_comp_len);
int int_get_preedit(char *str, GCIN_PREEDIT_ATTR attr[], int *cursor, int *sub_comp_len);
int pho_get_preedit(char *str, GCIN_PREEDIT_ATTR attr[], int *cursor, int *sub_comp_len);

int gcin_get_preedit(ClientState *cs, char *str, GCIN_PREEDIT_ATTR attr[], int *cursor, int *sub_comp_len)
{
//  dbg("gcin_get_preedit %x\n", current_CS);
  if (!current_CS) {
empty:
//    dbg("empty\n");
    str[0]=0;
    *cursor=0;
    return 0;
  }

  str[0]=0;
  *sub_comp_len=0;

//  cs->use_preedit = TRUE;

  switch(current_method_type()) {
    case method_type_PHO:
		return pho_get_preedit(str, attr, cursor, sub_comp_len);
    case method_type_INT_CODE:
		return int_get_preedit(str, attr, cursor, sub_comp_len);
#if USE_TSIN
    case method_type_TSIN:
      return tsin_get_preedit(str, attr, cursor, sub_comp_len);
#endif
#if USE_ANTHY
    case method_type_ANTHY:
      return anthy_get_preedit(str, attr, cursor);
#endif
    default:
      return gtab_get_preedit(str, attr, cursor, sub_comp_len);
//      dbg("metho %d\n", current_CS->in_method);
  }

  return 0;
}

void pho_reset();
int tsin_reset(),gcin_anthy_reset();
void gtab_reset();

void gcin_reset()
{
#if 1
  if (!current_CS)
    return;
//  dbg("gcin_reset\n");

  switch(current_method_type()) {
    case method_type_PHO:
      pho_reset();
      return;
    case method_type_INT_CODE:
      return;
#if USE_TSIN
    case method_type_TSIN:
      tsin_reset();
      return;
#endif
#if USE_ANTHY
    case method_type_ANTHY:
      gcin_anthy_reset();
      return;
#endif
    default:
      gtab_reset();
//      dbg("metho %d\n", current_CS->in_method);
  }
#endif
}


#if USE_XIM
int xim_gcin_FocusOut(IMChangeFocusStruct *call_data)
{
    IC *ic = FindIC(call_data->icid);
    ClientState *cs = &ic->cs;

    gcin_FocusOut(cs);

    return True;
}
#endif


gboolean gcin_edit_display_ap_only()
{
#if WIN32
  if (test_mode)
    return TRUE;
#endif
//  dbg("gcin_edit_display_ap_only %d\n", current_CS->use_preedit)
  return current_CS->use_preedit && gcin_edit_display==GCIN_EDIT_DISPLAY_ON_THE_SPOT;
}

void flush_anthy_input();

void flush_edit_buffer()
{
//  dbg("flush_edit_buffer\n");
  if (!current_CS)
    return;
//  dbg("gcin_reset\n");
  switch(current_method_type()) {
#if USE_TSIN
    case method_type_TSIN:
      flush_tsin_buffer();
      break;
#endif
#if USE_ANTHY
    case method_type_ANTHY:
      flush_anthy_input();
      break;
#endif
    default:
      output_gbuf();
//      dbg("metho %d\n", current_CS->in_method);
  }
#if 0
  dbg("output_bufferN:%d\n", output_bufferN);
  if (output_bufferN) {
    output_buffer_call_back();
  }
#endif
}

#if WIN32
void pho_save_gst(), tsin_save_gst(), gtab_save_gst();
void pho_restore_gst(), tsin_restore_gst(), gtab_restore_gst();

gboolean ProcessTestKeyPress(KeySym keysym, u_int kev_state)
{
  if (!current_CS)
    return TRUE;
//  dbg("gcin_reset\n");
  gboolean v;

  test_mode= TRUE;
  switch(current_method_type()) {
    case method_type_PHO:
      pho_save_gst();
      v = ProcessKeyPress(keysym, kev_state);
      pho_restore_gst();
      break;
    case method_type_INT_CODE:
      v = ProcessKeyPress(keysym, kev_state);
      break;
    case method_type_TSIN:
      tsin_save_gst();
      v = ProcessKeyPress(keysym, kev_state);
      tsin_restore_gst();
      break;
#if USE_ANTHY
    case method_type_ANTHY:
      break;
#endif
    default:
      gtab_save_gst();
      v = ProcessKeyPress(keysym, kev_state);
      gtab_restore_gst();
  }

  test_mode= FALSE;

  return v;
}


gboolean Process2KeyPress(KeySym keysym, u_int kev_state)
{
  gboolean tv = ProcessTestKeyPress(keysym, kev_state);
  gboolean v = ProcessKeyPress(keysym, kev_state);

  if (tv != v)
    dbg("ProcessKeyPress %x -> %d %d\n",keysym, tv, v);

  return v;
}


gboolean ProcessTestKeyRelease(KeySym keysym, u_int kev_state)
{
  test_mode = TRUE;
  gboolean v = ProcessKeyRelease(keysym, kev_state);
  test_mode = FALSE;
  return v;
}


gboolean Process2KeyRelease(KeySym keysym, u_int kev_state)
{
  gboolean tv = ProcessTestKeyRelease(keysym, kev_state);
  gboolean v = ProcessKeyRelease(keysym, kev_state);

  if (tv != v)
    dbg("ProcessKeyRelease %x -> %d %d\n",keysym, tv, v);

  return v;
}
#endif
