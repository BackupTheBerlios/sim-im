#ifndef STUBIMAGESTORAGE_H
#define STUBIMAGESTORAGE_H

#include "imagestorage/imagestorage.h"


namespace StubObjects
{
    class StubImageStorage : public SIM::ImageStorage
    {
        virtual QIcon icon(const QString& id) { return QIcon(); }
        virtual QImage image(const QString& id) { return QImage(); }
        virtual QPixmap pixmap(const QString& id) { return QPixmap(); }

        virtual bool addIconSet(SIM::IconSet* set) { return true; }
        virtual bool removeIconset(const QString& id) { return true; }

        virtual QString parseSmiles(const QString& input) { return QString(); }
    };
}

#endif // STUBIMAGESTORAGE_H
