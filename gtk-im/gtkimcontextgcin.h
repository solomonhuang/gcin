/* GTK - The GIMP Toolkit
 * Copyright (C) 2000 Red Hat Software
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

#ifndef __GTK_IM_CONTEXT_GCIN_H__
#define __GTK_IM_CONTEXT_GCIN_H__

#include <gtk/gtk.h>
#if !GTK_CHECK_VERSION(3,0,0)
#include <gtk/gtkimcontext.h>
#endif
#include "gdk/gdkx.h"
#include "../gcin-gtk-compatible.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


extern GType gtk_type_im_context_gcin;

#define GTK_TYPE_IM_CONTEXT_GCIN              gtk_type_im_context_gcin
#define GTK_IM_CONTEXT_GCIN(obj)              (GTK_CHECK_CAST ((obj), GTK_TYPE_IM_CONTEXT_GCIN, GtkIMContextGCIN))
#define GTK_IM_CONTEXT_GCIN_CLASS(klass)      (GTK_CHECK_CLASS_CAST ((klass), GTK_TYPE_IM_CONTEXT_GCIN, GtkIMContextGCINClass))
#define GTK_IS_IM_CONTEXT_GCIN(obj)           (GTK_CHECK_TYPE ((obj), GTK_TYPE_IM_CONTEXT_GCIN))
#define GTK_IS_IM_CONTEXT_GCIN_CLASS(klass)   (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_IM_CONTEXT_GCIN))
#define GTK_IM_CONTEXT_GCIN_GET_CLASS(obj)    (GTK_CHECK_GET_CLASS ((obj), GTK_TYPE_IM_CONTEXT_GCIN, GtkIMContextGCINClass))


typedef struct _GtkIMContextGCIN       GtkIMContextGCIN;
typedef struct _GtkIMContextGCINClass  GtkIMContextGCINClass;

struct _GtkIMContextGCINClass
{
  GtkIMContextClass parent_class;
};

void gtk_im_context_gcin_register_type (GTypeModule *type_module);
GtkIMContext *gtk_im_context_gcin_new (void);

void gtk_im_context_gcin_shutdown (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GTK_IM_CONTEXT_GCIN_H__ */
