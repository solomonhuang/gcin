#include "qgcininputcontext.h"
#include "qwidget.h"

#include <stdlib.h>
#include <X11/keysymdef.h>
#include "gcin-im-client.h"


QGCINInputContext::QGCINInputContext()
    : QInputContext(), gcin_ch(NULL)
{
//    printf("create_xim\n");
    Display *appDpy = QPaintDevice::x11AppDisplay();

    if (!gcin_ch) {
      if (!(gcin_ch = gcin_im_client_open(appDpy)))
        perror("cannot open gcin_ch");
        return;
    }
}


void QGCINInputContext::setHolderWidget( QWidget *widget )
{

    if ( ! widget )
	return;

    QInputContext::setHolderWidget( widget );

//    printf("setHolderWidget %x\n",  widget->winId());

    if (! widget->isTopLevel()) {
	qWarning("QInputContext: cannot create input context for non-toplevel widgets");
	return;
    }

    if (gcin_ch) {
        gcin_im_client_set_window(gcin_ch, widget->winId());
    }
}


QGCINInputContext::~QGCINInputContext()
{
//    printf("QGCINInputContext::~QGCINInputContext()\n");
    gcin_im_client_close(gcin_ch);
    gcin_ch = NULL;
}


bool QGCINInputContext::x11FilterEvent( QWidget *keywidget, XEvent *event )
{
#ifndef QT_NO_GCIN
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

            sendIMEvent( QEvent::IMStart );
            sendIMEvent( QEvent::IMEnd, inputText );
        }
    } else {
       result = gcin_im_client_forward_key_release(gcin_ch,
         keysym, state, &rstr);
    }


    if (rstr)
        free(rstr);

    return result;
}


void QGCINInputContext::sendIMEvent( QEvent::Type type, const QString &text,
				    int cursorPosition, int selLength )
{
    QInputContext::sendIMEvent( type, text, cursorPosition, selLength );
    if ( type == QEvent::IMCompose )
	composingText = text;
}


void QGCINInputContext::reset()
{
//    printf("reset %x %d %d\n", focusWidget(), isComposing(), composingText.isNull());

    if ( focusWidget() && isComposing() && ! composingText.isNull() ) {
	QInputContext::reset();

	resetClientState();
    }
}


void QGCINInputContext::resetClientState()
{
//    printf("resetClientState\n");
}


void QGCINInputContext::close( const QString &errMsg )
{
//    printf("close\n");
    qDebug( "%s", (const char*) errMsg );
    emit deletionRequested();
}


bool QGCINInputContext::hasFocus() const
{
//    printf("hasFocus\n");
    return ( focusWidget() != 0 );
}


void QGCINInputContext::setMicroFocus(int x, int y, int, int h, QFont *f)
{
    QWidget *widget = focusWidget();

    if (widget ) {
	QPoint p( x, y );
	QPoint p2 = widget->mapTo( widget->topLevelWidget(), QPoint( 0, 0 ) );
	p = widget->topLevelWidget()->mapFromGlobal( p );
	setComposePosition(p.x(), p.y() + h);
   }
}

void QGCINInputContext::mouseHandler( int , QEvent::Type type,
				     Qt::ButtonState button,
				     Qt::ButtonState)
{
}

void QGCINInputContext::setComposePosition(int x, int y)
{
//    printf("setComposePosition %d %d\n", x, y);
    if (gcin_ch) {
      gcin_im_client_set_cursor_location(gcin_ch, x, y);
    }
}


void QGCINInputContext::setComposeArea(int x, int y, int w, int h)
{
//    printf("setComposeArea %d %d %d %d\n", x, y, w, h);
}


void QGCINInputContext::setFocus()
{
//    printf("setFocus\n", gcin_ch);

    if (gcin_ch) {
      gcin_im_client_focus_in(gcin_ch);
    }
}

void QGCINInputContext::unsetFocus()
{
//    printf("unsetFocus\n");
    if (gcin_ch) {
      gcin_im_client_focus_out(gcin_ch);
    }
}


bool QGCINInputContext::isPreeditRelocationEnabled()
{
    return ( language() == "ja" );
}


bool QGCINInputContext::isPreeditPreservationEnabled()
{
    return ( language() == "ja" );
}


QString QGCINInputContext::identifierName()
{
    return "gcin";
}


QString QGCINInputContext::language()
{
    QString locale("zh_TW");

    _language = locale;

    return _language;
}

#endif //QT_NO_IM
