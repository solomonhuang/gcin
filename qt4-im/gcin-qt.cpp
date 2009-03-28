#include "gcin-qt.h"

#ifdef QT4
using namespace Qt;
#endif

/* Static variables */
static GCINQt *client = NULL;


/* Bindings */
void gcin_client_messenger_opened ()
{
    client->messenger_opened ();
}


void gcin_client_messenger_closed ()
{
    client->messenger_closed ();
}


/* Implementations */
GCINQt::GCINQt (): socket_notifier (NULL) {
    client = this;
}


GCINQt::~GCINQt () {
    client = NULL;
}


void GCINQt::messenger_opened ()
{
}


void GCINQt::messenger_closed ()
{
}


void GCINQt::handle_message ()
{
}

