#ifndef CLIENTCAPABILITY_H
#define CLIENTCAPABILITY_H

#include <QByteArray>
#include <QString>

class ClientCapability
{
public:
    ClientCapability(const QByteArray& id, int shortId, const QString& name, const QString& desc);

    QByteArray guid() const;
    int shortId() const;
    QString name() const;
    QString description() const;

private:
    QByteArray m_guid;
    int m_shortId;
    QString m_name;
    QString m_description;
};

#endif // CLIENTCAPABILITY_H
