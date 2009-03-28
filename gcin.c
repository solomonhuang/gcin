#include <signal.h>
// #include <openssl/ssl.h>
#include "gcin.h"
#include "config.h"
#include <libintl.h>

Display *dpy;
Window root;
int win_xl, win_yl;
int win_x, win_y;   // actual win x/y
int dpy_xl, dpy_yl;

DUAL_XIM_ENTRY xim_arr[1];

#if USE_XIM
DUAL_XIM_ENTRY *pxim_arr;
#endif

gboolean win_kbm_inited;

char *fullchar[]=
{"　","！","”","＃","＄","％","＆","’","（","）","＊","＋",
"，","－","．","／","０","１","２","３","４","５","６","７","８","９","：","；","＜","＝","＞","？",
"＠","Ａ","Ｂ","Ｃ","Ｄ","Ｅ","Ｆ","Ｇ","Ｈ","Ｉ","Ｊ","Ｋ","Ｌ","Ｍ","Ｎ","Ｏ","Ｐ","Ｑ","Ｒ","Ｓ",
"Ｔ","Ｕ","Ｖ","Ｗ","Ｘ","Ｙ","Ｚ","〔","＼","〕","︿","ˍ",
"‘","ａ","ｂ","ｃ","ｄ","ｅ","ｆ","ｇ","ｈ","ｉ","ｊ","ｋ","ｌ","ｍ",
"ｎ","ｏ","ｐ","ｑ","ｒ","ｓ","ｔ","ｕ","ｖ","ｗ","ｘ","ｙ","ｚ","｛","｜","｝","∼"
};

char *half_char_to_full_char(KeySym xkey)
{
  if (xkey < ' ' || xkey > 127)
    return NULL;
  return fullchar[xkey-' '];
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
static XIMStyle Styles[] = {
#if 0
        XIMPreeditCallbacks|XIMStatusCallbacks,		//OnTheSpot
        XIMPreeditCallbacks|XIMStatusArea,		//OnTheSpot
        XIMPreeditCallbacks|XIMStatusNothing,		//OnTheSpot
#endif
        XIMPreeditPosition|XIMStatusArea,		//OverTheSpot
        XIMPreeditPosition|XIMStatusNothing,		//OverTheSpot
        XIMPreeditPosition|XIMStatusNone,		//OverTheSpot
#if 0
        XIMPreeditArea|XIMStatusArea,			//OffTheSpot
        XIMPreeditArea|XIMStatusNothing,		//OffTheSpot
        XIMPreeditArea|XIMStatusNone,			//OffTheSpot
#endif
        XIMPreeditNothing|XIMStatusNothing,		//Root
        XIMPreeditNothing|XIMStatusNone,		//Root
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
            toggle_im_enabled(0);
        }
	return True;
    } else {
	/* never happens */
	return False;
    }
}

void switch_IC_index(int index);
void CreateIC(IMChangeICStruct *call_data);
void DeleteIC(CARD16 icid);
void SetIC(IMChangeICStruct * call_data);
void GetIC(IMChangeICStruct *call_data);
int xim_gcin_FocusIn(IMChangeFocusStruct *call_data);
int xim_gcin_FocusOut(IMChangeFocusStruct *call_data);


#define DEBUG 0

int gcin_ProtoHandler(XIMS ims, IMProtocol *call_data)
{
//  dbg("gcin_ProtoHandler %x ims\n", ims);
  int index=0;

  pxim_arr = &xim_arr[index];
  switch_IC_index(index);

//  dbg("index:%d\n", index);

  current_ims = ims;

  switch (call_data->major_code) {
  case XIM_OPEN:
#define MAX_CONNECT 20000
    {
      IMOpenStruct *pimopen=(IMOpenStruct *)call_data;

      if(pimopen->connect_id > MAX_CONNECT - 1)
        return True;

#if DEBUG
    dbg("open lang %s  connectid:%d\n", pimopen->lang.name, pimopen->connect_id);
#endif
      return True;
    }
  case XIM_CLOSE:
#if DEBUG
    dbg("XIM_CLOSE\n");
#endif
    return True;
  case XIM_CREATE_IC:
#if DEBUG
     dbg("CREATE_IC\n");
#endif
     CreateIC((IMChangeICStruct *)call_data);
     return True;
  case XIM_DESTROY_IC:
     {
       IMChangeICStruct *pimcha=(IMChangeICStruct *)call_data;
#if DEBUG
       dbg("DESTROY_IC %d\n", pimcha->icid);
#endif
       DeleteIC(pimcha->icid);
     }
     return True;
  case XIM_SET_IC_VALUES:
#if DEBUG
     dbg("SET_IC\n");
#endif
     SetIC((IMChangeICStruct *)call_data);
     return True;
  case XIM_GET_IC_VALUES:
#if DEBUG
     dbg("GET_IC\n");
#endif
     GetIC((IMChangeICStruct *)call_data);
     return True;
  case XIM_FORWARD_EVENT:
#if DEBUG
     dbg("XIM_FORWARD_EVENT\n");
#endif
     return xim_ForwardEventHandler((IMForwardEventStruct *)call_data);
  case XIM_SET_IC_FOCUS:
#if DEBUG
     dbg("XIM_SET_IC_FOCUS\n");
#endif
     return xim_gcin_FocusIn((IMChangeFocusStruct *)call_data);
  case XIM_UNSET_IC_FOCUS:
#if DEBUG
     dbg("XIM_UNSET_IC_FOCUS\n");
#endif
     return xim_gcin_FocusOut((IMChangeFocusStruct *)call_data);
  case XIM_RESET_IC:
#if DEBUG
     dbg("XIM_UNSET_IC_FOCUS\n");
#endif
     return True;
  case XIM_TRIGGER_NOTIFY:
#if DEBUG
     dbg("XIM_TRIGGER_NOTIFY\n");
#endif
     MyTriggerNotifyHandler((IMTriggerNotifyStruct *)call_data);
     return True;
  case XIM_PREEDIT_START_REPLY:
#if DEBUG
     dbg("XIM_PREEDIT_START_REPLY\n");
#endif
     return True;
  case XIM_PREEDIT_CARET_REPLY:
#if DEBUG
     dbg("XIM_PREEDIT_CARET_REPLY\n");
#endif
     return True;
  case XIM_STR_CONVERSION_REPLY:
#if DEBUG
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
          IMLocale,               xim_arr[0].server_locale,  //XIM server locale
          IMServerTransport,      "X/",      //Comm. protocol
          IMInputStyles,          &im_styles,   //faked styles
          IMEncodingList,         &encodings,
          IMProtocolHandler,      gcin_ProtoHandler,
          IMFilterEventMask,      KeyPressMask,
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

void disp_hide_tsin_status_row();
void change_tsin_line_color();

static void reload_data()
{
  dbg("reload_data\n");
  load_setttings();
  load_tsin_db();
  change_win0_style();
  change_win_gtab_style();
  change_win_pho_style();
  load_tab_pho_file();
  change_tsin_color();
  if (win_kbm_inited)
    update_win_kbm();
}

void change_tsin_font_size();
void change_gtab_font_size();
void change_pho_font_size();
void change_win_sym_font_size();


static void change_font_size()
{
  load_setttings();
  change_tsin_font_size();
  change_gtab_font_size();
  change_pho_font_size();
  change_win_sym_font_size();
  change_win0_style();
  change_win_gtab_style();
  change_win_pho_style();
}

static int xerror_handler(Display *d, XErrorEvent *eve)
{
  return 0;
}


Atom gcin_atom;
void cb_trad_sim_toggle()
{
  toggle_gb_output();
#if TRAY_ENABLED
  update_tray_icon();
#endif
}
void execute_message(char *message);
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

static GdkFilterReturn my_gdk_filter(GdkXEvent *xevent,
                                     GdkEvent *event,
                                     gpointer data)
{
   XEvent *xeve = (XEvent *)xevent;
#if 0
   dbg("a zzz %d\n", xeve->type);
#endif
   if (xeve->type == PropertyNotify) {
     XPropertyEvent *xprop = &xeve->xproperty;

     if (xprop->atom == gcin_atom) {
       Atom actual_type;
       int actual_format;
       unsigned long nitems,bytes_after;
       char *message;

       if (XGetWindowProperty(dpy, xprop->window, gcin_atom, 0, 64,
          False, AnyPropertyType, &actual_type, &actual_format,
          &nitems,&bytes_after,(u_char **)&message) != Success) {
          dbg("err prop");
          return GDK_FILTER_REMOVE;
       }

       dbg("message '%s'\n", message);

       if (!strcmp(message, CHANGE_FONT_SIZE)) {
         change_font_size();
       } else
       if (!strcmp(message, GB_OUTPUT_TOGGLE)) {
         cb_trad_sim_toggle();
       } else
       if (!strcmp(message, KBM_TOGGLE)) {
         kbm_toggle();
       } else
       if (strstr(message, "#gcin_message")) {
         execute_message(message);
       }
       else
         reload_data();

       XFree(message);
       return GDK_FILTER_REMOVE;
     }
   }

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



void hide_win0();
void do_exit();
void destroy_win0();
void destroy_win1();
void destroy_win_gtab();
void free_pho_mem(),free_tsin(),free_all_IC(), free_gtab(), free_phrase();

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

  gtk_main_quit();
}

char *get_gcin_xim_name();
void load_phrase(), init_TableDir(),  load_gtab_list();
void init_im_serv(), init_tray();
void init_gcin_im_serv(Window win);

int main(int argc, char **argv)
{

#if GCIN_i18n_message
  gtk_set_locale();
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
  textdomain(GETTEXT_PACKAGE);
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

  char *lc;

  signal(SIGCHLD, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);

  char *t = strchr(lc_ctype, '.');
  if (t) {
    int len = t - lc_ctype;
#if MAC_OS || FREEBSD
    lc = strdup(lc_ctype);
    lc[len] = 0;
#else
    lc = strndup(lc_ctype, len);
#endif
  }
  else
    lc = lc_ctype;

  xim_arr[0].server_locale = lc;
  char *xim_server_name = get_gcin_xim_name();

  strcpy(xim_arr[0].xim_server_name, xim_server_name);

#endif

#if USE_XIM
  dbg("gcin XIM will use %s as the default encoding\n", lc_ctype);
#endif

  if (argc == 2 && (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version") || !strcmp(argv[1], "-h")) ) {
    p_err(" version %s\n", GCIN_VERSION);
  }

  exec_setup_scripts();

  init_TableDir();
  load_setttings();
  load_gtab_list();

  gtk_init (&argc, &argv);
  dpy = GDK_DISPLAY();

  dpy_xl = gdk_screen_width(), dpy_yl = gdk_screen_height();

  root=DefaultRootWindow(dpy);
  start_inmd_window();
#if USE_XIM && 1
  open_xim();
#endif
  init_atom_property();

  gdk_window_add_filter(NULL, my_gdk_filter, NULL);

  signal(SIGINT, do_exit);
  signal(SIGHUP, do_exit);

  // disable the io handler abort
  // void *olderr =
    XSetErrorHandler((XErrorHandler)xerror_handler);

#if 1
  init_gcin_im_serv(xim_arr[0].xim_xwin);
#endif

#if TRAY_ENABLED
  if (gcin_status_tray) {
    init_tray();
  }
#endif


  gtk_main();

  return 0;
}
