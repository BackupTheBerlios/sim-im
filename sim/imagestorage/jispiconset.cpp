#include "jispiconset.h"
#include "qzip/qzip.h"

#include <QtXml>

namespace SIM {

JispIconSet::JispIconSet()
{
}

bool JispIconSet::load(const QString& filename)
{
    m_id = filename;
    UnZip uz(filename);
    QByteArray arr;
    if(uz.open() &&
       (uz.readFile("icondef.xml", &arr) ||
        uz.readFile(QFileInfo(uz.name()).baseName() + "/icondef.xml", &arr)))
        return parse(arr);
    else
        return false;
}

bool JispIconSet::parse(const QByteArray& arr)
{
    QDomDocument doc;
    doc.setContent(arr);
    QDomElement icondef = doc.firstChildElement("icondef");
    QDomElement meta = doc.firstChildElement("meta");
    m_name = meta.firstChildElement("name").text();
    QDomElement icon = icondef.firstChildElement("icon");
    while(!icon.isNull()) {
        QString name = icon.attribute("name");
        QDomElement object = icon.firstChildElement("object");
        QString pictfile = object.text();

        QPixmap image;
        if(image.load(pictfile))
            m_images.insert(name, pictfile);

        icon = icon.nextSiblingElement("icon");
    }
    return true;
}

QString JispIconSet::id() const
{
    return m_id;
}

QString JispIconSet::name() const
{
    return m_name;
}

bool JispIconSet::hasIcon(const QString& iconId)
{
    return m_images.contains(iconId);
}

QIcon JispIconSet::icon(const QString& iconId)
{
    return QIcon(m_images.value(iconId));
}

QPixmap JispIconSet::pixmap(const QString& iconId)
{
    return m_images.value(iconId);
}

} // namespace SIM
