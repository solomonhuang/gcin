#include <QApplication>
#include <QEvent>
#include <QFont>
#include <QInputContext>
#include <QInputMethodEvent>
#include <QObject>
#include <QPoint>
#include <QWidget>

// #include "gcin-imcontext.h"
// #include "gcin-common-qt.h"
#include <QX11Info>
/**
 * IMContext class for qt client.
 */
struct GCIN_client_handle_S;

class GCINIMContext: public QInputContext {
    public:
        /**
         * Allocate a new IMContext.
         *
         * @return A new IMContext.
         */
//        static GCINIMContext *alloc ();

        GCINIMContext ();
        ~GCINIMContext ();

        /**
         * Filter a event from X11.
         *
         * @param widget The widget.
         * @param A event from X11.
         * @return If this event is consumed or not.
         */
        bool x11FilterEvent (QWidget *widget, XEvent *event);

        /**
         * Filter a key event.
         *
         * @param event The key event.
         * @return If this event is consumed or not.
         */
        bool filterEvent (const QEvent *event);

        /**
         * The focus has been changed.
         */
        void update();

        /**
         * Get the identifier name for this input context.
         *
         * @return The identifier name.
         */
        QString identifierName();

        /**
         * Get the languages for the input context.
         *
         * @return The languages for the input context.
         */
        QString language();

        /**
         * Filter a mouse event.
         *
         * @param offset The cursor offset in the preedit string.
         * @param event The mouse event.
         */
        void mouseHandler (int offset, QMouseEvent *event);

        /**
         * The current focused widget is destroied.
         *
         * @param widget The widget under destroying.
         */
        void setFocusWidget (QWidget *widget);

        void widgetDestroyed (QWidget *widget);

        /**
         * Reset the current IME.
         */
        void reset ();

        GCIN_client_handle_S *gcin_ch;
        bool isComposing() const;

        void update_cursor(QWidget *);
};
