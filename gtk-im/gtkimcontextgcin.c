/* GTK - The GIMP Toolkit
 * Copyright (C) 2000 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include "locale.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "gtkintl.h"
#include "gtk/gtklabel.h"
#include "gtk/gtksignal.h"
#include "gtk/gtkwindow.h"
#include "gtkimcontextgcin.h"
// #include "gcin.h"  // for debug only
#include "gcin-im-client.h"
#include <X11/keysym.h>

// #define NEW_GTK_IM 1
#define DBG 0

typedef struct _GtkGCINInfo GtkGCINInfo;

struct _GtkIMContextGCIN
{
  GtkIMContext object;

  GdkWindow *client_window;
  GtkWidget *client_widget;
  GCIN_client_handle *gcin_ch;
  int timeout_handle;
  gboolean is_mozilla, dirty_fix_off;
};


#if NEW_GTK_IM
static void cancel_timeout(GtkIMContextGCIN *context)
{
#if DBG
  printf("cancel_timeout %d\n", context->timeout_handle);
#endif
  if (!context->timeout_handle)
    return;
  g_source_remove(context->timeout_handle);
  context->timeout_handle = 0;
}
#endif

static void     gtk_im_context_gcin_class_init         (GtkIMContextGCINClass  *class);
static void     gtk_im_context_gcin_init               (GtkIMContextGCIN       *im_context_gcin);
static void     gtk_im_context_gcin_finalize           (GObject               *obj);
static void     gtk_im_context_gcin_set_client_window  (GtkIMContext          *context,
						       GdkWindow             *client_window);
static gboolean gtk_im_context_gcin_filter_keypress    (GtkIMContext          *context,
						       GdkEventKey           *key);
static void     gtk_im_context_gcin_reset              (GtkIMContext          *context);
static void     gtk_im_context_gcin_focus_in           (GtkIMContext          *context);
static void     gtk_im_context_gcin_focus_out          (GtkIMContext          *context);
static void     gtk_im_context_gcin_set_cursor_location (GtkIMContext          *context,
						       GdkRectangle		*area);
static void     gtk_im_context_gcin_set_use_preedit    (GtkIMContext          *context,
						       gboolean		      use_preedit);
static void     gtk_im_context_gcin_get_preedit_string (GtkIMContext          *context,
						       gchar                **str,
						       PangoAttrList        **attrs,
						       gint                  *cursor_pos);

// static GObjectClass *parent_class;

GType gtk_type_im_context_gcin = 0;

void
gtk_im_context_gcin_register_type (GTypeModule *type_module)
{
  static const GTypeInfo im_context_gcin_info =
  {
    sizeof (GtkIMContextGCINClass),
    (GBaseInitFunc) NULL,
    (GBaseFinalizeFunc) NULL,
    (GClassInitFunc) gtk_im_context_gcin_class_init,
    NULL,           /* class_finalize */
    NULL,           /* class_data */
    sizeof (GtkIMContextGCIN),
    0,
    (GInstanceInitFunc) gtk_im_context_gcin_init,
  };

  gtk_type_im_context_gcin =
    g_type_module_register_type (type_module,
				 GTK_TYPE_IM_CONTEXT,
				 "GtkIMContextGCIN",
				 &im_context_gcin_info, 0);
}

static void
reinitialize_all_ics (GtkGCINInfo *info)
{
#if DBG
  puts("reinitialize_all_ics");
#endif
}

#if 0
static void gcin_display_closed (GdkDisplay *display,
			 gboolean    is_error,
                         GtkIMContextGCIN *context_xim)
{
#if DBG
  puts("gcin_display_closed");
#endif
#if NEW_GTK_IM
  cancel_timeout(context_xim);
#endif
  if (!context_xim->gcin_ch)
    return;

  gcin_im_client_close(context_xim->gcin_ch);
  context_xim->gcin_ch = NULL;
}
#endif

static void
preedit_style_change (GtkGCINInfo *info)
{
//  dbg("preedit_style_change\n");
}


static void
get_im (GtkIMContextGCIN *context_xim)
{
  GdkWindow *client_window = context_xim->client_window;
  GdkScreen *screen = gdk_drawable_get_screen (client_window);
  GdkDisplay *display = gdk_screen_get_display (screen);


  if (!context_xim->gcin_ch) {
    if (!(context_xim->gcin_ch = gcin_im_client_open(GDK_DISPLAY())))
      perror("cannot open gcin_ch");
#if 0
    // coredump
    g_signal_connect (display, "closed",
                      G_CALLBACK (gcin_display_closed), context_xim);
#endif
    if (context_xim->is_mozilla) {
      int rflag;
      gcin_im_client_set_flags(context_xim->gcin_ch,
        FLAG_GCIN_client_handle_raise_window, &rflag);

      context_xim->dirty_fix_off = (rflag & FLAG_GCIN_srv_ret_status_use_pop_up) > 0;
    }
  }
}

///
static void
gtk_im_context_gcin_class_init (GtkIMContextGCINClass *class)
{
  GtkIMContextClass *im_context_class = GTK_IM_CONTEXT_CLASS (class);
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

//  parent_class = g_type_class_peek_parent (class);
  im_context_class->set_client_window = gtk_im_context_gcin_set_client_window;
  im_context_class->filter_keypress = gtk_im_context_gcin_filter_keypress;
  im_context_class->reset = gtk_im_context_gcin_reset;
  im_context_class->get_preedit_string = gtk_im_context_gcin_get_preedit_string;
  im_context_class->focus_in = gtk_im_context_gcin_focus_in;
  im_context_class->focus_out = gtk_im_context_gcin_focus_out;
  im_context_class->set_cursor_location = gtk_im_context_gcin_set_cursor_location;
  im_context_class->set_use_preedit = gtk_im_context_gcin_set_use_preedit;
  gobject_class->finalize = gtk_im_context_gcin_finalize;
}

static void
gtk_im_context_gcin_init (GtkIMContextGCIN *im_context_gcin)
{
  im_context_gcin->timeout_handle = 0;
#if DBG
  printf("gtk_im_context_gcin_init %x\n", im_context_gcin);
#endif
#if NEW_GTK_IM
  int pid = getpid();
// probably only works for linux
  static char *moz[]={"mozilla", "firefox", "thunderbird", "nvu", "sunbird",
	"seamonkey", "gnuzilla", "iceweasel", "icedove", "iceape", "swiftfox",
	"iceowl", "kompozer", "swiftdove", "swiftweasel", "navigator"};
  char tstr0[64];
  char exec[256];
  sprintf(tstr0, "/proc/%d/exe", pid);
  int n;
  if ((n=readlink(tstr0, exec, sizeof(exec))) > 0) {
    exec[n]=0;
    int i;
    for(i=0; i < sizeof(moz)/sizeof(moz[0]); i++) {
      if (!strstr(exec, moz[i]))
        continue;
      im_context_gcin->is_mozilla = TRUE;
      break;
    }
  }
#endif
// dirty hack for mozilla...
}

static void
gtk_im_context_gcin_finalize (GObject *obj)
{
#if DBG
  printf("gtk_im_context_gcin_finalize %x\n", obj);
#endif
  GtkIMContextGCIN *context_xim = GTK_IM_CONTEXT_GCIN (obj);

#if NEW_GTK_IM
  cancel_timeout(context_xim);
#endif

  if (context_xim->gcin_ch) {
    gcin_im_client_close(context_xim->gcin_ch);
    context_xim->gcin_ch = NULL;
  }
}

/* Finds the GtkWidget that owns the window, or if none, the
 * widget owning the nearest parent that has a widget.
 */

static GtkWidget *
widget_for_window (GdkWindow *window)
{
  while (window)
    {
      gpointer user_data;
      gdk_window_get_user_data (window, &user_data);
      if (user_data)
	return user_data;

      window = gdk_window_get_parent (window);
    }

  return NULL;
}


#if 0
static void update_in_toplevel (GtkIMContextGCIN *context_xim)
{
}
static void
on_client_widget_hierarchy_changed (GtkWidget       *widget,
				    GtkWidget       *old_toplevel,
				    GtkIMContextGCIN *context_xim)
{
  update_in_toplevel (context_xim);
}
#endif

static void
set_ic_client_window (GtkIMContextGCIN *context_xim,
                      GdkWindow       *client_window)
{
#if DBG
  printf("set_ic_client_window %x %x\n", context_xim, client_window);
#endif
  if (!client_window)
    return;

  context_xim->client_window = client_window;

  if (context_xim->client_window)
    {
      get_im (context_xim);
      if (context_xim->gcin_ch) {
        gcin_im_client_set_window(context_xim->gcin_ch, GDK_DRAWABLE_XID(client_window));
      }
    }

#if 0
  // cause coredump
  GtkWidget *new_client_widget = widget_for_window (context_xim->client_window);

  if (new_client_widget != context_xim->client_widget)
    {
      if (context_xim->client_widget)
	{
	  g_signal_handlers_disconnect_by_func (context_xim->client_widget,
						G_CALLBACK (on_client_widget_hierarchy_changed),
						context_xim);
	}
      context_xim->client_widget = new_client_widget;
      if (context_xim->client_widget)
	{
	  g_signal_connect (context_xim->client_widget, "hierarchy-changed",
			    G_CALLBACK (on_client_widget_hierarchy_changed),
			    context_xim);
	}

      update_in_toplevel (context_xim);
    }
#endif
}

#if NEW_GTK_IM
static gboolean update_cursor_position(gpointer data)
{
  GtkIMContextGCIN *context = (GtkIMContextGCIN *)data;

  g_signal_emit_by_name(context, "preedit_changed");
  cancel_timeout(context);
  return FALSE;
}
#endif


#if NEW_GTK_IM
void add_cursor_timeout(GtkIMContextGCIN *context_xim)
{
  if (context_xim->timeout_handle)
    return;
  context_xim->timeout_handle = g_timeout_add(200, update_cursor_position, (gpointer)context_xim);
}
#endif

///
static void
gtk_im_context_gcin_set_client_window (GtkIMContext          *context,
				      GdkWindow             *client_window)
{
//  printf("gtk_im_context_gcin_set_client_window\n");

  GtkIMContextGCIN *context_xim = GTK_IM_CONTEXT_GCIN (context);
#if NEW_GTK_IM
  if (context_xim->is_mozilla && !context_xim->dirty_fix_off)
    add_cursor_timeout(context_xim);
#endif
  set_ic_client_window (context_xim, client_window);
}

///
GtkIMContext *
gtk_im_context_gcin_new (void)
{
//  printf("gtk_im_context_gcin_new\n");
  GtkIMContextGCIN *result;

  result = GTK_IM_CONTEXT_GCIN (g_object_new (GTK_TYPE_IM_CONTEXT_GCIN, NULL));

  return GTK_IM_CONTEXT (result);
}


///
static gboolean
gtk_im_context_gcin_filter_keypress (GtkIMContext *context,
				    GdkEventKey  *event)
{
  GtkIMContextGCIN *context_xim = GTK_IM_CONTEXT_GCIN (context);

  gchar static_buffer[256];
  unsigned char *buffer = static_buffer;
//  char *buffer = static_buffer;
  gint buffer_size = sizeof(static_buffer) - 1;
  gint num_bytes = 0;
  KeySym keysym = 0;
  Status status;
  gboolean result = FALSE;
  GdkWindow *root_window = gdk_screen_get_root_window (gdk_drawable_get_screen (event->window));

  XKeyPressedEvent xevent;

  xevent.type = (event->type == GDK_KEY_PRESS) ? KeyPress : KeyRelease;
  xevent.serial = 0;		/* hope it doesn't matter */
  xevent.send_event = event->send_event;
  xevent.display = GDK_DRAWABLE_XDISPLAY (event->window);
  xevent.window = GDK_DRAWABLE_XID (event->window);
  xevent.root = GDK_DRAWABLE_XID (root_window);
  xevent.subwindow = xevent.window;
  xevent.time = event->time;
  xevent.x = xevent.x_root = 0;
  xevent.y = xevent.y_root = 0;
  xevent.state = event->state;
  xevent.keycode = event->hardware_keycode;
  xevent.same_screen = True;

  char *rstr = NULL;
  num_bytes = XLookupString (&xevent, buffer, buffer_size, &keysym, NULL);

  // If it is latin key, XLookupString only works for UTF-8 env

#if (!FREEBSD || MAC_OS)
  char *lc_ctype = getenv("LC_CTYPE");
  if (!lc_ctype || !strstr(lc_ctype, "UTF-8")) {
    int uni = gdk_keyval_to_unicode(event->keyval);
    if (uni) {
      unsigned int rn;
      GError *err = NULL;
      char *utf8 = g_convert((char *)&uni, 4, "UTF-8", "UTF-32", &rn, &num_bytes, &err);

      if (utf8) {
        strcpy(buffer, utf8);
        g_free(utf8);
      }
    }
  }
#endif

  if (xevent.type == KeyPress) {
    result = gcin_im_client_forward_key_press(context_xim->gcin_ch,
      keysym, xevent.state, &rstr);

#if NEW_GTK_IM
    // this one is for mozilla, I know it is very dirty
    if (context_xim->is_mozilla) {
      if (context_xim->dirty_fix_off) {
        if (result && !rstr)
          g_signal_emit_by_name(context, "preedit_changed");
      } else {
        if (rstr || !result) {
          add_cursor_timeout(context_xim);
        }
      }
    }
#endif

//    printf("jj %x %x %d %x\n", rstr, result, num_bytes, (unsigned int)buffer[0]);
    if (!rstr && !result && num_bytes && buffer[0]>=0x20 && buffer[0]!=0x7f
        && !(xevent.state & (Mod1Mask|ControlMask))) {
      rstr = (char *)malloc(num_bytes + 1);
      memcpy(rstr, buffer, num_bytes);
      rstr[num_bytes] = 0;
      result = TRUE;
    }
  }
  else {
    result = gcin_im_client_forward_key_release(context_xim->gcin_ch,
      keysym, xevent.state, &rstr);
  }
#if 0
  printf("event->type:%d iiiii %d  %x %d rstr:%x\n", event->type, result, keysym,
    num_bytes, rstr);
#endif
  if (rstr) {
//    printf("emit %s\n", rstr);
    g_signal_emit_by_name (context, "commit", rstr);
    free(rstr);
  }

  return result;
}

///
static void
gtk_im_context_gcin_focus_in (GtkIMContext *context)
{
  GtkIMContextGCIN *context_xim = GTK_IM_CONTEXT_GCIN (context);

//  printf("gtk_im_context_gcin_focus_in\n");
  if (context_xim->gcin_ch) {
#if NEW_GTK_IM
    if (context_xim->is_mozilla && !context_xim->dirty_fix_off)
      add_cursor_timeout(context_xim);
#endif
    gcin_im_client_focus_in(context_xim->gcin_ch);
  }

  return;
}

static void
gtk_im_context_gcin_focus_out (GtkIMContext *context)
{
  GtkIMContextGCIN *context_xim = GTK_IM_CONTEXT_GCIN (context);
//  printf("gtk_im_context_gcin_focus_out\n");
#if NEW_GTK_IM
  cancel_timeout(context_xim);
#endif

  if (context_xim->gcin_ch) {
    gcin_im_client_focus_out(context_xim->gcin_ch);
  }

  return;
}

///
static void
gtk_im_context_gcin_set_cursor_location (GtkIMContext *context,
					GdkRectangle *area)
{
//  printf("gtk_im_context_gcin_set_cursor_location %d\n", area->x);
  GtkIMContextGCIN *context_xim = GTK_IM_CONTEXT_GCIN (context);

  if (context_xim->gcin_ch) {
    gcin_im_client_set_cursor_location(context_xim->gcin_ch, area->x, area->y + area->height);
  }

  return;
}

///
static void
gtk_im_context_gcin_set_use_preedit (GtkIMContext *context,
                                    gboolean      use_preedit)
{
//  printf("gtk_im_context_gcin_set_use_preedit %d\n", use_preedit);
}


///
static void
gtk_im_context_gcin_reset (GtkIMContext *context)
{
  GtkIMContextGCIN *context_gcin = GTK_IM_CONTEXT_GCIN (context);
#if DBG
  printf("gtk_im_context_gcin_reset %x\n", context_gcin);
#endif
}

/* Mask of feedback bits that we render
 */


static void
gtk_im_context_gcin_get_preedit_string (GtkIMContext   *context,
				       gchar         **str,
				       PangoAttrList **attrs,
                                       gint           *cursor_pos)
{
#if DBG
  printf("gtk_im_context_gcin_get_preedit_string %x %x %x\n", str, attrs, cursor_pos);
#endif
  GtkIMContextGCIN *context_xim = GTK_IM_CONTEXT_GCIN (context);

  if (str)
      *str = g_strdup ("");

  if (cursor_pos)
      *cursor_pos = 0;

  if (attrs)
      *attrs = pango_attr_list_new ();
}


/**
 * gtk_im_context_gcin_shutdown:
 *
 * Destroys all the status windows that are kept by the GCIN contexts.  This
 * function should only be called by the GCIN module exit routine.
 **/
void
gtk_im_context_gcin_shutdown (void)
{
#if DBG
 printf("shutdown\n");
#endif
}
