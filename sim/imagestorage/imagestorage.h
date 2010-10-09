#ifndef IMAGESTORAGE_H
#define IMAGESTORAGE_H

#include <QObject>
#include <QIcon>
#include <QImage>
#include <QPixmap>

#include "simapi.h"
#include "iconset.h"

namespace SIM {

class EXPORT ImageStorage
{
public:
    virtual ~ImageStorage() {}

    virtual QIcon icon(const QString& id) = 0;
    virtual QImage image(const QString& id) = 0;
    virtual QPixmap pixmap(const QString& id) = 0;

    virtual bool addIconSet(IconSet* set) = 0;
    virtual bool removeIconset(const QString& id) = 0;

    virtual QString parseSmiles(const QString& input) = 0;
};

EXPORT ImageStorage* getImageStorage();
void EXPORT setImageStorage(ImageStorage* storage);
void EXPORT createImageStorage();
void EXPORT destroyImageStorage();

} // namespace SIM

#endif // IMAGESTORAGE_H
