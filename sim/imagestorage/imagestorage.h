#ifndef IMAGESTORAGE_H
#define IMAGESTORAGE_H

#include <QObject>
#include <QIcon>
#include <QImage>
#include <QPixmap>

#include "simapi.h"
#include "iconset.h"

namespace SIM {

class EXPORT ImageStorage : public QObject
{
    Q_OBJECT
public:
    explicit ImageStorage(QObject *parent = 0);
    virtual ~ImageStorage();

    QIcon icon(const QString& id);
    QImage image(const QString& id);
    QPixmap pixmap(const QString& id);

    bool addIconSet(IconSet* set);
    bool removeIconset(const QString& id);

    QString parseSmiles(const QString& input);

private:
    QList<IconSet*> m_sets;
};

EXPORT ImageStorage* getImageStorage();
void EXPORT createImageStorage();
void EXPORT destroyImageStorage();

} // namespace SIM

#endif // IMAGESTORAGE_H
