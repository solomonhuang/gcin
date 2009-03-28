#include "gcin-imcontext-qt.h"
#include "gcin-common-qt.h"
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include "gcin-im-client.h"
#include <QColor>
#include <QPalette>
#include <QInputMethodEvent>
#include <QTextCharFormat>

using namespace Qt;
static QWidget *focused_widget;

GCINIMContext::GCINIMContext ()
{
  Display *display = QX11Info::display();
  if (!(gcin_ch = gcin_im_client_open(display))) {
    perror("cannot open gcin_ch");
    return;
  }
}

GCINIMContext::~GCINIMContext ()
{
  if (gcin_ch) {
    gcin_im_client_close(gcin_ch);
    gcin_ch = NULL;
  }
}

QString GCINIMContext::identifierName()
{
  return GCIN_IDENTIFIER_NAME;
}

void GCINIMContext::mouseHandler (int offset, QMouseEvent *event)
{
}

void GCINIMContext::widgetDestroyed (QWidget *widget)
{
}

void GCINIMContext::reset ()
{
}

void GCINIMContext::update_cursor(QWidget *fwidget)
{
  gcin_im_client_set_window(gcin_ch, fwidget->winId());
  QRect rect = fwidget->inputMethodQuery (ImMicroFocus).toRect ();
  QPoint point (rect.x (), rect.y () + rect.height ());
  QPoint gxy = fwidget->mapToGlobal (point);
  if (gcin_ch) {
    Display *dpy = QX11Info::display();
    WId ow;
    int wx, wy;
    XTranslateCoordinates(dpy, fwidget->winId(), DefaultRootWindow(dpy),
    0,0,  &wx, &wy, &ow);

    gcin_im_client_set_cursor_location(gcin_ch, gxy.x()-wx, gxy.y()-wy);
  }
}

void GCINIMContext::update()
{
    QWidget *focused_widget = qApp->focusWidget ();
    if (focused_widget != NULL) {
        if (focused_widget == NULL) {
          if (gcin_ch)
            gcin_im_client_focus_in(gcin_ch);
        }

        update_cursor(focused_widget);
    }
}


QString GCINIMContext::language ()
{
    return "";
}

void GCINIMContext::setFocusWidget(QWidget *widget)
{
  if (!widget)
    return;

  if (focused_widget != widget) {
    focused_widget = widget;
    gcin_im_client_focus_out(gcin_ch);
  }

  if (gcin_ch) {
    gcin_im_client_set_window(gcin_ch, widget->winId());
  }

  QInputContext::setFocusWidget (widget);
//  puts("setFocusWidget");
  if (gcin_ch)
    gcin_im_client_focus_in(gcin_ch);
}

bool GCINIMContext::x11FilterEvent (QWidget *widget, XEvent *event)
{
  KeySym keysym;
  char static_buffer[256];
  char *buffer = static_buffer;
  int buffer_size = sizeof(static_buffer) - 1;

  if (event->type != KeyPress && event->type != KeyRelease)
      return TRUE;

  XKeyEvent *keve = (XKeyEvent *) event;

  XLookupString (keve, buffer, buffer_size, &keysym, NULL);
  int result;
  char *rstr = NULL;
  unsigned int state = keve->state;


  if (event->type == KeyPress) {
      result = gcin_im_client_forward_key_press(gcin_ch,
        keysym, state, &rstr);

      if (rstr) {
          QString inputText = QString::fromUtf8(rstr);
          QInputMethodEvent commit_event;
          commit_event.setCommitString (inputText);
          sendEvent (commit_event);
      }
  } else {
     result = gcin_im_client_forward_key_release(gcin_ch,
       keysym, state, &rstr);
  }

  update_cursor(widget);

  if (rstr)
      free(rstr);

  return result;
}

bool GCINIMContext::filterEvent (const QEvent *event)
{
  return FALSE;
}

bool GCINIMContext::isComposing() const
{
  return FALSE;
}
