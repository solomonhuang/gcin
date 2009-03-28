/****************************************************************************
** QGCINInputContext meta object code from reading C++ file 'qgcininputcontext.h'
**
** Created: Fri Mar 3 10:21:32 2006
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.5   edited Sep 2 14:41 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "qgcininputcontext.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *QGCINInputContext::className() const
{
    return "QGCINInputContext";
}

QMetaObject *QGCINInputContext::metaObj = 0;
static QMetaObjectCleanUp cleanUp_QGCINInputContext( "QGCINInputContext", &QGCINInputContext::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString QGCINInputContext::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QGCINInputContext", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString QGCINInputContext::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QGCINInputContext", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* QGCINInputContext::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QInputContext::staticMetaObject();
    metaObj = QMetaObject::new_metaobject(
	"QGCINInputContext", parentObject,
	0, 0,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_QGCINInputContext.setMetaObject( metaObj );
    return metaObj;
}

void* QGCINInputContext::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "QGCINInputContext" ) )
	return this;
    return QInputContext::qt_cast( clname );
}

bool QGCINInputContext::qt_invoke( int _id, QUObject* _o )
{
    return QInputContext::qt_invoke(_id,_o);
}

bool QGCINInputContext::qt_emit( int _id, QUObject* _o )
{
    return QInputContext::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool QGCINInputContext::qt_property( int id, int f, QVariant* v)
{
    return QInputContext::qt_property( id, f, v);
}

bool QGCINInputContext::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
