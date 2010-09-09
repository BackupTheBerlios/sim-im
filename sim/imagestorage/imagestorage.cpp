#include <algorithm>

#include "imagestorage.h"

namespace SIM {

ImageStorage::ImageStorage(QObject *parent) :
    QObject(parent)
{
    addIconSet("icons/sim.jisp", true);
    d->m_workset = new WrkIconSet;
    d->defSets.append(d->m_workset);
    addIconSet("icons/smiles.jisp", false);
    addIconSet("icons/icqlite.jisp", false);
    addIconSet("icons/additional.jisp", false);
}

QIcon ImageStorage::icon(const QString& id)
{
    foreach(IconSet* set, m_sets)
    {
        if(set->hasIcon(id))
        {
            return set->icon(id);
        }
    }
}

QImage ImageStorage::image(const QString& id)
{
    return pixmap(id).toImage();
}

QPixmap ImageStorage::pixmap(const QString& id)
{
    foreach(IconSet* set, m_sets)
    {
        if(set->hasIcon(id))
        {
            return set->pixmap(id);
        }
    }
}

bool ImageStorage::addIconSet(IconSet* set)
{
    // TODO check for double adding
    m_sets.append(set);
    return true;
}

bool ImageStorage::removeIconset(const QString& id)
{
    for(QList<IconSet*>::iterator it = m_sets.begin(); it != m_sets.end(); ++it)
    {
        if((*it)->id() == id)
        {
            m_sets.erase(it);
            return true;
        }
    }
    return false;
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
