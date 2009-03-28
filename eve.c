#include "gcin.h"
#include "gtab.h"
#include <X11/extensions/XTest.h>


#define STRBUFLEN 64

extern Display *dpy;
#if USE_XIM
extern XIMS current_ims;
static IMForwardEventStruct *current_forward_eve;
#endif

static char callback_str_buffer[32];
Window focus_win;
static int timeout_handle;

static void send_fake_key_eve()
{
  KeyCode kc_shift_l = XKeysymToKeycode(dpy, XK_Shift_L);
  XTestFakeKeyEvent(dpy, kc_shift_l, True, CurrentTime);
  XTestFakeKeyEvent(dpy, kc_shift_l, False, CurrentTime);
}

void send_text_call_back(char *text)
{
  strcpy(callback_str_buffer, text);
  send_fake_key_eve();
}


ClientState *current_CS;
ClientState temp_CS;

gboolean init_in_method(int in_no);

char *output_buffer, *output_buffer_bak;
int  output_bufferN;

void clear_output_buffer()
{
  char *t;

  t = output_buffer;
  output_buffer = output_buffer_bak;
  output_buffer_bak = t;

  if (output_buffer)
    output_buffer[0] = 0;

  output_bufferN = 0;
}

gboolean gb_output = FALSE;

void send_text(char *text)
{
  if (!text)
    return;
  int len = strlen(text);

  char *utf8_gbtext = NULL;

  if (gb_output) {
    u_int rn = 0 , wn = 0;
    GError *err = NULL;
    char *big5 = g_convert(text, len, "big5", "UTF-8", &rn, &wn, &err);

    if (err) {
      dbg("utf8 -> gb  convert error %d %d\n", rn, wn);
      return;
    }

    err = NULL;
    char *gbtext = g_convert(big5, wn, "GB2312", "big5", &rn, &wn, &err);
    g_free(big5);

    if (err) {
      dbg("big5 -> gb  convert error %d %d\n", rn, wn);
      return;
    }

    err = NULL;
    utf8_gbtext = g_convert(gbtext, wn, "UTF-8", "GB2312", &rn, &wn, &err);
    g_free(gbtext);

    if (err) {
      dbg("gb -> utf8  convert error %d %d\n", rn, wn);
      return;
    }

    text = utf8_gbtext;
    len = wn;
  }

  int requiredN = len + 1 + output_bufferN;

  output_buffer = realloc(output_buffer, requiredN);
  output_buffer[output_bufferN] = 0;

  strcat(output_buffer, text);
  output_bufferN += len;

  g_free(utf8_gbtext);
}

void send_output_buffer_bak()
{
  send_text(output_buffer_bak);
}

void sendkey_b5(char *bchar)
{
  char tt[CH_SZ+1];
  int len = utf8_sz(bchar);

  memcpy(tt, bchar, len);
  tt[len]=0;

  send_text(tt);
}

#if USE_XIM
void export_text_xim()
{
  char *text = output_buffer;

  if (!output_bufferN)
    return;


  XTextProperty tp;
  char outbuf[512];

#if !SOL
  if (pxim_arr->b_send_utf8_str) {
    Xutf8TextListToTextProperty(dpy, &text, 1, XCompoundTextStyle, &tp);
  } else
#endif
  {
    utf8_big5(output_buffer, outbuf);
    text = outbuf;
    XmbTextListToTextProperty(dpy, &text, 1, XCompoundTextStyle, &tp);
  }

#if DEBUG
  dbg("sendkey_b5: %s\n", text);
#endif

  ((IMCommitStruct*)current_forward_eve)->flag |= XimLookupChars;
  ((IMCommitStruct*)current_forward_eve)->commit_string = tp.value;
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

void hide_in_win(ClientState *cs)
{
  if (!cs) {
#if DEBUG
    dbg("hide_in_win: ic is null");
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

  switch (cs->in_method) {
    case 3:
      hide_win_pho();
      break;
    case 6:
//      flush_tsin_buffer();
      hide_win0();
      break;
    case 0:
      hide_win_int();
      break;
    default:
      hide_win_gtab();
  }

  reset_current_in_win_xy();
  hide_win_status();
}

void show_win_pho();
void show_win0();
void show_win_int();
void show_win_gtab();

void show_in_win(ClientState *cs)
{
  if (!cs) {
#if DEBUG
    dbg("show_in_win: ic is null");
#endif
    return;
  }

  switch (cs->in_method) {
    case 3:
      show_win_pho();
      break;
    case 6:
      show_win0();
      break;
    case 0:
      show_win_int();
      break;
    default:
      show_win_gtab();
  }

  show_win_stautus();
}


void move_win_gtab(int x, int y);
void move_win_int(int x, int y);
void move_win0(int x, int y);
void move_win_pho(int x, int y);


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

#if DEBUG || 0
  dbg("move_in_win %d %d\n",x, y);
#endif
#if 1
  if (current_in_win_x == x && current_in_win_y == y)
    return;
#endif
  current_in_win_x = x ; current_in_win_y = y;

  switch (cs->in_method) {
    case 3:
      move_win_pho(x, y);
      break;
    case 6:
      move_win0(x, y);
      break;
    case 0:
      move_win_int(x, y);
      break;
    default:
      if (!cs->in_method)
        return;
      move_win_gtab(x, y);
  }
}

static int xerror_handler(Display *d, XErrorEvent *eve)
{
  return 0;
}

static void getRootXY(Window win, int wx, int wy, int *tx, int *ty)
{
  Window ow;
  void *olderr = XSetErrorHandler((XErrorHandler)xerror_handler);

  XTranslateCoordinates(dpy,win,root,wx,wy,tx,ty,&ow);

  XSetErrorHandler(olderr);
}


void move_IC_in_win(ClientState *cs)
{
#if DEBUG
   dbg("move_IC_in_win\n");
#endif
   Window inpwin = cs->client_win;

   if (!inpwin)
      return;

   // non focus win filtering is done in the client lib
   if (inpwin != focus_win && focus_win && !cs->b_gcin_protocol)
      return;

   int inpx = cs->spot_location.x;
   int inpy = cs->spot_location.y;
   XWindowAttributes att;

   XGetWindowAttributes(dpy, inpwin, &att);

   if (inpx >= att.width)
     inpx = att.width - 1;
   if (inpy >= att.height)
     inpy = att.height - 1;

   int tx, ty;
   getRootXY(inpwin, inpx, inpy, &tx, &ty);
#if DEBUG
   dbg("move_IC_in_win %d %d   txy:%d %d\n", inpx, inpy, tx, ty);
#endif

   move_in_win(cs, tx, ty+1);
}


void update_in_win_pos()
{
//  dbg("update_in_win_pos %d\n", current_CS->input_style);
  check_CS();

  if (current_CS->input_style == InputStyleRoot) {
    Window r_root, r_child;
    int winx, winy, rootx, rooty;
    u_int mask;


    XQueryPointer(dpy, root, &r_root, &r_child, &rootx, &rooty, &winx, &winy, &mask);

    winx++; winy++;

    Window inpwin = current_CS->client_win;
#if DEBUG
    dbg("update_in_win_pos\n");
#endif
    if (inpwin) {
      int tx, ty;
      Window ow;

      XTranslateCoordinates(dpy, root, inpwin, winx, winy, &tx, &ty, &ow);

      current_CS->spot_location.x = tx;
      current_CS->spot_location.y = ty;
    }

    move_in_win(current_CS, winx, winy);
  } else {
    move_IC_in_win(current_CS);
  }
}

void win_pho_disp_half_full();
void win_tsin_disp_half_full();
void win_gtab_disp_half_full();
extern char eng_full_str[], full_char_str[];

void disp_im_half_full()
{
  if (current_CS->im_state == GCIN_STATE_ENG_FULL) {
     set_win_status_half_full(eng_full_str);
  }
  else
  if (current_CS->im_state == GCIN_STATE_CHINESE)
     set_win_status_half_full(current_CS->b_half_full_char?full_char_str:"");
  else
     set_win_status_half_full("");

  switch (current_CS->in_method) {
    case 3:
      win_pho_disp_half_full();
      break;
    case 6:
      win_tsin_disp_half_full();
      break;
    default:
      win_gtab_disp_half_full();
      break;
  }
}

gboolean flush_tsin_buffer();
void reset_gtab_all();

void toggle_im_enabled(u_int kev_state)
{
//    dbg("toggle_im_enabled\n");
    check_CS();

    if (current_CS->in_method < 0 || current_CS->in_method >= gcin_switch_keysN)
      return;

    static u_int orig_caps_state;

    if (current_CS->im_state != GCIN_STATE_DISABLED) {
      if (current_CS->in_method== 6 && (kev_state & LockMask) != orig_caps_state &&
          tsin_chinese_english_toggle_key == TSIN_CHINESE_ENGLISH_TOGGLE_KEY_CapsLock) {
        KeyCode caps = XKeysymToKeycode(dpy, XK_Caps_Lock);
        XTestFakeKeyEvent(dpy, caps, True, CurrentTime);
        XTestFakeKeyEvent(dpy, caps, False, CurrentTime);
      }

      if (current_CS->im_state == GCIN_STATE_ENG_FULL) {
        current_CS->im_state = GCIN_STATE_CHINESE;
        disp_im_half_full();
        return;
      }

      if (current_CS->in_method == 6)
        flush_tsin_buffer();
      else {
        reset_gtab_all();
      }

      hide_in_win(current_CS);
      hide_win_status();
      current_CS->im_state = GCIN_STATE_DISABLED;
    } else {
      current_CS->im_state = GCIN_STATE_CHINESE;
      orig_caps_state = kev_state & LockMask;

      if (!current_CS->in_method) {
        init_in_method(default_input_method);
      }

      reset_current_in_win_xy();
#if 1
      show_in_win(current_CS);
      update_in_win_pos();
#else
      update_in_win_pos();
      show_in_win(current_CS);
#endif

      if (gcin_pop_up_win)
        show_win_stautus();
    }
}

void get_win_gtab_geom();
void get_win0_geom();
void get_win_pho_geom();

void update_active_in_win_geom()
{
//  dbg("update_active_in_win_geom\n");
  switch (current_CS->in_method) {
    case 3:
      get_win_pho_geom();
      break;
    case 6:
      get_win0_geom();
      break;
    case 0:
      break;
    default:
      get_win_gtab_geom();
      break;
  }
}

void disp_gtab_half_full(gboolean hf);
void tsin_toggle_half_full();
extern char eng_full_str[];

void toggle_half_full_char(u_int kev_state)
{

  if (current_CS->in_method == 6 && current_CS->im_state == GCIN_STATE_CHINESE) {
    tsin_toggle_half_full();
  }
  else {
    if (current_CS->im_state == GCIN_STATE_ENG_FULL) {
      current_CS->im_state = GCIN_STATE_DISABLED;
      disp_im_half_full();
      hide_in_win(current_CS);
      return;
    } else
    if (current_CS->im_state == GCIN_STATE_DISABLED) {
      if (gcin_shift_space_eng_full) {
        toggle_im_enabled(kev_state);
        current_CS->im_state = GCIN_STATE_ENG_FULL;
      } else
        return;
    } else
    if (current_CS->im_state == GCIN_STATE_CHINESE) {
      current_CS->b_half_full_char = !current_CS->b_half_full_char;
    }
//    dbg("current_CS->in_method %d\n", current_CS->in_method);
    disp_im_half_full();
  }
//  dbg("half full toggle\n");
}

void init_gtab(int inmdno, int usenow);
void init_inter_code(int usenow);
void init_tab_pp(int usenow);
void init_tab_pho();

void check_CS()
{
  if (!current_CS) {
    current_CS = &temp_CS;

    temp_CS.input_style = InputStyleOverSpot;
  }
  else
    temp_CS = *current_CS;
}


gboolean init_in_method(int in_no)
{
  gboolean status = TRUE;

  if (in_no < 0 || in_no > MAX_GTAB_NUM_KEY)
    return FALSE;

  check_CS();


  if (current_CS->in_method != in_no) {
    if (current_CS->in_method == 6)
      flush_tsin_buffer();

    hide_in_win(current_CS);
  }

  reset_current_in_win_xy();

//  dbg("switch init_in_method %x %d\n", current_CS, in_no);
  current_CS->in_method = in_no;


  switch (in_no) {
    case 3:
      init_tab_pho();
      break;
    case 6:
      init_tab_pp(True);
      break;
    case 0:
      init_inter_code(True);
      break;
    default:
      show_win_gtab();
//      init_gtab(in_no, True);
      break;
  }

  set_win_status_inmd(inmd[in_no].cname);

  update_in_win_pos();
  return status;
}


static void cycle_next_in_method()
{

  int i;

  for(i=1; i <= gcin_switch_keysN; i++) {
    int v = (current_CS->in_method + i) % gcin_switch_keysN;
    if (!(gcin_flags_im_enabled & (1<<v)))
      continue;
    if (!inmd[v].cname || !inmd[v].cname[0])
      continue;

    init_in_method(v);
    return;
  }
}

gboolean full_char_proc(KeySym keysym)
{
  char *s = half_char_to_full_char(keysym);

  if (!s)
    return 0;

  char tt[CH_SZ+1];

  utf8cpy(tt, s);

  send_text(tt);
  return 1;
}


int feedkey_pho(KeySym xkey, int kbstate);
int feedkey_pp(KeySym xkey, int state);
int feedkey_gtab(KeySym key, int kbstate);
int feed_phrase(KeySym ksym, int state);
int feedkey_intcode(KeySym key);
void tsin_set_eng_ch(int nmod);
static KeySym last_keysym;

gboolean timeout_raise_window()
{
  timeout_handle = 0;
  show_in_win(current_CS);
  return FALSE;
}


// return TRUE if the key press is processed
gboolean ProcessKeyPress(KeySym keysym, u_int kev_state)
{
#if 0
  dbg_time("key press %x %x", keysym, kev_state);
#endif
  check_CS();

  if (current_CS->client_win)
    focus_win = current_CS->client_win;

  if (strlen(callback_str_buffer)) {
    send_text(callback_str_buffer);
    callback_str_buffer[0]=0;
    return TRUE;
  }

  if (keysym == XK_space) {
#if 0
    dbg("state %x\n", kev->state);
    dbg("%x\n", Mod4Mask);
#endif
    if (
      ((kev_state & ControlMask) && gcin_im_toggle_keys==Control_Space) ||
      ((kev_state & Mod1Mask) && gcin_im_toggle_keys==Alt_Space) ||
      ((kev_state & ShiftMask) && gcin_im_toggle_keys==Shift_Space) ||
      ((kev_state & Mod4Mask) && gcin_im_toggle_keys==Windows_Space)
    ) {
      if (current_CS->in_method == 6) {
        tsin_set_eng_ch(1);
      }

      toggle_im_enabled(kev_state);
      return TRUE;
    }
  }

  if (keysym == XK_space && (kev_state & ShiftMask)) {
    if (last_keysym != XK_Shift_L && last_keysym != XK_Shift_R)
      return FALSE;

    toggle_half_full_char(kev_state);

    return TRUE;
  }


  if ((kev_state & (Mod1Mask|ShiftMask)) == (Mod1Mask|ShiftMask)) {
    return feed_phrase(keysym, kev_state);
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
      create_win_sym();

      return TRUE;
    }

    int kidx = gcin_switch_keys_lookup(keysym);
    if (kidx < 0)
      return FALSE;

    current_CS->im_state = GCIN_STATE_CHINESE;
    init_in_method(kidx);
    return TRUE;
  }

  last_keysym = keysym;

  if (current_CS->im_state == GCIN_STATE_DISABLED) {
    return FALSE;
  }


  if (((keysym == XK_Control_L || keysym == XK_Control_R)
                   && (kev_state & ShiftMask)) ||
      ((keysym == XK_Shift_L || keysym == XK_Shift_R)
                   && (kev_state & ControlMask))) {
     cycle_next_in_method();
     return TRUE;
  }

  if (current_CS->b_raise_window && !timeout_handle) {
    timeout_handle = g_timeout_add(200, timeout_raise_window, NULL);
  }

  switch(current_CS->in_method) {
    case 3:
      return feedkey_pho(keysym, kev_state);
    case 6:
      return feedkey_pp(keysym, kev_state);
    case 0:
      return feedkey_intcode(keysym);
    default:
      return feedkey_gtab(keysym, kev_state);
  }


  return FALSE;
}

int feedkey_pp_release(KeySym xkey, int kbstate);

// return TRUE if the key press is processed
gboolean ProcessKeyRelease(KeySym keysym, u_int kev_state)
{
  check_CS();

  if (current_CS->im_state == GCIN_STATE_DISABLED)
    return FALSE;

  switch(current_CS->in_method) {
    case 6:
      return feedkey_pp_release(keysym, kev_state);
  }

  return FALSE;
}


#if USE_XIM
int xim_ForwardEventHandler(IMForwardEventStruct *call_data)
{
    current_forward_eve = call_data;

    if (call_data->event.type != KeyPress) {
#if DEBUG
        dbg("bogus event type, ignored\n");
#endif
        return True;
    }

    char strbuf[STRBUFLEN];
    KeySym keysym;

    bzero(strbuf, STRBUFLEN);
    XKeyEvent *kev = (XKeyEvent*)&current_forward_eve->event;
    XLookupString(kev, strbuf, STRBUFLEN, &keysym, NULL);


    if (!ProcessKeyPress(keysym, kev->state))
      bounce_back_key();

    export_text_xim();

    return False;
}
#endif

IC *FindIC(CARD16 icid);
void load_IC(IC *rec);

int gcin_FocusIn(ClientState *cs)
{
  Window win = cs->client_win;

  reset_current_in_win_xy();

  if (cs) {
    Window win = cs->client_win;

    if (focus_win != win) {
      hide_in_win(current_CS);
      focus_win = win;
    }
  }

  current_CS = cs;

  if (win == focus_win) {
    if (cs->im_state != GCIN_STATE_DISABLED) {
#if 0
      /* something changed in gtk or X11, it is not possible to move window if the window
         is not visible
       */
      move_IC_in_win(cs);
      show_in_win(cs);
#else
      show_in_win(cs);
      move_IC_in_win(cs);
#endif
      set_win_status_inmd(inmd[cs->in_method].cname);
    } else
      hide_in_win(cs);
  }


#if 0
  dbg_time("gcin_FocusIn %x %x\n",cs, current_CS);
#endif
  return True;
}


#if USE_XIM
int xim_gcin_FocusIn(IMChangeFocusStruct *call_data)
{
    IC *ic = FindIC(call_data->icid);
    ClientState *cs = &ic->cs;

    if (ic) {
      gcin_FocusIn(cs);

      load_IC(ic);
    }

#if DEBUG
    dbg("xim_gcin_FocusIn %d\n", call_data->icid);
#endif
    return True;
}
#endif

int gcin_FocusOut(ClientState *cs)
{
    if (cs == current_CS) {
      hide_in_win(cs);
    }

    reset_current_in_win_xy();

    if (cs == current_CS)
      temp_CS = *current_CS;

    return True;
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
