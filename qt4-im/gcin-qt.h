#include <QObject>
#include <QSocketNotifier>

//#include "gcin-imcontext-qt.h"
#include "gcin-common-qt.h"

class GCINQt: public QObject
{

    Q_OBJECT

    public slots:

        void handle_message ();

    public:

        /**
         * Constructor.
         */
        GCINQt ();


        /**
         * Destructor.
         */
        ~GCINQt ();


        /**
         * A messenger is opened.
         */
        void messenger_opened ();

        /**
         * A messenger is closed.
         */
        void messenger_closed ();


    private:


        /**
         * The notifier for the messenger socket.
         */
        QSocketNotifier *socket_notifier;

};
