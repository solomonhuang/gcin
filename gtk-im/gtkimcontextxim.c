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

#include "gtkintl.h"
#include "gtk/gtklabel.h"
#include "gtk/gtksignal.h"
#include "gtk/gtkwindow.h"
#include "gtkimcontextxim.h"
#include "gcin-im-client.h"

typedef struct _GtkXIMInfo GtkXIMInfo;

struct _GtkIMContextXIM
{
  GtkIMContext object;

  GtkXIMInfo *im_info;

  gchar *locale;
  gchar *mb_charset;

  GdkWindow *client_window;
  GtkWidget *client_widget;

  XIC ic;

  guint filter_key_release : 1;
  guint use_preedit : 1;
  guint finalizing : 1;
  guint in_toplevel : 1;
  guint has_focus : 1;
};

struct _GtkXIMInfo
{
  GdkScreen *screen;
  XIM im;
  GCIN_client_handle *gcin_ch;

  char *locale;
  XIMStyle style;
  GtkSettings *settings;
  gulong status_set;
  gulong preedit_set;
  XIMStyles *xim_styles;
  GSList *ics;

  guint reconnecting :1;
};


static void     gtk_im_context_xim_class_init         (GtkIMContextXIMClass  *class);
static void     gtk_im_context_xim_init               (GtkIMContextXIM       *im_context_xim);
static void     gtk_im_context_xim_finalize           (GObject               *obj);
static void     gtk_im_context_xim_set_client_window  (GtkIMContext          *context,
						       GdkWindow             *client_window);
static gboolean gtk_im_context_xim_filter_keypress    (GtkIMContext          *context,
						       GdkEventKey           *key);
static void     gtk_im_context_xim_reset              (GtkIMContext          *context);
static void     gtk_im_context_xim_focus_in           (GtkIMContext          *context);
static void     gtk_im_context_xim_focus_out          (GtkIMContext          *context);
static void     gtk_im_context_xim_set_cursor_location (GtkIMContext          *context,
						       GdkRectangle		*area);
static void     gtk_im_context_xim_set_use_preedit    (GtkIMContext          *context,
						       gboolean		      use_preedit);
static void     gtk_im_context_xim_get_preedit_string (GtkIMContext          *context,
						       gchar                **str,
						       PangoAttrList        **attrs,
						       gint                  *cursor_pos);

static void reinitialize_ic      (GtkIMContextXIM *context_xim);
static void set_ic_client_window (GtkIMContextXIM *context_xim,
				  GdkWindow       *client_window);

static void xim_destroy_callback   (XIM      xim,
				    XPointer client_data,
				    XPointer call_data);

static XIC gtk_im_context_xim_get_ic (GtkIMContextXIM *context_xim);
static GObjectClass *parent_class;

GType gtk_type_im_context_xim = 0;

GSList *open_ims = NULL;

/* List of status windows for different toplevels */
static GSList *status_windows = NULL;

void
gtk_im_context_xim_register_type (GTypeModule *type_module)
{
  static const GTypeInfo im_context_xim_info =
  {
    sizeof (GtkIMContextXIMClass),
    (GBaseInitFunc) NULL,
    (GBaseFinalizeFunc) NULL,
    (GClassInitFunc) gtk_im_context_xim_class_init,
    NULL,           /* class_finalize */
    NULL,           /* class_data */
    sizeof (GtkIMContextXIM),
    0,
    (GInstanceInitFunc) gtk_im_context_xim_init,
  };

  gtk_type_im_context_xim =
    g_type_module_register_type (type_module,
				 GTK_TYPE_IM_CONTEXT,
				 "GtkIMContextXIM",
				 &im_context_xim_info, 0);
}

#define PREEDIT_MASK (XIMPreeditCallbacks | XIMPreeditPosition | \
		      XIMPreeditArea | XIMPreeditNothing | XIMPreeditNone)
#define STATUS_MASK (XIMStatusCallbacks | XIMStatusArea | \
		      XIMStatusNothing | XIMStatusNone)


static void
reinitialize_all_ics (GtkXIMInfo *info)
{
  GSList *tmp_list;

  for (tmp_list = info->ics; tmp_list; tmp_list = tmp_list->next)
    reinitialize_ic (tmp_list->data);
}


static void
setup_im (GtkXIMInfo *info)
{
  XIMValuesList *ic_values = NULL;
  XIMCallback im_destroy_callback;

  if (info->im == NULL)
    return;

  im_destroy_callback.client_data = (XPointer)info;
  im_destroy_callback.callback = (XIMProc)xim_destroy_callback;
  XSetIMValues (info->im,
		XNDestroyCallback, &im_destroy_callback,
		NULL);

  XGetIMValues (info->im,
		XNQueryInputStyle, &info->xim_styles,
		XNQueryICValuesList, &ic_values,
		NULL);

  info->settings = gtk_settings_get_for_screen (info->screen);

  info->style = XIMPreeditPosition;
  reinitialize_all_ics (info);

  if (ic_values)
    XFree (ic_values);
}

static void
xim_info_display_closed (GdkDisplay *display,
			 gboolean    is_error,
			 GtkXIMInfo *info)
{
  GSList *ics, *tmp_list;

  open_ims = g_slist_remove (open_ims, info);

  ics = info->ics;
  info->ics = NULL;

  for (tmp_list = ics; tmp_list; tmp_list = tmp_list->next)
    set_ic_client_window (tmp_list->data, NULL);

  g_slist_free (ics);

  g_signal_handler_disconnect (info->settings, info->status_set);
  g_signal_handler_disconnect (info->settings, info->preedit_set);

  XFree (info->xim_styles->supported_styles);
  XFree (info->xim_styles);
  g_free (info->locale);

  if (info->im)
    XCloseIM (info->im);

  g_free (info);
}


/* initialize info->im */
static void
xim_info_try_im (GtkXIMInfo *info)
{
  GdkScreen *screen = info->screen;
  GdkDisplay *display = gdk_screen_get_display (screen);

  printf("xim_info_try_im\n");

  g_assert (info->im == NULL);
  if (info->reconnecting)
    return;

  if (XSupportsLocale ())
    {
      if (!XSetLocaleModifiers (""))
	g_warning ("Unable to set locale modifiers with XSetLocaleModifiers()");
      info->im = XOpenIM (GDK_DISPLAY_XDISPLAY (display), NULL, NULL, NULL);
      printf("XOpenIM %x\n", info->im);
      setup_im (info);
      g_signal_connect (display, "closed",
			G_CALLBACK (xim_info_display_closed), info);
    }


  printf("iiiiiiiiiiiiiiii\n");

#if 0
  fflush(stdout);
  if (!(info->gcin_ch = gcin_im_client_open()))
    perror("cannot open gcin_ch");
#endif
}

static void
xim_destroy_callback (XIM      xim,
		      XPointer client_data,
		      XPointer call_data)
{
  GtkXIMInfo *info = (GtkXIMInfo*)client_data;

  info->im = NULL;

  g_signal_handler_disconnect (info->settings, info->status_set);
  g_signal_handler_disconnect (info->settings, info->preedit_set);

  reinitialize_all_ics (info);
  xim_info_try_im (info);
  return;
}

static GtkXIMInfo *
get_im (GdkWindow *client_window,
	const char *locale)
{
  GSList *tmp_list;
  GtkXIMInfo *info;
  GdkScreen *screen = gdk_drawable_get_screen (client_window);

  printf("get_im\n");

  info = NULL;
  tmp_list = open_ims;
  while (tmp_list)
    {
      GtkXIMInfo *tmp_info = tmp_list->data;
      if (tmp_info->screen == screen &&
	  strcmp (tmp_info->locale, locale) == 0)
	{
	  if (tmp_info->im)
	    {
	      return tmp_info;
	    }
	  else
	    {
	      tmp_info = tmp_info;
	      break;
	    }
	}
      tmp_list = tmp_list->next;
    }

  if (info == NULL)
    {
      printf("new GtkXIMInfo\n");
      info = g_new (GtkXIMInfo, 1);
      open_ims = g_slist_prepend (open_ims, info);

      info->screen = screen;
      info->locale = g_strdup (locale);
      info->xim_styles = NULL;
      info->settings = NULL;
      info->preedit_set = 0;
      info->status_set = 0;
      info->ics = NULL;
      info->reconnecting = FALSE;
      info->im = NULL;
    }

  xim_info_try_im (info);
  return info;
}

///
static void
gtk_im_context_xim_class_init (GtkIMContextXIMClass *class)
{
  GtkIMContextClass *im_context_class = GTK_IM_CONTEXT_CLASS (class);
  GObjectClass *gobject_class = G_OBJECT_CLASS (class);

  parent_class = g_type_class_peek_parent (class);

  im_context_class->set_client_window = gtk_im_context_xim_set_client_window;
  im_context_class->filter_keypress = gtk_im_context_xim_filter_keypress;
  im_context_class->reset = gtk_im_context_xim_reset;
  im_context_class->get_preedit_string = gtk_im_context_xim_get_preedit_string;
  im_context_class->focus_in = gtk_im_context_xim_focus_in;
  im_context_class->focus_out = gtk_im_context_xim_focus_out;
  im_context_class->set_cursor_location = gtk_im_context_xim_set_cursor_location;
  im_context_class->set_use_preedit = gtk_im_context_xim_set_use_preedit;
  gobject_class->finalize = gtk_im_context_xim_finalize;
}

static void
gtk_im_context_xim_init (GtkIMContextXIM *im_context_xim)
{
  im_context_xim->use_preedit = TRUE;
  im_context_xim->filter_key_release = FALSE;
  im_context_xim->finalizing = FALSE;
  im_context_xim->has_focus = FALSE;
  im_context_xim->in_toplevel = FALSE;
}

static void
gtk_im_context_xim_finalize (GObject *obj)
{
  printf("gtk_im_context_xim_finalize\n");

  GtkIMContextXIM *context_xim = GTK_IM_CONTEXT_XIM (obj);

  context_xim->finalizing = TRUE;

  set_ic_client_window (context_xim, NULL);

  g_free (context_xim->locale);
  g_free (context_xim->mb_charset);
}

static void
reinitialize_ic (GtkIMContextXIM *context_xim)
{
  if (context_xim->ic)
    {
      XDestroyIC (context_xim->ic);
      context_xim->ic = NULL;
    }
  /*
     reset filter_key_release flag, otherwise keystrokes will be doubled
     until reconnecting to XIM.
  */
  context_xim->filter_key_release = FALSE;
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
static void update_in_toplevel (GtkIMContextXIM *context_xim)
{
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
}


static void
on_client_widget_hierarchy_changed (GtkWidget       *widget,
				    GtkWidget       *old_toplevel,
				    GtkIMContextXIM *context_xim)
{
  update_in_toplevel (context_xim);
}


static void
set_ic_client_window (GtkIMContextXIM *context_xim,
		      GdkWindow       *client_window)
{
  printf("set_ic_client_window\n");
  reinitialize_ic (context_xim);
  if (context_xim->client_window)
    {
      context_xim->im_info->ics = g_slist_remove (context_xim->im_info->ics, context_xim);
      context_xim->im_info = NULL;
    }

  context_xim->client_window = client_window;

  if (context_xim->client_window)
    {
      context_xim->im_info = get_im (context_xim->client_window, context_xim->locale);
      context_xim->im_info->ics = g_slist_prepend (context_xim->im_info->ics, context_xim);
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
gtk_im_context_xim_set_client_window (GtkIMContext          *context,
				      GdkWindow             *client_window)
{
  printf("gtk_im_context_xim_set_client_window\n");

  GtkIMContextXIM *context_xim = GTK_IM_CONTEXT_XIM (context);

  set_ic_client_window (context_xim, client_window);
}

///
GtkIMContext *
gtk_im_context_xim_new (void)
{
  printf("gtk_im_context_xim_new\n");
  GtkIMContextXIM *result;
  const gchar *charset;

  result = GTK_IM_CONTEXT_XIM (g_object_new (GTK_TYPE_IM_CONTEXT_XIM, NULL));

  result->locale = g_strdup (setlocale (LC_CTYPE, NULL));

  g_get_charset (&charset);
  result->mb_charset = g_strdup (charset);

  return GTK_IM_CONTEXT (result);
}

static char *
mb_to_utf8 (GtkIMContextXIM *context_xim,
	    const char      *str)
{
  GError *error = NULL;
  gchar *result;

  if (strcmp (context_xim->mb_charset, "UTF-8") == 0)
    result = g_strdup (str);
  else
    {
      result = g_convert (str, -1,
			  "UTF-8", context_xim->mb_charset,
			  NULL, NULL, &error);
      if (!result)
	{
	  g_warning ("Error converting text from IM to UTF-8: %s\n", error->message);
	  g_error_free (error);
	}
    }

  return result;
}

///
static gboolean
gtk_im_context_xim_filter_keypress (GtkIMContext *context,
				    GdkEventKey  *event)
{
  GtkIMContextXIM *context_xim = GTK_IM_CONTEXT_XIM (context);
  XIC ic = gtk_im_context_xim_get_ic (context_xim);
  gchar static_buffer[256];
  gchar *buffer = static_buffer;
  gint buffer_size = sizeof(static_buffer) - 1;
  gint num_bytes = 0;
  KeySym keysym;
  Status status;
  gboolean result = FALSE;
  GdkWindow *root_window = gdk_screen_get_root_window (gdk_drawable_get_screen (event->window));

  XKeyPressedEvent xevent;

  if (event->type == GDK_KEY_RELEASE)
    printf("key release\n");
  else
    printf("key press\n");

  if (event->type == GDK_KEY_RELEASE && !context_xim->filter_key_release)
    return FALSE;

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

  if (XFilterEvent ((XEvent *)&xevent, GDK_DRAWABLE_XID (context_xim->client_window)))
    return TRUE;

 again:
  if (ic)
    num_bytes = XmbLookupString (ic, &xevent, buffer, buffer_size, &keysym, &status);
  else
    {
      num_bytes = XLookupString (&xevent, buffer, buffer_size, &keysym, NULL);
      status = XLookupBoth;
    }

  if (status == XBufferOverflow)
    {
      buffer_size = num_bytes;
      if (buffer != static_buffer)
	g_free (buffer);
      buffer = g_malloc (num_bytes + 1);
      goto again;
    }

  /* I don't know how we should properly handle XLookupKeysym or XLookupBoth
   * here ... do input methods actually change the keysym? we can't really
   * feed it back to accelerator processing at this point...
   */
  if (status == XLookupChars || status == XLookupBoth)
    {
      char *result_utf8;

      buffer[num_bytes] = '\0';

      result_utf8 = mb_to_utf8 (context_xim, buffer);
      if (result_utf8)
	{
	  if ((guchar)result_utf8[0] >= 0x20 &&
	      result_utf8[0] != 0x7f) /* Some IM have a nasty habit of converting
				       * control characters into strings
				       */
	    {
	      g_signal_emit_by_name (context, "commit", result_utf8);
	      result = TRUE;
	    }

	  g_free (result_utf8);
	}
    }

  if (buffer != static_buffer)
    g_free (buffer);

  return result;
}

///
static void
gtk_im_context_xim_focus_in (GtkIMContext *context)
{
  GtkIMContextXIM *context_xim = GTK_IM_CONTEXT_XIM (context);

  printf("gtk_im_context_xim_focus_in\n");

  if (!context_xim->has_focus)
    {
      XIC ic = gtk_im_context_xim_get_ic (context_xim);

      context_xim->has_focus = TRUE;

      if (ic)
	XSetICFocus (ic);
    }

  return;
}

static void
gtk_im_context_xim_focus_out (GtkIMContext *context)
{
  GtkIMContextXIM *context_xim = GTK_IM_CONTEXT_XIM (context);

  if (context_xim->has_focus)
    {
      XIC ic = gtk_im_context_xim_get_ic (context_xim);

      context_xim->has_focus = FALSE;

      if (ic)
	XUnsetICFocus (ic);
    }

  return;
}

///
static void
gtk_im_context_xim_set_cursor_location (GtkIMContext *context,
					GdkRectangle *area)
{
  printf("gtk_im_context_xim_set_cursor_location\n");

  GtkIMContextXIM *context_xim = GTK_IM_CONTEXT_XIM (context);

  // should not update
  if (!context_xim->has_focus)
    return;

  XIC ic = gtk_im_context_xim_get_ic (context_xim);

  XVaNestedList preedit_attr;
  XPoint          spot;

  if (!ic)
    return;

  spot.x = area->x;
  spot.y = area->y + area->height;

//  printf("cursor %d %d\n", area->x, area->y);
  preedit_attr = XVaCreateNestedList (0,
				      XNSpotLocation, &spot,
				      0);
  XSetICValues (ic,
		XNPreeditAttributes, preedit_attr,
		NULL);
  XFree(preedit_attr);

  return;
}

///
static void
gtk_im_context_xim_set_use_preedit (GtkIMContext *context,
				    gboolean      use_preedit)
{
  printf("gtk_im_context_xim_set_use_preedit\n");
}


///
static void
gtk_im_context_xim_reset (GtkIMContext *context)
{
  printf("gtk_im_context_xim_reset\n");
}

/* Mask of feedback bits that we render
 */
#define FEEDBACK_MASK (XIMReverse | XIMUnderline)


static void
gtk_im_context_xim_get_preedit_string (GtkIMContext   *context,
				       gchar         **str,
				       PangoAttrList **attrs,
				       gint           *cursor_pos)
{
  printf("gtk_im_context_xim_get_preedit_string\n");
}



static XIC
gtk_im_context_xim_get_ic (GtkIMContextXIM *context_xim)
{
  if (context_xim->im_info == NULL || context_xim->im_info->im == NULL)
    return NULL;

  if (!context_xim->ic)
    {
      const char *name1 = NULL;
      XVaNestedList list1 = NULL;
      const char *name2 = NULL;
      XVaNestedList list2 = NULL;
      XIMStyle im_style = 0;
      XIC xic = 0;

      if ((context_xim->im_info->style & PREEDIT_MASK) == XIMPreeditNone)
        im_style |= XIMPreeditNone;
      else
        im_style |= XIMPreeditNothing;

      im_style |= XIMStatusNone;

      XFontSet fontset = NULL;

      XPoint          spot;
      spot.x = spot.y = 0;
      XRectangle      rect;
      rect.x = rect.y = 0;
      rect.width = rect.height = 32;

      int missing_charsetcount;
      char **missing_charsetlist, *def_string;

      fontset = XCreateFontSet(GDK_DISPLAY(),
                                        "10x20,10x20",
                                        &missing_charsetlist,
                                        &missing_charsetcount,
                                        &def_string);

      list1 = XVaCreateNestedList(0,
                  XNArea, &rect,
                  XNSpotLocation, &spot,
                  XNForeground, 0,
                  XNBackground, 0,
                  XNFontSet, fontset,
        NULL);

      im_style = XIMPreeditPosition| XIMStatusNothing;
      name1 = XNPreeditAttributes;

      xic = XCreateIC (context_xim->im_info->im,
		       XNInputStyle, im_style,
                       XNClientWindow, GDK_DRAWABLE_XID (context_xim->client_window),
		       name1, list1,
		       name2, list2,
		       NULL);
      if (fontset)
        XFreeFontSet(GDK_DISPLAY(),fontset);

      if (list1)
	XFree (list1);
      if (list2)
	XFree (list2);

      if (xic)
	{
	  /* Don't filter key released events with XFilterEvents unless
	   * input methods ask for. This is a workaround for Solaris input
	   * method bug in C and European locales. It doubles each key
	   * stroke if both key pressed and released events are filtered.
	   * (bugzilla #81759)
	   */
	  gulong mask = 0;
	  XGetICValues (xic,
			XNFilterEvents, &mask,
			NULL);
	  context_xim->filter_key_release = (mask & KeyReleaseMask) != 0;
	}

      context_xim->ic = xic;

      if (xic && context_xim->has_focus)
	XSetICFocus (xic);
    }
  return context_xim->ic;
}



/**
 * gtk_im_context_xim_shutdown:
 *
 * Destroys all the status windows that are kept by the XIM contexts.  This
 * function should only be called by the XIM module exit routine.
 **/
void
gtk_im_context_xim_shutdown (void)
{
}
