/*
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

#include <cassert>

#include <Qt>
#include <QInputContextPlugin>

using namespace Qt;

#include "gcin-qt.h"
#include "gcin-imcontext-qt.h"

/* Static Variables */
// static GCINClientQt *client = NULL;

/* The class Definition */
class GCINInputContextPlugin: public QInputContextPlugin
{

    private:

        static QStringList gcin_languages;

    public:

        GCINInputContextPlugin ();

        ~GCINInputContextPlugin ();

        QStringList keys () const;

        QStringList languages (const QString &key);

        QString description (const QString &key);

        QInputContext *create (const QString &key);

        QString displayName (const QString &key);

};


/* Implementations */
QStringList GCINInputContextPlugin::gcin_languages;


GCINInputContextPlugin::GCINInputContextPlugin ()
{
}


GCINInputContextPlugin::~GCINInputContextPlugin ()
{
#if 0
    delete client;
    client = NULL;
#endif
}

QStringList GCINInputContextPlugin::keys () const {
    QStringList identifiers;
    identifiers.push_back (GCIN_IDENTIFIER_NAME);
    return identifiers;
}


QStringList GCINInputContextPlugin::languages (const QString &key)
{
    if (gcin_languages.empty ()) {
        gcin_languages.push_back ("zh_TW");
        gcin_languages.push_back ("zh_CN");
        gcin_languages.push_back ("zh_HK");
        gcin_languages.push_back ("ja");
    }
    return gcin_languages;
}


QString GCINInputContextPlugin::description (const QString &key)
{
    return QString::fromUtf8 ("Qt immodule plugin for gcin");
}


QInputContext *GCINInputContextPlugin::create (const QString &key)
{
    if (key.toLower () != GCIN_IDENTIFIER_NAME) {
        return NULL;
    } else {
        return new GCINIMContext;
    }
}


QString GCINInputContextPlugin::displayName (const QString &key)
{
    return key;
}

Q_EXPORT_PLUGIN2 (GCINInputContextPlugin, GCINInputContextPlugin)
