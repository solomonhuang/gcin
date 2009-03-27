#include "gcin.h"
#include <signal.h>

Display *dpy;
Window root;
static Window xim_xwin;
int initial_inmd = 6;
int win_xl, win_yl;
int win_x, win_y;
int dpy_xl, dpy_yl;
int gcin_font_size=14;

u_char fullchar[]=
"¡@¡I¡¨¡­¢C¢H¡®¡¦¡]¡^¡¯¡Ï¡A¡Ð¡D¡þ¢¯¢°¢±¢²¢³¢´¢µ¢¶¢·¢¸¡G¡F¡Õ¡×¡Ö¡H"
"¢I¢Ï¢Ð¢Ñ¢Ò¢Ó¢Ô¢Õ¢Ö¢×¢Ø¢Ù¢Ú¢Û¢Ü¢Ý¢Þ¢ß¢à¢á¢â¢ã¢ä¢å¢æ¢ç¢è¡e¢@¡f¡s¡Å"
"¡¥¢é¢ê¢ë¢ì¢í¢î¢ï¢ð¢ñ¢ò¢ó¢ô¢õ¢ö¢÷¢ø¢ù¢ú¢û¢ü¢ý¢þ£@£A£B£C¡a¡U¡b¡ã  ";


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
};
#endif

/* Supported Encodings */
static XIMEncoding chEncodings[] = {
        "COMPOUND_TEXT",
};
static XIMEncodings encodings;
static XIMS xims;

int gcin_ForwardEventHandler(IMForwardEventStruct *call_data);

XIMS current_ims;
IMProtocol *current_call_data;
extern void toggle_im_enabled();
extern void toggle_half_full_char();

MyTriggerNotifyHandler(IMTriggerNotifyStruct *call_data)
{
//    dbg("MyTriggerNotifyHandler %d %x\n", call_data->key_index, call_data->event_mask);

    if (call_data->flag == 0) { /* on key */
        if (call_data->key_index == 0) {  // dirty solution
            toggle_im_enabled();
        } else
        if (call_data->key_index == 3) { // dirty solution
            dbg("halt full char switch\n");
        }
	return True;
    } else {
	/* never happens */
	return False;
    }
}



int gcin_ProtoHandler(XIMS ims, IMProtocol *call_data)
{
//  dbg("gcin_ProtoHandler %x ims  %x\n", xims);
  current_ims = ims;
  current_call_data = call_data;

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

  char *get_gcin_xim_name();
  char *xim_server_name = get_gcin_xim_name();

  if ((xims = IMOpenIM(dpy,
          IMServerWindow,         xim_xwin,        //input window
          IMModifiers,            "Xi18n",        //X11R6 protocol
          IMServerName,           xim_server_name,         //XIM server name
          IMLocale,               "zh_TW",       //XIM server locale
          IMServerTransport,      "X/",      //Comm. protocol
          IMInputStyles,          &im_styles,   //faked styles
          IMEncodingList,         &encodings,
          IMProtocolHandler,      gcin_ProtoHandler,
          IMFilterEventMask,      KeyPressMask,
	  IMOnKeysList, &triggerKeys,
//	  IMOffKeysList, &triggerKeys,
          NULL)) == NULL) {
          p_err("IMOpenIM failed. Maybe another XIM server is running.\n");
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
  XSetSelectionOwner(dpy, gcin_atom, xwin0, CurrentTime);
}



void create_win0();
void hide_win0();
void do_exit();

void do_exit()
{
  dbg("----------------- do_ exit ----------------\n");

  free_pho_mem();
  free_tsin();
  free_all_IC();

#if 1
  destory_win0();
  destory_win1();
#endif

  gtk_main_quit();
}

void start_inmd_window()
{
  extern Window xwin0;

  switch (initial_inmd) {
    case 6:
      create_win0();
      xim_xwin = xwin0;
      break;
  }
}


main(int argc, char **argv)
{
  setlocale(LC_ALL, "zh_TW.Big5");

  load_setttings();

  gtk_init (&argc, &argv);
  dpy = GDK_DISPLAY();

  dpy_xl = gdk_screen_width(), dpy_yl = gdk_screen_height();

  root=DefaultRootWindow(dpy);
  start_inmd_window();

  open_xim();

  init_atom_property();

  gdk_window_add_filter(NULL, my_gdk_filter, NULL);

  signal(SIGINT, do_exit);
#if 1
  // disable the io handler abort
  void *olderr = XSetErrorHandler((XErrorHandler)xerror_handler);
#endif
  gtk_main();

  return 0;
}
