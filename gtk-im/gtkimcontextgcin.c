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

//#include "gtkintl.h"
#include <gtk/gtk.h>
#if !GTK_CHECK_VERSION(3,0,0)
#include "gtk/gtklabel.h"
#include "gtk/gtksignal.h"
#include "gtk/gtkwindow.h"
#endif
#include "gtkimcontextgcin.h"
// #include "gcin.h"  // for debug only
#include "gcin-im-client.h"
#include <X11/keysym.h>

#define DBG 0

typedef struct _GtkGCINInfo GtkGCINInfo;

struct _GtkIMContextGCIN
{
  GtkIMContext object;

  GdkWindow *client_window;
#if 0
  GtkWidget *client_widget;
#endif
  GCIN_client_handle *gcin_ch;
  int timeout_handle;
  char *pe_str;
  int old_sub_comp_len;
  gboolean pe_started;
  GCIN_PREEDIT_ATTR *pe_att;
  int pe_attN;
  int pe_cursor;
};


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
                                                       GdkRectangle             *area);
static void     gtk_im_context_gcin_set_use_preedit    (GtkIMContext          *context,
                                                       gboolean               use_preedit);
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
  if (!context_xim->gcin_ch)
    return;

  gcin_im_client_close(context_xim->gcin_ch);
  context_xim->gcin_ch = NULL;
}
#endif


static void
get_im (GtkIMContextGCIN *context_xim)
{
  GdkWindow *client_window = context_xim->client_window;
  if (!client_window)
    return;
  GdkScreen *screen = gdk_drawable_get_screen (client_window);
  if (!screen)
    return;

  GdkDisplay *display = gdk_screen_get_display (screen);
  if (!display)
    return;

  if (!context_xim->gcin_ch) {
    if (!(context_xim->gcin_ch = gcin_im_client_open(GDK_DISPLAY())))
      perror("cannot open gcin_ch");
#if 1
    context_xim->timeout_handle = 0;
    context_xim->pe_attN = 0;
    context_xim->pe_att = NULL;
    context_xim->pe_str = NULL;
    context_xim->pe_cursor = 0;
    context_xim->pe_started = FALSE;
#endif

#if 0
    // coredump
    g_signal_connect (display, "closed",
                      G_CALLBACK (gcin_display_closed), context_xim);
#endif
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
  im_context_gcin->pe_attN = 0;
  im_context_gcin->pe_att = NULL;
  im_context_gcin->pe_str = NULL;
  im_context_gcin->pe_cursor = 0;
  im_context_gcin->gcin_ch = NULL;
//  test_gdm();

#if DBG
  printf("gtk_im_context_gcin_init %x\n", im_context_gcin);
#endif
// dirty hack for mozilla...
}


void clear_preedit(GtkIMContextGCIN *context_gcin)
{
  if (!context_gcin)
    return;

  if (context_gcin->pe_str) {
    free(context_gcin->pe_str);
    context_gcin->pe_str = NULL;
  }

  if (context_gcin->pe_att) {
    free(context_gcin->pe_att);
    context_gcin->pe_att = NULL;
    context_gcin->pe_attN = 0;
  }

  context_gcin->pe_cursor = 0;
}


static void
gtk_im_context_gcin_finalize (GObject *obj)
{
#if DBG
  printf("gtk_im_context_gcin_finalize %x\n", obj);
#endif
  GtkIMContextGCIN *context_xim = GTK_IM_CONTEXT_GCIN (obj);
  clear_preedit(context_xim);

  if (context_xim->gcin_ch) {
    gcin_im_client_close(context_xim->gcin_ch);
    context_xim->gcin_ch = NULL;
  }
}


static void set_ic_client_window (GtkIMContextGCIN *context_xim,
                      GdkWindow       *client_window)
{
#if DBG
  printf("set_ic_client_window %x %x\n", context_xim, client_window);
#endif
  if (!client_window)
    return;

  context_xim->client_window = client_window;

  if (context_xim->client_window) {
    get_im (context_xim);
    if (context_xim->gcin_ch) {
      gcin_im_client_set_window(context_xim->gcin_ch, GDK_DRAWABLE_XID(client_window));
    }
  }
}


///
static void
gtk_im_context_gcin_set_client_window (GtkIMContext          *context,
                                      GdkWindow             *client_window)
{
#if DBG
  printf("gtk_im_context_gcin_set_client_window\n");
#endif
  GtkIMContextGCIN *context_xim = GTK_IM_CONTEXT_GCIN (context);
  set_ic_client_window (context_xim, client_window);
}

///
GtkIMContext *
gtk_im_context_gcin_new (void)
{
#if DBG
  printf("gtk_im_context_gcin_new\n");
#endif
  GtkIMContextGCIN *result;

  result = GTK_IM_CONTEXT_GCIN (g_object_new (GTK_TYPE_IM_CONTEXT_GCIN, NULL));

  return GTK_IM_CONTEXT (result);
}

#include <gdk/gdkkeysyms.h>

#if 0
static gboolean open_gcin_win(int sub_comp_len, GtkIMContextGCIN *context_xim)
{
    return !(context_xim->old_sub_comp_len & 1) && (sub_comp_len & 1)
        || !(context_xim->old_sub_comp_len & 2) && (sub_comp_len & 2)
        ||  !(context_xim->old_sub_comp_len & 4) && (sub_comp_len & 4);
}
#endif


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
  gsize num_bytes = 0;
  KeySym keysym = 0;
  Status status;
  gboolean result = FALSE;
#if !GTK_CHECK_VERSION(3,0,0)
  GdkWindow *root_window = gdk_screen_get_root_window (gdk_drawable_get_screen (event->window));
#else
  GdkWindow *root_window = NULL;
  GdkScreen *screen = gdk_window_get_screen (event->window);
  if (screen)
    root_window = gdk_screen_get_root_window (screen);
  else
    return result;
#endif

  XKeyPressedEvent xevent;
  xevent.type = (event->type == GDK_KEY_PRESS) ? KeyPress : KeyRelease;
  xevent.serial = 0;            /* hope it doesn't matter */
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
  num_bytes = XLookupString (&xevent, buffer, buffer_size, &keysym, NULL);

  char *rstr = NULL;

#if (!FREEBSD || MAC_OS)
//  if (event->type == GDK_KEY_PRESS)
//    printf("kval %x %x\n",event->keyval, keysym);

  int uni = gdk_keyval_to_unicode(event->keyval);
  if (uni) {
    gsize rn;
    GError *err = NULL;
    char *utf8 = g_convert((char *)&uni, 4, "UTF-8", "UTF-32", &rn, &num_bytes, &err);

    if (utf8) {
//      printf("conv %s\n", utf8);
      strcpy(buffer, utf8);
      g_free(utf8);
    }
  }
#endif

  gboolean preedit_changed = FALSE;
  gboolean context_pe_started = context_xim->pe_started;
  gboolean context_has_str = context_xim->pe_str && context_xim->pe_str[0];
  char *tstr = NULL;
  int sub_comp_len = 0;
  GCIN_PREEDIT_ATTR att[GCIN_PREEDIT_ATTR_MAX_N];
  int cursor_pos;
  gboolean has_str = FALSE;

  if (event->type == GDK_KEY_PRESS) {
    result = gcin_im_client_forward_key_press(context_xim->gcin_ch,
      keysym, xevent.state, &rstr);
  } else {
//    printf("release %x %x\n", xevent.state, event->state);
    result = gcin_im_client_forward_key_release(context_xim->gcin_ch,
     keysym, xevent.state, &rstr);
  }

    preedit_changed = result;

    int attN = gcin_im_client_get_preedit(context_xim->gcin_ch, &tstr, att, &cursor_pos, &sub_comp_len);
    has_str = tstr && tstr[0];


#if DBG
    printf("result keysym:%x %d sub_comp_len:%x tstr:%x\n", keysym, result, sub_comp_len, tstr);
#endif

    if (sub_comp_len) {
      has_str = TRUE;
//      preedit_changed = TRUE;
    }

    context_xim->old_sub_comp_len = sub_comp_len;

    if (!context_pe_started && has_str) {
#if DBG
      printf("emit preedit-start\n");
#endif
      g_signal_emit_by_name (context, "preedit-start");
      context_pe_started = context_xim->pe_started = TRUE;
    }

    if (context_has_str != has_str || (tstr && context_xim->pe_str && strcmp(tstr, context_xim->pe_str))) {
      if (context_xim->pe_str)
        free(context_xim->pe_str);
      context_xim->pe_str = tstr;
//      preedit_changed = TRUE;
    }


    int attsz = sizeof(GCIN_PREEDIT_ATTR)*attN;
    if (context_xim->pe_attN != attN ||
      context_xim->pe_att && memcmp(context_xim->pe_att, att, attsz)) {
//      printf("att changed pe_att:%x:%d %d\n", context_xim->pe_att, context_xim->pe_attN, attN);
      context_xim->pe_attN = attN;
      if (context_xim->pe_att)
        free(context_xim->pe_att);

      context_xim->pe_att = NULL;
      if (attN)
        context_xim->pe_att = malloc(attsz);
      memcpy(context_xim->pe_att, att, attsz);
//      printf("context_xim->pe_att %x\n", context_xim->pe_att);
//      preedit_changed = TRUE;
    }

    if (context_xim->pe_cursor != cursor_pos) {
#if DBG
      printf("cursor changed %d %d\n", context_xim->pe_cursor, cursor_pos);
#endif
      context_xim->pe_cursor = cursor_pos;
//      preedit_changed = TRUE;
    }

#if DBG
    printf("seq:%d rstr:%s result:%x num_bytes:%d %x\n", context_xim->gcin_ch->seq, rstr, result, num_bytes, (unsigned int)buffer[0]);
#endif
    if (event->type == GDK_KEY_PRESS && !rstr && !result && num_bytes &&
#if 1
   buffer[0]>=0x20 && buffer[0]!=0x7f
#else
   (event->keyval < 0xf000 || buffer[0]>=0x20 && buffer[0] < 0x7f)
#endif
        && !(xevent.state & (Mod1Mask|ControlMask))) {
      rstr = (char *)malloc(num_bytes + 1);
      memcpy(rstr, buffer, num_bytes);
      rstr[num_bytes] = 0;
      result = TRUE;
    }

#if 1
  if (preedit_changed) {
#if DBG
    printf("preedit-change\n");
#endif
    g_signal_emit_by_name(context_xim, "preedit_changed");
  }
#endif




#if DBG
  printf("seq:%d event->type:%d iiiii %d  %x %d rstr:%x\n",context_xim->gcin_ch->seq, event->type, result, keysym,
    num_bytes, rstr);
#endif
  if (rstr) {
#if DBG
    printf("emit %s\n", rstr);
#endif
    g_signal_emit_by_name (context, "commit", rstr);
    free(rstr);
  }

  if (!has_str && context_pe_started) {
    clear_preedit(context_xim);
    context_xim->pe_started = FALSE;
#if DBG
    printf("preedit-end %x\n", has_str);
#endif
    g_signal_emit_by_name (context, "preedit-end");
  }


  return result;
}

///
static void
gtk_im_context_gcin_focus_in (GtkIMContext *context)
{
  GtkIMContextGCIN *context_xim = GTK_IM_CONTEXT_GCIN (context);
#if DBG
  printf("gtk_im_context_gcin_focus_in\n");
#endif
  if (context_xim->gcin_ch) {
    gcin_im_client_focus_in(context_xim->gcin_ch);
  }

  return;
}

static void
gtk_im_context_gcin_focus_out (GtkIMContext *context)
{
  GtkIMContextGCIN *context_xim = GTK_IM_CONTEXT_GCIN (context);
//  printf("gtk_im_context_gcin_focus_out\n");

  if (context_xim->gcin_ch) {
    char *rstr;
    gcin_im_client_focus_out2(context_xim->gcin_ch , &rstr);
    context_xim->pe_started = FALSE;

    if (rstr) {
      g_signal_emit_by_name (context, "commit", rstr);
      clear_preedit(context_xim);
      g_signal_emit_by_name(context, "preedit_changed");
      free(rstr);
    }

  }

  return;
}

///
static void
gtk_im_context_gcin_set_cursor_location (GtkIMContext *context,
                                        GdkRectangle *area)
{
#if DBG
  printf("a gtk_im_context_gcin_set_cursor_location %d,%d,%d\n", area->x, area->y, area->height);
#endif
  if (!area) {
    return;
  }


  GtkIMContextGCIN *context_xim = GTK_IM_CONTEXT_GCIN (context);

  if (!context_xim->gcin_ch) {
    get_im(context_xim);
  }

  if (context_xim->gcin_ch) {
    gcin_im_client_set_cursor_location(context_xim->gcin_ch, area->x, area->y + area->height);
  }

#if DBG
  printf("b gtk_im_context_gcin_set_cursor_location %d,%d,%d\n", area->x, area->y, area->height);
#endif

  return;
}

///
static void
gtk_im_context_gcin_set_use_preedit (GtkIMContext *context,
                                    gboolean      use_preedit)
{
  GtkIMContextGCIN *context_gcin = GTK_IM_CONTEXT_GCIN (context);
#if DBG
  printf("gtk_im_context_gcin_set_use_preedit %x %d\n", context_gcin->gcin_ch, use_preedit);
#endif
  if (!context_gcin->gcin_ch)
    return;
  int ret;
  if (use_preedit)
    gcin_im_client_set_flags(context_gcin->gcin_ch, FLAG_GCIN_client_handle_use_preedit, &ret);
  else
    gcin_im_client_clear_flags(context_gcin->gcin_ch, FLAG_GCIN_client_handle_use_preedit, &ret);
}


///
static void
gtk_im_context_gcin_reset (GtkIMContext *context)
{
  GtkIMContextGCIN *context_gcin = GTK_IM_CONTEXT_GCIN (context);
#if DBG
  printf("gtk_im_context_gcin_reset %x\n", context_gcin);
#endif

  context_gcin->pe_started = FALSE;
#if 1
  if (context_gcin->gcin_ch) {
    gcin_im_client_reset(context_gcin->gcin_ch);
    if (context_gcin->pe_str && context_gcin->pe_str[0]) {
#if DBG
      printf("clear %x\n", context_gcin);
#endif
      clear_preedit(context_gcin);
      g_signal_emit_by_name(context, "preedit_changed");
    }
  }
#endif
}

/* Mask of feedback bits that we render
 */

static void
add_preedit_attr (PangoAttrList *attrs,
		   const gchar *str,  GCIN_PREEDIT_ATTR *att)
{
//  printf("att %d %d\n", att->ofs0, att->ofs1);
  PangoAttribute *attr;
  gint start_index = g_utf8_offset_to_pointer (str, att->ofs0) - str;
  gint end_index = g_utf8_offset_to_pointer (str, att->ofs1) - str;

  if (att->flag & GCIN_PREEDIT_ATTR_FLAG_UNDERLINE)
    {
      attr = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
      attr->start_index = start_index;
      attr->end_index = end_index;
      pango_attr_list_change (attrs, attr);
    }

  if (att->flag & GCIN_PREEDIT_ATTR_FLAG_REVERSE)
    {
      attr = pango_attr_foreground_new (0xffff, 0xffff, 0xffff);
      attr->start_index = start_index;
      attr->end_index = end_index;
      pango_attr_list_change (attrs, attr);

      attr = pango_attr_background_new (0, 0, 0);
      attr->start_index = start_index;
      attr->end_index = end_index;
      pango_attr_list_change (attrs, attr);
    }
}

static void
gtk_im_context_gcin_get_preedit_string (GtkIMContext   *context,
				       gchar         **str,
				       PangoAttrList **attrs,
                                       gint           *cursor_pos)
{
  GtkIMContextGCIN *context_gcin = GTK_IM_CONTEXT_GCIN (context);

#if DBG
  printf("gtk_im_context_gcin_get_preedit_string %x %x %x\n", str, attrs, cursor_pos);
#endif

  if (context_gcin->gcin_ch && cursor_pos) {
    int ret;
    gcin_im_client_set_flags(context_gcin->gcin_ch, FLAG_GCIN_client_handle_use_preedit, &ret);
  }

  if (cursor_pos)
      *cursor_pos = 0;

  if (attrs)
      *attrs = pango_attr_list_new ();

  if (!str)
    return;

#if 1
  if (!context_gcin->gcin_ch) {
empty:
    *str=g_strdup("");
    return;
  }

  if (cursor_pos) {
    *cursor_pos = context_gcin->pe_cursor;
  }

  if (context_gcin->pe_str) {
    *str=g_strdup(context_gcin->pe_str);
  } else {
    goto empty;
  }

#if DBG
  printf("gtk_im_context_gcin_get_preedit_string %x attN:%d '%s'\n", context_gcin->pe_att,
    context_gcin->pe_attN, *str);
#endif
  int i;
  if (attrs)
  for(i=0; i < context_gcin->pe_attN; i++) {
    add_preedit_attr(*attrs, *str, &(context_gcin->pe_att[i]));
  }

#else
  if (str)
    *str = g_strdup("");
#endif
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
