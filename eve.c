#include "gcin.h"
#include "gtab.h"
#include <X11/extensions/XTest.h>


#define STRBUFLEN 64

extern Display *dpy;
extern XIMS current_ims;
static IMForwardEventStruct *current_forward_eve;

static char callback_str_buffer[32];
Window focus_win;


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

gboolean init_in_method(int in_no);

char *output_buffer;
int  output_bufferN, output_bufferN_a;


void send_text(char *text)
{
  int len = strlen(text);
  int requiredN = len + 1 + output_bufferN;

  if (requiredN >= output_bufferN_a) {
    output_bufferN_a = requiredN;
    output_buffer = realloc(output_buffer, output_bufferN_a);
    output_buffer[output_bufferN] = 0;
  }

  strcat(output_buffer, text);
  output_bufferN += len;
}


void sendkey_b5(char *bchar)
{
  char tt[CH_SZ+1];
  int len = utf8_sz(bchar);

  memcpy(tt, bchar, len);
  tt[len]=0;

  send_text(tt);
}


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

  output_bufferN = 0; output_buffer[0] = 0;

  XFree(tp.value);
}



static void bounce_back_key()
{
    IMForwardEventStruct forward_ev = *(current_forward_eve);
    IMForwardEvent(current_ims, (XPointer)&forward_ev);
}

void hide_win0();
void hide_win_gtab();
void hide_win_int();
void hide_win_pho();

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
//  dbg("hide_in_win\n");
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
}

void move_win_gtab(int x, int y);
void move_win_int(int x, int y);
void move_win0(int x, int y);
void move_win_pho(int x, int y);

void move_in_win(ClientState *cs, int x, int y)
{
  if (!cs) {
#if DEBUG
    dbg("move_in_win: cs is null");
#endif
    return;
  }

#if DEBUG || 0
  dbg("move_in_win %d %d\n",x, y);
#endif

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

void move_IC_win0(ClientState *rec);
void move_IC_in_win(ClientState *rec);

void update_in_win_pos()
{
  if (current_CS && current_CS->input_style == InputStyleRoot) {
    Window r_root, r_child;
    int winx, winy, rootx, rooty;
    u_int mask;

    XQueryPointer(dpy, root, &r_root, &r_child, &rootx, &rooty, &winx, &winy, &mask);

    winx++; winy++;

    if (current_CS) {
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
    }

    move_in_win(current_CS, winx, winy);
  } else {
    if (current_CS)
      move_IC_in_win(current_CS);
  }
}

IC *findIC();
gboolean flush_tsin_buffer();

void toggle_im_enabled(u_int kev_state)
{
//    dbg("toggle_im_enabled\n");
    if (!current_CS)
      return;

    if (current_CS->in_method < 0 || current_CS->in_method > 10)
      return;

    static u_int orig_caps_state;

    if (current_CS->b_im_enabled) {
      if (current_CS->in_method== 6 && (kev_state & LockMask) != orig_caps_state &&
          tsin_chinese_english_toggle_key == TSIN_CHINESE_ENGLISH_TOGGLE_KEY_CapsLock) {
        KeyCode caps = XKeysymToKeycode(dpy, XK_Caps_Lock);
        XTestFakeKeyEvent(dpy, caps, True, CurrentTime);
        XTestFakeKeyEvent(dpy, caps, False, CurrentTime);
      }

      if (current_CS->in_method == 6)
        flush_tsin_buffer();

      hide_in_win(current_CS);
      current_CS->b_im_enabled = FALSE;
    } else {
      current_CS->b_im_enabled = TRUE;
      orig_caps_state = kev_state & LockMask;

      if (!current_CS->in_method) {
        init_in_method(default_input_method);
      }

      update_in_win_pos();

      show_in_win(current_CS);
    }
}

void get_win_gtab_geom();
void get_win0_geom();

void update_active_in_win_geom()
{
//  dbg("update_active_in_win_geom\n");
  switch (current_CS->in_method) {
    case 3:
      get_win0_geom();
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

void toggle_half_full_char()
{

  if (current_CS->in_method == 6)
    tsin_toggle_half_full();
  else {
    current_CS->b_half_full_char ^= 1;
    disp_gtab_half_full(current_CS->b_half_full_char);
  }
//  dbg("half full toggle\n");
}

void init_gtab(int inmdno, int usenow);
void init_inter_code(int usenow);
void init_tab_pp(int usenow);
void init_tab_pho();

gboolean init_in_method(int in_no)
{
  gboolean status = TRUE;

  if (in_no < 0 || in_no > MAX_GTAB_NUM_KEY)
    return FALSE;

  if (current_CS->in_method != in_no)
    hide_in_win(current_CS);

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

  update_in_win_pos();
  return status;
}


static void cycle_next_in_method()
{

  int i;

  for(i=1; i <= 10; i++) {
    int v = (current_CS->in_method + i) % 10;
    if (!(gcin_flags_im_enabled & (1<<v)))
      continue;
    if (!inmd[v].cname[0])
      continue;

    init_in_method(v);
    return;
  }
}


int feedkey_pho(KeySym xkey);
int feedkey_pp(KeySym xkey, int state);
int feedkey_gtab(KeySym key, int kbstate);
int feed_phrase(KeySym ksym);
int feedkey_intcode(KeySym key);
void tsin_set_eng_ch(int nmod);

// return TRUE if the key press is processed
gboolean ProcessKeyPress(KeySym keysym, u_int kev_state)
{
  if (!current_CS)
    return FALSE;

  if (current_CS->client_win)
    focus_win = current_CS->client_win;

  if (strlen(callback_str_buffer)) {
    send_text(callback_str_buffer);
    callback_str_buffer[0]=0;
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

  if (!current_CS || !current_CS->b_im_enabled) {
    return FALSE;
  }

  if (keysym == XK_space && (kev_state & ShiftMask)) {
    toggle_half_full_char();
    return TRUE;
  }

  if (((keysym == XK_Control_L || keysym == XK_Control_R)
                   && (kev_state & ShiftMask)) ||
      ((keysym == XK_Shift_L || keysym == XK_Shift_R)
                   && (kev_state & ControlMask))) {
     cycle_next_in_method();
     return TRUE;
  }

  if ((kev_state & ControlMask) && (kev_state&(Mod1Mask|Mod5Mask))) {
    init_in_method(keysym- XK_0);
    return TRUE;
  }

  if ((kev_state & (Mod1Mask|ShiftMask)) == (Mod1Mask|ShiftMask)) {
    return feed_phrase(keysym);
  }


  switch(current_CS->in_method) {
    case 3:
      return feedkey_pho(keysym);
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
  if (!current_CS || !current_CS->b_im_enabled)
    return FALSE;

  if (current_CS->client_win)
    focus_win = current_CS->client_win;

  switch(current_CS->in_method) {
    case 6:
      return feedkey_pp_release(keysym, kev_state);
  }

  return FALSE;
}



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

IC *FindIC(CARD16 icid);
void load_IC(IC *rec);

int gcin_FocusIn(ClientState *cs)
{
  Window win = cs->client_win;

  if (cs) {
    Window win = cs->client_win;

    if (focus_win != win) {
      hide_in_win(current_CS);
      focus_win = win;
    }
  }

  if (current_CS != cs && (win == focus_win || !current_CS))
    current_CS = cs;

  if (win == focus_win) {
    if (cs->b_im_enabled) {
      move_IC_in_win(cs);
      show_in_win(cs);
    } else
      hide_in_win(cs);
  }


#if DEBUG || 0
  dbg("gcin_FocusIn %x %x\n",cs, current_CS);
#endif
  return True;
}



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

int gcin_FocusOut(ClientState *cs)
{
    if (cs == current_CS) {
      hide_in_win(cs);
    }
#if DEBUG || 0
    dbg("focus out %x %x\n", cs, current_CS);
#endif
//    focus_win = 0;

    return True;
}

int xim_gcin_FocusOut(IMChangeFocusStruct *call_data)
{
    IC *ic = FindIC(call_data->icid);
    ClientState *cs = &ic->cs;

    gcin_FocusOut(cs);

    return True;
}
