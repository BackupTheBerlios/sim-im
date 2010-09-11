#ifndef JISPICONSET_H
#define JISPICONSET_H

#include "iconset.h"
#include "qzip/qzip.h"
#include <QMap>

namespace SIM {

class JispIconSet : public IconSet
{
public:
    JispIconSet();

    bool load(const QString& filename);

    virtual QString id() const;
    virtual QString name() const;
    virtual bool hasIcon(const QString& iconId);
    virtual QIcon icon(const QString& iconId);
    virtual QPixmap pixmap(const QString& iconId);
    virtual QString parseSmiles(const QString& input);

private:
    bool parse(const QByteArray& arr);

    QMap<QString, QString> m_images;
    QMap<QString, QString> m_smiles;
    QString m_id;
    QString m_name;
    UnZip m_uz;
};

} // namespace SIM

#endif // JISPICONSET_H
