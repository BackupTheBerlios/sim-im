#ifndef STANDARDIMAGESTORAGE_H
#define STANDARDIMAGESTORAGE_H

#include "imagestorage.h"

namespace SIM
{
    class StandardImageStorage : public ImageStorage
    {
    public:
        StandardImageStorage();
        virtual ~StandardImageStorage();

        QIcon icon(const QString& id);
        QImage image(const QString& id);
        QPixmap pixmap(const QString& id);

        bool addIconSet(IconSet* set);
        bool removeIconset(const QString& id);

        QString parseSmiles(const QString& input);

    private:
        QList<IconSet*> m_sets;
    };
}

#endif // STANDARDIMAGESTORAGE_H
