#include <algorithm>

#include "imagestorage.h"
#include "jispiconset.h"
#include "log.h"

namespace SIM {

ImageStorage::ImageStorage(QObject *parent) :
    QObject(parent)
{
    foreach(const QString& s, QStringList() <<
            "icons/sim.jisp" << "icons/smiles.jisp" <<
            "icons/icqlite.jisp" << "icons/additional.jisp") {
        JispIconSet* set = new JispIconSet();
        if(!set->load(s))
        {
            log(L_WARN, "Unable to load default icon set: %s", qPrintable(s));
            delete set;
        }
    }
}

ImageStorage::~ImageStorage()
{
    foreach(IconSet* set, m_sets) {
        delete set;
    }
}

QIcon ImageStorage::icon(const QString& id)
{
    foreach(IconSet* set, m_sets) {
        if(set->hasIcon(id)) {
            return set->icon(id);
        }
    }
    return QIcon();
}

QImage ImageStorage::image(const QString& id)
{
    return pixmap(id).toImage();
}

QPixmap ImageStorage::pixmap(const QString& id)
{
    foreach(IconSet* set, m_sets) {
        if(set->hasIcon(id)) {
            return set->pixmap(id);
        }
    }
    return QPixmap();
}

bool ImageStorage::addIconSet(IconSet* set)
{
    // TODO check for double adding
    m_sets.append(set);
    return true;
}

bool ImageStorage::removeIconset(const QString& id)
{
    for(QList<IconSet*>::iterator it = m_sets.begin(); it != m_sets.end(); ++it) {
        if((*it)->id() == id) {
            m_sets.erase(it);
            return true;
        }
    }
    return false;
}

QString ImageStorage::parseSmiles(const QString& input)
{
    QString result = input;
    foreach(IconSet* set, m_sets) {
        result = set->parseSmiles(result);
    }
    return result;
}

static ImageStorage* g_imageStorage = 0;
EXPORT ImageStorage* getImageStorage()
{
    return g_imageStorage;
}

void EXPORT createImageStorage()
{
    Q_ASSERT(!g_imageStorage);
    g_imageStorage = new ImageStorage();
}

void EXPORT destroyImageStorage()
{
    Q_ASSERT(g_imageStorage);
    delete g_imageStorage;
    g_imageStorage = 0;
}

} // namespace SIM
