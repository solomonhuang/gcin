/****************************************************************************
** QGCINInputContextPlugin meta object code from reading C++ file 'qgcininputcontextplugin.h'
**
** Created: Fri Aug 19 18:36:17 2005
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.4   edited Jan 21 18:14 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "qgcininputcontextplugin.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *QGCINInputContextPlugin::className() const
{
    return "QGCINInputContextPlugin";
}

QMetaObject *QGCINInputContextPlugin::metaObj = 0;
static QMetaObjectCleanUp cleanUp_QGCINInputContextPlugin( "QGCINInputContextPlugin", &QGCINInputContextPlugin::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString QGCINInputContextPlugin::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QGCINInputContextPlugin", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString QGCINInputContextPlugin::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QGCINInputContextPlugin", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* QGCINInputContextPlugin::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QInputContextPlugin::staticMetaObject();
    metaObj = QMetaObject::new_metaobject(
	"QGCINInputContextPlugin", parentObject,
	0, 0,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_QGCINInputContextPlugin.setMetaObject( metaObj );
    return metaObj;
}

void* QGCINInputContextPlugin::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "QGCINInputContextPlugin" ) )
	return this;
    return QInputContextPlugin::qt_cast( clname );
}

bool QGCINInputContextPlugin::qt_invoke( int _id, QUObject* _o )
{
    return QInputContextPlugin::qt_invoke(_id,_o);
}

bool QGCINInputContextPlugin::qt_emit( int _id, QUObject* _o )
{
    return QInputContextPlugin::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool QGCINInputContextPlugin::qt_property( int id, int f, QVariant* v)
{
    return QInputContextPlugin::qt_property( id, f, v);
}

bool QGCINInputContextPlugin::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
