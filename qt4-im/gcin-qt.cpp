/*
 * SCIM Bridge
 *
 * Copyright (c) 2006 Ryo Dairiki <ryo-dairiki@users.sourceforge.net>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation and
 * appearing in the file LICENSE.LGPL included in the package of this file.
 * You can also redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation and
 * appearing in the file LICENSE.GPL included in the package of this file.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

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

#if 0
    if (gcin_client_initialize ()) {
        scim_bridge_perrorln ("Failed to init scim bridge...");
    } else {
        gcin_client_open_messenger ();
    }
    GCINIMContext::static_initialize ();
#endif
}


GCINQt::~GCINQt () {
#if 0
    if (gcin_client_finalize ()) {
        scim_bridge_perrorln ("Failed to finalize scim bridge...");
    }
    GCINIMContext::static_finalize ();
#endif

    client = NULL;
}


void GCINQt::messenger_opened ()
{
#if 0
    const int fd = gcin_client_get_messenger_fd ();
    socket_notifier = new QSocketNotifier (fd, QSocketNotifier::Read);
    connect (socket_notifier, SIGNAL (activated (int)), this, SLOT (handle_message ()));

    GCINIMContext::connection_opened ();
#endif
}


void GCINQt::messenger_closed ()
{
    if (socket_notifier) {
        socket_notifier->setEnabled (false);
        socket_notifier->deleteLater ();
        socket_notifier = NULL;
    }

//    GCINIMContext::connection_closed ();
}


void GCINQt::handle_message ()
{
#if 0
    const int socket_fd = gcin_client_get_messenger_fd ();

    fd_set read_set;
    FD_ZERO (&read_set);
    FD_SET (socket_fd, &read_set);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    if (select (socket_fd + 1, &read_set, NULL, NULL, &timeout) > 0) {
        if (gcin_client_read_and_dispatch ()) {
            scim_bridge_perrorln ("An IOException occurred at handle_message ()");
            return;
        }
    }
#endif
}

