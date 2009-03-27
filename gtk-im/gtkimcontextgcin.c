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

typedef struct _GtkGCINInfo GtkGCINInfo;

struct _GtkIMContextGCIN
{
  GtkIMContext object;

  GtkGCINInfo *im_info;

  GdkWindow *client_window;
  GtkWidget *client_widget;

  gint preedit_size;
  gint preedit_length;
  gunichar *preedit_chars;
  XIMFeedback *feedbacks;
  gint preedit_cursor;

  GCIN_client_handle *gcin_ch;

//  guint in_toplevel : 1;
//  guint has_focus : 1;
};

struct _GtkGCINInfo
{
  GdkScreen *screen;

  GtkSettings *settings;
  gulong status_set;
  gulong preedit_set;
//  GSList *ics;

  guint reconnecting :1;
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
						       GdkRectangle		*area);
static void     gtk_im_context_gcin_set_use_preedit    (GtkIMContext          *context,
						       gboolean		      use_preedit);
static void     gtk_im_context_gcin_get_preedit_string (GtkIMContext          *context,
						       gchar                **str,
						       PangoAttrList        **attrs,
						       gint                  *cursor_pos);

static void set_ic_client_window (GtkIMContextGCIN *context_xim,
				  GdkWindow       *client_window);

static GObjectClass *parent_class;

GType gtk_type_im_context_gcin = 0;

GSList *open_ims = NULL;

/* List of status windows for different toplevels */
static GSList *status_windows = NULL;

void
gtk_im_context_xim_register_type (GTypeModule *type_module)
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

#define PREEDIT_MASK (GCINPreeditCallbacks | GCINPreeditPosition | \
		      GCINPreeditArea | GCINPreeditNothing | GCINPreeditNone)
#define STATUS_MASK (GCINStatusCallbacks | GCINStatusArea | \
		      GCINStatusNothing | GCINStatusNone)


static void
reinitialize_all_ics (GtkGCINInfo *info)
{
#if 0
  GSList *tmp_list;

  for (tmp_list = info->ics; tmp_list; tmp_list = tmp_list->next)
    reinitialize_ic (tmp_list->data);
#endif
}

static void gcin_display_closed (GdkDisplay *display,
			 gboolean    is_error,
			 GCIN_client_handle *gcin_ch)
{
  gcin_im_client_close(gcin_ch);
}


static GtkGCINInfo *
get_im (GtkIMContextGCIN *context_xim)
{
  GdkWindow *client_window = context_xim->client_window;
  GSList *tmp_list;
  GtkGCINInfo *info;
  GdkScreen *screen = gdk_drawable_get_screen (client_window);
  GdkDisplay *display = gdk_screen_get_display (screen);

  info = g_new (GtkGCINInfo, 1);
  open_ims = g_slist_prepend (open_ims, info);

  info->screen = screen;
  info->settings = NULL;
  info->preedit_set = 0;
  info->status_set = 0;
//  info->ics = NULL;
  info->reconnecting = FALSE;
//  info->im = NULL;

  if (!context_xim->gcin_ch) {
    if (!(context_xim->gcin_ch = gcin_im_client_open(GDK_DISPLAY())))
      perror("cannot open gcin_ch");

    g_signal_connect (display, "closed",
                      G_CALLBACK (gcin_display_closed), context_xim->gcin_ch);
  }

  return info;
}

///
static void
gtk_im_context_gcin_class_init (GtkIMContextGCINClass *class)
{
  GtkIMContextClass *im_context_class = GTK_IM_CONTEXT_CLASS (class);
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  parent_class = g_type_class_peek_parent (class);

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
//  im_context_gcin->has_focus = FALSE;
//  im_context_gcin->in_toplevel = FALSE;
}

static void
gtk_im_context_gcin_finalize (GObject *obj)
{
//  printf("gtk_im_context_gcin_finalize\n");
  GtkIMContextGCIN *context_xim = GTK_IM_CONTEXT_GCIN (obj);

//  context_xim->finalizing = TRUE;

  if (context_xim->gcin_ch) {
    gcin_im_client_close(context_xim->gcin_ch);
    context_xim->gcin_ch = NULL;
  }
//  set_ic_client_window (context_xim, NULL);
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


/* Updates the in_toplevel flag for @context_xim
 */
static void update_in_toplevel (GtkIMContextGCIN *context_xim)
{
#if 0
  if (context_xim->client_widget)
    {
      GtkWidget *toplevel = gtk_widget_get_toplevel (context_xim->client_widget);

      context_xim->in_toplevel = (toplevel && GTK_WIDGET_TOPLEVEL (toplevel));
    }
  else
    context_xim->in_toplevel = FALSE;

  /* Some paranoia, in case we don't get a focus out */
  if (!context_xim->in_toplevel)
    context_xim->has_focus = FALSE;
#endif
}


static void
on_client_widget_hierarchy_changed (GtkWidget       *widget,
				    GtkWidget       *old_toplevel,
				    GtkIMContextGCIN *context_xim)
{
  update_in_toplevel (context_xim);
}


static void
set_ic_client_window (GtkIMContextGCIN *context_xim,
		      GdkWindow       *client_window)
{
//  printf("set_ic_client_window\n");
  if (context_xim->client_window)
    {
//      context_xim->im_info->ics = g_slist_remove (context_xim->im_info->ics, context_xim);
      context_xim->im_info = NULL;
    }

  context_xim->client_window = client_window;


  if (context_xim->client_window)
    {
      context_xim->im_info = get_im (context_xim);
//      context_xim->im_info->ics = g_slist_prepend (context_xim->im_info->ics, context_xim);
#if 1
      if (context_xim->gcin_ch) {
        gcin_im_client_set_window(context_xim->gcin_ch, GDK_DRAWABLE_XID(client_window));
      }
#endif
    }


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
}


///
static void
gtk_im_context_gcin_set_client_window (GtkIMContext          *context,
				      GdkWindow             *client_window)
{
//  printf("gtk_im_context_gcin_set_client_window\n");

  GtkIMContextGCIN *context_xim = GTK_IM_CONTEXT_GCIN (context);

  set_ic_client_window (context_xim, client_window);
}

///
GtkIMContext *
gtk_im_context_xim_new (void)
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
  gchar *buffer = static_buffer;
  gint buffer_size = sizeof(static_buffer) - 1;
  gint num_bytes = 0;
  KeySym keysym;
  Status status;
  gboolean result = FALSE;
  GdkWindow *root_window = gdk_screen_get_root_window (gdk_drawable_get_screen (event->window));

  XKeyPressedEvent xevent;

#if 0
  if (event->type == GDK_KEY_RELEASE && !context_xim->filter_key_release)
    return FALSE;
#endif

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

  if (xevent.type == KeyPress) {
    result = gcin_im_client_forward_key_press(context_xim->gcin_ch,
      keysym, xevent.state, &rstr);

//    dbg("yyyyyyyyy %d %d num_bytes:%d %x\n", rstr, result, num_bytes, buffer[0]);

    if (!rstr && !result && num_bytes && buffer[0]>=0x20 && buffer[0]!=0x7f) {
//      dbg("buffer %c\n", buffer[0]);
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

//  printf("event->type:%d iiiii %d\n", event->type, result);

  if (rstr) {
//    dbg("emit %s\n", rstr);
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
    gcin_im_client_focus_out(context_xim->gcin_ch);
  }

  return;
}

///
static void
gtk_im_context_gcin_set_cursor_location (GtkIMContext *context,
					GdkRectangle *area)
{
//  printf("gtk_im_context_gcin_set_cursor_location\n");
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
//  printf("gtk_im_context_gcin_set_use_preedit\n");
}


///
static void
gtk_im_context_gcin_reset (GtkIMContext *context)
{
//  printf("gtk_im_context_gcin_reset\n");
}

/* Mask of feedback bits that we render
 */

#define FEEDBACK_MASK (XIMReverse | XIMUnderline)

static void
add_feedback_attr (PangoAttrList *attrs,
		   const gchar   *str,
                   XIMFeedback    feedback,
		   gint           start_pos,
		   gint           end_pos)
{
  PangoAttribute *attr;

  gint start_index = g_utf8_offset_to_pointer (str, start_pos) - str;
  gint end_index = g_utf8_offset_to_pointer (str, end_pos) - str;

  if (feedback & XIMUnderline)
    {
      attr = pango_attr_underline_new (PANGO_UNDERLINE_SINGLE);
      attr->start_index = start_index;
      attr->end_index = end_index;

      pango_attr_list_change (attrs, attr);
    }

  if (feedback & XIMReverse)
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

  if (feedback & ~FEEDBACK_MASK)
    g_warning ("Unrendered feedback style: %#lx", feedback & ~FEEDBACK_MASK);
}

static void
gtk_im_context_gcin_get_preedit_string (GtkIMContext   *context,
				       gchar         **str,
				       PangoAttrList **attrs,
				       gint           *cursor_pos)
{
  GtkIMContextGCIN *context_xim = GTK_IM_CONTEXT_GCIN (context);
  gchar *utf8 = g_ucs4_to_utf8 (context_xim->preedit_chars, context_xim->preedit_length, NULL, NULL, NULL);

  if (attrs)
    {
      int i;
      XIMFeedback last_feedback = 0;
      gint start = -1;

      *attrs = pango_attr_list_new ();

      for (i = 0; i < context_xim->preedit_length; i++)
	{
	  XIMFeedback new_feedback = context_xim->feedbacks[i] & FEEDBACK_MASK;
	  if (new_feedback != last_feedback)
	    {
	      if (start >= 0)
		add_feedback_attr (*attrs, utf8, last_feedback, start, i);

	      last_feedback = new_feedback;
	      start = i;
	    }
	}

      if (start >= 0)
	add_feedback_attr (*attrs, utf8, last_feedback, start, i);
    }

  if (str)
    *str = utf8;
  else
    g_free (utf8);

  if (cursor_pos)
    *cursor_pos = context_xim->preedit_cursor;
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
}
