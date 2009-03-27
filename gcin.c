#include "gcin.h"
#include <signal.h>

Display *dpy;
Window root;
int win_xl, win_yl;
int win_x, win_y;
int dpy_xl, dpy_yl;
gboolean dual_xim=TRUE;

DUAL_XIM_ENTRY xim_arr[2];
DUAL_XIM_ENTRY *pxim_arr;


u_char fullchar[]=
"¡@¡I¡¨¡­¢C¢H¡®¡¦¡]¡^¡¯¡Ï¡A¡Ð¡D¡þ¢¯¢°¢±¢²¢³¢´¢µ¢¶¢·¢¸¡G¡F¡Õ¡×¡Ö¡H"
"¢I¢Ï¢Ð¢Ñ¢Ò¢Ó¢Ô¢Õ¢Ö¢×¢Ø¢Ù¢Ú¢Û¢Ü¢Ý¢Þ¢ß¢à¢á¢â¢ã¢ä¢å¢æ¢ç¢è¡e¢@¡f¡s¡Å"
"¡¥¢é¢ê¢ë¢ì¢í¢î¢ï¢ð¢ñ¢ò¢ó¢ô¢õ¢ö¢÷¢ø¢ù¢ú¢û¢ü¢ý¢þ£@£A£B£C¡a¡U¡b¡ã  ";

char *half_char_to_full_char(KeySym xkey)
{
  if (xkey < ' ' || xkey > 127)
    return NULL;
  return &fullchar[(xkey-' ') * CH_SZ];
}


void create_win0();
extern Window xwin0, xwin1, xwin_gtab, xwin_pho;
void start_inmd_window()
{

  switch (default_input_method) {
    case 6:
      create_win0();

      if (dual_xim)
        create_win1();

      xim_arr[0].xim_xwin = xwin0;
      xim_arr[1].xim_xwin = xwin1;
      break;
    default:
      create_win_gtab();

      if (dual_xim)
        create_win_pho();

      xim_arr[0].xim_xwin = xwin_gtab;
      xim_arr[1].xim_xwin = xwin_pho;
      break;
  }
}


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
};
static XIMEncodings encodings;

int gcin_ForwardEventHandler(IMForwardEventStruct *call_data);

XIMS current_ims;
IMProtocol *current_call_data;
extern void toggle_im_enabled();


MyTriggerNotifyHandler(IMTriggerNotifyStruct *call_data)
{
//    dbg("MyTriggerNotifyHandler %d %x\n", call_data->key_index, call_data->event_mask);

    if (call_data->flag == 0) { /* on key */
//        dbg("trigger %d\n", call_data->key_index);
        if (call_data->key_index == 0 && gcin_im_toggle_keys==Control_Space ||
            call_data->key_index == 3 && gcin_im_toggle_keys==Shift_Space ||
            call_data->key_index == 6 && gcin_im_toggle_keys==Alt_Space ||
            call_data->key_index == 9 && gcin_im_toggle_keys==Windows_Space
            ) {
            toggle_im_enabled();
        }
	return True;
    } else {
	/* never happens */
	return False;
    }
}


int gcin_ProtoHandler(XIMS ims, IMProtocol *call_data)
{
//  dbg("gcin_ProtoHandler %x ims\n", ims);
  int index;

  if (ims == xim_arr[0].xims)
    index = 0;
  else
  if (ims == xim_arr[1].xims)
    index = 1;
  else
     p_err("bad ims %x\n", ims);

  pxim_arr = &xim_arr[index];
  switch_IC_index(index);

//  dbg("index:%d\n", index);

  current_ims = ims;
  current_call_data = call_data;

  switch (call_data->major_code) {
  case XIM_OPEN:
#define MAX_CONNECT 20000
    {
      IMOpenStruct *pimopen=(IMOpenStruct *)call_data;
      if(pimopen->connect_id > MAX_CONNECT - 1)
        return True;
#if 1
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
     return gcin_ForwardEventHandler((IMForwardEventStruct *)call_data);
  case XIM_SET_IC_FOCUS:
#if DEBUG
     dbg("XIM_SET_IC_FOCUS\n");
#endif
     return gcin_FocusIn((IMChangeFocusStruct *)call_data);
  case XIM_UNSET_IC_FOCUS:
#if DEBUG
     dbg("XIM_UNSET_IC_FOCUS\n");
#endif
     return gcin_FocusOut((IMChangeFocusStruct *)call_data);
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
}


void open_xim()
{
  XIMTriggerKeys triggerKeys;

  im_styles.supported_styles = Styles;
  im_styles.count_styles = sizeof(Styles)/sizeof(Styles[0]);

  triggerKeys.count_keys = sizeof(trigger_keys)/sizeof(trigger_keys[0]);
  triggerKeys.keylist = trigger_keys;

  encodings.count_encodings = sizeof(chEncodings)/sizeof(XIMEncoding);
  encodings.supported_encodings = chEncodings;


  int dualN = dual_xim ? 2 : 1;
  int i;

  for(i=0; i < dualN; i++) {
    if ((xim_arr[i].xims = IMOpenIM(dpy,
            IMServerWindow,         xim_arr[i].xim_xwin,        //input window
            IMModifiers,            "Xi18n",        //X11R6 protocol
            IMServerName,           xim_arr[i].xim_server_name, //XIM server name
            IMLocale,               xim_arr[i].server_locale,  //XIM server locale
            IMServerTransport,      "X/",      //Comm. protocol
            IMInputStyles,          &im_styles,   //faked styles
            IMEncodingList,         &encodings,
            IMProtocolHandler,      gcin_ProtoHandler,
            IMFilterEventMask,      KeyPressMask,
            IMOnKeysList, &triggerKeys,
  //        IMOffKeysList, &triggerKeys,
            NULL)) == NULL) {
            p_err("IMOpenIM failed. Maybe another XIM server is running.\n");
    }
  }
}


void load_tsin_db();
void load_tsin_conf();

static void reload_data()
{
  dbg("reload_data\n");
  load_tsin_conf();
  load_tsin_db();
  load_tab_pho_file();
  load_setttings();
}

void change_tsin_font_size();


static void change_font_size()
{
  load_setttings();
  change_tsin_font_size();
}

static int xerror_handler(Display *d, XErrorEvent *eve)
{
  return 0;
}

Atom gcin_atom;

static GdkFilterReturn my_gdk_filter(GdkXEvent *xevent,
                                     GdkEvent *event,
                                     gpointer data)
{
   XEvent *xeve = (XEvent *)xevent;

   if (xeve->type == PropertyNotify) {
     XPropertyEvent *xprop = &xeve->xproperty;

     if (xprop->atom == gcin_atom) {
       Atom actual_type;
       int actual_format;
       u_long nitems,bytes_after;
       char *message;

       if (XGetWindowProperty(dpy, xprop->window, gcin_atom, 0, 64,
          False, AnyPropertyType, &actual_type, &actual_format,
          &nitems,&bytes_after,(u_char **)&message) != Success) {
          dbg("err prop");
          return;
       }

       dbg("message '%s'\n", message);

       if (!strcmp(message, CHANGE_FONT_SIZE)) {
         change_font_size();
       } else
         reload_data();

       XFree(message);
       return GDK_FILTER_REMOVE;
     }
   }

   if (XFilterEvent(xeve, None) == True)
     return GDK_FILTER_REMOVE;

   return GDK_FILTER_CONTINUE;
}

void init_atom_property()
{
  gcin_atom = get_gcin_atom(dpy);
  XSetSelectionOwner(dpy, gcin_atom, xim_arr[0].xim_xwin, CurrentTime);
}



void hide_win0();
void do_exit();

void do_exit()
{
  dbg("----------------- do_ exit ----------------\n");

  free_pho_mem();
  free_tsin();
  free_all_IC();
  free_gtab();
  free_phrase();

#if 1
  destory_win0();
  destory_win1();
  destroy_win_gtab();
#endif

  gtk_main_quit();
}


static exec_script(char *name)
{
  char scr[128];

  sprintf(scr, GCIN_SCRIPT_DIR"/%s", name);
  system(scr);
}

static void exec_setup_scripts()
{
  exec_script("gcin-utf8-setup");
  exec_script("gcin-user-setup "GCIN_TABLE_DIR);
}


static char *gcin_db_locale="zh_TW.Big5";
char *get_gcin_xim_name();

main(int argc, char **argv)
{
  dual_xim = getenv("GCIN_DUAL_XIM_OFF") == NULL;

  char *locale_str;
  xim_arr[0].server_locale = "zh_TW";
  char *xim_server_name = get_gcin_xim_name();

  strcpy(xim_arr[0].xim_server_name, xim_server_name);
  strcpy(xim_arr[1].xim_server_name, xim_server_name);
  if ((locale_str=getenv("LC_ALL")) && !strcmp(locale_str, "zh_TW.UTF-8")) {
    dbg("LC_ALL=%s, gcin will use UTF-8\n", locale_str);

    xim_arr[0].b_send_utf8_str = TRUE;
    xim_arr[1].b_send_utf8_str = FALSE;
    xim_arr[1].server_locale = "zh_TW.Big5";
    strcat(xim_arr[1].xim_server_name, ".Big5");
  } else {
    xim_arr[0].b_send_utf8_str = FALSE;
    xim_arr[1].b_send_utf8_str = TRUE;
    xim_arr[1].server_locale = "zh_TW.UTF-8";
    strcat(xim_arr[1].xim_server_name, ".UTF-8");
  }


  if (argc == 2 && (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version"))) {
    p_err(" version %s\n", GCIN_VERSION);
  }

  setlocale(LC_ALL, gcin_db_locale);
  setenv("LC_ALL", gcin_db_locale, TRUE);

  exec_setup_scripts();

  init_TableDir();
  load_setttings();
  load_gtab_list();
  load_phrase();

  gtk_init (&argc, &argv);
  dpy = GDK_DISPLAY();

  dpy_xl = gdk_screen_width(), dpy_yl = gdk_screen_height();

  root=DefaultRootWindow(dpy);
  start_inmd_window();

  open_xim();

  init_atom_property();

  gdk_window_add_filter(NULL, my_gdk_filter, NULL);

  signal(SIGINT, do_exit);

  // disable the io handler abort
  void *olderr = XSetErrorHandler((XErrorHandler)xerror_handler);

  gtk_main();

  return 0;
}
