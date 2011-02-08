#include "gcin.h"
#include "config.h"
#include "gcin-version.h"
#if UNIX
#include <signal.h>
#endif
#if GCIN_i18n_message
#include <libintl.h>
#endif

#if UNIX
Window root;
#endif
Display *dpy;

int win_xl, win_yl;
int win_x, win_y;   // actual win x/y
int dpy_xl, dpy_yl;
DUAL_XIM_ENTRY xim_arr[1];

gboolean win_kbm_inited;

unich_t *fullchar[]=
{_L("　"),_L("！"),_L("”"),_L("＃"),_L("＄"),_L("％"),_L("＆"),_L("’"),_L("（"),_L("）"),_L("＊"),_L("＋"),
_L("，"),_L("－"),_L("．"),_L("／"),_L("０"),_L("１"),_L("２"),_L("３"),_L("４"),_L("５"),_L("６"),_L("７"),_L("８"),_L("９"),_L("："),_L("；"),_L("＜"),_L("＝"),_L("＞"),_L("？"),
_L("＠"),_L("Ａ"),_L("Ｂ"),_L("Ｃ"),_L("Ｄ"),_L("Ｅ"),_L("Ｆ"),_L("Ｇ"),_L("Ｈ"),_L("Ｉ"),_L("Ｊ"),_L("Ｋ"),_L("Ｌ"),_L("Ｍ"),_L("Ｎ"),_L("Ｏ"),_L("Ｐ"),_L("Ｑ"),_L("Ｒ"),_L("Ｓ"),
_L("Ｔ"),_L("Ｕ"),_L("Ｖ"),_L("Ｗ"),_L("Ｘ"),_L("Ｙ"),_L("Ｚ"),_L("〔"),_L("＼"),_L("〕"),_L("︿"),_L("ˍ"),
_L("‘"),_L("ａ"),_L("ｂ"),_L("ｃ"),_L("ｄ"),_L("ｅ"),_L("ｆ"),_L("ｇ"),_L("ｈ"),_L("ｉ"),_L("ｊ"),_L("ｋ"),_L("ｌ"),_L("ｍ"),
_L("ｎ"),_L("ｏ"),_L("ｐ"),_L("ｑ"),_L("ｒ"),_L("ｓ"),_L("ｔ"),_L("ｕ"),_L("ｖ"),_L("ｗ"),_L("ｘ"),_L("ｙ"),_L("ｚ"),_L("｛"),_L("｜"),_L("｝"),_L("～")
};

char *half_char_to_full_char(KeySym xkey)
{
  if (xkey < ' ' || xkey > 127)
    return NULL;
  return _(fullchar[xkey-' ']);
}


void create_win0();
void create_win1(), create_win_gtab(),create_win_pho();
extern Window xwin0, xwin1, xwin_gtab, xwin_pho;


void start_inmd_window()
{

  switch (default_input_method) {
    case 3:
      create_win_pho();
      xim_arr[0].xim_xwin = xwin_pho;
      break;
#if USE_TSIN
    case 6:
      create_win0();
      xim_arr[0].xim_xwin = xwin0;
      break;
#endif
    default:
      create_win_gtab();
      xim_arr[0].xim_xwin = xwin_gtab;
      break;
  }
}


#if USE_XIM
char *lc;

static XIMStyle Styles[] = {
#if 1
        XIMPreeditCallbacks|XIMStatusCallbacks,         //OnTheSpot
        XIMPreeditCallbacks|XIMStatusArea,              //OnTheSpot
        XIMPreeditCallbacks|XIMStatusNothing,           //OnTheSpot
#endif
        XIMPreeditPosition|XIMStatusArea,               //OverTheSpot
        XIMPreeditPosition|XIMStatusNothing,            //OverTheSpot
        XIMPreeditPosition|XIMStatusNone,               //OverTheSpot
#if 1
        XIMPreeditArea|XIMStatusArea,                   //OffTheSpot
        XIMPreeditArea|XIMStatusNothing,                //OffTheSpot
        XIMPreeditArea|XIMStatusNone,                   //OffTheSpot
#endif
        XIMPreeditNothing|XIMStatusNothing,             //Root
        XIMPreeditNothing|XIMStatusNone,                //Root
};
static XIMStyles im_styles;

#if 1
static XIMTriggerKey trigger_keys[] = {
        {XK_space, ControlMask, ControlMask},
        {XK_space, ShiftMask, ShiftMask},
        {XK_space, Mod1Mask, Mod1Mask},   // Alt
        {XK_space, Mod4Mask, Mod4Mask},   // Windows
};
#endif

/* Supported Encodings */
static XIMEncoding chEncodings[] = {
        "COMPOUND_TEXT",
        0
};
static XIMEncodings encodings;

int xim_ForwardEventHandler(IMForwardEventStruct *call_data);

XIMS current_ims;
extern void toggle_im_enabled();


int MyTriggerNotifyHandler(IMTriggerNotifyStruct *call_data)
{
//    dbg("MyTriggerNotifyHandler %d %x\n", call_data->key_index, call_data->event_mask);

    if (call_data->flag == 0) { /* on key */
//        db(g("trigger %d\n", call_data->key_index);
        if ((call_data->key_index == 0 && gcin_im_toggle_keys==Control_Space) ||
            (call_data->key_index == 3 && gcin_im_toggle_keys==Shift_Space) ||
            (call_data->key_index == 6 && gcin_im_toggle_keys==Alt_Space) ||
            (call_data->key_index == 9 && gcin_im_toggle_keys==Windows_Space)
            ) {
            toggle_im_enabled();
        }
        return True;
    } else {
        /* never happens */
        return False;
    }
}

#if 0
void switch_IC_index(int index);
#endif
void CreateIC(IMChangeICStruct *call_data);
void DeleteIC(CARD16 icid);
void SetIC(IMChangeICStruct * call_data);
void GetIC(IMChangeICStruct *call_data);
int xim_gcin_FocusIn(IMChangeFocusStruct *call_data);
int xim_gcin_FocusOut(IMChangeFocusStruct *call_data);

int gcin_ProtoHandler(XIMS ims, IMProtocol *call_data)
{
//  dbg("gcin_ProtoHandler %x ims\n", ims);

  current_ims = ims;

  switch (call_data->major_code) {
  case XIM_OPEN:
#define MAX_CONNECT 20000
    {
      IMOpenStruct *pimopen=(IMOpenStruct *)call_data;

      if(pimopen->connect_id > MAX_CONNECT - 1)
        return True;

#if DEBUG && 0
    dbg("open lang %s  connectid:%d\n", pimopen->lang.name, pimopen->connect_id);
#endif
      return True;
    }
  case XIM_CLOSE:
#if DEBUG && 0
    dbg("XIM_CLOSE\n");
#endif
    return True;
  case XIM_CREATE_IC:
#if DEBUG && 0
     dbg("CREATE_IC\n");
#endif
     CreateIC((IMChangeICStruct *)call_data);
     return True;
  case XIM_DESTROY_IC:
     {
       IMChangeICStruct *pimcha=(IMChangeICStruct *)call_data;
#if DEBUG && 0
       dbg("DESTROY_IC %d\n", pimcha->icid);
#endif
       DeleteIC(pimcha->icid);
     }
     return True;
  case XIM_SET_IC_VALUES:
#if DEBUG && 0
     dbg("SET_IC\n");
#endif
     SetIC((IMChangeICStruct *)call_data);
     return True;
  case XIM_GET_IC_VALUES:
#if DEBUG && 0
     dbg("GET_IC\n");
#endif
     GetIC((IMChangeICStruct *)call_data);
     return True;
  case XIM_FORWARD_EVENT:
#if DEBUG && 0
     dbg("XIM_FORWARD_EVENT\n");
#endif
     return xim_ForwardEventHandler((IMForwardEventStruct *)call_data);
  case XIM_SET_IC_FOCUS:
#if DEBUG && 0
     dbg("XIM_SET_IC_FOCUS\n");
#endif
     return xim_gcin_FocusIn((IMChangeFocusStruct *)call_data);
  case XIM_UNSET_IC_FOCUS:
#if DEBUG && 0
     dbg("XIM_UNSET_IC_FOCUS\n");
#endif
     return xim_gcin_FocusOut((IMChangeFocusStruct *)call_data);
  case XIM_RESET_IC:
#if DEBUG && 0
     dbg("XIM_UNSET_IC_FOCUS\n");
#endif
     return True;
  case XIM_TRIGGER_NOTIFY:
#if DEBUG && 0
     dbg("XIM_TRIGGER_NOTIFY\n");
#endif
     MyTriggerNotifyHandler((IMTriggerNotifyStruct *)call_data);
     return True;
  case XIM_PREEDIT_START_REPLY:
#if DEBUG && 0
     dbg("XIM_PREEDIT_START_REPLY\n");
#endif
     return True;
  case XIM_PREEDIT_CARET_REPLY:
#if DEBUG && 0
     dbg("XIM_PREEDIT_CARET_REPLY\n");
#endif
     return True;
  case XIM_STR_CONVERSION_REPLY:
#if DEBUG && 0
     dbg("XIM_STR_CONVERSION_REPLY\n");
#endif
     return True;
  default:
     printf("Unknown major code.\n");
     break;
  }

  return True;
}


void open_xim()
{
  XIMTriggerKeys triggerKeys;

  im_styles.supported_styles = Styles;
  im_styles.count_styles = sizeof(Styles)/sizeof(Styles[0]);

  triggerKeys.count_keys = sizeof(trigger_keys)/sizeof(trigger_keys[0]);
  triggerKeys.keylist = trigger_keys;

  encodings.count_encodings = sizeof(chEncodings)/sizeof(XIMEncoding) - 1;
  encodings.supported_encodings = chEncodings;

  if ((xim_arr[0].xims = IMOpenIM(dpy,
          IMServerWindow,         xim_arr[0].xim_xwin,        //input window
          IMModifiers,            "Xi18n",        //X11R6 protocol
          IMServerName,           xim_arr[0].xim_server_name, //XIM server name
#if 0
          IMLocale,               xim_arr[0].server_locale,
#else
          IMLocale,               lc,
#endif
          IMServerTransport,      "X/",      //Comm. protocol
          IMInputStyles,          &im_styles,   //faked styles
          IMEncodingList,         &encodings,
          IMProtocolHandler,      gcin_ProtoHandler,
          IMFilterEventMask,      KeyPressMask|KeyReleaseMask,
          IMOnKeysList, &triggerKeys,
//        IMOffKeysList, &triggerKeys,
          NULL)) == NULL) {
          p_err("IMOpenIM '%s' failed. Maybe another XIM server is running.\n",
          xim_arr[0].xim_server_name);
  }
}

#endif // if USE_XIM

void load_tsin_db();
void load_tsin_conf(), load_setttings(), load_tab_pho_file();

void disp_hide_tsin_status_row(), gcb_main(), update_win_kbm_inited();
void change_tsin_line_color(), change_win0_style(), change_tsin_color();
void change_win_gtab_style();
void update_item_active_all();
void destroy_inmd_menu();
void load_gtab_list(gboolean);

static void reload_data()
{
  dbg("reload_data\n");
  load_setttings();
//  load_tsin_db();
  change_win0_style();
  change_win_gtab_style();
//  change_win_pho_style();
  load_tab_pho_file();
  change_tsin_color();
  update_win_kbm_inited();

  destroy_inmd_menu();
  load_gtab_list(TRUE);

  update_item_active_all();
#if USE_GCB
  gcb_main();
#endif
}

void change_tsin_font_size();
void change_gtab_font_size();
void change_pho_font_size();
void change_win_sym_font_size();
void change_win_gtab_style();
extern int win_kbm_on;

static void change_font_size()
{
  load_setttings();
  change_tsin_font_size();
  change_gtab_font_size();
  change_pho_font_size();
  change_win_sym_font_size();
  change_win0_style();
  change_win_gtab_style();
  update_win_kbm_inited();
//  change_win_pho_style();
}

#if UNIX
static int xerror_handler(Display *d, XErrorEvent *eve)
{
  return 0;
}
#endif

#if UNIX
Atom gcin_atom;
#endif
void disp_tray_icon(), toggle_gb_output();

void cb_trad_sim_toggle()
{
  toggle_gb_output();
#if TRAY_ENABLED
  disp_tray_icon();
#endif
}
void execute_message(char *message), show_win_kbm(), hide_win_kbm();
int b_show_win_kbm=0;
void kbm_toggle()
{
  win_kbm_inited = 1;
  b_show_win_kbm^=1;
  if (b_show_win_kbm)
    show_win_kbm();
  else
    hide_win_kbm();
}


void reload_tsin_db();
void do_exit();

void message_cb(char *message)
{
//   dbg("message '%s'\n", message);

   if (!strcmp(message, CHANGE_FONT_SIZE)) {
     change_font_size();
   } else
   if (!strcmp(message, GB_OUTPUT_TOGGLE)) {
     cb_trad_sim_toggle();
     update_item_active_all();
   } else
   if (!strcmp(message, KBM_TOGGLE)) {
     kbm_toggle();
   } else
#if UNIX
   if (strstr(message, "#gcin_message")) {
     execute_message(message);
   } else
#endif
   if (!strcmp(message, RELOAD_TSIN_DB)) {
     reload_tsin_db();
   } else
   if (!strcmp(message, GCIN_EXIT_MESSAGE)) {
     do_exit();
   } else
     reload_data();
}

#if UNIX
static GdkFilterReturn my_gdk_filter(GdkXEvent *xevent,
                                     GdkEvent *event,
                                     gpointer data)
{
   XEvent *xeve = (XEvent *)xevent;
#if 0
   dbg("a zzz %d\n", xeve->type);
#endif

   // only very old WM will enter this
   if (xeve->type == FocusIn || xeve->type == FocusOut) {
#if 0
     dbg("focus %s\n", xeve->type == FocusIn ? "in":"out");
#endif
     return GDK_FILTER_REMOVE;
   }

#if USE_XIM
   if (XFilterEvent(xeve, None) == True)
     return GDK_FILTER_REMOVE;
#endif

   return GDK_FILTER_CONTINUE;
}

void init_atom_property()
{
  gcin_atom = get_gcin_atom(dpy);
  XSetSelectionOwner(dpy, gcin_atom, xim_arr[0].xim_xwin, CurrentTime);
}
#endif


void hide_win0();
void destroy_win0();
void destroy_win1();
void destroy_win_gtab();
void free_pho_mem(),free_tsin(),free_all_IC(), free_gtab(), free_phrase(), destroy_tray_win32();

void do_exit()
{
  dbg("----------------- do_ exit ----------------\n");

  free_pho_mem();
  free_tsin();
#if USE_XIM
  free_all_IC();
#endif
  free_gtab();
  free_phrase();

#if 1
  destroy_win0();
  destroy_win1();
  destroy_win_gtab();
#endif

#if WIN32
  destroy_tray_win32();
#endif
  gtk_main_quit();
}

void sig_do_exit(int sig)
{
  do_exit();
}

char *get_gcin_xim_name();
void load_phrase(), init_TableDir();
void init_tray(), exec_setup_scripts();
#if UNIX
void init_gcin_im_serv(Window win);
#else
void init_gcin_im_serv();
#endif
void gcb_main(), init_tray_win32();

#if WIN32
void init_gcin_program_files();
 #pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#endif
int win32_tray_disabled = 1;


gboolean delayed_start_cb(gpointer data)
{
#if WIN32
  Sleep(200);
#endif

  win32_tray_disabled = 0;

#if TRAY_ENABLED
  if (gcin_status_tray) {
#if UNIX
    if (gcin_win32_icon)
      init_tray_win32();
    else
      init_tray();
#endif
  }
#endif

  dbg("after init_tray\n");

#if USE_GCB
  if (gcb_position)
    gcb_main();

  dbg("after gcb_main\n");
#endif

  return FALSE;
}


int main(int argc, char **argv)
{
#if UNIX
  signal(SIGCHLD, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);

  if (getenv("GCIN_DAEMON")) {
    daemon(1,1);
#if FREEBSD
    setpgid(0, getpid());
#else
    setpgrp();
#endif
  }
#endif

//putenv("GDK_NATIVE_WINDOWS=1");
#if WIN32
#if 1
        typedef BOOL (WINAPI* pImmDisableIME)(DWORD);
        pImmDisableIME pd;
        HMODULE imm32=LoadLibraryA("imm32");
//      printf("****** imm32 %x\n", imm32);
        if (imm32 && (pd=(pImmDisableIME)GetProcAddress(imm32, "ImmDisableIME"))) {
//              puts("imm loaded");
                (*pd)(0);
        }
#endif
  init_gcin_program_files();
  init_gcin_im_serv();
#endif

#if USE_XIM
  char *lc_ctype = getenv("LC_CTYPE");
  char *lc_all = getenv("LC_ALL");
  char *lang = getenv("LANG");

  dbg("gcin get env LC_CTYPE=%s  LC_ALL=%s  LANG=%s\n", lc_ctype, lc_all, lang);

  if (!lc_ctype && lang)
    lc_ctype = lang;

  if (lc_all)
    lc_ctype = lc_all;

  if (!lc_ctype)
    lc_ctype = "zh_TW.Big5";

  char *t = strchr(lc_ctype, '.');
  if (t) {
    int len = t - lc_ctype;
#if MAC_OS || FREEBSD
    lc = strdup(lc_ctype);
    lc[len] = 0;
#else
    lc = g_strndup(lc_ctype, len);
#endif
  }
  else
    lc = lc_ctype;


  char *xim_server_name = get_gcin_xim_name();

  strcpy(xim_arr[0].xim_server_name, xim_server_name);
#endif

#if USE_XIM
  dbg("gcin XIM will use %s as the default encoding\n", lc_ctype);
#endif

  if (argc == 2 && (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version") || !strcmp(argv[1], "-h")) ) {
    p_err(" version %s\n", GCIN_VERSION);
  }

  init_TableDir();
  load_setttings();
  load_gtab_list(TRUE);

  gtk_init (&argc, &argv);

#if GCIN_i18n_message
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);
#endif

  dbg("after gtk_init\n");

#if UNIX
  dpy = GDK_DISPLAY();
  root=DefaultRootWindow(dpy);
#endif
  dpy_xl = gdk_screen_width(), dpy_yl = gdk_screen_height();

  dbg("display width:%d height:%d\n", dpy_xl, dpy_yl);

#if UNIX
  start_inmd_window();
#endif

#if USE_XIM
  open_xim();
#endif

#if UNIX
  gdk_window_add_filter(NULL, my_gdk_filter, NULL);

  init_atom_property();
  signal(SIGINT, sig_do_exit);
  signal(SIGHUP, sig_do_exit);
  // disable the io handler abort
  // void *olderr =
    XSetErrorHandler((XErrorHandler)xerror_handler);
#endif

#if UNIX
  init_gcin_im_serv(xim_arr[0].xim_xwin);
#endif

  exec_setup_scripts();

#if UNIX
  g_timeout_add(5000, delayed_start_cb, NULL);
#else
  delayed_start_cb(NULL);
#endif

  dbg("before gtk_main\n");
  gtk_main();

  return 0;
}
