/******************************************************************

         Copyright 1994, 1995 by Sun Microsystems, Inc.
         Copyright 1993, 1994 by Hewlett-Packard Company

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of Sun Microsystems, Inc.
and Hewlett-Packard not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
Sun Microsystems, Inc. and Hewlett-Packard make no representations about
the suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.

SUN MICROSYSTEMS INC. AND HEWLETT-PACKARD COMPANY DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
SUN MICROSYSTEMS, INC. AND HEWLETT-PACKARD COMPANY BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

  Author: Hidetoshi Tajima(tajima@Eng.Sun.COM) Sun Microsystems, Inc.

******************************************************************/
#include "gcin.h"

static IC *ic_list = (IC *)NULL;
static IC *free_list = (IC *)NULL;

static IC *ic_lists[2];
static IC *free_lists[2];
static int current_IC_index;

// dirty solution
void switch_IC_index(int index)
{
    if (index != current_IC_index) {
        ic_lists[current_IC_index] = ic_list;
        free_lists[current_IC_index] = free_list;

        ic_list = ic_lists[current_IC_index];
        free_list = free_lists[current_IC_index];

        current_IC_index = index;
    }
}


static void free_IC_list(IC *list)
{
    while (list) {
       IC *next = list->next;
       free(list);
       list = next;
    }
}

void free_all_IC()
{
    free_IC_list(ic_list);
    free_IC_list(free_list);
}


static IC
*NewIC()
{
    static CARD16 icid = 0;
    IC *rec;

    if (free_list != NULL) {
	rec = free_list;
	free_list = free_list->next;
    } else {
	rec = (IC *)malloc(sizeof(IC));
    }

    bzero(rec, sizeof(IC));

    rec->id = ++icid;

    rec->next = ic_list;
    ic_list = rec;
    return rec;
}

IC *FindIC(CARD16 icid)
{
    IC *rec = ic_list;

    while (rec != NULL) {
	if (rec->id == icid)
	  return rec;
	rec = rec->next;
    }

    return NULL;
}

void hide_in_win(ClientState *ic);

void DeleteIC(CARD16 icid)
{
    IC *rec, *last;

    last = NULL;
    for (rec = ic_list; rec != NULL; last = rec, rec = rec->next) {
        if (rec->id == icid) {

          if (&rec->cs == current_CS) {
            hide_in_win(&rec->cs);
          }

          if (last != NULL)
            last->next = rec->next;
          else
            ic_list = rec->next;

          rec->next = free_list;
          free_list = rec;
          return;
	}
    }
    return;
}

static int Is(char *attr, XICAttribute *attr_list) {
	return !strcmp(attr, attr_list->name);
}

extern Window root;
extern Display *dpy;

static int xerror_handler(Display *d, XErrorEvent *eve)
{
  return 0;
}


static void getRootXY(Window win, int wx, int wy, int *tx, int *ty)
{
  Window ow;
  void *olderr = XSetErrorHandler((XErrorHandler)xerror_handler);

  XTranslateCoordinates(dpy,win,root,wx,wy,tx,ty,&ow);

  XSetErrorHandler(olderr);
}



void move_in_win(ClientState *cs, int x, int y);
void show_in_win(ClientState *cs);
extern Window focus_win;

void move_IC_in_win(ClientState *cs)
{
#if DEBUG
   dbg("move_IC_in_win\n");
#endif
   Window inpwin = cs->client_win;

   if (!inpwin)
      return;
#if 1
   if (inpwin != focus_win)
      return;
#endif
   int inpx = cs->spot_location.x;
   int inpy = cs->spot_location.y;
   XWindowAttributes att;

   XGetWindowAttributes(dpy, inpwin, &att);

   if (inpx >= att.width)
     inpx = att.width - 1;
   if (inpy >= att.height)
     inpy = att.height - 1;

   int tx, ty;
   getRootXY(inpwin, inpx, inpy, &tx, &ty);
#if DEBUG
   dbg("move_IC_in_win %d %d   txy:%d %d\n", inpx, inpy, tx, ty);
#endif

   move_in_win(cs, tx, ty+1);
}

void load_IC(IC *rec)
{
   ClientState *cs = &rec->cs;
   Window win = cs->client_win;

#if 1
   if (current_CS != cs && (win == focus_win || !current_CS))
     current_CS = cs;

   if (win == focus_win) {
     if (!cs->b_im_enabled)
       hide_in_win(cs);
     else
     if (cs->b_im_enabled)
       show_in_win(cs);
   }
#endif

   if (cs->input_style & XIMPreeditCallbacks) {
     cs->input_style = InputStyleOnSpot;
#if 0
     if (cs->b_im_enabled)
       move_IC_in_win(cs);
#endif
   } else
   if (cs->input_style & XIMPreeditPosition) {
     cs->input_style = InputStyleOverSpot;
#if 1
     if (cs->b_im_enabled)
       move_IC_in_win(cs);
#endif
   }

   if (cs->input_style & XIMPreeditNothing) {
#if 1
     cs->input_style = InputStyleRoot;
     move_IC_in_win(cs);
#endif
   }
}


static void
StoreIC(IC *rec, IMChangeICStruct *call_data)
{
        ClientState *cs = &rec->cs;
	XICAttribute *ic_attr = call_data->ic_attr;
	XICAttribute *pre_attr = call_data->preedit_attr;
	XICAttribute *sts_attr = call_data->status_attr;
	register int i;
#if DEBUG
        dbg(".... StoreIC\n");
#endif
	for (i = 0; i < (int)call_data->ic_attr_num; i++, ic_attr++) {
		if (Is (XNInputStyle, ic_attr)) {
                    rec->cs.input_style = *(INT32*)ic_attr->value;
		}

		else if (Is (XNClientWindow, ic_attr)) {
                    rec->cs.client_win = *(Window*)ic_attr->value;
#if DEBUG
		    dbg("rec->client_win %x\n", cs->client_win);
#endif
		}
		else if (Is (XNFocusWindow, ic_attr)) {
                    rec->focus_win = *(Window*)ic_attr->value;
                }
		else
		    fprintf(stderr, "Unknown attr: %s a\n", ic_attr->name);
	}

	for (i = 0; i < (int)call_data->preedit_attr_num; i++, pre_attr++) {
		if (Is (XNArea, pre_attr)) {
                    rec->pre_attr.area = *(XRectangle*)pre_attr->value;
#if DEBUG
                   dbg("pre_attr->value: %d %d\n", rec->pre_attr.area.x, rec->pre_attr.area.y);
#endif
                }
		else if (Is (XNAreaNeeded, pre_attr))
		    rec->pre_attr.area_needed = *(XRectangle*)pre_attr->value;

		else if (Is (XNSpotLocation, pre_attr)) {
                    cs->spot_location = *(XPoint*)pre_attr->value;
                    move_IC_in_win(cs);
                }
		else if (Is (XNColormap, pre_attr))
		    rec->pre_attr.cmap = *(Colormap*)pre_attr->value;

		else if (Is (XNStdColormap, pre_attr))
		    rec->pre_attr.cmap = *(Colormap*)pre_attr->value;

		else if (Is (XNForeground, pre_attr))
		    rec->pre_attr.foreground = *(CARD32*)pre_attr->value;

		else if (Is (XNBackground, pre_attr))
		    rec->pre_attr.background = *(CARD32*)pre_attr->value;

		else if (Is (XNBackgroundPixmap, pre_attr))
		    rec->pre_attr.bg_pixmap = *(Pixmap*)pre_attr->value;

		else if (Is (XNFontSet, pre_attr)) {
			int str_length = strlen(pre_attr->value);

			if (rec->pre_attr.base_font != NULL) {
				if (Is (rec->pre_attr.base_font, pre_attr))
					continue;
				XFree(rec->pre_attr.base_font);
			}
			rec->pre_attr.base_font = malloc(str_length + 1);
			strcpy(rec->pre_attr.base_font, pre_attr->value);

		} else if (Is (XNLineSpace, pre_attr))
			    rec->pre_attr.line_space = *(CARD32*)pre_attr->value;

		else if (Is (XNCursor, pre_attr))
		    rec->pre_attr.cursor = *(Cursor*)pre_attr->value;

		else
		    fprintf(stderr, "Unknown attr: %s b\n", pre_attr->name);
	}


	for (i = 0; i < (int)call_data->status_attr_num; i++, sts_attr++) {
                if (Is (XNArea, sts_attr)) {
		    rec->sts_attr.area = *(XRectangle*)sts_attr->value;
                }
                else if (Is (XNAreaNeeded, sts_attr)) {
		    rec->sts_attr.area_needed = *(XRectangle*)sts_attr->value;
                }
                else if (Is (XNColormap, sts_attr)) {
		    rec->sts_attr.cmap = *(Colormap*)sts_attr->value;
                }
                else if (Is (XNStdColormap, sts_attr)) {
		    rec->sts_attr.cmap = *(Colormap*)sts_attr->value;
		}
                else if (Is (XNForeground, sts_attr)) {
		    rec->sts_attr.foreground = *(CARD32*)sts_attr->value;
                }
		else if (Is (XNBackground, sts_attr))
		    rec->sts_attr.background = *(CARD32*)sts_attr->value;

		else if (Is (XNBackgroundPixmap, sts_attr))
		    rec->sts_attr.bg_pixmap = *(Pixmap*)sts_attr->value;

                else if (Is (XNFontSet, sts_attr)) {
			int str_length = strlen(sts_attr->value);

			if (rec->sts_attr.base_font != NULL) {
				if (Is (rec->sts_attr.base_font, sts_attr))
					continue;
				XFree(rec->sts_attr.base_font);
			}
			rec->sts_attr.base_font = malloc(str_length + 1);
			strcpy(rec->sts_attr.base_font, sts_attr->value);
                } else if (Is (XNLineSpace, sts_attr)) {
			    rec->sts_attr.line_space= *(CARD32*)sts_attr->value;
                }
		else if (Is (XNCursor, sts_attr))
		    rec->sts_attr.cursor = *(Cursor*)sts_attr->value;

		else
		    fprintf(stderr, "Unknown attr: %s c\n", ic_attr->name);
	}

	load_IC(rec);
#if DEBUG
        dbg("exit StoreIC\n");
#endif
}

void CreateIC(IMChangeICStruct *call_data)
{
    IC *rec;

    rec = NewIC();
    if (rec == NULL)
      return;

    StoreIC(rec, call_data);
    call_data->icid = rec->id;
    load_IC(rec);
#if DEBUG
    dbg("CreateIC  .. exit\n");
#endif
    return;
}

void
DestroyIC(call_data)
IMChangeICStruct *call_data;
{
    DeleteIC(call_data->icid);
    return;
}

void SetIC(IMChangeICStruct * call_data)
{
    IC *rec = FindIC(call_data->icid);

    if (rec == NULL)
      return;

    load_IC(rec);

    StoreIC(rec, call_data);
#if DEBUG
    dbg(".... exit SetIC\n");
#endif
    return;
}

void GetIC(IMChangeICStruct *call_data)
{
    XICAttribute *ic_attr = call_data->ic_attr;
    XICAttribute *pre_attr = call_data->preedit_attr;
    XICAttribute *sts_attr = call_data->status_attr;
    register int i;
    IC *rec = FindIC(call_data->icid);

    if (rec == NULL)
      return;

    ClientState *cs = &rec->cs;

    for (i = 0; i < (int)call_data->ic_attr_num; i++, ic_attr++) {
	if (Is (XNFilterEvents, ic_attr)) {
	    ic_attr->value = (void *)malloc(sizeof(CARD32));
	    *(CARD32*)ic_attr->value = KeyPressMask|KeyReleaseMask;
	    ic_attr->value_length = sizeof(CARD32);
	}
    }

    /* preedit attributes */
    for (i = 0; i < (int)call_data->preedit_attr_num; i++, pre_attr++) {
	if (Is (XNArea, pre_attr)) {
	    pre_attr->value = (void *)malloc(sizeof(XRectangle));
            *(XRectangle*)pre_attr->value = rec->pre_attr.area;
            pre_attr->value_length = sizeof(XRectangle);
#if DEBUG
            dbg("pre_attr->value: %d %d\n", rec->pre_attr.area.x, rec->pre_attr.area.y);
#endif

	} else if (Is (XNAreaNeeded, pre_attr)) {
	    pre_attr->value = (void *)malloc(sizeof(XRectangle));
	    *(XRectangle*)pre_attr->value = rec->pre_attr.area_needed;
	    pre_attr->value_length = sizeof(XRectangle);

	} else if (Is (XNSpotLocation, pre_attr)) {
	    pre_attr->value = (void *)malloc(sizeof(XPoint));
            *(XPoint*)pre_attr->value = cs->spot_location;
	    pre_attr->value_length = sizeof(XPoint);
#if DEBUG
            dbg("over splot %d %d\n", cs->spot_location.x,  cs->spot_location.y);
#endif
	} else if (Is (XNFontSet, pre_attr)) {
	    CARD16 base_len = (CARD16)strlen(rec->pre_attr.base_font);
	    int total_len = sizeof(CARD16) + (CARD16)base_len;
	    char *p;

	    pre_attr->value = (void *)malloc(total_len);
	    p = (char *)pre_attr->value;
	    memmove(p, &base_len, sizeof(CARD16));
	    p += sizeof(CARD16);
	    strncpy(p, rec->pre_attr.base_font, base_len);
	    pre_attr->value_length = total_len;

	} else if (Is (XNForeground, pre_attr)) {
	    pre_attr->value = (void *)malloc(sizeof(long));
	    *(long*)pre_attr->value = rec->pre_attr.foreground;
	    pre_attr->value_length = sizeof(long);

	} else if (Is (XNBackground, pre_attr)) {
	    pre_attr->value = (void *)malloc(sizeof(long));
	    *(long*)pre_attr->value = rec->pre_attr.background;
	    pre_attr->value_length = sizeof(long);

	} else if (Is (XNLineSpace, pre_attr)) {
	    pre_attr->value = (void *)malloc(sizeof(long));
#if 0
	    *(long*)pre_attr->value = rec->pre_attr.line_space;
#endif
	    *(long*)pre_attr->value = 18;
	    pre_attr->value_length = sizeof(long);
	}
    }

    /* status attributes */
    for (i = 0; i < (int)call_data->status_attr_num; i++, sts_attr++) {
	if (Is (XNArea, sts_attr)) {
	    sts_attr->value = (void *)malloc(sizeof(XRectangle));
	    *(XRectangle*)sts_attr->value = rec->sts_attr.area;
	    sts_attr->value_length = sizeof(XRectangle);

	} else if (Is (XNAreaNeeded, sts_attr)) {
	    sts_attr->value = (void *)malloc(sizeof(XRectangle));
	    *(XRectangle*)sts_attr->value = rec->sts_attr.area_needed;
	    sts_attr->value_length = sizeof(XRectangle);

	} else if (Is (XNFontSet, sts_attr)) {
	    CARD16 base_len = (CARD16)strlen(rec->sts_attr.base_font);
	    int total_len = sizeof(CARD16) + (CARD16)base_len;
	    char *p;

	    sts_attr->value = (void *)malloc(total_len);
	    p = (char *)sts_attr->value;
	    memmove(p, &base_len, sizeof(CARD16));
	    p += sizeof(CARD16);
	    strncpy(p, rec->sts_attr.base_font, base_len);
	    sts_attr->value_length = total_len;

	} else if (Is (XNForeground, sts_attr)) {
	    sts_attr->value = (void *)malloc(sizeof(long));
	    *(long*)sts_attr->value = rec->sts_attr.foreground;
	    sts_attr->value_length = sizeof(long);

	} else if (Is (XNBackground, sts_attr)) {
	    sts_attr->value = (void *)malloc(sizeof(long));
	    *(long*)sts_attr->value = rec->sts_attr.background;
	    sts_attr->value_length = sizeof(long);

	} else if (Is (XNLineSpace, sts_attr)) {
	    sts_attr->value = (void *)malloc(sizeof(long));
#if 0
	    *(long*)sts_attr->value = rec->sts_attr.line_space;
#endif
	    *(long*)sts_attr->value = 18;
	    sts_attr->value_length = sizeof(long);
	}
    }
}


