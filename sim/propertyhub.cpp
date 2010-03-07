
#include <QStringList>
#include <QDomElement>
#include <stdio.h>
#include "propertyhub.h"
#include "log.h"
#include "profilemanager.h"


namespace SIM
{
    PropertyHubPtr PropertyHub::create()
    {
        return PropertyHubPtr(new PropertyHub());
    }

    PropertyHubPtr PropertyHub::create(const QString& ns)
    {
        return PropertyHubPtr(new PropertyHub(ns));
    }


    PropertyHub::PropertyHub()
    {
    }

    PropertyHub::PropertyHub(const QString& ns) : m_namespace(ns)
    {
    }

    PropertyHub::~PropertyHub()
    {
    }

    bool PropertyHub::serialize( QDomElement element )
    {
        return serializeVariantMap( element, m_data );
    }

    bool PropertyHub::deserialize( QDomElement element )
    {
        return deserializeVariantMap( element, m_data );
    }

    bool PropertyHub::serializeVariantMap( QDomElement element, const QVariantMap &map )
    {
        if( map.size() == 0 )
            return false;

        foreach( QString key, map.keys() )
        {
            QDomElement node = element.ownerDocument().createElement( "node" );
            node.setAttribute( "name", key );
            node.setAttribute( "type", map[key].typeName() );
            if( serializeVariant( node, map[key] ) ) {
                element.appendChild( node );
            }
        }
        return true;
    }

    bool PropertyHub::deserializeVariantMap( const QDomElement &element, QVariantMap &map )
    {
        QDomNodeList list = element.childNodes();
        for( int i = 0 ; i < list.size() ; i++ ) {
            QDomElement el = list.at(i).toElement();
            if( !el.isNull() ) {
                QString name = el.attribute( "name" );
                if( !name.isEmpty() ) {
                    QVariant v;
                    if( deserializeVariant( el, v ) )
                        map[name] = v;
                }
            }
        }
        return true;
    }

    bool PropertyHub::deserializeVariant( const QDomElement &element, QVariant &v )
    {
        QVariant::Type type = QVariant::nameToType( element.attribute( "type" ).toLatin1().data() );
        switch( type ) {
            case QVariant::Invalid :
                return false;
            case QVariant::Bool :
            case QVariant::Char :
            case QVariant::Date :
            case QVariant::DateTime :
            case QVariant::Double :
            case QVariant::Int :
            case QVariant::LongLong :
            case QVariant::String :
            case QVariant::Time :
            case QVariant::UInt :
            case QVariant::ULongLong : {
                QVariant val( type );
                val = element.text();
                if( !val.convert( type ) )
                    return false;
                v = val;
                break;
            }
            case QVariant::StringList : {
                QStringList list;
                if( !deserializeStringList( element, list ) )
                    return false;
                v = list;
                break;
            }
            case QVariant::ByteArray : {
                QByteArray array;
                if( !deserializeByteArray( element, array ) )
                    return false;
                v = array;
                break;
            }
            case QVariant::Map : {
                QVariantMap map;
                if( !deserializeVariantMap( element, map ) )
                    return false;
                v = map;
                break;
            }
            default : {
                log(L_ERROR, "Unable to load property with type '%s'", v.typeName());
                return false;
            }
        }

        return true;
    }

    bool PropertyHub::serializeStringList( QDomElement element, const QStringList& list )
    {
        if( list.size() == 0 )
            return false;

        foreach( QString str, list )
        {
            QDomElement stringelement = element.ownerDocument().createElement( "string" );
            QDomText text = element.ownerDocument().createTextNode( str );
            stringelement.appendChild( text );
            element.appendChild( stringelement );
        }

        return true;
    }

    bool PropertyHub::deserializeStringList( const QDomElement &element, QStringList &list )
    {
        QDomNodeList nodes = element.elementsByTagName( "string" );
        for( int i = 0 ; i < nodes.size() ; i++ ) {
            QDomElement el = nodes.at(i).toElement();
            if( !el.isNull() ) {
                list << el.text();
            }
        }

        return true;
    }

    bool PropertyHub::serializeByteArray( QDomElement element, const QByteArray& array ) {
        if( array.size() == 0 )
            return false;
        QByteArray base64ed = array.toBase64();
        QDomText text = element.ownerDocument().createTextNode( QString::fromLatin1( base64ed, base64ed.size() ) );
        element.appendChild( text );

        return true;
    }

    bool PropertyHub::deserializeByteArray( const QDomElement &element, QByteArray &array ) {
        QString str = element.text();
        if( str.isEmpty() )
            return false;
        QByteArray base64ed( str.toLatin1().data() );
        array = QByteArray::fromBase64( base64ed );

        return true;
    }

    bool PropertyHub::serializeVariant( QDomElement element, const QVariant& v )
    {
        switch( v.type() ) {
            case QVariant::Invalid :
                return false;
            case QVariant::Bool :
            case QVariant::Char :
            case QVariant::Date :
            case QVariant::DateTime :
            case QVariant::Double :
            case QVariant::Int :
            case QVariant::LongLong :
            case QVariant::String :
            case QVariant::Time :
            case QVariant::UInt :
            case QVariant::ULongLong : {
                QString str = v.toString();
                if( str.isEmpty() )
                    return false;
                QDomText node = element.ownerDocument().createTextNode( str );
                element.appendChild( node );
                break;
            }
            case QVariant::StringList : {
                return serializeStringList( element, v.toStringList() );
            }
            case QVariant::ByteArray : {
                return serializeByteArray( element, v.toByteArray() );
            }
            case QVariant::Map : {
                return serializeVariantMap( element, v.toMap() );
            }
            default : {
                log(L_ERROR, "Unable to save property with type '%s'", v.typeName());
                return false;
            }
        }

        return true;
    }

    void PropertyHub::clear()
    {
        m_data.clear();
    }

    void PropertyHub::parseSection(const QString& string)
    {
        // Probably, we should remove '\r' from the string first
        QStringList lines = string.split('\n');
        for(QStringList::iterator it = lines.begin(); it != lines.end(); ++it)
        {
            QStringList line = it->split('=');
            if(line.size() != 2)
                continue;
            this->setValue(line[0].trimmed(), line[1]);
        }
    }

    void PropertyHub::setStringMapValue(const QString& mapname, int key, const QString& value)
    {
        setValue(QString("%1/val%2").arg(mapname).arg(key), value);
    }

    QString PropertyHub::stringMapValue(const QString& mapname, int key)
    {
        return value(QString("%1/val%2").arg(mapname).arg(key)).toString();
    }

    void PropertyHub::clearStringMap(const QString& /*mapname*/)
    {
       //trallala ;) Implement me
    }



    QList<QString> PropertyHub::allKeys()
    {
        return m_data.keys();
    }

    void PropertyHub::setValue(const QString& key, const QVariant& value)
    {
        m_data.insert(key, value);
    }

    QVariant PropertyHub::value(const QString& key)
    {
        QVariantMap::const_iterator it = m_data.find(key);
        if(it != m_data.end())
            return it.value();
        return QVariant();
    }

}

// vim: set expandtab:

